#include "networking.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#define SERVER_PORT 8080
#define SERVER_BACKLOG 16
#define BUFFER_SIZE 64

void handle_connection(int fd);
void handle_request(int fd, char *request);

void errlog(char *msg) {
	fprintf(stderr, "%s\n", msg);
}

int main(void) {
	int sockfd, connfd;
	
	sockfd = open_socket(SERVER_PORT);
	bind(sockfd, SERVER_PORT);
	listen(sockfd, SERVER_BACKLOG);
	
	while (1){
		connfd = accept(sockfd);
		handle_connection(connfd);
		close(connfd);
	}

	close(sockfd);
}

char *dynamic_strcat(char *s1, char *s2) {
	char *result;
   	if (!(result = malloc(strlen(s1) + strlen(s2) + 1))) {
		errlog("malloc failed");
		return NULL;
	}
	result[0] = '\0';
	strcat(result, s1);
	strcat(result, s2);
	return result;
}

/* connection handler. reads from given socket descriptor, parses input stream, and sends responses. */
void handle_connection(int fd){
	const char EOR[] = "\r\n\r\n";
	unsigned int EOR_match;
	int count, i, ri;
	char buf[BUFFER_SIZE], req_buf[BUFFER_SIZE];
	char *request, *temp;

	EOR_match = 0;
	while ((count = read(fd, &buf, BUFFER_SIZE - 1)) > 0) {
		/* check for end of request */
		ri = 0;
		for (i = 0; i < count; i++) {
			if (buf[i] == EOR[EOR_match]) {
				EOR_match++;
				if (EOR_match == strlen(EOR)) {
					/* found end of request */
					temp = request;
					req_buf[ri] = '\0';
					request = dynamic_strcat(request, req_buf);
					
					handle_request(fd, request);
				
					free(temp);
					EOR_match = 0;
					ri = 0;
				}
			} else {
				EOR_match = 0;
				req_buf[ri++] = buf[i];
			}
		}
	}
	if (count == -1) {
		errlog(strerror(errno));
	}
}
