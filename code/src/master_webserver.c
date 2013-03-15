/* zigbee includes */
#include "zb_packets.h"
#include "zb_transport.h"
#include "diagnostics.h"
#include "requesthandlers.h"

/* api paths */
#define PATH_API_BASE		"api/"
#define PATH_MEASURE 		"api/measure/"
#define PATH_CALIBRATE		"api/calibrate/"
#define PATH_DATA			"api/data/"
#define PATH_PING			"api/ping/"

/* webserver includes */
#include "linkedstringbuffer.h"

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

#define SERVER_PORT 8080
#define SERVER_BACKLOG 16
#define REQUEST_BUFFER_SIZE 64

void handle_connection(int fd);
void handle_request(int fd, char *request);

/* zigbee serial parser and responder */
static void *thread_zb_listen(void *arg);

enum http_mime { MIME_TEXT_PLAIN, MIME_TEXT_HTML, MIME_IMAGE_JPEG, MIME_IMAGE_GIF, MIME_IMAGE_PNG, MIME_APPLICATION_OCTET_STREAM, MIME_TEXT_CSS, MIME_TEXT_JAVASCRIPT, NUM_MIME};
enum http_status {HTTP_OK = 200, HTTP_NOT_FOUND = 404, HTTP_BAD_REQUEST = 400, HTTP_INTERNAL_SERVER_ERROR = 500};

void errlog(char *msg) {
	fprintf(stderr, "WEBSERVER: %s\n", msg);
}

int main(void) {
	int 				sockfd, connfd;
	struct sockaddr_in	addr, cliaddr;
	socklen_t			cliaddrlen = sizeof(cliaddr);
	pthread_t			zbthread;

	//set up zigbee specific stuff
	zb_packets_init();
	zb_set_broadcast_mode(1);
	zb_set_device_id(0);

	pthread_create(&zbthread, NULL, thread_zb_listen, NULL);

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
	zb_transport_stop();
	return 0;
}


/* connection handler. reads from given socket descriptor, parses input stream, and sends responses. */
void handle_connection(int fd) {
	const char EOR[] = "\r\n\r\n";
	unsigned int EOR_match;
	int count, i;
	char buf[REQUEST_BUFFER_SIZE];
	char *request_string;	
	lsb request;
	int start_reading;

	errlog("Accepted connection");

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


	errlog("Closing connection.");
	close(fd);
	lsb_destroy(request);
}

void http_headers(int fd, int status, char *status_str, int content_type, int content_length) {
	char buf[1024];
	char *content_types[NUM_MIME];
	content_types[MIME_TEXT_PLAIN] = "text/plain";
	content_types[MIME_TEXT_HTML]  = "text/html";
	content_types[MIME_IMAGE_GIF]  = "text/gif";
	content_types[MIME_IMAGE_PNG]  = "text/png";
	content_types[MIME_IMAGE_JPEG] = "text/jpeg";
	content_types[MIME_TEXT_CSS] = "text/css";
	content_types[MIME_TEXT_JAVASCRIPT] = "text/javascript";
	content_types[MIME_APPLICATION_OCTET_STREAM] = "application/octet-stream";

	snprintf(buf, sizeof(buf), "HTTP/1.1 %i %s\r\n", status, status_str);
	write(fd, buf, strlen(buf));
	
	snprintf(buf, sizeof(buf), "Content-Type: %s\r\n", content_types[content_type]);
	
	
	write(fd, buf, strlen(buf));

	snprintf(buf, sizeof(buf), "Content-Length: %i\r\n", content_length);
	write(fd, buf, strlen(buf));

	snprintf(buf, sizeof(buf),  "Connection: close\r\n");
	write(fd, buf, strlen(buf));
	
	snprintf(buf, sizeof(buf), "\r\n");
	write(fd, buf, strlen(buf));
}

/* check if the given request host matches any of this servers' hostnames
 * TODO strip out port number in *host
 */
int host_match(const char *host) {
	char hostname[1024], hostreq[1024];

	strncpy(hostreq, host, sizeof(hostreq));

	if (strchr(hostreq, ':')) {
		*strchr(hostreq, ':') = '\0';
	}

	gethostname(hostname, sizeof(hostname));

	return 1;
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

	if (strcasecmp(ext, "txt") == 0){
		return MIME_TEXT_PLAIN;
	} else if (strcasecmp(ext, "htm") == 0 || strcasecmp(ext, "html") == 0) {
		return MIME_TEXT_HTML;
	} else if (strcasecmp(ext, "jpg") == 0 || strcasecmp(ext, "jpeg") == 0) {
		return MIME_IMAGE_JPEG;
	} else if (strcasecmp(ext, "gif") == 0) {
		return MIME_IMAGE_GIF;
	} else if (strcasecmp(ext, "png") == 0) {
		return MIME_IMAGE_PNG;
	} else if (strcasecmp(ext, "js") == 0) {
		return MIME_TEXT_JAVASCRIPT;
	} else if (strcasecmp(ext, "css") == 0) {
		return MIME_TEXT_CSS;
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
	char writebuf[1024*1024];
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
	char response_buf[REQUEST_RESULT_BUFSIZE];
	long content_length = 0;
	enum http_status status = HTTP_OK;
	enum http_mime mime = MIME_TEXT_HTML;
	int is_api_request = 0;

	fprintf(stderr, "parsing request:\n%s\n", request);

	/* parse request data */
	response = "";
	if( sscanf(request, "GET /%1023s HTTP/1.1", path) != 1 ) {
		/* we only support GET, so might as well hard code it... */
		status = HTTP_BAD_REQUEST;
		errlog("request does not match GET /%as HTTP/1.1");
	} else if (!host_match(request)) {
		/* unknown host */
		errlog("unknown host");
		status = HTTP_BAD_REQUEST;	
	} else if (path[0] == '.' || path[0] == '/') {
		/* path definitely outside document root. */
		errlog("invalid path, cannot start with . or /");
		status = HTTP_BAD_REQUEST;
	} else if (strncmp(path, PATH_API_BASE, strlen(PATH_API_BASE)) == 0) {
		mime = MIME_TEXT_PLAIN;
		status = HTTP_OK;
		if (strcmp(path, PATH_MEASURE) == 0) {
			REQUEST_measure(response_buf);
		} else if (strcmp(path, PATH_CALIBRATE) == 0) {
			REQUEST_calibrate(response_buf);
		} else if (strcmp(path, PATH_DATA) == 0) {
			REQUEST_data(response_buf);
		} else if (strcmp(path, PATH_PING) == 0) {
			REQUEST_ping(response_buf);
		} else {
			status = HTTP_NOT_FOUND;
		}

		if (status == HTTP_OK) {
			is_api_request = 1;
			response = strdup(response_buf);
		}
	} else if ((content_length = file_size( (strcmp(path, "HTTP/1.1") != 0) ? path : strcpy(path, "index.html"))) < 0) {
		/* TODO let's not do black magic assignments and ternaries in if conditions. */
		/* file not found or un-stat-able */
		errlog("file not found");
		status = HTTP_NOT_FOUND;
	} else {
		errlog("file found"); 
		status = HTTP_OK;
		mime = file_mime(path);
	}

	/* handle http status codes */
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
		default: //Internal server error
			status_str = "Internal Server Error";
			status = HTTP_INTERNAL_SERVER_ERROR;
			response = "<html><body><h1>500 internal server error</h1></body></html>";
			break;
	}

	if (status != HTTP_OK || (status == HTTP_OK && is_api_request)) {
		content_length = strlen(response);
	}

	http_headers(fd, status, status_str, mime, content_length);

	if (status == HTTP_OK && !is_api_request) {
		respond_file(fd, path);
	} else {
		respond_string(fd, response);
		if (is_api_request) {
			free(response);
		}
	}
	errlog("--- request handling complete ---");
}



static void *thread_zb_listen(void *arg) {
	char c;

	while(1){
		c = zb_getc();
		switch (zb_parse(c)) {
			case ZB_VALID_PACKET:
				printf("\n(valid packet of %d characters with op code %x from device %x: '%s')\n", zb_packet_len, zb_packet_op, zb_packet_from, strndup(zb_packet_data, zb_packet_len));
				HANDLE_packet_received();
				break;
			case ZB_INVALID_PACKET:
				printf("\n(invalid packet)\n");
				break;
			default:
				break;
		}
	}
	return NULL;
}
