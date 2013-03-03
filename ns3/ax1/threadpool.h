/*
 * Author: Kristian Hentschel
 * Matric: 1003734h
 * Submission: Networked Systems 3 Assessed Exercise 1
 *
 * This file is my own work.
 */

#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include <pthread.h>
#include <stdlib.h>

typedef struct tp *TP;

/* initialise the threadpool.
 * Create nthreads, and start them using the worker method.
 * the worker method will be wrapped in a run method that repeatedly calls it. A single work item is passed to the run method.
 * free_item(void *) is called on each item after the worker is finished with it.
 * initialise a buffer of work items of capacity buffer_size.*/
TP tp_init(unsigned nthreads,
		void (*free_item)(void *),
		void (*worker)(void *));

/* set killflag, wait until all threads have shutdown, destroy the structures */
void tp_destroy(TP tp);

/* give work item to the next free worker. block until another work item can be accepted. */
void tp_dispatch(TP tp, void *work);

#endif /* __THREADPOOL_H__ */
