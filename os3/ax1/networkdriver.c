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
		if ((RX[i] = createBB( RX_BUF_SIZE )) == NULL) {
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
	PacketDescriptor pd, bufpd;
	NetworkDevice nd;

	/* TODO this might block (deadlock?) if applications have acquired all the free packet descriptors from the store
	 * before this gets scheduled. Only really deadlocks if network device doesn't serve send requests before reads.
	 * might solve this by setting one aside and handing it to this thread before returning the fpds_ptr in init().
	 * */
	DIAGNOSTICS("Receive: Initialised thread, waiting to get buffer packet descriptor.\n");
	blocking_get_pd(FPDS, &bufpd);

	nd = (NetworkDevice) arg;

	while(1) {
		/* use the previously acquired buffer */
		init_packet_descriptor(&bufpd);
		register_receiving_packetdescriptor(nd, &bufpd);
		
		/* blocks until packet received from network. */
		DIAGNOSTICS("Receive: Waiting for network packet.\n");
		await_incoming_packet(nd);
	
		/* copy the packet into a new packet descriptor taken from the free packet descriptor store */
		pd = bufpd;
		if (nonblocking_get_pd(FPDS, &bufpd)) {
			DIAGNOSTICS("Receive: Dropped packet as Free PD Store was empty.\n");
			continue;
		} 

		/* attempt to put the packet descriptor into the process's rx buffer */
		pid = packet_descriptor_get_pid(&pd);
		if (!nonblockingWriteBB(RX[pid], pd)) {
			DIAGNOSTICS("Receive: Dropped packet as RX buffer for PID %d is full.\n", pid);
			continue;
		} else {
			DIAGNOSTICS("Receive: Got packet and successfully stored it in RX buffer for PID %d.\n", pid);
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

	DIAGNOSTICS("Transmit: Thread initialised.\n");
	while (1) {
		pd = blockingReadBB(TX);
		retries = MAX_RETRIES;
		while (retries > 0) {
			tx_status = send_packet(nd, pd);
			retries--;
			
			DIAGNOSTICS("Transmit: Attempted to send. tx_status: %d, retries: %d\n", tx_status, retries);
			if (tx_status == 1) {
				break;
			}
			
			if (retries == 0) {
				DIAGNOSTICS("Transmit: Packet dropped after retries exceeded.\n");
			}
		}
		
	}

	return NULL;
}

void blocking_send_packet(PacketDescriptor pd) {
	DIAGNOSTICS("Send: Blocking\n");
	blockingWriteBB(TX, pd);
}

int nonblocking_send_packet(PacketDescriptor pd) {
	DIAGNOSTICS("Send: Non-Blocking\n");
	return nonblockingWriteBB(TX, pd);
}

void blocking_get_packet(PacketDescriptor *pd, PID pid) {
	DIAGNOSTICS("Get: Blocking\n");
	assert(pid <= APP_COUNT);
	*pd = blockingReadBB(RX[pid]);
}

int nonblocking_get_packet(PacketDescriptor *pd, PID pid) {
	DIAGNOSTICS("Get: Non-Blocking\n");
	assert(pid <= APP_COUNT);
	return nonblockingReadBB(RX[pid], pd);
}
