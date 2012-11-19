#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

struct node {
	void *item;
	struct node *next;
};

struct queue {
	struct node *head;
	struct node *tail;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	//TODO include a function pointer to item destroy function?
};

typedef struct queue *ts_fifo_queue;
typedef ts_fifo_queue ts_fifo;
typedef struct node *ts_fifo_node;

static void ts_fifo_node_destroy(ts_fifo_node node);
static ts_fifo_node ts_fifo_node_create(void *item);


/* creates and initialises an empty queue */
ts_fifo ts_fifo_create(){
	ts_fifo_queue result;

	if( (result = (ts_fifo_queue) malloc(sizeof(struct queue))) == NULL)
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
void *ts_fifo_dequeue( ts_fifo q ){
	void *result;
	ts_fifo_node node;
	
	//TODO lock and wait
	pthread_mutex_lock(&q->mutex);
	while( q->head == NULL ){
		pthread_cond_wait(&q->cond, &q->mutex);
	}
	
	//an item is available now
	node = q->head;
	q->head = node->next;
	if (node->next == NULL) {
		q->tail = NULL;
	}
	result = node->item;
	ts_fifo_node_destroy(node);	
	
	//done, unlock
	pthread_mutex_unlock(&q->mutex);

	return result;
}

/*
 * adds an item to the tail of the queue
 * blocks until the lock is freed.
 */
void ts_fifo_enqueue( ts_fifo q, void *item ){
	ts_fifo_node node;
	//TODO lock and wait?

	if( !(node = ts_fifo_node_create(item)) ) {
		pthread_mutex_unlock(&q->mutex);
		
		fprintf(stderr, "can't add item to fifo queue");
		return;
	}

	if (q->tail == NULL) {
		q->head = q->tail = node;
	} else {
		q->tail->next = node;
		q->tail = node;
		if (q->head == NULL) {
			q->head = node;
		}
	}

	pthread_mutex_unlock(&q->mutex);
	pthread_cond_broadcast(&q->cond);
}

/*
 * destroys the queue and all nodes still in it.
 * TODO add function pointer to destroy function for items.
 * current implementation does not free items themselves, only the pointers to them.
 */
void ts_fifo_destroy( ts_fifo q ){
	ts_fifo_node node;
	
	pthread_mutex_lock(&q->mutex);

	while((node = (ts_fifo_node) q->head) != NULL) {
		q->head = node->next;
		ts_fifo_node_destroy(node);
	}

	pthread_mutex_unlock(&q->mutex);
	pthread_mutex_destroy(&q->mutex);
	free(q);
}

/*
 * creates a node for our queue.
 */
static ts_fifo_node ts_fifo_node_create(void *item) {
	ts_fifo_node node;

	if((node = (ts_fifo_node) malloc(sizeof(struct node))) == NULL) 
		return NULL;

	node->next = NULL;
	node->item = item;

	return node;
}

/*
 * frees a node structure (not freeing the item it contains yet!)
 */

static void ts_fifo_node_destroy(ts_fifo_node node) {
	free(node);
}
