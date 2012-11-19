#include <stdio.h>
#include <pthread.h>
#include "ts_fifo.h"

int main(int argc, char *argv[]){
	ts_fifo q = ts_fifo_create();

	ts_fifo_enqueue(q, "Hello");

	ts_fifo_enqueue(q, "World");

	printf( "%s\n", (char *) ts_fifo_dequeue(q));
	printf( "%s\n", (char *) ts_fifo_dequeue(q));

	ts_fifo_destroy(q);

	/* TODO more comprehensive testing with threads needed. */

	return 0;
}
