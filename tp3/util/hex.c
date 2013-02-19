#include <stdio.h>
#include <unistd.h>

int main(void) {
	FILE *serial;
	int c;

	serial = fopen("/dev/ttyAMA0", "r");


	while ((c = fgetc(serial)) != EOF) {
		if (c == 0x7e) {
			printf("\n");
		}
		printf("%02x ", (char) c);
	}

	fclose(serial);
	return 0;
}

