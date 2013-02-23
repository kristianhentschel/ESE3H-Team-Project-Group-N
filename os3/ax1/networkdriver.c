
#include "networkdriver.h"
#include <assert.h>
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
	pthread_cond_t nonfull;
	pthread_cond_t nonempty;
	int count;
	int first;
	int last;
	PacketDescriptor packets[TX_BUF_SIZE];
} TXBuffer;

/* ================
 * GLOBAL VARIABLES
 */

static FreePacketDescriptorStore *FPDS;
static TXBuffer TX;
static RXContainer RX;
static NetworkDevice ND;

/* ======================
 * METHOD IMPLEMENTATIONS
		pthread_cond_signal(&RX.available);
 */

void init_network_driver(NetworkDevice               nd, 
                         void *                      mem_start, 
                         unsigned long               mem_length,
                         FreePacketDescriptorStore * fpds_ptr) {
	ND = nd;

	TX.lock;
	TX.nonfull;
	TX.nonempty;
	TX.count = 0;
	TX.first = 0;
	TX.last = 0;

	/* TODO: initialize packet decriptor store from mem_start and mem_length and 
	 * hand it back in fpds_ptr, also store it in FPDS.
	 */

	/* TODO: initialise worker threads (receiver and transmitter) */
}


void blocking_send_packet(PacketDescriptor pd) {
	pthread_mutex_lock(&TX.lock);
	while(TX.count == TX_BUF_SIZE) {
		pthread_cond_wait(&TX.nonfull, &TX.lock);
	}

	TX.packets[TX.last++ % TX_BUF_SIZE] = pd;
	TX.count++;
	/* TODO code duplication with nonblocking version */

	pthread_cond_signal(&TX.nonempty);
	pthread_mutex_unlock(&TX.lock);
}

int  nonblocking_send_packet(PacketDescriptor pd) {
	int ret = 0;
	pthread_mutex_lock(&TX.lock);

	if (TX.count < TX_BUF_SIZE) {
		TX.packets[TX.last++ % TX_BUF_SIZE] = pd;
		TX.count++;
		ret = 1;
	}

	pthread_cond_signal(&TX.nonempty);
	pthread_mutex_unlock(&TX.lock);

	return ret;
}

void blocking_get_packet(PacketDescriptor *pd, PID pid) {
	assert(pid <= APP_COUNT);
	
	/* TODO code duplication with nonblocking version */
	
	pthread_mutex_lock(&RX.lock);
	while(RX.buffer[pid].packet == NULL) {
		pthread_cond_wait(&RX.buffer[pid].available, &RX.lock);
	}

	*pd = RX.buffer[pid].packet;
	RX.buffer[pid].packet = NULL;

	pthread_cond_signal(&RX.buffer[pid].available);
	pthread_mutex_unlock(&RX.lock);
}
int  nonblocking_get_packet(PacketDescriptor *pd, PID pid) {
	int ret = 0;
	assert(pid <= APP_COUNT);
	
	pthread_mutex_lock(&RX.lock);
	if (RX.buffer[pid].packet != NULL) {
		*pd = RX.buffer[pid].packet;
		RX.buffer[pid].packet = NULL;
		pthread_cond_signal(&RX.buffer[pid].available);
		ret = 1;
	}

	pthread_mutex_unlock(&RX.lock);
	return ret;
}
