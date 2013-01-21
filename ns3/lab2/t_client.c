#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#define MESSAGE "TIME\r\n"
#define STR_PORT "5001"
#define BUFSIZE 16

/* get a file descriptor for the connection or return -1 on error */
int connect_hostname(const char hostname[]);

int main( int argc, char* argv[] ) {
	int servfd;
	int count, i;
	char buf[BUFSIZE], *p;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s server_hostname\n", argv[0]);
		return 1;
	}


	if ((servfd = connect_hostname(argv[1])) < 0) {
		return 1;
	}

	for(i = 0; i < 10; i++) {
		//write
		write(servfd, MESSAGE, sizeof(MESSAGE));
		write(servfd, MESSAGE, sizeof(MESSAGE));

		//read response and print to stdout
		while ((count = read(servfd, &buf, BUFSIZE-1)) > 0) {
			if (count == -1) {
				perror("Read failed");
				close(servfd);
				return 1;
			}
			buf[count] = '\0';
			

			fwrite(&buf, sizeof(char), count, stdout);
			
			/* got a \0 from the socket, not from here?
			 * If we did, assume anything after is junk, */
			for (p = buf; *p != '\0'; p++)
				;
			if (p - buf < count - 1) {
				break;
			}
		}
		fprintf(stdout, "\n");
	
		sleep(1);
	}

	//close connection
	close(servfd);


	return 0;
}


int connect_hostname(const char hostname[]) {
	struct addrinfo		hints, *ai, *ai0;
	int fd, r;

	//memset fills area with n given bytes - in this case, 0 entire struct.
	memset(&hints, 0, sizeof(hints));
	
	hints.ai_family		 = PF_UNSPEC;
	hints.ai_socktype	 = SOCK_STREAM;

	if ((r = getaddrinfo(hostname, STR_PORT, &hints, &ai0) != 0)) {
		fprintf(stderr, "DNS lookup failed: %s\n", gai_strerror(r));
		return -1;
	}

	for (ai = ai0; ai != NULL; ai = ai->ai_next) {
		fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		/* TODO does this not re-open an existing socket if socket works but connect fails? */
		if (fd == -1) {
			perror("Could not create socket:");
			continue;
		}

		if (connect(fd, ai->ai_addr, ai->ai_addrlen) == -1) {
			perror("Could not connect:");
			close(fd);
			continue;
		}
		
		/* successful connection */
		return fd;
	}

	/* could not connect */
	close(fd);
	return -1;
}
