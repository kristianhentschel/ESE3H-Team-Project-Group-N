#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "ts_fifo.h"

struct node {
	void *item;
	struct node *next;
};

struct ts_fifo {
	struct node *head;
	struct node *tail;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	//TODO include a function pointer to item destroy function?
};

typedef struct node node;

static void node_destroy(node *node);
static node *node_create(void *item);


/* creates and initialises an empty queue */
ts_fifo *ts_fifo_create(){
	ts_fifo *result;

	if( (result = (ts_fifo *) malloc(sizeof(ts_fifo))) == NULL)
		return NULL;


	result->head = NULL;
	result->tail = NULL;
	result->mutex = PTHREAD_MUTEX_INITIALIZER;
	result->cond = PTHREAD_COND_INITIALIZER;

	return result;
}

/* 
 * removes an item from the head of the queue.
 * blocks if the queue is empty, until an item becomes available.
 */
void *ts_fifo_remove( ts_fifo *q ){
	void *result;
	node *n;

	fprintf(stderr, "ts_fifo_remove()\n");

	pthread_mutex_lock(&q->mutex);
	while( q->head == NULL ){
		pthread_cond_wait(&q->cond, &q->mutex);
	}
	
	//an item is available now
	n = q->head;
	q->head = n->next;
	if (n->next == NULL) {
		q->tail = NULL;
	}
	result = n->item;
	node_destroy(n);	
	
	//done, unlock
	pthread_mutex_unlock(&q->mutex);
	return result;
}

/*
 * adds an item to the tail of the queue
 * blocks until the lock is freed.
 */
int ts_fifo_add( ts_fifo *q, void *item ){
	node *n;

	fprintf(stderr, "ts_fifo_add()\n");
	
	pthread_mutex_lock(&q->mutex);

	if( !(n = node_create(item)) ) {
		pthread_mutex_unlock(&q->mutex);
		
		fprintf(stderr, "can't add item to fifo queue\n");
		return 0;
	}

	if (q->tail == NULL) {
		q->head = q->tail = n;
	} else {
		q->tail->next = n;
		q->tail = n;
		if (q->head == NULL) {
			q->head = n;
		}
	}

	pthread_mutex_unlock(&q->mutex);
	pthread_cond_broadcast(&q->cond);
	return 1;
}

/*
 * destroys the queue and all nodes still in it.
 * does not free items themselves, only the pointers to them.
 * User should ensure queue is emptied before it is being destroyed.
 */
void ts_fifo_destroy( ts_fifo *q ){
	node *n;
	
	fprintf(stderr, "ts_fifo_destroy()\n");

	pthread_mutex_lock(&q->mutex);

	while((n = q->head) != NULL) {
		q->head = n->next;
		node_destroy(n);
	}

	pthread_mutex_unlock(&q->mutex);
	pthread_mutex_destroy(&q->mutex);
	free(q);
}

/*
 * creates a node for our queue.
 */
static node *node_create(void *item) {
	node *n;

	if((n = (node *) malloc(sizeof(node))) == NULL) 
		return NULL;

	n->next = NULL;
	n->item = item;

	return n;
}

/*
 * frees a node structure (not freeing the item it contains yet!)
 */

static void node_destroy(node *n) {
	free(n);
}
