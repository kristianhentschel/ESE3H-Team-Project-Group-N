/*
 * Author: Kristian Hentschel / 1003734h
 * Advanced Programming 3, Assessed Exercise AX1
 */

#include <stdio.h>
#include <stdlib.h>
#include "mlist.h"

#define INITIAL_SIZE 10
#define MAX_BUCKET_SIZE 10

int ml_verbose=0;		/* if true, print diagnostics on stderr */

typedef struct mlistnode {
	MEntry *entry;
	struct mlistnode *next;
} MListNode;

typedef struct mlistbucket {
	MListNode *head;
	int size;
} MListBucket;

struct mlist {
	struct mlistbucket **buckets;
	int size;
};

static MList *ml_create2(int n);
static void ml_resize(MList **ml);

static void mlistnode_destroy(MListNode *m);

static MListBucket *mlistbucket_create(void);
static void mlistbucket_destroy(MListBucket *b);

/* ml_create - creates a mailing list of defined original size */
MList *ml_create(void){
	return ml_create2(INITIAL_SIZE);
}

/* ml_create - creates a new mailing list of given size */
static MList *ml_create2(int n)
{
	MList *p;
	int i;

	p = malloc(sizeof(struct mlist));
	if(p == NULL)
		return NULL;
	
	p->size = n;
	
	p->buckets = malloc(p->size * sizeof(MListBucket *));
	for(i = 0; i < p->size; i++){
		/* TODO handle malloc failure (_create returns NULL) */
		p->buckets[i] = mlistbucket_create();
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
	MListBucket *bucket;

	hash = me_hash(me, (*ml)->size);

	if(ml_verbose) fprintf(stderr, "ml_add: Entry with surname %s hashed to %ld\n", me->surname, hash);

	bucket = (*ml)->buckets[hash];
	p = (*ml)->buckets[hash]->head;
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
		bucket->head = p; 
		p->next = NULL;
	} else {
		p->next = tail->next;
		tail->next = p;		
	}
	
	bucket->size++;
	if(ml_verbose) fprintf(stderr, "bucket %d size is %d after add.\n", hash, bucket->size);
	
	if(bucket->size > MAX_BUCKET_SIZE)
		ml_resize(ml);

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

	p = ml->buckets[hash]->head;

	while( p != NULL ){
		cmp = me_compare(me, p->entry);
		if(ml_verbose) fprintf(stderr, "compared to %s: %d\n", p->entry->surname, cmp);

		if (cmp == 0)
			/* duplicate found */
			return p->entry;
		else if (cmp < 0)
			/* passed position of potential duplicates */
			return NULL;
		
		p = p->next;
	}
	return NULL;
}

/* ml_destroy - destroy the mailing list */
void ml_destroy(MList *ml)
{
	if(ml_verbose) fprintf(stderr, "ml_destroy\n");

	int i;

	for (i = 0; i < ml->size; i++)
		mlistbucket_destroy(ml->buckets[i]);

	free(ml->buckets);
	free(ml);
}

/* ml_resize resizes the mailing list, doubling the number of containers. */
void ml_resize(MList **ml){
	MList *oldml, *newml;
	int i;
	MListNode *p;

	oldml = *ml;
	newml = ml_create2(2 * oldml->size);
	
	if (ml_verbose) fprintf(stderr, "Resizing mailing list from %d to %d buckets.\n", oldml->size, newml->size);
	/* TODO more statistics on bucket sizes would be nice. */

	for (i = 0; i < oldml->size; i++) {
		p = oldml->buckets[i]->head;
		while (p != NULL){
			ml_add(&newml, p->entry);
			p->entry = NULL;
			p = p->next;
		}
	}

	/* done, swap lists and delete original (now empty) table */
	ml_destroy(oldml);
	*ml = newml;
}

/* creates and initialises and empty bucket */
MListBucket *mlistbucket_create() {
	MListBucket *b;

	b = malloc(sizeof(MListBucket));
	if(b == NULL)
		return NULL;

	b->size = 0;
	b->head = NULL;

	return b;
}

/* frees bucket and all its node entries. */
void mlistbucket_destroy(MListBucket *b)
{
	if(ml_verbose) fprintf(stderr, "mlistbucket_destroy\n");
	mlistnode_destroy(b->head);
	free(b);
}
/* frees node container, its entry, and all following nodes in the linked list. */
void mlistnode_destroy(MListNode *m)
{
	if(ml_verbose) fprintf(stderr, "mlistnode_destroy\n");
	if (m == NULL)
		return;

	me_destroy(m->entry);
	mlistnode_destroy(m->next);
	free(m);
}
