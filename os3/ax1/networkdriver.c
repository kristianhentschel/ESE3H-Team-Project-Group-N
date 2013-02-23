#include "networkdriver.h"

#include <pthread.h>


#define APP_COUNT MAX_PID + 1
#define TX_BUF_SIZE 10

/* thread to await packets from the network and store them in the corresponding rx buffer. */
void *thread_packet_receiver(void *arg);

/* thread to take packets from the transmit queue and send them to the network device */
void *thread_packet_transmitter(void *arg);

/* receive buffer for a single application, holding a single packet. */
typedef struct struct_simple_rx_buffer {
	pthread_cond_t available;
	PacketDescriptor packet;
} rx_buffer;

/* Overall container for receive buffers. hold one buffer for each pid. */
typedef struct struct_rx_container{
	pthread_mutex_t lock;
	rx_buffer[APP_COUNT];
} rx_container;

/* Transmit buffer - implemented as a circular queue */
typedef struct struct_tx_buffer {
	pthread_mutex_t lock;
	pthread_cond_t full;
	pthread_cond_t empty;
	int count;
	int first;
	int last;
	PacketDescriptor packets[TX_BUF_SIZE];
}

/*
 * =================================
 * METHOD IMPLEMENTATIONS BEGIN HERE
 */

void init_network_driver(NetworkDevice               nd, 
                         void *                      mem_start, 
                         unsigned long               mem_length,
                         FreePacketDescriptorStore * fpds_ptr) {

}
