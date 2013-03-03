#ifndef __BUFFERTHREADPOOL_H__ 
#define __BUFFERTHREADPOOL_H__

#include <pthread.h>
#include <stdlib.h>

typedef struct tp *TP;

/* initialise the threadpool.
 * Create nthreads, and start them using the worker method.
 * the worker method will be wrapped in a run method that repeatedly calls it. A single work item is passed to the run method.
 * free_item(void *) is called on each item after the worker is finished with it.
 * initialise a buffer of work items of capacity buffer_size.*/
TP tp_init(unsigned nthreads,
		unsigned buffer_size,
		void (*free_item)(void *),
		void (*worker)(void *));

/* set killflag, wait until all threads have shutdown, destroy the structures */
void tp_destroy(TP tp);

/* add a work item to the buffer. blocks until space is available if the buffer is full. */
void tp_dispatch(TP tp, void *work);

#endif /* __BUFFERTHREADPOOL_H__ */
