#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BACKLOG 1
#define BUFSIZE 16
#define PORT 5000

int main( void ) {
	int 				sockfd, connfd, count;
	struct sockaddr_in	addr, cliaddr;
	socklen_t			cliaddrlen = sizeof(cliaddr);
	char				buf[BUFSIZE];

	//allocate a socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		fprintf(stderr, "Could not create socket: %s\n", strerror(errno));
		return 1;
	}

	//bind to port 5000 for any interface
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_family		 = AF_INET;
	addr.sin_port		 = htons(PORT);

	if (bind(sockfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		fprintf(stderr, "Could not bind socket: %s\n", strerror(errno));
		close(sockfd);
		return 1;
	}
	
	//listen for incoming connections
	if (listen(sockfd, BACKLOG) == -1) {
		fprintf(stderr, "Could not listen: %s\n", strerror(errno));
		close(sockfd);
		return 1;
	}

	//accept (blocks until client connects)
	connfd = accept(sockfd, (struct sockaddr *) &cliaddr, &cliaddrlen);
	if (connfd == -1) {
		fprintf(stderr, "Accept failed: %s\n", strerror(errno));
		close(sockfd);
		return 1;
	}
	
	//read all we can
	while ((count = read(connfd, &buf, BUFSIZE)) > 0) {
		if (count == -1) {
			fprintf(stderr, "Read failed: %s\n", strerror(errno));
			close(connfd);
			close(sockfd);
			return 1;
		}

		write(1, &buf, count);
	}
	fprintf(stdout, "\n");

	//close
	close(connfd);
	close(sockfd);

	return 0;
}
