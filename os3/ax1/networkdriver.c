#include "networkdriver.h"

#include <pthread.h>


#define APP_COUNT MAX_PID + 1
#define TX_BUF_SIZE 10

/* =====================================
 * DATA TYPES & STATIC METHOD SIGNATURES 
 */

/* thread to await packets from the network and store them in the corresponding rx buffer. */
void *thread_packet_receiver(void *arg);

/* thread to take packets from the transmit queue and send them to the network device */
void *thread_packet_transmitter(void *arg);

/* receive buffer for a single application, holding a single packet.
 * all data structures are bounded and use static memory. */
typedef struct struct_simple_RXBuffer {
	pthread_cond_t available;
	PacketDescriptor packet;
} RXBuffer;

/* Overall container for receive buffers. hold one buffer for each pid. */
typedef struct struct_RXContainer{
	pthread_mutex_t lock;
	RXBuffer buffer[APP_COUNT];
} RXContainer;

/* Transmit buffer - implemented as a static circular queue */
typedef struct struct_TXBuffer {
	pthread_mutex_t lock;
	pthread_cond_t full;
	pthread_cond_t empty;
	int count;
	int first;
	int last;
	PacketDescriptor packets[TX_BUF_SIZE];
} TXBuffer;

/* ================
 * GLOBAL VARIABLES
 */

static FreePacketDescriptorStore global_FPDS;
static TXBuffer global_TX;
static RXContainer global_RX;

/* ======================
 * METHOD IMPLEMENTATIONS
 */

void init_network_driver(NetworkDevice               nd, 
                         void *                      mem_start, 
                         unsigned long               mem_length,
                         FreePacketDescriptorStore * fpds_ptr) {

}
