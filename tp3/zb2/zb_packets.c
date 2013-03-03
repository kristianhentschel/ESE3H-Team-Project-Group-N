#include "zb_packets.h"
#include "zb_transport.h"

#include <string.h>
#include <ctype.h>

/* TODO define this somewhere more sensible, maybe in its own header file? */
#ifndef DEVICE_ID
#define DEVICE_ID 0x01
#endif

#define PACKET_DELIMETER 0x7E
/*
 * zb_packets.h
 *
 * Author: Kristian Hentschel
 * Team Project 3. University of Glasgow. 2013
 *
 * Implementation of a simple packetization layer.
 *
 */

static zb_checksum(char *buf, char len);

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
void zb_send_command(char cmd[2], char *data, char len) {
	char buf[MAX_PACKET_SIZE];
	char n, i;

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
	buf[n] = '\n'; /* TODO check which character is officially required for terminating an AT Command. */

	zb_send(buf, n);
}


/*
 * assembles a full packet with sender address, length, checksum
 */
void zb_send_packet(char type, char *data, char len) {
	char buf[MAX_PACKET_SIZE];
	char n, i;

	n = 0;
	buf[n++] = PACKET_DELIMETER;
	buf[n++] = DEVICE_ID;
	buf[n++] = len;
	
	for (i = 0; i < len; i++) {
		buf[n++] = data[n];
	}
	
	buf[n] = zb_checksum(&buf[1], n-1);

	zb_send(buf, n);
}

/*
 * use same checksum algorithm defined for ZigBEE API mode:
 * add all bytes, not including delimeter and checksum.
 * keeping only lower 8 bits, subtract from 0xFF.
 */
char zb_checksum(char *buf, char len) {
	char i, result;

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
enum zb_parse_state = {LEX_WAITING, LEX_IN_WORD, LEX_PACKET_OP, LEX_PACKET_FROM, LEX_PACKET_LENGTH, LEX_PACKET_DATA, LEX_PACKET_CHECKSUM};

enum zb_parse_response zb_parse(char c) {
	static char checksum;
	static char packet_data_count;
	static enum zb_parse_state state;

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
			if (isalnum(c)) {
				zb_word_data[0] = c;
				zb_word_len = 1;
				state = LEX_IN_WORD;
			}
			break;
		case LEX_IN_WORD:
			if (isalnum(c)) {
				zb_word_data[zb_word_len++] = c;
				state = LEX_IN_WORD;
			} else {
				state = LEX_WAITING;
				return ZB_PLAIN_WORD;
			}
			break;
		case LEX_PACKET_OP:
			zb_packet_op = c;
			state = LEX_PACKET_FROM;
			break;
		case LEX_PACKET_FROM:
			zb_packet_from = c;
			state = LEX_PACKET_LENGTH;
			break;
		case LEX_PACKET_LENGTH:
			zb_packet_len = c;
			state = LEX_PACKET_DATA;
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
			if (0xFF - checksum == c) {
				return ZB_VALID_PACKET;
			}
		default:
			break;
	}

	return ZB_PARSING;
}
