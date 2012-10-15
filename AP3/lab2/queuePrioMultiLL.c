#include "queue.h"
#include <stdlib.h>
#include <string.h>

/*
 * implementation of a LIFO queue using a linked list
 * ignore priority argument in add()
 */

struct q_element {
	struct q_element *next;
	void *value;
};

struct queue {
	struct q_element *heads[MIN_PRIO + 1];
};

/*
 * create a Queue that holds Items
 * returns NULL if the create call failed (malloc failure)
 */
Queue *q_create(void) {
	struct queue *p;
	int i;

	if ((p = (struct queue *)malloc(sizeof(struct queue))) != NULL) {
		for (i = 0; i <= MIN_PRIO; i++) {
			p->heads[i] = NULL;
		}
	}
	return p;
}

/*
 * add Item to the Queue; 3rd arg is priority in MIN_PRIO..MAX_PRIO;
 * return 1/0 if successful/not-successful
 * For Priority, adds after last element with given priority. adds at the head.
 */
int q_add(Queue *q, Item i, int prio) {
	struct q_element *p;
	struct q_element **temp;

	p = (struct q_element *)malloc(sizeof(struct q_element));
	if (p != NULL) {
		p->value = i;


		temp = &q->heads[prio];

		if (*temp == NULL)
			*temp = p;
		else {
			p->next = *temp;
			*temp = p;
		}
		return 1;
	}
	return 0;
}

/*
 * remove next Item from queue; returns NULL if queue is empty
 */
Item q_remove(Queue *q) {
	struct q_element *p;
	Item i;
	int k;

	for (k = MAX_PRIO; k <= MIN_PRIO && q->heads[k] == NULL; k++)
		;

	if (q->heads[k] == NULL)
		return NULL;

	p = q->heads[k];

	if(p->next == NULL)
		q->heads[k] = NULL;
	else
		q->heads[k] = p->next;
	
	i = p->value;
	free(p);
	
	return i;
}
