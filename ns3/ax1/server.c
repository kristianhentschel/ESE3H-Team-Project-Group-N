#include "networking.h"
#include "linkedstringbuffer.h"
#include "workerthreadpool.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>

#include <pthread.h>

#define SERVER_PORT 8080
#define SERVER_BACKLOG 0 
#define REQUEST_BUFFER_SIZE 64
#define NTHREADS 4
#define WORK_BUFFER_SIZE 1 

static int SERVER_RUNNING;

void connection_worker(void *arg);
void close_connection(void *arg);

void handle_connection(int fd);
void handle_request(int fd, char *request);

enum http_mime { MIME_TEXT_PLAIN, MIME_TEXT_HTML, MIME_IMAGE_JPEG, MIME_IMAGE_GIF, MIME_IMAGE_PNG, MIME_APPLICATION_OCTET_STREAM};
enum http_status {HTTP_OK = 200, HTTP_NOT_FOUND = 404, HTTP_BAD_REQUEST = 400, HTTP_INTERNAL_SERVER_ERROR = 500};

static pthread_mutex_t mutex_stdio = PTHREAD_MUTEX_INITIALIZER;

void errlog(const char *msg) {
	pthread_mutex_lock(&mutex_stdio);
	fprintf(stderr, "%s\n", msg);
	pthread_mutex_unlock(&mutex_stdio);
}

void signal_handler(int signal) {
	if (signal == SIGINT) {
		errlog("Server slowly shutting down after receiving SIGINT.");
		SERVER_RUNNING = 0;
	}
}

int main(void) {
	int 				sockfd, connfd, *pointerfd;
	struct sockaddr_in	addr, cliaddr;
	socklen_t			cliaddrlen = sizeof(cliaddr);
	WTP	wtp;

	/* set up all shared data structures for threading */
	wtp = wtp_init(NTHREADS, WORK_BUFFER_SIZE, &close_connection, &connection_worker);
	SERVER_RUNNING = 1;

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
		return 1;
	}

	/* set up signal handling TODO */

	/* accept (blocks until a client connects) */
	while(SERVER_RUNNING) {
		connfd = accept(sockfd, (struct sockaddr *) &cliaddr, &cliaddrlen);
		if (connfd == -1) {
			perror("Accept failed");
			break;
		} else {
			pointerfd = malloc(sizeof(int)); /* TODO malloc never fails... */
			*pointerfd = connfd;
			errlog("main:\t adding a connection to the work buffer.");
			wtp_put(wtp, pointerfd);
		
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

/* closes the connection and frees the arg pointer. */
void close_connection(void *arg) {
	close(* (int *) arg);
	free(arg);
}

/* connection handler. reads from given socket descriptor, parses input stream, and sends responses.
 * Does not close the file descriptor. */
void handle_connection(int fd) {
	const char EOR[] = "\r\n\r\n";
	unsigned int EOR_match;
	int count, i;
	char buf[REQUEST_BUFFER_SIZE];
	char *request_string;	
	lsb request;
	int start_reading;

	errlog("Worker: Accepted connection");

	request = lsb_create();
	EOR_match = 0;
	
	while ((count = read(fd, &buf, REQUEST_BUFFER_SIZE - 1)) > 0) {
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
				errlog("matched end of request");
				EOR_match = 0;
				
				/* send request, including current buffer, to be handled. */
				lsb_add(request, &buf[start_reading]);
				
				request_string = lsb_string(request);
				handle_request(fd, request_string);
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

void http_headers(int fd, int status, char *status_str, int content_type, int content_length) {
	char buf[1024];
	char *content_types[sizeof(enum http_mime)];
	content_types[MIME_TEXT_PLAIN] = "text/plain";
	content_types[MIME_TEXT_HTML]  = "text/html";
	content_types[MIME_IMAGE_GIF]  = "text/gif";
	content_types[MIME_IMAGE_PNG]  = "text/png";
	content_types[MIME_IMAGE_JPEG] = "text/jpeg";

	sprintf(buf, "HTTP/1.1 %i %s\r\n", status, status_str);
	write(fd, buf, strlen(buf));
	
	sprintf(buf, "Content-Type: %s\r\n", content_types[content_type]);
	
	
	write(fd, buf, strlen(buf));

	sprintf(buf, "Content-Length: %i\r\n", content_length);
	write(fd, buf, strlen(buf));

	sprintf(buf, "\r\n");
	write(fd, buf, strlen(buf));
}

/* check if the given request host matches any of this servers' hostnames
 */
int host_match(const char *request) {
	char hostname[1024], hostreq[1024];

	sscanf(strstr(request, "Host"), "Host: %1023s", hostreq);

	if (strchr(hostreq, ':')) {
		*strchr(hostreq, ':') = '\0';
	}

	gethostname(hostname, sizeof(hostname));

	return (strcmp(hostreq, hostname) == 0
			|| strcmp(hostreq, strcat(hostname, ".dcs.gla.ac.uk")) == 0
			|| strcmp(hostreq, "localhost") == 0);
}

/* returns the file size or -1 if the file is not accessible */
long file_size(const char *path){
	struct stat s;

	if( stat(path, &s) != 0) {
		return -1L;
	} else {

		return s.st_size;
	}
}

/* get the mime type based on path/file name extension.
 * user must guarantee that path is always \0 terminated. */
enum http_mime file_mime(char *path){
	char *c, *ext;
	
	for (c = path; *c != '\0'; c++) {
		if (*c == '.') {
			ext = c + 1;
		}
	}

	errlog(ext);

	if (strcasecmp(c, "txt") == 0){
		return MIME_TEXT_PLAIN;
	} else if (strcasecmp(c, "htm") == 0 || strcasecmp(c, "html")) {
		return MIME_TEXT_HTML;
	} else if (strcasecmp(c, "jpg") == 0 || strcasecmp(c, "jpeg")) {
		return MIME_IMAGE_JPEG;
	} else if (strcasecmp(c, "gif") == 0) {
		return MIME_IMAGE_GIF;
	} else if (strcasecmp(c, "png") == 0) {
		return MIME_IMAGE_PNG;
	} else {
		return MIME_APPLICATION_OCTET_STREAM;
	}
}

/* write a string to the file descriptor */
void respond_string(int fd, char *response) {
	errlog("responding with string");
	write(fd, response, strlen(response));
}

/* write a file to the connection file descriptor
 * TODO error checking for write system call, as client may close socket unexpectedly. Also, catch or block SIGPIPE signal when doing this.
 */
void respond_file(int fd, char *path) {
	char writebuf[1024*1024]; /* TODO #define this*/
	FILE *docfd;
	ssize_t count;

	docfd = fopen(path, "r");

	if(docfd == NULL) {
		errlog("could not open file");
	} else {
		while ( (count = fread(writebuf, 1, sizeof(writebuf), docfd)) > 0) {
			write(fd, writebuf, count);
			fprintf(stderr, "%lu ", count);
			errlog("bytes written from file.");
		}
	}
	fclose(docfd);
	errlog("file written completely.");
}

/* handle a single request, parsing the given string until EOR (\r\n\r\n) and ignoring any data after that.
 * Handles paths of up to 1023 characters.
 */ 
void handle_request(int fd, char *request) {
	char *response, *status_str;
	char path[1024]; 
	long content_length = 0;
	enum http_status status = HTTP_OK;
	enum http_mime mime = MIME_TEXT_HTML;


	/* parse request data */

	if( sscanf(request, "GET /%1023s HTTP/1.1", path) != 1 ) {
		/* we only support GET, so might as well hard code it... */
		status = HTTP_BAD_REQUEST;
		errlog("request does not match GET /%as HTTP/1.1");
	} else if (!host_match(request)) {
		/* unknown host */
		errlog("hostname does not match");
		status = HTTP_BAD_REQUEST;	
	} else if (path[0] == '.' || path[0] == '/') {
		/* path definitely outside document root. */
		errlog("invalid path, cannot start with . or /");
		status = HTTP_BAD_REQUEST;
	} else if ((content_length = file_size(path)) < 0) {
		/* file not found or un-stat-able */
		errlog("file not found");
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

	http_headers(fd, status, status_str, mime, content_length);

	if (status == HTTP_OK) {
		respond_file(fd, path);
	} else {
		respond_string(fd, response);
	}

	printf("--- Thread %lu request for %s complete.\n", pthread_self() % 1000, path);
}
