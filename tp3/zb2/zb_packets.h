#ifndef __ZB_PACKETS_H__
#define __ZB_PACKETS_H__

/*
 * zb_packets.h
 *
 * Author: Kristian Hentschel
 * Team Project 3. University of Glasgow. 2013
 *
 * simple packet generation/parsing tool set for transmission over zigbee nodes.
 *
 * packet mechanism completely unrelated to zigbee implementation.
 *
 * Zigbees used in transparent (AT) mode.
 *
 */

#define MAX_PACKET_SIZE 72

/* global variables to hold parser results */
char	zb_word_data[MAX_PACKET_SIZE]
int		zb_word_len;

char	zb_packet_data[MAX_PACKET_SIZE];
int 	zb_packet_from;
int		zb_packet_len;

/* return type of parser function */
enum zb_parse_response = {
	ZB_PARSING,
	ZB_PLAIN_WORD,
	ZB_START_PACKET,
	ZB_VALID_PACKET
};

/* enters command mode. It is the user's responsibility to exit from this by letting the timeout expire or sending an ATCN command. */
void zb_enter_command_mode();

/* sends an AT command. Must be in command mode to use this function. if data is not NULL, it is send after a space after the command. */
void zb_send_command(char cmd[2], char *data, char len);

/* packetizes the data and sends it using the linked transport implementation. */
void zb_send_packet(char type, char *data, char len);

/* parses the response, should be called in order on every character received.
 *
 * Return values:
 *  - ZB_PARSING - no valid response yet.
 *  - ZB_PLAIN_WORD - not a valid packet, but an alphanumeric word separated by spaces or line endings.
 *  	Result will be valid in zb_word_data and zb_word_len global variables.
 *  - ZB_VALID_PACKET - A complete valid packet, matching the checksum and length fields, has been received.
 *  	Result will be valid in zb_packet_data, zb_packet_from, and zb_packet_len.
 */
enum zb_parse_response zb_parse(char c);

#endif /* __ZB_PACKETS_H__ */
