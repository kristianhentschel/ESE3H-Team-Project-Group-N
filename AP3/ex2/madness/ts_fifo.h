#ifndef _TS_FIFO_H_
#define _TS_FIFO_H_

/* public interface to thread safe fifo queue */
typedef struct ts_fifo ts_fifo;

/* creates and initialises an empty queue */
ts_fifo* ts_fifo_create();

/* 
 * removes an item from the head of the queue.
 * blocks if the queue is empty, until an item becomes available.
 */
void *ts_fifo_remove( ts_fifo *q );

/*
 * adds an item to the tail of the queue
 * blocks until the lock is freed.
 */
int ts_fifo_add( ts_fifo *q, void *item );

/*
 * destroys the queue and all nodes and items remaining in the queue.
 */
void ts_fifo_destroy( ts_fifo *q );

#endif /* _TS_FIFO_H_ */
