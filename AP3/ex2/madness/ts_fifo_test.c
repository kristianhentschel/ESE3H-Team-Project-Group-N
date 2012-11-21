#include <stdio.h>
#include <pthread.h>
#include "ts_fifo.h"

int main(int argc, char *argv[]){
	ts_fifo *q = ts_fifo_create();

	ts_fifo_add(q, "Hello");

	ts_fifo_add(q, "World");

	printf( "%s\n", (char *) ts_fifo_remove(q));
	printf( "%s\n", (char *) ts_fifo_remove(q));

	ts_fifo_destroy(q);

	/* TODO more comprehensive testing with threads needed. */

	return 0;
}
