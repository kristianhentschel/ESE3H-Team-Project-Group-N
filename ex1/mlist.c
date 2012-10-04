/*
 * Author: Kristian Hentschel / 1003734h
 * Advanced Programming 3, Assessed Exercise AX1
 */

#include <stdio.h>
#include "mlist.h"

int ml_verbose=0;		/* if true, print diagnostics on stderr */

/* ml_create - created a new mailing list */
MList *ml_create(void)
{
	/* TODO */
}

/* ml_add - adds a new MEntry to the list;
 * returns 1 if successful, 0 if error (malloc)
 * returns 1 if it is a duplicate
 * TODO check in finddupl.c: should maybe be -1 for duplicate?!
 */
int ml_add(MList **ml, MEntry *me)
{
	/* TODO */
}

/* ml_lookup - looks for MEntry in the list, returns matching entry or NULL */
MEntry *ml_lookup(MList *ml, MEntry *me)
{
	/* TODO */
}

/* ml_destroy - destroy the mailing list */
void ml_destroy(MList *ml)
{
	/* TODO */
}
