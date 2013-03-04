#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <termios.h>
#include "zigbee.h"

void *monitor(void *arg);

void print_packet(const char *buf, const int n) {
	int i;
	printf("[");
	for (i = 0; i < n; i++) {
		printf("%02x ", buf[i]);
	}
	printf("]\n" );
}

int main(void) {
	int serial;
	pthread_t mthread;
	int c, s;
	char buf[FRAME_MAX_SIZE], pbuf[FRAME_MAX_SIZE];
	struct termios tc;

	/* open serial port */
	serial = open("/dev/ttyAMA0", O_RDWR | O_NOCTTY | O_NDELAY);
	fcntl(serial, F_SETFL, 0);
	
	/* set tc options for serial port transfers */
	tcgetattr(serial, &tc);

	cfsetospeed(&tc, 9600);
	cfsetispeed(&tc, 9600);

	tc.c_cflag |= (CLOCAL | CREAD);

	tc.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	/* apply configuration */
	tcsetattr(serial, TCSANOW, &tc);


	pthread_create(&mthread, NULL, monitor, (void *) serial);

	c = 0;
	while (c != 'q') {
		c = getc(stdin);
		switch(c) {
			case 'i':
				printf("sending node identifier command\n");
				zb_AT_payload("NI", NULL, pbuf);
				s = zb_frame(pbuf, buf);
				print_packet(buf, s);
				write(serial, buf, s);
				printf("done\n");
				break;
			case 'd':
				printf("sending node discover command\n");
				zb_AT_payload("ND", NULL, pbuf);
				s = zb_frame(pbuf, buf);
				print_packet(buf, s);
				write(serial, buf, s);
				printf("done\n");
				break;
			case 'm':
				printf("sending MY (64-bit self addr) command\n");
				zb_AT_payload("MY", NULL, pbuf);
				s = zb_frame(pbuf, buf);
				print_packet(buf, s);
				write(serial, buf, s);
				printf("done\n");
				break;
			case '+':
				printf("Sending +++\n");
				write(serial, "+++", 3);
				break;
			case 'q':
				printf("quitting\n");
				break;
			case '\n':
				break;
			case '\r':
				break;
			case ' ':
				break;
			default:
				printf("unknown command %c\n", c);
				break;
		}
	}

	printf("attempting to cancel monitor thread\n");
	pthread_cancel(mthread);

	printf("closing port.\n");
	close(serial);

	printf("done.\n");
	return 0;
}

void *monitor(void *arg) {
	int serial;
	int c;

	printf("serial monitoring thread started\n");

	serial = (int) arg;

	while ( read(serial, &c, 1) > 0 ) {
		if ((char) c == 0x7e) {
			fprintf(stderr, "\n");
		}

		fprintf(stderr, "%02x ", (char) c);

		if ((char) c == 0x0d) {
			fprintf(stderr, "\n");
		}
	}
	return NULL;
}

