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

int main (int argc, char *argv[]){
	int sockfd;
	struct sockaddr_in addr;
	ssize_t count;

	if (argc < 2) {
		printf("usage: %s message\n", argv[0]);
		return 1;
	}

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		printf("could not open socket\n");
		return 1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_port = UDP_PORT;
	inet_pton(AF_INET, GROUP_ADDR, &addr.sin_addr);

	/* TODO fill buffer with given format (FROM getlogin()\n Message \n) */

	count = sendto(sockfd, argv[1], sizeof(argv[1]), 0, (struct sockaddr *) &addr, sizeof(addr));
	if (count < 0) {
		perror("sendto failed");
	}

	close(sockfd);
	return 0;
}
