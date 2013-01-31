#include "FSM.h"
#include <assert.h>
#include <stdlib.h>

struct hole {
	unsigned long size;
	unsigned long start;
	struct hole *next;
};

struct fsm {
	unsigned long startindex;
	unsigned long lastindex;
	struct hole *first;
};

struct hole *createHole(unsigned long start, unsigned long size) {
	struct hole *result;
	
	if ((result = malloc(sizeof(struct hole))) != NULL) {
		result->next = NULL;
		result->start = start;
		result->size = size;
	}

	return result;
}

FSM createFSM(unsigned long first, unsigned long last) {
	struct fsm *result;

	assert(last > first);

	if ((result = (struct fsm*) malloc(sizeof(struct fsm))) == NULL) {
		return NULL;
	}

	result->startindex = first;
	result->lastindex = last;
	result->first = createHole(first, last - first);

	return (FSM) result;
}

void destroyFSM(FSM f) {
	struct hole *p, *q;

	if (f == NULL) {
		return;
	}

	p = ((struct fsm *) f)->first;

	while (p != NULL) {
		q = p;
		p = p->next;
		free(q);
	}

	free(f);
}


int allocate(FSM f, unsigned long number, unsigned long *first) {
	struct hole *p;


	p = ((struct fsm *)f)->first;

	while (p != NULL && p->size < number) {
		p = p->next;
	}

	if (p == NULL) {
		return 0;
	} else {
		*first = p->start;
		
		
		p->start += number;
		p->size  -= number;

		return 1;
	}
}

void deallocate(FSM f, unsigned long first, unsigned long number) {
	struct hole *p, *prev;


	p = (struct fsm *)f->first;
	prev = NULL;

	while (p != NULL) {
		if (p->start > first) {
			
			
			return;
		}
		
		prev = p;
		p = p->next;
	}

	/* p is NULL. */

}
