#include "zb_transport.h"
#include "zb_packets.h"
#include <string.h>
#include <stdio.h>

/*
 * master_test.c
 *
 * simple test application for sending, receiving, and parsing packets as the master unit.
 * This will later be integrated with a web-server for a friendlier user interface.
 *
 * Author: Kristian Hentschel
 * Team Project 3. University of Glasgow. 2013
 */

int main(void) {
	char c;

	zb_transport_init();

	zb_enter_command_mode();
	zb_send_command("NI");
	zb_send_command("CN");

	zb_send_packet(42, "Hello World", 12);

	zb_send_packet(0x01, "ping", 4);
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
