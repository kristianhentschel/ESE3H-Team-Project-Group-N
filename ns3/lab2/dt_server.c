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
	len = strftime( data, TIMEBUFSIZE, "%H:%M:%S", tm );
	write(fd, data, len);
}
static void do_DATE(int fd){
	char data[TIMEBUFSIZE];
	int len;
	time_t t = time(NULL);
	struct tm *tm = localtime( &t );
	len = strftime( data, TIMEBUFSIZE, "%d %b %Y", tm );
	write(fd, data, len);

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
	int count;
	int c;
	char type_buf[BUFSIZE];
	enum {IN_KEYWORD, OUT, AFTER_KEYWORD} state;

	//read one character at a time
	printf("connection accpeted\n");	
	while (read(connfd, &c, 1) > 0 ) {
		if ( state == OUT && isalpha(c)){
			state = IN_KEYWORD;
			count = 0;
		}
		if ( state == IN_KEYWORD && !isalpha(c)){
			state = AFTER_KEYWORD;
			type_buf[count] = '\0';
			printf("captured keyword %s\n", type_buf);
			/* a single keyword is all we need to handle a basic request */
			if( strcmp(type_buf, "DATE") == 0 || strcmp(type_buf, "TIME") ) {	
				/* ignore rest of line after simple keyword */
				while( read(connfd, &c, 1) > 0 && c != '\n' )
					;
				printf("handling simple %s request\n", type_buf);
				handle(connfd, type_buf, NULL);
			}

		} else if (state == IN_KEYWORD && count < BUFSIZE-1) {
			printf("added %c to keyword buffer\n", (char) c);
			type_buf[count++] = (char) c;
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
