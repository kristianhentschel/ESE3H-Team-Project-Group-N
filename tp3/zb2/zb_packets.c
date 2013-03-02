#include "zb_packets.h"
#include "zb_transport.h"

#include <string.h>


/*
 * zb_packets.h
 *
 * Author: Kristian Hentschel
 * Team Project 3. University of Glasgow. 2013
 *
 * Implementation of a simple packetization layer.
 *
 */

/*
 * send command mode sequence within guard times of 1 second silence.
 *
 * after this, the word OK should be received and the device will accept AT commands.
 * Must exit command mode by sending ATCN command.
 */
void zb_enter_command_mode() {
	zb_guard_delay();
	zb_send("+++", 3);
	zb_guard_delay();
}

/*
 * sends data verbatim for use with AT commands:
 * "AT" cmd data "\r\n"
 * data can be NULL, in that case a command without parameter (read or action) is sent.
 */
void zb_send_command(char cmd[2], char *data, int len) {
	char buf[MAX_PACKET_SIZE];
	char *p;
	int n;

	n = 0;
	buf[n++] = cmd[0];
	buf[n++] = cmd[1];

	if (data != NULL) {
		buf[n++] = ' ';
		for (i = 0; i < len; i++) {
			buf[n++] = data[i]; 
		}
	}

	buf[n++] = '\r';
	buf[n] = '\n'; /* TODO check which character is officially required */

	zb_send(buf, n);
}


void zb_send_packet(char *data, int len);
enum zb_parse_response zb_parse(char c);
