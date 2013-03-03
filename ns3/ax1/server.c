/*
 * Author: Kristian Hentschel
 * Matric: 1003734h
 * Submission: Networked Systems 3 Assessed Exercise 1
 *
 * This file is my own work.
 *
 * A simple web-server using a threadpool implemented using a work-queue (see bufferthreadpool.c)
 * The threadpool interface is defined in threadpool.h as I experimented with two separate implementations of the threadpool) for handling multiple connections.
 * Request capturing is done using a linked list of character arrays (see linkedstringbuffer.[ch]).
 *
 * Known issues:
 *  - In some cases, aborting a request (client closing the connection before a transfer is complete) causes the server to receive a SIGPIPE signal,
 *    this is despite all write() return codes are handled and the socket is closed on write failures.
 *  - Browsers open many keep-alive connection. If the server is run with less threads than incoming client connections, and no connection: close headers are sent,
 *    these requests will be ignored until a thread becomes available due to the browser closing an earlier connection (Chrome: after a long time).
 *  - There is no graceful way to shutdown the server - SIGINT kills it, but is not currently handled to correctly stop the threads and free all data structures.
 */
#include "linkedstringbuffer.h"
#include "threadpool.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

/* Debugging - uncomment one of these to turn prints on or off */
#define DEBUG printf
/* #define DEBUG (void) */

/* General server performance settings */
#define SERVER_PORT 8080
#define NTHREADS 32 
#define DOMAIN_SUFFIX ".dcs.gla.ac.uk"
#define SERVER_BACKLOG 1 

/* Read buffer for individual socket reads */
#define REQUEST_BUFFER_SIZE 1024

/* Buffers used for individual HTTP headers, path names, hostnames, etc. */
#define SHORT_LINE_BUFFER 1024

/* Files are read/written in blocks of this size. */
#define FILE_COPY_BUFFER 1024*1024

/* helper macros for using numeric defines in strings (sscanf buffer sizes), source:
 * http://stackoverflow.com/questions/5459868/c-preprocessor-concatenate-int-to-string
 */
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

void connection_worker(void *arg);
void close_connection(void *arg);

void handle_connection(int fd);
int handle_request(int fd, char *request);

enum http_mime {MIME_TEXT_PLAIN, MIME_TEXT_HTML, MIME_IMAGE_JPEG, MIME_IMAGE_GIF, MIME_IMAGE_PNG, MIME_APPLICATION_OCTET_STREAM};
enum http_status {HTTP_OK = 200, HTTP_NOT_FOUND = 404, HTTP_BAD_REQUEST = 400, HTTP_INTERNAL_SERVER_ERROR = 500};

int main(void) {
	int 				sockfd, connfd, *pointerfd;
	struct sockaddr_in	addr, cliaddr;
	socklen_t			cliaddrlen = sizeof(cliaddr);
	TP	tp;
	

	/* allocate a socket */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("Could not create socket");
		return 1;
	}

	/* bind to port for any interface */
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_family		 = AF_INET;
	addr.sin_port		 = htons(SERVER_PORT);

	if (bind(sockfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		perror("Could not bind socket");
		close(sockfd);
		return 1;
	}
	
	/* listen for incoming connections */
	if (listen(sockfd, SERVER_BACKLOG) == -1) {
		perror("Could not listen");
		close(sockfd);
		return 1;
	}

	/* set up all shared data structures for threading */
	DEBUG("Initialising thread pool with %d threads\n", NTHREADS);
	tp = tp_init(NTHREADS, &close_connection, &connection_worker);

	/* accept (blocks until a client connects) */
	while(1) {
		connfd = accept(sockfd, (struct sockaddr *) &cliaddr, &cliaddrlen);
		if (connfd == -1) {
			perror("Accept failed");
			break;
		} else {
			if( (pointerfd = malloc(sizeof(int))) == NULL) {
				perror("malloc failed");
				break;
			}
			*pointerfd = connfd;
			DEBUG("main:\t Accepted connection, DISPATCHING.\n");
			tp_dispatch(tp, pointerfd);
		}
	}
	close(sockfd);
	
	/* free shared data structures */
	tp_destroy(tp);
	return 0;
}

/* wraps around handle_connection, given to thread pool as worker method.
 * arg is a (int *) cast to a (void *) for the generic buffer. */
void connection_worker(void *arg) {
	handle_connection(* (int *) arg);
}

/* closes the connection and frees the arg pointer. given to threadpool for freeing buffer items. */
void close_connection(void *arg) {
	close(* (int *) arg);
	free(arg);
}

/* connection handler. reads from given socket descriptor, parses input stream, and sends responses.
 * Does not close the file descriptor. */
void handle_connection(int fd) {
	const char EOR[] = "\r\n\r\n";
	unsigned int EOR_match;
	ssize_t count;
	int i;
	char buf[REQUEST_BUFFER_SIZE];
	char *request_string;	
	lsb request;
	int start_reading;

	DEBUG("Worker: Accepted connection\n");

	request = lsb_create();
	EOR_match = 0;
	
	while ((count = read(fd, &buf, (REQUEST_BUFFER_SIZE - 1))) > 0) {
		buf[count] = '\0';

		start_reading = 0;

		/* check for end of request */
		for (i = 0; i < count; i++) {
			if (buf[i] == EOR[EOR_match]) {
				EOR_match++;
			} else {
				EOR_match = 0;
			}

			if (EOR_match == strlen(EOR)) {
				EOR_match = 0;
				
				/* send request, including current buffer, to be handled. */
				lsb_add(request, &buf[start_reading]);
				
				request_string = lsb_string(request);
				if( !handle_request(fd, request_string) ) {
					DEBUG("connection handling failed. Abandoning connection.\n");
					free(request_string);
					lsb_destroy(request);
					return;
				}
				free(request_string);
				lsb_destroy(request);
				
				/* reset for next request */
				start_reading = i + 1; 
				request = lsb_create();
			}
		}

		lsb_add(request, &buf[start_reading]);
	}

	if (count == -1) {
		DEBUG(strerror(errno));
	}

	lsb_destroy(request);
}

/* sends http headers on given connection. returns 0 on write failure, 1 normally. */
int http_headers(int fd, int status, char *status_str, int content_type, size_t content_length) {
	char buf[SHORT_LINE_BUFFER];
	char *content_types[sizeof(enum http_mime)];
	content_types[MIME_TEXT_PLAIN] = "text/plain";
	content_types[MIME_TEXT_HTML]  = "text/html";
	content_types[MIME_IMAGE_GIF]  = "image/gif";
	content_types[MIME_IMAGE_PNG]  = "image/png";
	content_types[MIME_IMAGE_JPEG] = "image/jpeg";

	sprintf(buf, "HTTP/1.1 %i %s\r\n", status, status_str);
	if(write(fd, buf, strlen(buf)) == -1) {
		perror("writing headers failed.");
		return 0;
	}
	
	sprintf(buf, "Content-Type: %s\r\n", content_types[content_type]);
	if(write(fd, buf, strlen(buf)) == -1) {
		perror("writing headers failed.");
		return 0;
	}
	
	sprintf(buf, "Content-Length: %lu\r\n", (unsigned long) content_length);
	if(write(fd, buf, strlen(buf)) == -1) {
		perror("writing headers failed.");
		return 0;
	}

	sprintf(buf, "\r\n");
	if(write(fd, buf, strlen(buf)) == -1) {
		perror("writing headers failed.");
		return 0;
	}

	return 1;
}

void normalise_host_suffix(char *hostname, size_t size) {
	if (!strchr(hostname, '.')) {
		strncat(hostname, DOMAIN_SUFFIX, size);
	}
}

/* check if the given request host matches any of this servers' hostnames
 */
int host_match(const char *request) {
	char hostname[SHORT_LINE_BUFFER + 1], hostreq[SHORT_LINE_BUFFER + 1];

	sscanf(strstr(request, "Host"), "Host: %" STR(SHORT_LINE_BUFFER) "s", hostreq);

	/* strip out port number */
	if (strchr(hostreq, ':')) {
		*strchr(hostreq, ':') = '\0';
	}

	/* always answer to localhost */
	if (strcmp(hostreq, "localhost") == 0) {
		return 1;
	}

	/* normalise host names to include .dcs.gla.ac.uk suffix */
	gethostname(hostname, sizeof(hostname));
	normalise_host_suffix(hostname, SHORT_LINE_BUFFER);
	normalise_host_suffix(hostreq, SHORT_LINE_BUFFER);

	return (strcmp(hostreq, hostname) == 0);
}

/* returns the file size or -1 if the file is not accessible */
size_t file_size(const char *path){
	struct stat s;

	if( stat(path, &s) != 0) {
		return 0;
	} else if (!S_ISREG(s.st_mode)) {
		/* not a regular file (e.g. directory or symlink) */
		DEBUG("not a regular file\n");
		return 0;
	} else {
		return (size_t) s.st_size;
	}
}

/* get the mime type based on path/file name extension.
 * user must guarantee that path is always \0 terminated. */
enum http_mime file_mime(char *path){
	char *c, *ext;

	ext = 0;
	for (c = path; *c != '\0'; c++) {
		if (*c == '.') {
			ext = c + 1;
		}
	}

	if (ext == 0) {
		return MIME_APPLICATION_OCTET_STREAM; /* no extension in filename, assume binary file */
	}else if (strcasecmp(ext, "txt") == 0){
		return MIME_TEXT_PLAIN;
	} else if (strcasecmp(ext, "htm") == 0 || strcasecmp(ext, "html") == 0) {
		return MIME_TEXT_HTML;
	} else if (strcasecmp(ext, "jpg") == 0 || strcasecmp(ext, "jpeg") == 0) {
		return MIME_IMAGE_JPEG;
	} else if (strcasecmp(ext, "gif") == 0) {
		return MIME_IMAGE_GIF;
	} else if (strcasecmp(ext, "png") == 0) {
		return MIME_IMAGE_PNG;
	} else {
		return MIME_APPLICATION_OCTET_STREAM;
	}
}

/* write a string to the file descriptor */
int respond_string(int fd, char *response) {
	if( write(fd, response, strlen(response)) == -1) {
		perror("writing error document failed\n");
		return 0;
	}
	return 1;
}

/* copy a file to the connection file descriptor
 */
int respond_file(int fd, char *path) {
	char writebuf[FILE_COPY_BUFFER];
	FILE *docfd;
	size_t count;
	ssize_t written;

	docfd = fopen(path, "r");

	if(docfd == NULL) {
		DEBUG("could not open file\n");
	} else {
		while ( (count = fread(writebuf, 1, sizeof(writebuf), docfd)) > 0) {
			written = write(fd, writebuf, count);
			if(written != -1){
				DEBUG("%lu bytes written from file.\n", written);
			} else {
				perror("write failed");
				fclose(docfd);
				return 0;
			}
		}
	}

	fclose(docfd);
	return 1;
}

/* handle a single request, parsing the given string until EOR (\r\n\r\n) and ignoring any data after that.
 * Handles paths of up to 1023 characters.
 */ 
int handle_request(int fd, char *request) {
	char *response, *status_str;
	char path[SHORT_LINE_BUFFER + 1]; /* +1 to allow for \0 */
	size_t content_length = 0;
	enum http_status status = HTTP_OK;
	enum http_mime mime = MIME_TEXT_HTML;


	/* parse request data */
	DEBUG(request);

	if( sscanf(request, "GET /%" STR(SHORT_LINE_BUFFER) "s HTTP/1.1", path) != 1 ) {
		/* we only support GET, so might as well hard code it... */
		status = HTTP_BAD_REQUEST;
		DEBUG("Unsupported request method, HTTP version, or header format.\n");
	} else if (!host_match(request)) {
		DEBUG("hostname does not match\n");
		status = HTTP_BAD_REQUEST;	
	} else if (path[0] == '.' || path[0] == '/') {
		DEBUG("invalid path, cannot start with . or /\n");
		status = HTTP_BAD_REQUEST;
	} else if ((content_length = file_size(path)) == 0) {
		DEBUG("file not found or invalid\n");
		status = HTTP_NOT_FOUND;
	} else {
		DEBUG("--- Thread %lu serving request for %s.\n", pthread_self() % 1000, path);
		status = HTTP_OK;
		mime = file_mime(path);
	}

	/* handle http status codes */
	response = "";
	status_str = "";
	switch(status) {
		case HTTP_OK:
			status_str = "OK";
			break;
		case HTTP_NOT_FOUND:
			status_str = "Not Found";
			response = "<html><body><h1>404 File Not Found</h1></body></html>";
			break;
		case HTTP_BAD_REQUEST:
			status_str = "Bad Request";
			response = "<html><body><h1>400 Bad Request</h1></body></html>";
			break;
		default: /* Internal server error */
			status_str = "Internal Server Error";
			status = HTTP_INTERNAL_SERVER_ERROR;
			response = "<html><body><h1>500 Internal Server Error</h1></body></html>";
			break;
	}

	if (status != HTTP_OK) {
		content_length = strlen(response);
	}

	if(!http_headers(fd, status, status_str, mime, content_length)) {
		DEBUG("Writing headers failed. abandoning request.\n");
		return 0;
	}

	if (status == HTTP_OK) {
		if(!respond_file(fd, path)) {
			DEBUG("Writing file response failed. abandoning request.\n");
			return 0;
		}
	} else {
		if(!respond_string(fd, response)) {
			DEBUG("writing error message failed. abandoning request.\n");
			return 0;
		}
	}

	DEBUG("--- Thread %lu request for %s on connection %d complete.\n", pthread_self() % 1000, path, fd);
	return 1; /* success */
}
