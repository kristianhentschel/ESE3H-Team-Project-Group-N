#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <time.h>

#define BACKLOG 1
#define BUFSIZE 16
#define PORT 5001

static void do_TIME(int fd);
static void do_DATE(int fd);
static void serve_request(int connfd);

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
		serve_request(connfd);
	}
	
	//close individual connection and socket.
	close(connfd);
	close(sockfd);

	return 0;
}

#define TIMEBUFSIZE 16

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
