#include <stdio.h>
#include "isprime.h"
#include <sys/time.h>
#include <string.h>

int main(int argc, char *argv[]) {
	/* Time measurement */
	unsigned long msec;
	double msperprime;
	struct timeval start, stop;
	
	/* Arguments */
	int i, j, nthread;
	unsigned long block, limit;
	
	
	/* Parse Arguments */
	block = 1;
	limit = 100;
	nthread = 1;
	for (i = 1; i < argc; ) {
		if ((j = i + 1) == argc) {
			fprintf(stderr, "usage: ./mtprimes [-b block] [-l limit] [-t nthread]\n", USAGE);
			return -1;
		}
		if (strcmp(argv[i], "-b") == 0)
			sscanf(argv[j], "%lu", &block);
		else if (strcmp(argv[i], "-l") == 0)
			sscanf(argv[j], "%lu", &limit);
		else if (strcmp(argv[i], "-t") == 0)
			sscanf(argv[j], "%d", &nthread);
		else {
			fprintf(stderr, "Unknown flag: %s %s\n", argv[i], argv[j]);
			return -1;
		}
		i = j + 1;
	}
	
	/* Temporary: Print Arguments */
	fprintf(stderr, "Looking for %lu primes, using block size %lu, with %d threads.\n", limit, block, nthread);


	/* Note Start Time */
	gettimeofday(&start, NULL);


	/* Compute Time Usage */
	gettimeofday(&stop, NULL);
	if (stop.tv_usec < start.tv_usec) {
		stop.tv_usec += 1000000;
		stop.tv_sec--;
	}
	msec = 1000 * (stop.tv_sec - start.tv_sec) +
		(stop.tv_usec - start.tv_usec) / 1000;
	msperprime= (double) msec / (double) limit;
	fprintf(stderr, "%lu primes computed in %lu.%03lu seconds, %.3f ms/prime\n",
			limit, msec/1000, msec%1000, msperprime);
	return 0;
}
