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
	return 0;
}
