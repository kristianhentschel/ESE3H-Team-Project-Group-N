#include "zb_transport.h"
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <pthread.h>

#define RECEIVE_BUFFER_SIZE 256
#define SERIAL_DEVICE "/dev/ttyAMA0"
#define SERIAL_BAUD_RATE 9600

/*
 * zb_transport_tty.h
 *
 * Seriel communication interface for tty device on the Raspberry Pi.
 *
 * Realised using pthreads for the receive buffer with a separate thread monitoring
 * the serial device.
 *
 *
 * Author: Kristian Hentschel
 * Team Project 3. University of Glasgow. 2013
 */

/* worker method. argument is a file descriptor for the serial interface */
static void *serial_monitor(void *arg);



typedef struct buffer {
	pthread_mutex_t lock;
	pthread_cond_t	nonfull;
	pthread_cond_t	nonempty;
	int size;
	int last;
	int first;
	char elements[RECEIVE_BUFFER_SIZE];
} Buffer;

/* global variable to hold the receiver thread so it can be shut down later. */
static pthread_t pthread_receiver;



/* open and setup the serial device.
 * initialise the buffer structure, locks, and condition variables.
 * start the monitoring thread
 */
void zb_transport_init() {
	struct termios tc;
	int fd;

	/* open serial port */
	fd = open("/dev/ttyAMA0", O_RDWR | O_NOCTTY | O_NDELAY);
	fcntl(fd, F_SETFL, 0);
	
	/* set tc options for serial port transfers */
	tcgetattr(fd, &tc);

	cfsetospeed(&tc, 9600);
	cfsetispeed(&tc, 9600);
	tc.c_cflag |= (CLOCAL | CREAD);
	tc.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	tcsetattr(fd, TCSANOW, &tc);

	/* set up buffer structures and locks */
	/* TODO */

	/* start receiving thread to fill the buffer */	
	pthread_create(&pthread_receiver, NULL, serial_monitor, (void *) fd);
}
