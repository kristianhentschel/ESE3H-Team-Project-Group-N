#include "networking.h"
#include "linkedstringbuffer.h"
#include "threadpool.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

#include <pthread.h>

#define SERVER_PORT 8080
#define SERVER_BACKLOG 1 
#define NTHREADS 1
#define DOMAIN_SUFFIX ".dcs.gla.ac.uk"

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

static pthread_mutex_t mutex_stdio = PTHREAD_MUTEX_INITIALIZER;

void errlog(const char *msg) {
	pthread_mutex_lock(&mutex_stdio);
	fprintf(stderr, "%s\n", msg);
	pthread_mutex_unlock(&mutex_stdio);
}

int main(void) {
	int 				sockfd, connfd, *pointerfd;
	struct sockaddr_in	addr, cliaddr;
	socklen_t			cliaddrlen = sizeof(cliaddr);
	TP	tp;

	/* set up all shared data structures for threading */
	printf("Initialising thread pool with %d threads\n", NTHREADS);
	tp = tp_init(NTHREADS, &close_connection, &connection_worker);

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
			errlog("main:\t Accepted connection, DISPATCHING.");
			tp_dispatch(tp, pointerfd);
		}
	}
	close(sockfd);
	
	/* free shared data structures */
	pthread_mutex_lock(&mutex_stdio);
	pthread_mutex_destroy(&mutex_stdio);
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

	errlog("Worker: Accepted connection");

	request = lsb_create();
	EOR_match = 0;
	
	/* TODO count is of type size_t, the define - 1 is an int. */
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
					errlog("connection handling failed. Abandoning connection.");
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
		errlog(strerror(errno));
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
		errlog("not a regular file");
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
		perror("writing error document failed");
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
		errlog("could not open file");
	} else {
		while ( (count = fread(writebuf, 1, sizeof(writebuf), docfd)) > 0) {
			written = write(fd, writebuf, count);
			if(written != -1){
				fprintf(stderr, "%lu ", written);
				errlog("bytes written from file.");
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
	errlog(request);

	if( sscanf(request, "GET /%" STR(SHORT_LINE_BUFFER) "s HTTP/1.1", path) != 1 ) {
		/* we only support GET, so might as well hard code it... */
		status = HTTP_BAD_REQUEST;
		errlog("Unsupported request method, HTTP version, or header format.");
	} else if (!host_match(request)) {
		errlog("hostname does not match");
		status = HTTP_BAD_REQUEST;	
	} else if (path[0] == '.' || path[0] == '/') {
		errlog("invalid path, cannot start with . or /");
		status = HTTP_BAD_REQUEST;
	} else if ((content_length = file_size(path)) == 0) {
		errlog("file not found or invalid");
		status = HTTP_NOT_FOUND;
	} else {
		printf("--- Thread %lu serving request for %s.\n", pthread_self() % 1000, path);
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
			response = "<html><body><h1>404 File  not found</h1></body></html>";
			break;
		case HTTP_BAD_REQUEST:
			status_str = "Bad Request";
			response = "<html><body><h1>400 bad request</h1></body></html>";
			break;
		default: /* Internal server error */
			status_str = "Internal Server Error";
			status = HTTP_INTERNAL_SERVER_ERROR;
			response = "<html><body><h1>500 internal server error</h1></body></html>";
			break;
	}

	if (status != HTTP_OK) {
		content_length = strlen(response);
	}

	if(!http_headers(fd, status, status_str, mime, content_length)) {
		errlog("Writing headers failed. abandoning request.");
		return 0;
	}

	if (status == HTTP_OK) {
		if(!respond_file(fd, path)) {
			errlog("Writing file response failed. abandoning request.");
			return 0;
		}
	} else {
		if(!respond_string(fd, response)) {
			errlog("writing error message failed. abandoning request.");
			return 0;
		}
	}

	printf("--- Thread %lu request for %s on connection %d complete.\n", pthread_self() % 1000, path, fd);
	return 1; /* success */
}
