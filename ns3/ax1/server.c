#include "networking.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthreads.h>
#include <string.h>
#include <types.h>

#define SERVER_PORT 8080
#define SERVER_BACKLOG 16

void handle_connection(int fd);
void log(char *msg) {
	sprintf(stderr, msg);
}

int main(void) {
	int sockfd, connfd;
	
	sockfd = open_socket(SERVER_PORT);
	listen(sockfd, SERVER_BACKLOG);
	
	while (true){
		connfd = accept();
		handle_connection(fd)
		close(connfd);
	}

	close(sockfd);
}


/* connection handler. reads from given socket descriptor, parses input stream, and sends responses. */
void handle_connection(int fd){
	char *dbuf, *oldbuf, *p;
	char sbuf[BUFFER_SIZE];
	int len, EOR_match;
	const char EOR = "\r\n\r\n";	
	
	EOR_match = 0;
	while ((count = read(fd, &sbuf, BUFFER_SIZE - 1)) > 0) {
		/* concatenate old buffer with new data */
		len = len + count;
		dbuf = malloc(len + count);
		oldbuf = dbuf;
		strcpy(dbuf, oldbuf);
		p = dbuf + len;
		memcpy(p, sbuf, count);
		free(oldbuf);
		
		/* check for end of request */
		for(;p < len + count; p++) {
			if(*p == EOR[EOR_match]) {
				EOR_match++;
				if (EOR_match == sizeof(EOR)) {
					handle_request(dbuf);
					oldbuf = dbuf;
					dbuf = malloc(p - dbuf);
					strcpy(dbuf, p);
					free(oldbuf);


					free(oldbuf)
			} else {
				EOR_match = 0;
			}
	}
	if (count == -1) {
		log(strerror("Read from socket failed"));
	}
}
