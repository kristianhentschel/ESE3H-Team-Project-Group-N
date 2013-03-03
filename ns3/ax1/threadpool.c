#include "threadpool.h"
#include <stdio.h>
#include <assert.h>

struct tp {
	unsigned nthreads;
	unsigned busy;
	int selfdestruct;

	void *nextwork;

	void (*worker)(void *);
	void (*free_item)(void *);

	pthread_mutex_t lock;

	pthread_cond_t idlethreads;
	pthread_cond_t workavailable;

	pthread_t *threads;
};


static void *tp_take(TP tp); 
static void *tp_worker(void *arg);

TP tp_init(unsigned nthreads, void (*free_item)(void*), void (*worker)(void*)) {
	TP tp;
	unsigned i;
	
	if( !(tp = malloc(sizeof(struct tp))) ) {
		return NULL;
	}

	pthread_mutex_init(&tp->lock, NULL);
	pthread_mutex_lock(&tp->lock);

	pthread_cond_init(&tp->idlethreads, NULL); 
	pthread_cond_init(&tp->workavailable, NULL); 

	tp->nthreads	= nthreads;
	tp->free_item	= free_item;
	tp->worker 		= worker;

	tp->busy = 0;
	tp->selfdestruct = 0;
	
	if( !(tp->threads = malloc(nthreads * sizeof(pthread_t)))) {
		free(tp->threads);
		pthread_mutex_unlock(&tp->lock);
		pthread_mutex_destroy(&tp->lock);
		free(tp);
		return NULL;
	}

	for (i = 0; i < nthreads; i++) {
		pthread_create(&tp->threads[i], NULL, tp_worker, (void *) tp);
	}

	pthread_mutex_unlock(&tp->lock);
	return tp;
}

void tp_destroy(TP tp) {
	unsigned i;
	void *item;

	/* set selfdestruct flag and let all workers continue */
	pthread_mutex_lock(&tp->lock);
	tp->selfdestruct = 1;
	pthread_cond_broadcast(&tp->idlethreads);
	pthread_mutex_unlock(&tp->lock);
	
	/* wait for all threads to die gracefully - ie after finishing the current request. */
	for (i = 0; i < tp->nthreads; i++) {
		pthread_join(tp->threads[i], NULL);
	}
	
	/* destroy all items left in buffer */
	while ((item = tp_take(tp)) != NULL) {
		tp->free_item(item);
	}

	/* destroy wtb data structure and locks */
	pthread_mutex_lock(&tp->lock);
	pthread_mutex_destroy(&tp->lock);

	/* TODO destroy new cond variables */
	free(tp->threads);
	free(tp);
}

void tp_dispatch(TP tp, void *work) {
	pthread_mutex_lock(&tp->lock);
	
	printf("DISPATCH: wait for idle before adding work.\n");
	while (tp->busy == tp->nthreads && !tp->selfdestruct) {
		pthread_cond_wait(&tp->idlethreads, &tp->lock);
	}

	if (tp->selfdestruct) {
		tp->free_item(work);
	} else {
		tp->nextwork = work;
		pthread_cond_signal(&tp->workavailable);
	}

	printf("DISPATCH: wait for idle after adding work.\n");
	while ((tp->nextwork != NULL || tp->busy == tp->nthreads) && !tp->selfdestruct) {
		printf("DISPATCH: still waiting for idle after. %u threads are currently busy.\n", tp->busy);
		pthread_cond_wait(&tp->idlethreads, &tp->lock);
	}

	pthread_mutex_unlock(&tp->lock);
	return;
}

static void *tp_take(TP tp) {
	void *item;

	pthread_mutex_lock(&tp->lock);
	
	printf("TAKE: blocking until work is available\n");
	while (tp->nextwork == NULL && !tp->selfdestruct) {
		pthread_cond_wait(&tp->workavailable, &tp->lock);
	}

	if (tp->selfdestruct) {
		pthread_mutex_unlock(&tp->lock);
		return NULL;
	} else {
		item = tp->nextwork;
		tp->nextwork = NULL;
		pthread_mutex_unlock(&tp->lock);
		return item;
	}
}

/* wait until item is available in buffer, take it, and give it to the specified worker method.
 * then free the item using the given free_item method on it.
 */
static void *tp_worker(void *arg) {
	TP tp;
	void *item;

	tp = (TP) arg;
	
	while(1) {
		/* wait for and retrieve work */
		pthread_mutex_lock(&tp->lock);
		
		printf("WORKER: %lu now idle\n", pthread_self() % 1000);
		while (tp->nextwork == NULL && !tp->selfdestruct) {
			pthread_cond_wait(&tp->workavailable, &tp->lock);
		}

		if (tp->selfdestruct) {
			pthread_mutex_unlock(&tp->lock);
			break;
		}

		item = tp->nextwork;
		tp->nextwork = NULL;
		
		printf("WORKER: %lu now busy\n", pthread_self() % 1000);
		tp->busy++;
		pthread_cond_signal(&tp->idlethreads);
		pthread_mutex_unlock(&tp->lock);
		
		/* do actual work */
		tp->worker(item);
		tp->free_item(item);

		/* clean up and mark thread as idle */
		pthread_mutex_lock(&tp->lock);
		assert(tp->busy > 0);
		printf("WORKER: %lu now idle\n", pthread_self() % 1000);
		tp->busy--;
		pthread_cond_signal(&tp->idlethreads);
		pthread_mutex_unlock(&tp->lock);
	}

	return NULL;
}
