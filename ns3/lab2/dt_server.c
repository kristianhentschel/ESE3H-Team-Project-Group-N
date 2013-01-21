#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <assert.h>
#include <ctype.h>
#include <time.h>

#define BACKLOG 1
#define BUFSIZE 16
#define PORT 5001
#define TIMEBUFSIZE 64

static void do_TIME(int fd);
static void do_DATE(int fd);
static void serve_request(int connfd);
static void serve_requests(int connfd);

int main( void ) {
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
	addr.sin_port		 = htons(PORT);

	if (bind(sockfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		perror("Could not bind socket");
		close(sockfd);
		return 1;
	}
	
	//listen for incoming connections
	if (listen(sockfd, BACKLOG) == -1) {
		perror("Could not listen");
		return 1;
	}

	//accept (blocks until a client connects)
	connfd = accept(sockfd, (struct sockaddr *) &cliaddr, &cliaddrlen);
	if (connfd == -1) {
		perror("Accept failed");
		close(sockfd);
		return 1;
	} else {
		serve_requests(connfd);
	}
	
	//close overall socket
	close(sockfd);

	return 0;
}


static void do_TIME(int fd) {
	char data[TIMEBUFSIZE];
	int len;
	time_t t = time(NULL);
	struct tm *tm = localtime( &t );
//TODO there msut be a get timespec for current local time function, so we can plug something directly into strftime...
	len = strftime( data, TIMEBUFSIZE, "%H:%M:%S\n", tm );
	write(fd, data, len);
	fprintf(stderr, "served TIME request.\n");
}

static void do_DATE(int fd){
	char data[TIMEBUFSIZE];
	int len;
	time_t t = time(NULL);
	struct tm *tm = localtime( &t );
	len = strftime( data, TIMEBUFSIZE, "%d %b %Y\n", tm );
	write(fd, data, len);
	fprintf(stderr, "served DATE request.\n");
}

static void handle(int connfd, const char request_type[], const char request_data[]) {
	if (strcmp("DATE", request_type) == 0) {
		do_DATE(connfd);
	}else if (strcmp("TIME", request_type) == 0) {
		do_TIME(connfd);
	}else {
		fprintf(stderr, "Unknown request %s %s\n", request_type, request_data);
	}
}

static void serve_requests(int connfd) {
	int type_count, buf_count;
	char *p, buf[BUFSIZE], type[BUFSIZE], c;
	enum {S_INIT, S_IN_TYPE, S_AFTER_TYPE} state;

	printf("connection accepted\n");
	
	state = S_INIT;
	while ( (buf_count = read(connfd, &buf, BUFSIZE-1)) > 0 ) {
		buf[buf_count] = '\0';
		
		printf("read into buffer: %s\n", buf);

		for (p = buf; p < buf + buf_count; p++) {
			c = *p;
						
			switch (state) {
				case S_INIT:
					if ( isalpha((int) c) ) {
						state = S_IN_TYPE;
						type_count = 0;
						type[type_count++] = c;
					}
					break;
				case S_IN_TYPE:
					if ( isalpha((int) c) ) {
						type[type_count++] = c;
						if (type_count > BUFSIZE - 1) {
							state = S_INIT;
							printf("buffer overflow in scanning for type keyword\n");
						}
					} else {
						state = S_AFTER_TYPE;
						type[type_count] = '\0';
					}
					break;
				case S_AFTER_TYPE:
					if (c == '\n') {
						handle(connfd, type, "");
						state = S_INIT;
					}
					break;
				default:
					assert(0);

			}
		}
	}


	close(connfd);

}

static void serve_request(int connfd) {
	int count;
	char buf[BUFSIZE];

	//read into buffer.
	count = read(connfd, &buf, BUFSIZE-1);
	buf[count] = '\0';
	
	if (strcmp(buf, "DATE\r\n") == 0) {
		do_DATE(connfd);
	} else if (strcmp(buf, "TIME\r\n") == 0) {
		do_TIME(connfd);
	} else {
		fprintf(stderr, "invalid command\n");
	}
	close(connfd);

}
