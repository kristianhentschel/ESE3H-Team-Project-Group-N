#include "zigbee.h"

/*
 * wraps a payload in a frame that includes the delimeter, length, and checksum bytes.
 */
int zb_frame(const char *payload, char *buf) {
	unsigned len, n;
	const char *p;

	n = 0;

	buf[0] = FRAME_DELIMETER;

	n = 3;
	len = 0;

	for (p = payload; *p != FRAME_DELIMETER; p++) {
		buf[n++] = *p;
		len++;
	}
	
	buf[1] = (len & 0xff00) >> 8;
	buf[2] = (len & 0x00ff);
	
	buf[n++] = zb_checksum(payload);
	buf[n] = '\0';

	return n;
}

/* 
 * take a 0-delimited payload, that is, the entire API frame except for delimiter, and length bytes. Add all up, keeping lower 8 bits, subtract result from 0xff.
 */
char zb_checksum(const char *payload) {
	char sum;
   	const char *p;
	
	for (p = payload; *p != '\0'; p++) {
		sum += *p;
	}

	return 0xFF - sum;
}

/*
 * generates a transmit payload, overwriting the contents of buffer.
 * the payload must then be given to zb_frame to generate a complete uart packet.
 */
char *zb_transmit_payload(const char frameid,
		const uint64_t daddr,
		const uint16_t naddr,
		const char *data,
		char *buf) {
	int n, i;
	const char *p;
	
	n = 0;

	buf[n++] = ZB_API_TRANSMIT;
	
	for (i = 0; i < 8; i++) {
		buf[n++] = (daddr >> (8 - i)*8) & 0xff;
	}

	buf[n++] = 0x00; /* Broadcast radius: 0x00 = maximum, range 0x01-0x10 */
	buf[n++] = 0x00; /* Options: 0x01 disable ACK, 0x02 disable addr discovery */

	for (p = data; *p != '\0'; p++) {
		buf[n++] = *p; 
	}

	buf[n++] = '\0';
	return buf;
}

/*
 * generate a payload for transmission to all hosts on the PAN.
 */
char *zb_transmit_payload_broadcast( const char *data, char *buf ) {
	return zb_transmit_payload( 0x00, ZB_BROADCAST_64, ZB_BROADCAST_16, data, buf);
}

/*
 * create a AT command request payload
 */
char *zb_AT_payload( const char *cmd, const char *val, char *buf ) {
	int n;
	const char *p;

	n = 0;

	buf[n++] = ZB_API_ATCMD;
	buf[n++] = 0x00; /* frame id we don't care about yet */
	buf[n++] = cmd[0];
	buf[n++] = cmd[1];

	if (val != NULL) {
		for (p = val; *p != '\0'; p++) {
			buf[n++] = *p; 
		}
	}
	buf[n] = FRAME_DELIMETER;
	return buf;
}
