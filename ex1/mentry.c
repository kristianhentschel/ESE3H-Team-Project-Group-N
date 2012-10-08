/*
 * Author: Kristian Hentschel / 1003734h
 * Advanced Programming 3, Assessed Exercise AX1
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "mentry.h"

#define BUFFER_SIZE 1000
#define ADDR_LINES 3

static char* surname_get(char *name);

/* me_get returns the next file entry, or NULL if end of file*/
MEntry *me_get(FILE *fd)
{
	/* TODO */

	MEntry* e;	
	char address[BUFFER_SIZE];
	char* line;
	int c, n, nlines;
	
	enum {NAME, STREET, POSTCODE} state;
	
	state = NAME;

	e = (MEntry *) malloc(sizeof(MEntry));

	nlines = n = 0;
	line = &address[0];


	while ((c = getchar()) != EOF && n < (BUFFER_SIZE - 1) && nlines < ADDR_LINES) {
		address[n++] = c;
		/* TODO remove outer check and switch statement.
		   work on characters, put them into buffers or sth and increase
		   state when c==\n.*/
		if (c == '\n') {
			address[n] = '\0';
			switch (state) {
				case NAME:
					printf("Name: %s\n", surname_get(line));
					
					state++;
					break;
				case STREET:
					printf("House Number: %d\n", atoi(line));
					state++;
					break;
				case POSTCODE:
					printf("Post code: %s\n", line);
					break;
			}
			line = &address[n];
			nlines++;
		}
	}
	address[n] = '\0';

	e->full_address = (char *) malloc(n);
	strcpy(e->full_address, address);

	return e;
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

/* extracts the surname (last alphabetic token) from given string and returns
 * malloc'd char pointer to surname.
 * TODO convert to lower case. use buffer array rather than pointer magic?
 */
char* surname_get(char *name)
{
	int c, len;
	char *start, *result;

	enum {OUT, IN} state;

	start = name;
	len = 0;
	state = OUT;

	while ((c = *(name++)) != '\n') {
		if (isalpha(c)) {
			if( state == OUT ) {
				state = IN;
				start = name - 1;
				len = 0;
			}
			len++;
		} else {
			state = OUT;
		}
	}

	/* len + 1 for name and terminating \0 */
	result = (char*) malloc(len + 1);

	/* copy len characters and set terminating \0 */
	result = strncpy(result, start, len);
	result[len] = '\0';

	return result;
}


