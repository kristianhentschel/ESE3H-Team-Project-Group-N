/*
 * Author: Kristian Hentschel / 1003734h
 * Advanced Programming 3, Assessed Exercise AX1
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mentry.h"

#define BUFFER_SIZE 1000
#define ADDR_LINES 3

/* me_get returns the next file entry, or NULL if end of file*/
MEntry *me_get(FILE *fd)
{
	/* TODO
	 * malloc storage for mentry?
	 */

	MEntry* e;	
	char address[BUFFER_SIZE];
	int c, n, nlines;
	
	e = (MEntry *) malloc(sizeof(MEntry));

	nlines = n = 0;

	while ((c = getchar()) != EOF && n < (BUFFER_SIZE - 1) && nlines < ADDR_LINES) {
		printf("%4d: %c\n", n, c);
		address[n++] = c;
		if (c == '\n') {
			nlines++;
		}
	}
	address[n] = '\0';

	e->full_address = (char *) malloc(n);
	strcpy(e->full_address, address);

	printf("%s\n", e->full_address);
}

/* me_hash computes a hash of the MEntry, mod size */
unsigned long me_hash(MEntry *me, unsigned long size)
{
	/* TODO */
}

/* me_print prints the full address on fd */
void me_print(MEntry *me, FILE *fd)
{
	/* TODO */
}

/* me_compare compares two mail entries, returning <0, 0, >0 if
 * me1<me2, me1==me2, me1>me2
 */
int me_compare(MEntry *me1, MEntry *me2)
{
	/* TODO */
}

/* me_destroy destroys the mail entry
 */
void me_destroy(MEntry *me)
{
	/* TODO */
}
