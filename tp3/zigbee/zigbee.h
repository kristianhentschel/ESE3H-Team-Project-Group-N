#ifndef __ZIGBEE_FRAMES_H__
#define __ZIGBEE_FRAMES_H__

#include <inttypes.h>

#ifndef NULL
#define NULL 0
#endif

#define FRAME_MAX_SIZE 256
#define FRAME_DELIMETER 0x7E

#define ZB_API_ATCMD		0x08
#define ZB_API_TRANSMIT 	0x10

#define ZB_AT_NODEDISCOVER	"ND"

#define ZB_BROADCAST_64		0xFFFF
#define ZB_BROADCAST_16		0xFFFE

typedef enum nodeid { NODE_MASTER, NODE_SCALE1, NODE_SCALE2, NODE_SCALE3, NODE_SCALE4 } nodeid_t;

typedef struct zigbee_response {  
	char data[FRAME_MAX_SIZE];
	uint16_t length;
} ZBR;

int zb_frame(const char *payload, char *buf);

char zb_checksum(const char *payload);

char *zb_transmit_payload_nodeid( const char frameid,
		const nodeid_t node,
		const char *data,
		char *buf );

char *zb_transmit_payload_broadcast( const char *data, char *buf );

char *zb_AT_payload( const char *cmd, const char *val, char *buf );

char *zb_node_discovery_payload( char *buf );

char *zb_parse_response( const char *response );

#endif /* __ZIGBEE_FRAMES_H__ */
