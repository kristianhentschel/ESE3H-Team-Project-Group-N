#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define UDP_PORT 5010
#define GROUP_ADDR "224.0.0.20"
#define BUF_SIZE 1024

int main (void){
	int sockfd;
	char buffer[BUF_SIZE];
	struct sockaddr addr;
	socklen_t addrlen;
	struct sockaddr_in saddr;
	struct ip_mreq imr;
	ssize_t count;

	saddr.sin_addr.s_addr	= INADDR_ANY;
	saddr.sin_family		= AF_INET;
	saddr.sin_port			= htons(UDP_PORT);

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	bind(sockfd, (struct sockaddr *) &saddr, sizeof(saddr));

	inet_pton(AF_INET, GROUP_ADDR, &(imr.imr_multiaddr.s_addr));
	imr.imr_interface.s_addr = INADDR_ANY;

	if(setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &imr, sizeof(imr)) < 0) {
		perror("setsockopt could not join group");
		close(sockfd);
		return 1;
	}

	printf("ready to receive.\n");

	count = recvfrom(sockfd, buffer, BUF_SIZE-1, 0, &addr, &addrlen);
	if (count < 0) {
		perror("recvfrom failed");
		return 1;
	} else {
		/* TODO parse format and check for printable characters first. */
		buffer[count] = '\0';
		printf("received: %s\n", buffer);
	}

	
	if(setsockopt(sockfd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &imr, sizeof(imr)) < 0) {
		perror("setsockopt could not leave group");
	}

	close(sockfd);
	return 0;
}
