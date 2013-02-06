#include "networking.h"
#include "linkedstringbuffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>


#define SERVER_PORT 8080
#define SERVER_BACKLOG 16
#define BUFFER_SIZE 64

void handle_connection(int fd);
void handle_request(int fd, char *request);

void errlog(char *msg) {
	fprintf(stderr, "%s\n", msg);
}

int main(void) {
	int 				sockfd, connfd;
	struct sockaddr_in	addr, cliaddr;
	socklen_t			cliaddrlen = sizeof(cliaddr);

	//allocate a socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("Could not create socket");
		return 1;
	}

	//bind to port for any interface
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_family		 = AF_INET;
	addr.sin_port		 = htons(SERVER_PORT);

	if (bind(sockfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		perror("Could not bind socket");
		close(sockfd);
		return 1;
	}
	
	//listen for incoming connections
	if (listen(sockfd, SERVER_BACKLOG) == -1) {
		perror("Could not listen");
		return 1;
	}

	//accept (blocks until a client connects)
	while(1) {
		connfd = accept(sockfd, (struct sockaddr *) &cliaddr, &cliaddrlen);
		if (connfd == -1) {
			perror("Accept failed");
			close(sockfd);
			return 1;
		} else {
			handle_connection(connfd);
		}
	}
	close(sockfd);
}


/* connection handler. reads from given socket descriptor, parses input stream, and sends responses. */
void handle_connection(int fd) {
	const char EOR[] = "\r\n\r\n";
	unsigned int EOR_match;
	int count, i;
	char buf[BUFFER_SIZE];
	char *request_string;	
	lsb request;
	int start_reading;

	errlog("Accepted connection");

	request = lsb_create();
	EOR_match = 0;
	
	while ((count = read(fd, &buf, BUFFER_SIZE - 1)) > 0) {
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


	errlog("Closing connection.");
	close(fd);
	lsb_destroy(request);
}

void http_headers(int fd, int status, int content_type, int content_length) {
	char buf[1024];

	sprintf(buf, "HTTP/1.1 %i OK\r\n", status);
	/* TODO handle error codes with correct status description (e.g. 404 Not Found) */
	write(fd, buf, strlen(buf));
	
	sprintf(buf, "Content-Type: %s\r\n", "text/plain");
	write(fd, buf, strlen(buf));

	sprintf(buf, "Content-Length: %i\r\n", content_length);
	write(fd, buf, strlen(buf));

	//sprintf(buf, "Connection: close\r\n");
	//write(fd, buf, strlen(buf));
	
	sprintf(buf, "\r\n");
	write(fd, buf, strlen(buf));


}

int host_match(const char *host) {
	return 1;
}

long file_size(const char *path){
	struct stat s;


	//TODO potential security issue: path may start with .. or / and access system files with permissions of webserver user.
	if( stat(path, &s) != 0) {
		return -1L;
	} else {
		return s.st_size;
	}
}

enum http_mime { MIME_TEXT_PLAIN, MIME_TEXT_HTML, MIME_IMAGE_JPEG, MIME_IMAGE_GIF, MIME_APPLICATION_OCTET_STREAM};
enum http_status {HTTP_OK = 200, HTTP_NOT_FOUND = 404, HTTP_BAD_REQUEST = 400, HTTP_INTERNAL_SERVER_ERROR = 500};

/* handle a single request, parsing the given string until EOR (\r\n\r\n) and ignoring any data after that. */
void handle_request(int fd, char *request) {
	char *response;
	char path[1024]; //TODO #define
	long content_length = 0;
	enum http_status status = HTTP_OK;
	enum http_mime mime = MIME_TEXT_PLAIN;

	fprintf(stderr, "parsing request:\n%s\n", request);

	/* parse request data */

	if( sscanf(request, "GET /%1023s HTTP/1.1", path) != 1 ) {
		/* we only support GET, so might as well hard code it... */
		status = HTTP_BAD_REQUEST;
		errlog("request does not match GET /%as HTTP/1.1");
	} else if (!host_match(request)) {
		/* unknown host */
		status = HTTP_INTERNAL_SERVER_ERROR;	
	} else if ((content_length = file_size(path)) < 0) {
		status = HTTP_NOT_FOUND;
	} else {
		status = HTTP_OK;
	}

	/* send response */
	response = "";
	switch(status) {
		case HTTP_OK:
			break;
		case HTTP_NOT_FOUND:
			response = "<html><body><h1>404 File  not found</h1></body></html>";
			break;
		case HTTP_BAD_REQUEST:
			response = "<html><body><h1>400 bad request</h1></body></html>";
			break;
		default: //Internal server error
			status = HTTP_INTERNAL_SERVER_ERROR;
			response = "<html><body><h1>500 internal server error</h1></body></html>";
			break;
	}
	
	http_headers(fd, status, mime, content_length);
	
	if (status == HTTP_OK) {
		char writebuf[1024];//TODO #define this
		FILE *docfd = fopen(path, "r");
		int count;

		while ( (count = fread(writebuf, 1, 1024, docfd) > 0 ) ) {
			write(fd, writebuf, count);
		}

		fclose(docfd);
	} else {
		write(fd, response, strlen(response));
	}
}
