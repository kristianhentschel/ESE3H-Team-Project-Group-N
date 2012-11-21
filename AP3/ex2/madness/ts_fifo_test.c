#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "ts_fifo.h"

#define WORKERS 5

ts_fifo *work_queue;


/* worker takes an item off the (blocking) work queue until the item NULL is received. */
void *worker(void *args) {
	char *item;
	
	fprintf(stderr, "started worker thread %lu\n", (unsigned long) pthread_self()); 

	while((item = (char *) ts_fifo_remove(work_queue)) != NULL){
		printf("Worker %lu received \t%s\n", (unsigned long) pthread_self(), item);
		
		free(item);
	}
	
	fprintf(stderr, "returning from worker thread %lu after NULL item was received.\n", (unsigned long) pthread_self());

	return NULL;

}


int main(int argc, char *argv[]){
	ts_fifo* q = work_queue = ts_fifo_create();
	pthread_t threads[WORKERS];
	int i;

	for (i = 0; i < WORKERS; i++) {
		pthread_create(&threads[i], NULL, worker, NULL);
		//TODO check for errors
	}


	//start filling queue
	for (i = 0; i < 10; i++){
		ts_fifo_add(q, strdup("Hello"));
		ts_fifo_add(q, strdup("World"));
	}
	//place suicide commands
	for (i = 0; i < WORKERS; i++) {
		ts_fifo_add(work_queue, NULL);
	}

	//wait for workers to complete
	for (i = 0; i < WORKERS; i++) {
		pthread_join(threads[i], NULL);
	}
	

	//cleanup
	ts_fifo_destroy(q);

	return 0;
}
