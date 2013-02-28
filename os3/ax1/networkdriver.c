#include "BoundedBuffer.h"
#include <string.h>
#include "freepacketdescriptorstore__full.h"
#include "packetdescriptorcreator.h"
#include "networkdriver.h"
#include "diagnostics.h"
#include <assert.h>
#include <pthread.h>

#define APP_COUNT MAX_PID + 1
#define RX_BUF_SIZE 1
#define TX_BUF_SIZE 10
#define MAX_RETRIES 2

/* =====================================
 * DATA TYPES & STATIC METHOD SIGNATURES 
 */

/* thread to await packets from the network and store them in the corresponding rx buffer. */
void *thread_packet_receiver(void *arg);

/* thread to take packets from the transmit queue and send them to the network device */
void *thread_packet_transmitter(void *arg);

/* Receive and Transmit Buffers */
typedef BoundedBuffer RXBuffer;
typedef BoundedBuffer TXBuffer;


/* ================
 * GLOBAL VARIABLES
 */

static FreePacketDescriptorStore FPDS;
static TXBuffer TX;
static RXBuffer RX[APP_COUNT];

/* ======================
 * METHOD IMPLEMENTATIONS
		pthread_cond_signal(&RX.available);
 */

void init_network_driver(NetworkDevice               nd, 
                         void *                      mem_start, 
                         unsigned long               mem_length,
                         FreePacketDescriptorStore * fpds_ptr) {
	int i;
	pthread_t worker_rx, worker_tx;

	/*
	 * Initialise all receive and transmit buffers
	 */
	if ((TX = createBB( TX_BUF_SIZE )) == NULL) {
		DIAGNOSTICS("could not create bounded buffer for TX buffer.\n");
	}

	for (i = 0; i < APP_COUNT; i++) {
		if ((TX = createBB( RX_BUF_SIZE )) == NULL) {
			DIAGNOSTICS("could not create bounded buffer for RX (%d) buffer.\n", i);
		}
	}

	/*
	 * initialize packet decriptor store from mem_start and mem_length
	 */
	FPDS = create_fpds();
	create_free_packet_descriptors(FPDS, mem_start, mem_length);
	*fpds_ptr = FPDS;
	/* initialise threads (receiver and transmitter) */
	pthread_create(&worker_rx, NULL, thread_packet_receiver, (void *) nd);
	pthread_create(&worker_tx, NULL, thread_packet_transmitter, (void *) nd);
}


void *thread_packet_receiver(void *arg) {
	PID pid;
	PacketDescriptor *pd, bufpd;
	NetworkDevice nd;
	int status;

	nd = (NetworkDevice) arg;

	while(1) {
		/* use our own static packet descriptor here. */
		init_packet_descriptor(&bufpd);
		register_receiving_packetdescriptor(nd, &bufpd);
		
		/* blocks until packet received from network. */
		await_incoming_packet(nd);
		
		/* copy the packet into a packet descriptor taken from the free packet descriptor store */
		if (nonblocking_get_pd(FPDS, pd)) {
			DIAGNOSTICS("Receive: Dropped packet as Free Packet Descriptor was empty.");
			continue;
		} else {
			memcpy(pd, &bufpd, sizeof(bufpd));
		}

		/* attempt to put the packet descriptor into the process's rx buffer */
		pid = packet_descriptor_get_pid(pd);
		status = nonblockingWriteBB(RX[pid], pd);
		if (!status) {
			DIAGNOSTICS("Receive: Dropped packet as RX buffer for PID %d is full.\n", pid);
			continue;
		}

	}

	return NULL;
}


void *thread_packet_transmitter(void *arg) {
	PacketDescriptor pd;
	int tx_status = 0;
	int retries = 0;
	NetworkDevice nd;

	nd = (NetworkDevice) arg;

	while (1) {
		pd = blockingReadBB(TX);
		retries = MAX_RETRIES;
		while (retries > 0) {
			tx_status = send_packet(nd, pd);
			retries--;
			
			DIAGNOSTICS("Transmit: Attempted to send. tx_status: %d, retries: %d", tx_status, retries);
			if (tx_status == 1) {
				break;
			}
			
			if (retries == 0) {
				DIAGNOSTICS("Transmit: Packet dropped after retries exceeded.");
			}
		}
		
	}

	return NULL;
}

void blocking_send_packet(PacketDescriptor pd) {
	blockingWriteBB(TX, pd);
}

int nonblocking_send_packet(PacketDescriptor pd) {
	return nonblockingWriteBB(TX, pd);
}

void blocking_get_packet(PacketDescriptor *pd, PID pid) {
	assert(pid <= APP_COUNT);
	*pd = blockingReadBB(RX[pid]);
}

int nonblocking_get_packet(PacketDescriptor *pd, PID pid) {
	assert(pid <= APP_COUNT);
	return nonblockingReadBB(RX[pid], pd);
}
