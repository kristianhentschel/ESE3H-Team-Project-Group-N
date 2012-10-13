/*
 * Author: Kristian Hentschel / 1003734h
 * Advanced Programming 3, Assessed Exercise AX1
 */

#include <stdio.h>
#include <stdlib.h>
#include "mlist.h"

#define INITIAL_SIZE 10
#define MAX_BUCKET_SIZE 4

int ml_verbose=0;		/* if true, print diagnostics on stderr */

typedef struct mlistnode {
	MEntry *entry;
	struct mlistnode *next;
} MListNode;

struct mlist {
	 struct mlistnode **buckets;
	 int size;
};

static void mlistnode_destroy(MListNode *m);

/* ml_create - created a new mailing list */
MList *ml_create(void)
{
	MList *p;
	int i;

	p = malloc(sizeof(struct mlist));
	if(p == NULL)
		return NULL;
	
	/* TODO provision for resizing, static implementation follows. */
	
	p->size = INITIAL_SIZE;
	
	p->buckets = malloc(p->size * sizeof(MListNode *));
	for(i = 0; i < p->size; i++){
		p->buckets[i] = NULL;
	}

	return p;
}



/* ml_add - adds a new MEntry to the list;
 * returns 1 if successful, 0 if error (malloc)
 * returns 1 if it is a duplicate
 * TODO check in finddupl.c: should maybe be -1 for duplicate?!
 */
int ml_add(MList **ml, MEntry *me)
{
	unsigned long hash;
	int cmp;   
	MListNode *p, *tail;
	
	hash = me_hash(me, (*ml)->size);

	if(ml_verbose) fprintf(stderr, "ml_add: Entry with surname %s hashed to %ld\n", me->surname, hash);

	p = (*ml)->buckets[hash];
	tail = NULL;

	while( p != NULL ){
		cmp = me_compare(me, p->entry);

		tail = p;
		p = p->next;
		
		if (cmp == 0)
			/* duplicate */
			return 1;
		else if (cmp > 0)
			/* me > p->entry, insert before node p */
			break;
	}

	p = malloc(sizeof(MListNode));
	if (p == NULL)
		/* malloc error */
		return 0;

	p->entry = me;

	if (tail == NULL) {
		(*ml)->buckets[hash] = p; 
		p->next = NULL;
	} else {
		p->next = tail->next;
		tail->next = p;		
	}
	return 1;
}


/* ml_lookup - looks for MEntry in the list, returns matching entry or NULL */
MEntry *ml_lookup(MList *ml, MEntry *me)
{
	unsigned long hash;
	int cmp;
	MListNode *p;

	hash = me_hash(me, ml->size);

	if(ml_verbose) fprintf(stderr, "ml_lookup: Entry with surname %s hashed to %ld\n", me->surname, hash);

	p = ml->buckets[hash];

	while( p != NULL ){
		cmp = me_compare(me, p->entry);

		if (cmp == 0)
			/* duplicate found */
			return p->entry;
		else if (cmp > 0)
			/* passed position of potential duplicates */
			return NULL;
		
		p = p->next;
	}
	return NULL;
}

/* ml_destroy - destroy the mailing list */
void ml_destroy(MList *ml)
{
	/* TODO */
	int i;

	for (i = 0; i < ml->size; i++)
		mlistnode_destroy(ml->buckets[i]);

	free(ml->buckets);
	free(ml);
}

/* frees node container, its entry, and all following nodes in the linked list. */
void mlistnode_destroy(MListNode *m)
{
	if (m == NULL)
		return;

	me_destroy(m->entry);
	mlistnode_destroy(m->next);
	free(m);
}
