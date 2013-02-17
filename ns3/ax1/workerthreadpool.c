#include "workerthreadpool.h"
#include <assert.h>

struct wtp {
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

static void *wtp_worker(void *arg);

WTP wtp_init(unsigned nthreads, unsigned buffer_size, void (*free_item)(void*), void (*worker)(void*)) {
	WTP wtp;
	unsigned i;
	
	if( !(wtp = malloc(sizeof(struct wtp))) ) {
		return NULL;
	}

	pthread_mutex_init(&wtp->lock, NULL);
	pthread_mutex_lock(&wtp->lock);

	pthread_cond_init(&wtp->nonfull, NULL);
	pthread_cond_init(&wtp->nonempty, NULL); 

	wtp->nthreads		= nthreads;
	wtp->buffer_size	= buffer_size;
	wtp->free_item		= free_item;
	wtp->worker 		= worker;

	wtp->buffer_count = 0;
	wtp->buffer_start = 0;
	wtp->selfdestruct = 0;
	
	if( !(wtp->threads = malloc(nthreads * sizeof(pthread_t)))
			|| !(wtp->buffer = malloc(buffer_size * sizeof(void *)))) {
		free(wtp->threads);
		free(wtp->buffer);
		pthread_mutex_unlock(&wtp->lock);
		pthread_mutex_destroy(&wtp->lock);
		free(wtp);
		return NULL;
	}

	for (i = 0; i < nthreads; i++) {
		pthread_create(&wtp->threads[i], NULL, wtp_worker, (void *) wtp);
	}

	pthread_mutex_unlock(&wtp->lock);
	return wtp;
}

void wtp_destroy(WTP wtp) {
	unsigned i;
	void *item;

	/* set selfdestruct flag and let all workers continue */
	pthread_mutex_lock(&wtp->lock);
	wtp->selfdestruct = 1;
	pthread_cond_broadcast(&wtp->nonempty);
	pthread_mutex_unlock(&wtp->lock);
	
	/* wait for all threads to die gracefully - ie after finishing the current request. */
	for (i = 0; i < wtp->nthreads; i++) {
		pthread_join(wtp->threads[i], NULL);
	}
	
	/* destroy all items left in buffer */
	while ((item = wtp_take(wtp)) != NULL) {
		wtp->free_item(item);
	}

	/* destroy wtb data structure and locks */
	pthread_mutex_lock(&wtp->lock);
	pthread_mutex_destroy(&wtp->lock);
	pthread_cond_destroy(&wtp->nonfull);
	pthread_cond_destroy(&wtp->nonempty);
	
	free(wtp->threads);
	free(wtp->buffer);
	free(wtp);
}

void wtp_put(WTP wtp, void *work) {
	pthread_mutex_lock(&wtp->lock);
	
	while (wtp->buffer_count == wtp->buffer_size && !wtp->selfdestruct) {
		pthread_cond_wait(&wtp->nonfull, &wtp->lock);
	}

	if (wtp->selfdestruct) {
		wtp->free_item(work);
		pthread_mutex_unlock(&wtp->lock);
		return;
	} else {
		wtp->buffer[(wtp->buffer_start + wtp->buffer_count) % wtp->buffer_size] = work;
		wtp->buffer_count++;
		pthread_cond_signal(&wtp->nonempty);
		pthread_mutex_unlock(&wtp->lock);
		return;
	}
}

void *wtp_take(WTP wtp) {
	void *item;

	pthread_mutex_lock(&wtp->lock);
	
	while (wtp->buffer_count == 0 && !wtp->selfdestruct) {
		pthread_cond_wait(&wtp->nonempty, &wtp->lock);
	}

	if (wtp->selfdestruct) {
		pthread_mutex_unlock(&wtp->lock);
		return NULL;
	} else {
		assert(wtp->buffer_count > 0);
		item = wtp->buffer[wtp->buffer_start];
		wtp->buffer_start = (wtp->buffer_start + 1) % wtp->buffer_size;
		wtp->buffer_count--;
		pthread_cond_signal(&wtp->nonfull);
		pthread_mutex_unlock(&wtp->lock);
		return item;
	}
}

/* wait until item is available in buffer, take it, and give it to the specified worker method.
 * then free the item using the given free_item method on it.
 */
static void *wtp_worker(void *arg) {
	WTP wtp;
	void *item;

	wtp = (WTP) arg;
	
	while(1) {
		if((item = wtp_take(wtp)) != NULL) {
			wtp->worker(item);
			wtp->free_item(item);
		} else {
			break;
		}
	}

	return NULL;
}
