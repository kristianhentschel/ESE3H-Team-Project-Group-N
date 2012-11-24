#ifndef _ITERATOR_
#define _ITERATOR_

typedef struct iterator iterator;

iterator *iterator_create(long size, void **elements);

void iterator_destroy(iterator *it);

void *iterator_get_next(iterator *it);

int iterator_has_next(iterator *it);

#endif /* _ITERATOR_ */
