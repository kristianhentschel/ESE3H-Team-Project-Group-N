#include "zb_packets.h"
#include "zb_transport.h"
#include "diagnostics.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#define PACKET_DELIMETER 0x7E

/* TODO define this somewhere more sensible, maybe in its own header file or compile-time definition on cmd line for make? */
#ifndef DEVICE_ID
#define DEVICE_ID 0x02
#endif


#define ZB_API_TRANSMITREQUEST 0x10
#define ZB_API_RECEIVEPACKET 0x90
/* TODO this defines where all packets from this module go and should be an api method, not a #define. */
#define DEST_BROADCAST

/*
 * zb_packets.h
 *
 * Author: Kristian Hentschel
 * Team Project 3. University of Glasgow. 2013
 *
 * Implementation of a subset of the ZigBEE2 API packet protocol.
 */

/* global variables as declared in .h file */

char	zb_word_data[MAX_PACKET_SIZE];
int		zb_word_len;

char	zb_packet_data[MAX_PACKET_SIZE];
char	zb_packet_op;
char	zb_packet_from;
char	zb_packet_len;

/* private utility functions */
static unsigned char zb_checksum(unsigned char *buf, unsigned char len);
static void zb_send_frame(unsigned char *buf, unsigned char len);

/*
 * Command mode is not required with API firmware, so this does nothing.
 */
void zb_enter_command_mode() {
	/* this space intentionally left empty */
	return;
}

/*
 * Sends a request for an AT style command.
 * data can be NULL, in that case a command without parameter (read or action) is sent.
 *
 * This implements the AT Request API Frame.
 */
void zb_send_command_with_argument(char cmd[2], char *data, unsigned char len) {
	unsigned char buf[MAX_PACKET_SIZE];
	unsigned char n, i;

	n = 0;
	buf[n++] = 'A'; 
	buf[n++] = 'T';
	buf[n++] = cmd[0];
	buf[n++] = cmd[1];

	if (data != NULL) {
		buf[n++] = ' ';
		for (i = 0; i < len; i++) {
			buf[n++] = data[i]; 
		}
	}

	buf[n++] = '\r';
	buf[n++] = '\n';
	zb_send(buf, n);
	
	buf[n] = '\0';
	DIAGNOSTICS("sent command %s\n", buf);
}

/*
 * wrapper to send a simple command without an argument
 */
void zb_send_command(char cmd[2]) {
	zb_send_command_with_argument(cmd, NULL, 0);
}

/*
 * assembles a full packet with sender address, length, checksum
 *
 * This implements the RF Transmission Request API Frame.
 *
 * broadcast or unicast needs to be defined - this is a hack and the packets API should be updated with
 * zb_send_packet_broadcast(op, data, len) and zb_send_packet_unicast(address, op, data, len). (TODO)
 */
void zb_send_packet(char op, unsigned char *data, unsigned char len) {
	unsigned char buf[MAX_PACKET_SIZE];
	unsigned char n, i;
	
	n = 0;
	buf[n++] = ZB_API_TRANSMITREQUEST;

	/* frame id. 0 = no ack sent. */
	buf[n++] = 0x00;

	/* 64 bit destination address. for coord and broadcast, first 6 bytes are 0. */
	for (i = 0; i < 6; i++) {
		buf[n++] = 0x00;
	}

#ifdef DEST_BROADCAST
	buf[n++] = 0xff;
	buf[n++] = 0xff;

	/* 16 bit broadcast network address */
	buf[n++] = 0xff;
	buf[n++] = 0xfe; /* not a typo - see spec! */
#else
	buf[n++] = 0x00;
	buf[n++] = 0x00;

	/* 16 bit coordinator network address */
	buf[n++] = 0x00;
	buf[n++] = 0x00;
#endif

	/* broadcast hop radius (0 = max) */
	buf[n++] = 0x00;

	/* options (0x01 = disable ack, 0x02 = disable network address discovery */
	buf[n++] = 0x00;

	/* RF data: payload (op, from, data) */
	buf[n++] = op;
	buf[n++] = DEVICE_ID;
	
	for (i = 0; i < len; i++) {
		buf[n] = data[i];
		n++;
	}

	zb_send_frame(buf, n);
}

/* packages the api-specific structure part in a serial frame with a checksum */
void zb_send_frame(unsigned char *buf, unsigned char len){
	unsigned char frame[MAX_PACKET_SIZE];
	unsigned char n, i;

	n = 0;
	frame[n++] = PACKET_DELIMETER;
	frame[n++] = 0x00;
	frame[n++] = len;

	for (i = 0; i < len; i++) {
		frame[n++] = buf[i];
	}

	frame[n++] = zb_checksum(buf, len);
	frame[n++] = '\0';
	
	DIAGNOSTICS("Packaged %d bytes in a frame of %d bytes and sent it to the transport layer.\n", len, n);
	zb_send(frame, n);
}

/*
 * checksum: sum of all bytes, keeping only lower 8 bits, subtract from 0xFF.
 */
static unsigned char zb_checksum(unsigned char *buf, unsigned char len) {
	unsigned char i, result;

	result = 0;
	for (i = 0; i < len; i++) {
		result += buf[i];
	}

	return 0xFF - result;
}

/*
 * TODO: It's complicated.
 *
 * for return values see header file comment.
 *
 * keep a lot of state in static variables local to this function.
 *
 * store results in global variables defined in header file.
 */
enum zb_parse_state {LEX_WAITING, LEX_IN_WORD,
	LEX_FRAME_LENGTH_MSB, LEX_FRAME_LENGTH_LSB, LEX_API_ID, LEX_FRAME_ADDR64, LEX_FRAME_NETWORK_MSB, LEX_FRAME_NETWORK_LSB, LEX_FRAME_OPTIONS, 
	LEX_PACKET_OP, LEX_PACKET_FROM, LEX_PACKET_LENGTH, LEX_PACKET_DATA, LEX_PACKET_CHECKSUM};

enum zb_parse_response zb_parse(unsigned char c) {
	static unsigned char checksum, packet_data_count;
	static uint16_t			frame_length;
	static uint16_t			frame_bytes_seen;
	static uint16_t			frame_address_bytes_seen;
	static enum zb_parse_state state = LEX_WAITING;
	/* see the start of a packet - discard everything else.
	 * the delimeter character is illegal in data except in a checksum.
	 */

	if (state != LEX_PACKET_CHECKSUM && c == PACKET_DELIMETER) {
		state = LEX_FRAME_LENGTH_MSB;
		checksum = 0;
		frame_length = 0;
		frame_bytes_seen = 0;

		zb_packet_op = 0;
		zb_packet_from = 0;
		zb_packet_len = 0;

		return ZB_START_PACKET;
	}

	switch (state) {
		case LEX_WAITING:
			break;
		case LEX_FRAME_LENGTH_MSB:
			frame_length = (c << 8) & 0xff00;
			state = LEX_FRAME_LENGTH_LSB;
			break;
		case LEX_FRAME_LENGTH_LSB:
			frame_length &= (c & 0x00ff);
			state = LEX_API_ID;
			frame_bytes_seen = 0;
			break;
		case LEX_API_ID:
			frame_bytes_seen++;
			if (c == ZB_API_RECEIVEPACKET) {
				state = LEX_FRAME_ADDR64;
			} else {
				/* ignore this packet */
				state = LEX_WAITING;
				DIAGNOSTICS("Parse: seen packet with unhandled api id %x, ignoring.\n", c);
				return ZB_INVALID_PACKET;
			}
			break;
		case LEX_FRAME_ADDR64:
			frame_bytes_seen++;
			frame_address_bytes_seen++;
			if (frame_address_bytes_seen == 8) {
				state = LEX_FRAME_NETWORK_MSB;
			} else {
				state = LEX_FRAME_ADDR64;
			}
			break;
		case LEX_FRAME_NETWORK_MSB:
			/* ignoring */
			frame_bytes_seen++;
			state = LEX_FRAME_NETWORK_LSB;
			break;
		case LEX_FRAME_NETWORK_LSB:
			/* ignoring */
			frame_bytes_seen++;
			state = LEX_FRAME_OPTIONS;
			break;
		case LEX_FRAME_OPTIONS:
			/* ignoring */
			frame_bytes_seen++;
			state = LEX_PACKET_OP;
			break;
		case LEX_PACKET_OP:
			frame_bytes_seen++;
			zb_packet_op = c;
			checksum += c;
			state = LEX_PACKET_FROM;
			break;
		case LEX_PACKET_FROM:
			frame_bytes_seen++;
			zb_packet_from = c;
			checksum += c;
			state = LEX_PACKET_DATA;
			break;
		case LEX_PACKET_DATA:
			frame_bytes_seen++;
			zb_packet_data[packet_data_count++] = c;
			checksum += c;
			if (frame_bytes_seen == frame_length) {
				state = LEX_PACKET_CHECKSUM;
			} else {
				state = LEX_PACKET_DATA;
			}
			break;
		case LEX_PACKET_CHECKSUM:
			state = LEX_WAITING;
			DIAGNOSTICS("Sum character from packet: %0x, actual sum of received bytes: %0x\n", c, 0xff-checksum);
			if (0xFF - checksum == c) {
				return ZB_VALID_PACKET;
			} else {
				return ZB_INVALID_PACKET;
			}
		default:
			break;
	}

	return ZB_PARSING;
}
