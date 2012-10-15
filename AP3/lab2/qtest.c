#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

#define PRIODIF (MIN_PRIO - MAX_PRIO + 1)
#define MAXLINE 1024

/*
 * this program reads a list of integers from the standard input, one per line
 * it adds each integer to a queue until it sees EOF
 * it then removes each item from the queue, printing out the integer on
 * a line by itself on standard output
 */
int main()
{
	Queue *q;
	char buf[MAXLINE];
	int *i, prio, mod;

	if ((q = q_create()) == NULL) {
		fprintf(stderr, "Unable to create queue of integers\n");
		return -1;
	}
	while (fgets(buf, MAXLINE, stdin) != NULL) {
		i = (int *)malloc(sizeof(int));
		if (! i) {
			fprintf(stderr, "Unable to allocate int on heap\n");
			return -2;
		}
		*i = atoi(buf);
		mod = (*i - MAX_PRIO) % PRIODIF;
		if (mod < 0)
			mod = -mod;
		prio = MAX_PRIO + mod;
		if (! (q_add(q, i, prio))) {
			fprintf(stderr, "Unable to add %d to the Queue\n", *i);
			return -2;
		}
	}
	while ((i = (int *)q_remove(q))) {
		printf("%d\n", *i);
		free(i);
	}

	free(q);
	return 0;
}
