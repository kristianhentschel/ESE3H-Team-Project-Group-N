#include "threadpool.h"
#include <assert.h>
#define TP_BUF_SIZE 10
struct tp {
	unsigned nthreads;
	unsigned buffer_size;

	void (*worker)(void *);
	void (*free_item)(void *);

	pthread_mutex_t lock;

	pthread_cond_t nonfull;
	pthread_cond_t nonempty;

	pthread_t *threads;
	void **buffer;
	int selfdestruct;

	unsigned buffer_count;
	unsigned buffer_start;
};

static void *tp_worker(void *arg);
void *tp_take(TP tp);

TP tp_init(unsigned nthreads, void (*free_item)(void*), void (*worker)(void*)) {
	TP tp;
	unsigned i;
	unsigned buffer_size = TP_BUF_SIZE;
	
	if( !(tp = malloc(sizeof(struct tp))) ) {
		return NULL;
	}

	pthread_mutex_init(&tp->lock, NULL);
	pthread_mutex_lock(&tp->lock);

	pthread_cond_init(&tp->nonfull, NULL);
	pthread_cond_init(&tp->nonempty, NULL); 

	tp->nthreads		= nthreads;
	tp->buffer_size	= buffer_size;
	tp->free_item		= free_item;
	tp->worker 		= worker;

	tp->buffer_count = 0;
	tp->buffer_start = 0;
	tp->selfdestruct = 0;
	
	if( !(tp->threads = malloc(nthreads * sizeof(pthread_t)))
			|| !(tp->buffer = malloc(buffer_size * sizeof(void *)))) {
		free(tp->threads);
		free(tp->buffer);
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
	pthread_cond_broadcast(&tp->nonempty);
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
	pthread_cond_destroy(&tp->nonfull);
	pthread_cond_destroy(&tp->nonempty);
	
	free(tp->threads);
	free(tp->buffer);
	free(tp);
}

void tp_dispatch(TP tp, void *work) {
	pthread_mutex_lock(&tp->lock);
	
	while (tp->buffer_count == tp->buffer_size && !tp->selfdestruct) {
		pthread_cond_wait(&tp->nonfull, &tp->lock);
	}

	if (tp->selfdestruct) {
		tp->free_item(work);
		pthread_mutex_unlock(&tp->lock);
		return;
	} else {
		tp->buffer[(tp->buffer_start + tp->buffer_count) % tp->buffer_size] = work;
		tp->buffer_count++;
		pthread_cond_signal(&tp->nonempty);
		pthread_mutex_unlock(&tp->lock);
		return;
	}
}

void *tp_take(TP tp) {
	void *item;

	pthread_mutex_lock(&tp->lock);
	
	while (tp->buffer_count == 0 && !tp->selfdestruct) {
		pthread_cond_wait(&tp->nonempty, &tp->lock);
	}

	if (tp->selfdestruct) {
		pthread_mutex_unlock(&tp->lock);
		return NULL;
	} else {
		assert(tp->buffer_count > 0);
		item = tp->buffer[tp->buffer_start];
		tp->buffer_start = (tp->buffer_start + 1) % tp->buffer_size;
		tp->buffer_count--;
		pthread_cond_signal(&tp->nonfull);
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
		if((item = tp_take(tp)) != NULL) {
			tp->worker(item);
			tp->free_item(item);
		} else {
			break;
		}
	}

	return NULL;
}
