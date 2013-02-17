#ifndef __WORKER_THREADPOOL_H__
#define __WORKER_THREADPOOL_H__

#include <pthread.h>
#include <stdlib.h>

typedef struct wtp *WTP;

/* initialise the threadpool.
 * Create nthreads, and start them using the worker method.
 * the worker method will be wrapped in a run method that repeatedly calls it. A single work item is passed to the run method.
 * free_item(void *) is called on each item after the worker is finished with it.
 * initialise a buffer of work items of capacity buffer_size.*/
WTP wtp_init(unsigned nthreads,
		unsigned buffer_size,
		void (*free_item)(void *),
		void (*worker)(void *));

/* set killflag, wait until all threads have shutdown, destroy the structures */
void wtp_destroy(WTP wtp);

/* add a work item to the buffer. blocks until space is available if the buffer is full. */
void wtp_put(WTP wtp, void *work);

/* take a work item out of the buffer. blocks until an item is available or the thread pool is destroyed. */
void *wtp_take(WTP wtp);

#endif /* __WORKER_THREADPOOL_H__ */
