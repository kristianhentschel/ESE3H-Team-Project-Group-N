#include "workerthreadpool.h"

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
	bool selfdescruct;

	unsigned buffer_count;
	unsigned buffer_start;
	
};

WTP wtp_init(unsigned nthreads, unsigned buffer_size, void (*free_item)(void*), void (*worker)(void*)) {
	WTP wtp;
	
	if( wtp = malloc(sizeof(struct wtp)) ) {
		return NULL;
	}

	wtp->lock = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_lock(wtp->lock);

	pthread_cond_init(&wtp->nonfull, NULL);
	pthread_cond_init(&wtp->nonempty, NULL); 

	wtp->nthreads		= nthreads;
	wtp->buffer_size	= buffer_size;
	wtp->free_item		= free_item;
	wtp->worker 		= worker;

	wtp->buffer_count = 0;
	wtp->buffer_start = 0;
	wtp->selfdestruct = 0;
	
	if(wtp->threads = malloc(nthreads * sizeof(pthread_t))
			|| wtp->buffer = malloc(buffer_size * sizeof(void *))) {
		free(wtp->threads);
		free(wtp->buffer);
		pthread_mutex_destroy(wtp->lock);
		free(wtp);
		return NULL;
	}

	pthread_mutex_unlock(wtp->lock);
	return wtp;
}

void wtp_destroy(WTP wtp, void *work) {
	unsigned i;
	void *item;

	/* set selfdestruct flag and let all workers continue */
	pthread_mutex_lock(wtp->lock);
	wtp->selfdestruct = 1;
	pthread_broadcast(wtp->nonempty);
	pthread_mutex_unlock(wtp->lock);
	
	/* wait for all threads to die gracefully - ie after finishing the current request. */
	for (i = 0; i < nthreads; i++) {
		pthread_join(threads[i]);
		pthread_destroy(threads[i]);
	}
	
	/* destroy all items left in buffer */
	while ((item = wtp_take(wtb)) != NULL) {
		wtb->free_item(item);
	}

	/* destroy wtb data structure and locks */
	pthread_mutex_lock(wtp->lock);
	pthread_mutex_destroy(wtp->lock);
	pthread_cond_destroy(wtp->nonfull);
	pthread_cond_destroy(wtp->nonempty);
	
	free(wtp->threads);
	free(wtp->buffer);
	free(wtp);
}

void wtp_put(WTP wtp, void *work) {
	pthread_mutex_lock(wtp->lock);
	
	while (wtp->buffer_count == buffer_size && !wtp>selfdestruct) {
		wait(wtp->nonfull);
	}

	if (wtp->selfdestruct) {
		wtp->free_item(work);
		pthread_mutex_unlock(wtp->lock);
		return;
	} else {
		wtp->buffer[(wtp->buffer_start + wtp->buffer_count) % wtp->buffer_size] = work;
		wtp->buffer_count++;
		pthread_signal(wtp->nonempty);
		pthread_mutex_unlock(wtp->lock);
		return;
	}
}

void *wtp_take(WTP wtp) {
	void *item;

	pthread_mutex_lock(wtp->lock);
	
	while (wtp->buffer_count == 0 && !wtp>selfdestruct) {
		wait(wtp->nonempty);
	}

	if (wtp->selfdestruct) {
		pthread_mutex_unlock(wtp->lock);
		return NULL;
	} else {
		assert(wtp->buffer_count > 0);
		item = wtp->buffer[wtp->buffer_start];
		wtp->buffer_start = (wtp->buffer_start + 1) % wtp->buffer_size;
		wtp->buffer_count--;
		pthread_signal(wtp->nonfull);
		pthread_mutex_unlock(wtp->lock);
		return item;
	}
}
