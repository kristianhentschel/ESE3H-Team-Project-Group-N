#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>

#define UDP_PORT 5008
#define STR_PORT "5008"
#define BUF_SIZE 1024

int main (int argc, char *argv[]){
	int sockfd, r;
	char buffer[] = "Hello World!";
	struct addrinfo hints;
	struct addrinfo *addr;
	ssize_t count;
	char *hostname;

	if (argc < 2) {
		printf("usage: %s hostname\n", argv[0]);
		return 1;
	}
	hostname = argv[1];

	/* use first address returned by getaddrinfo as per handout. */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family		 = AF_UNSPEC;
	hints.ai_socktype	 = SOCK_DGRAM;

	if ((r = getaddrinfo(hostname, STR_PORT, &hints, &addr) != 0)) {
		printf("DNS lookup failed: %s\n", gai_strerror(r));
		return 1;
	}

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		printf("connection failed\n");
		return 1;
	}

	count = sendto(sockfd, buffer, sizeof(buffer), 0, addr->ai_addr, addr->ai_addrlen);
	
	if (count < 0) {
		perror("sendto failed");
	}

	close(sockfd);
	return 0;
}
