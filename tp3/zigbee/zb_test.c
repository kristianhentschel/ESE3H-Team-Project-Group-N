#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "zigbee.h"

void *monitor(void *arg);

int main(void) {
	FILE *serial;
	pthread_t mthread;
	int c, s;
	char buf[FRAME_MAX_SIZE], pbuf[FRAME_MAX_SIZE];

	serial = fopen("/dev/ttyAMA0", "r");

	pthread_create(&mthread, NULL, monitor, (void *) serial);

	c = 0;
	while (c != 'q') {
		c = getc(stdin);
		switch(c) {
			case 'n':
				printf("sending node discover command\n");
				zb_AT_payload("ND", NULL, pbuf);
				s = zb_frame(pbuf, buf);
				printf("sending frame: %s\n", buf);
				fwrite(buf, 1, s, serial);
				printf("written\n");
				fflush(serial);
				break;
			case 'q':
				printf("quitting\n");
				break;
			default:
				printf("unknown command\n");
				break;
		}
	}


	printf("attempting to cancel monitor thread\n");
	pthread_cancel(mthread);

	printf("closing port.\n");
	fclose(serial);

	printf("done.\n");
	return 0;
}

void *monitor(void *arg) {
	FILE *serial;
	int c;

	printf("serial monitoring thread started\n");

	serial = (FILE *) arg;

	while ((c = fgetc(serial)) != EOF) {
		if (c == 0x7e) {
			printf("\n");
		}
		printf("%02x ", (char) c);
	}
	return NULL;
}

