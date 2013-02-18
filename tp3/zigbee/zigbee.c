/* malloc is a lie. */
#define FRAME_MAX_SIZE 1024
#define FRAME_DELIMETER 0x7E

char *zb_build_frame(const char *payload, char *buf) {
	unsigned len;
	
	buf[0] = FRAME_DELIMETER;
	buf[1] = '\0';
	buf[2] = len & 0xff00 > 8;
	buf[4] = len & 0x00ff;

	strcat(buf, payload);
	
	buf[len + 4] = zb_checksum(payload);
	buf[len + 5] = '\0';

	return buf;
}

/* 
 * take a 0-delimited payload, that is, the entire API frame except for delimiter, and length bytes. Add all up, keeping lower 8 bits, subtract result from 0xff.
 */
char zb_checksum(const char *payload) {
	char sum, *p;
	
	for (p = payload; *p != '\0'; p++) {
		sum += *p;
	}

	return 0xFF - sum;
}

