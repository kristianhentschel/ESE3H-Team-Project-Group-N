#include "iterator.h"
#include <stdlib.h>

struct iterator {
	long size;
	long pos;
	void **items;
};

iterator *iterator_create(long size, void **elements) {
	iterator *it;
	
	if ((it = (iterator *) malloc(sizeof(struct iterator))) == NULL){
		return NULL;
	}
	/*
	if (it->items = (void **) malloc(sizeof(void *) * size) == NULL){
		free(it);
		return NULL;
	}
	*/

	it->size = size;
	it->pos = 0;
	it->items = elements;
	return it;
}

void iterator_destroy(iterator *it) {
	free(it->items);
	free(it);
}

/* returns the next item or NULL if there is none. */
void *iterator_get_next(iterator *it) {
	if(iterator_has_next(it))
		return it->items[it->pos++];
	else
		return NULL;
}

/* returns 1 if an item is available, 0 otherwise. */
int iterator_has_next(iterator *it) {
	if (it->pos < it->size - 1)
		return 1;
	else
		return 0;
}

