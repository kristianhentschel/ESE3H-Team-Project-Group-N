#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define UDP_PORT 5008
#define BUF_SIZE 1024

int main (void){
	int sockfd;
	char buffer[BUF_SIZE];
	struct sockaddr addr;
	struct sockaddr_in saddr;
	socklen_t addrlen;
	ssize_t count;

	saddr.sin_addr.s_addr	= INADDR_ANY;
	saddr.sin_family		= AF_INET;
	saddr.sin_port			= htons(UDP_PORT);

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	bind(sockfd, (struct sockaddr *) &saddr, sizeof(saddr));

	count = recvfrom(sockfd, buffer, BUF_SIZE-1, 0, &addr, &addrlen);
	if (count < 0) {
		perror("recvfrom failed");
	} else {
		buffer[count] = '\0';
		printf("%s\n", buffer);
	}

	close(sockfd);
	return 0;
}
