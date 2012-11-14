#include <stdio.h>
#include "isprime.h"
#include <sys/time.h>
int main(int argc, char *argv[]) {
	unsigned long i, limit;
	unsigned long count;
	unsigned long msec;
	double msperprime;
	struct timeval start, stop;
	if (argc == 1)
		limit = 100;
	else if (argc > 2) {
		fprintf(stderr, "usage: ./primes [limit]\n");
		return -1;
	} else
		sscanf(argv[1], "%lu", &limit);
	count = 0;
	gettimeofday(&start, NULL); /* note start time */
	for (i = 1; count < limit; i++) {
		if (is_prime(i)) {
			printf("%lu\n", i);
			count++;
		}
	}
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
