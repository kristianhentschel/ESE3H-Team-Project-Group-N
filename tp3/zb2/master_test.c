#include <string.h>
#include <stdio.h>
#include "zb_transport.h"
#include "zb_packets.h"
#include "master_requesthandlers.h"

/*
 * master_test.c
 *
 * simple test application for sending, receiving, and parsing packets as the master unit.
 * This will later be integrated with a web-server for a friendlier user interface.
 *
 * Reads commands from standard input to emulate asynchronously appearing HTTP requests.
 *
 * Author: Kristian Hentschel
 * Team Project 3. University of Glasgow. 2013
 */


/* for testing only. will be replaced by webserver implementation. */
int main(void) {
	char c;

	zb_transport_init();
	
	c = 0;
	while (c != 'q') {
		switch (c) {
			case 'm':
				REQUEST_measure(stdout);
				break;
			case 'c':
				REQUEST_calibrate(stdout);
				break;
			case 'd':
				REQUEST_data(stdout);
				break;
			case 'p':
				REQUEST_ping(stdout);
				break;
			default:
				printf("unknown command %c\n", c);
		}
	}

	printf("good-bye");
	zb_transport_stop();	
	return;
}
	
/* this could/should be a separate thread. */
void parse(void *arg) {
	char c;

	while(1){
		c = zb_getc();
		printf("%0x ", c);
		fflush(stdout);

		switch (zb_parse(c)) {
			case ZB_START_PACKET:
				printf("\n(start of packet)\n");
				break;
			case ZB_PLAIN_WORD:
				printf("\n(plain word of %d characters)\n", zb_word_len);
				break;
			case ZB_VALID_PACKET:
				printf("\n(valid packet of %d characters with op code %x from device %x: '%s')\n", zb_packet_len, zb_packet_op, zb_packet_from, strndup(zb_packet_data, zb_packet_len));
				break;
			case ZB_INVALID_PACKET:
				printf("\n(invalid packet)\n");
				break;
			default:
				break;
		}
	}	
}
