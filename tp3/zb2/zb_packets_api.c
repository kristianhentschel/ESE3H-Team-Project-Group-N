#include "zb_packets.h"
#include "zb_transport.h"
#include "diagnostics.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

/* TODO define this somewhere more sensible, maybe in its own header file or compile-time definition on cmd line for make? */
#ifndef DEVICE_ID
#define DEVICE_ID 0x02
#endif

#define PACKET_DELIMETER 0x7E
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

/* utility fucntion to calculate checksum */
static char zb_checksum(char *buf, unsigned char len);
static void zb_send_frame(char *buf, unsigned char len);

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
	char buf[MAX_PACKET_SIZE];
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
 */
void zb_send_packet(char op, char *data, char len) {
	char buf[MAX_PACKET_SIZE];
	unsigned char n, i, chk;

	n = 0;
	buf[n++] = PACKET_DELIMETER;
	buf[n++] = op;
	buf[n++] = DEVICE_ID;
	buf[n++] = len;
	
	for (i = 0; i < len; i++) {
		buf[n] = data[i];
		n++;
	}
	
	chk = zb_checksum(buf, n);
	buf[n++] = chk;
	buf[n++] = '\n';

	zb_send_frame(buf, n);

	DIAGNOSTICS("sent packet of %d bytes.\n", n);
}

/* packages the api-specific structure part in a serial frame with a checksum */
void zb_send_frame(char *buf, unsigned char len){
	char frame[MAX_PACKET_SIZE];
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
 * checksum: sum of all bytes except checksum and initial delimeter.
 * keeping only lower 8 bits, subtract from 0xFF.
 */
char zb_checksum(char *buf, unsigned char len) {
	unsigned char i, result;

	result = 0;
	/* start counting at one to skip packet delimeter byte. */
	for (i = 1; i < len; i++) {
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
enum zb_parse_state {LEX_WAITING, LEX_IN_WORD, LEX_PACKET_OP, LEX_PACKET_FROM, LEX_PACKET_LENGTH, LEX_PACKET_DATA, LEX_PACKET_CHECKSUM};

enum zb_parse_response zb_parse(char c) {
	static unsigned char checksum;
	static unsigned char packet_data_count;
	static enum zb_parse_state state = LEX_WAITING;

	/* see the start of a packet - discard everything else.
	 * the delimeter character is illegal in data except in a checksum.
	 */

	if (state != LEX_PACKET_CHECKSUM && c == PACKET_DELIMETER) {
		state = LEX_PACKET_OP;
		checksum = 0;
		packet_data_count = 0;
		
		zb_packet_op = 0;
		zb_packet_from = 0;
		zb_packet_len = 0;

		return ZB_START_PACKET;
	}

	switch (state) {
		case LEX_WAITING:
			if (isalnum((int) c)) {
				zb_word_data[0] = c;
				zb_word_len = 1;
				state = LEX_IN_WORD;
			}
			break;
		case LEX_IN_WORD:
			if (isalnum((int) c)) {
				zb_word_data[zb_word_len++] = c;
				state = LEX_IN_WORD;
			} else {
				state = LEX_WAITING;
				return ZB_PLAIN_WORD;
			}
			break;
		case LEX_PACKET_OP:
			zb_packet_op = c;
			checksum += c;
			state = LEX_PACKET_FROM;
			break;
		case LEX_PACKET_FROM:
			zb_packet_from = c;
			checksum += c;
			state = LEX_PACKET_LENGTH;
			break;
		case LEX_PACKET_LENGTH:
			zb_packet_len = c;
			checksum += c;
			if (zb_packet_len == 0) {
				state = LEX_PACKET_CHECKSUM;
			} else {
				state = LEX_PACKET_DATA;
			}
			break;
		case LEX_PACKET_DATA:
			zb_packet_data[packet_data_count++] = c;
			checksum += c;
			if (packet_data_count == zb_packet_len) {
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
