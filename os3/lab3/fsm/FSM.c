#include "FSM.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#define DEBUG 1

struct hole {
	unsigned long size;
	unsigned long start;
	struct hole *next;
};

struct fsm {
	unsigned long startindex;
	unsigned long lastindex;
	struct hole *head;
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

void debugPrintFSM(FSM fv) {
	struct fsm *f;
	struct hole *p;

	f = fv;
	p = f->head;


	while (p != NULL) {
		printf("[%lu - %lu - %lu] ", p->start, p->size, p->start + p->size);
		p = p->next;
	}

	printf("\r\n");

}

FSM createFSM(unsigned long first, unsigned long last) {
	struct fsm *result;

	assert(last > first);

	if ((result = (struct fsm*) malloc(sizeof(struct fsm))) == NULL) {
		return NULL;
	}

	result->startindex = first;
	result->lastindex = last;
	result->head = createHole(first, last - first + 1);

	return (FSM) result;
}

void destroyFSM(FSM f) {
	struct hole *p, *q;

	if (f == NULL) {
		return;
	}

	p = ((struct fsm *) f)->head;

	while (p != NULL) {
		q = p;
		p = p->next;
		free(q);
	}

	free(f);
}


int allocate(FSM f, unsigned long number, unsigned long *first) {
	struct hole *p, *prev;

	if (DEBUG) printf("alloc %lu\n", number);

	p = ((struct fsm *)f)->head;
	prev = NULL;


	while (p != NULL && p->size < number) {
		prev = p;
		p = p->next;
	}

	if (p == NULL) {
		if (DEBUG) debugPrintFSM(f);
		return 0;
	} else {
		*first = p->start;
		
		p->start += number;
		p->size  -= number;

		if (p->size == 0) {
			if (prev == NULL) {
				((struct fsm *)f)->head = p->next;
			} else {
				prev->next = p->next;
			}
			free(p);
		}
		if (DEBUG) debugPrintFSM(f);
		return 1;
	}
}

void deallocate(FSM f, unsigned long first, unsigned long number) {
	struct hole *p, *prev, *new;

	assert(first + number <= ((struct fsm *)f)->lastindex + 1);

	p = ((struct fsm *)f)->head;
	prev = NULL;

	if (DEBUG) printf("dealloc %lu, starting at %lu\n", number, first);

	while (p != NULL) {
		if (p->start > first) {
			if (first + number == p->start) {
				/* add free space to beginning of found hole */
				p->start = first;
				p->size += number;
			} else {
				/* add hole inbetween the found hole and the one before it. */
				assert(first + number < p->start);
				new = createHole(first, number);
				new->next = p;
				if( prev != NULL) {
					prev->next = new;
				} else  {
					((struct fsm *)f)->head = new;
				}

				/* so code below can check if prev and p are adjacent */
				p = new;
			}
			
			if (prev != NULL && prev->start + prev->size == p->start) {
				prev->size += p->size;
				prev->next = p->next;
				free(p);
			}

			if (DEBUG) printf("returned free space before p\r\n");

			if (DEBUG) debugPrintFSM(f);
			return;
		}
		
		prev = p;
		p = p->next;
	}

	/* p is NULL, prev is the last hole in the list. */
	if (prev == NULL) {
		/* all frames were allocated, no holes exist. */
		assert(((struct fsm *)f)->head == NULL);
		((struct fsm *)f)->head = createHole(first, number);
		if (DEBUG) printf("created one and only hole\r\n");
	} else if (first == prev->start + prev->size) {
		/* chunk is directly after last hole */
		assert(prev->next == NULL);
		prev->size += number;
	} else {
		/* chunk is somewhere after last hole */
		assert(prev->next == NULL);
		prev->next = createHole(first, number);
		if (DEBUG) printf("added new last hole\r\n");
	}
}
