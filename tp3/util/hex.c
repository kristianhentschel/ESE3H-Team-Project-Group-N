#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

int main(void) {
	char c;
	int serial;

	serial = open("/dev/ttyAMA0", O_RDWR | O_NOCTTY | O_NDELAY);

	if (serial < 0) {
		perror("could not open serial port");
	}

   	/* want blocking reads. otherwise set FNDELAY */
	if( fcntl(serial, F_SETFL, 0) == -1 ) {
		perror("could not change serial port file settings");
	}

	if( write(serial, "+++", 3) == -1 ) {
		perror("could not write +++");
	}

	while ( read(serial, &c, 1) > 0 ){
		if (c == 0x7e) {
			printf("\n");
		}
		printf("%02x ", c);
	}

	close(serial);
	return 0;
}

