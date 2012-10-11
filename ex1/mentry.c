/*
 * Author: Kristian Hentschel / 1003734h
 * Advanced Programming 3, Assessed Exercise AX1
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "mentry.h"

#define ADDRESS_BUFFER 1024

static char* surname_get(char *name);
static char* postcode_get(char *name);
static char* strtolower(char *str);

/* me_get returns the next file entry, or NULL if end of file*/
MEntry *me_get(FILE *fd)
{
	MEntry* e;	
	char address[ADDRESS_BUFFER];
	char* line;
	int c, n;
	enum {NAME, STREET, POSTCODE, DONE} state;
	
	e = (MEntry *) malloc(sizeof(MEntry));
	if (!e) {
		/* malloc failure */
		return NULL; 
	}

	state = NAME;
	n = 0;
	line = address;

	/* get characters of current address block, up to and including trailing \n */
	while ((c = fgetc(fd)) != EOF && n < (ADDRESS_BUFFER - 1) && state != DONE) {
		address[n++] = c;
		if (c == '\n') {
			address[n] = '\0';
			switch (state) {
				case NAME:
					e->surname = strtolower(surname_get(line));
					break;
				case STREET:
					e->house_number = atoi(line);
					break;
				case POSTCODE:
					e->postcode = strtolower(postcode_get(line));
					break;
				case DONE:
					break;
			}
			line = &address[n];
			state++;
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
	return 0UL;
}

/* me_print prints the full address on fd */
void me_print(MEntry *me, FILE *fd)
{
	char *c = me->full_address;
	while (*(c++) != '\0')
		fputc(*c, fd);
}

/* me_compare compares two mail entries, returning <0, 0, >0 if
 * me1<me2, me1==me2, me1>me2
 */
int me_compare(MEntry *me1, MEntry *me2)
{
	/* TODO: what is the order for these? */
	int result;
	result = strcmp(me1->surname, me2->surname);
	
	if (result == 0)
		result = (me1->house_number == me2->house_number);

	if (result == 0)
		result = strcmp(me1->postcode, me2->postcode);

	return result;
}

/* me_destroy destroys the mail entry
 */
void me_destroy(MEntry *me)
{
	/* TODO test with valgrind */
	free(me->surname);
	free(me->postcode);
	free(me->full_address);
	free(me);	
}

/* extracts the surname (last alphabetic token) from given string and returns
 * malloc'd char pointer to surname.
 * TODO use buffer array rather than pointer magic?
 */
char* surname_get(char *name)
{
	int c, len; //TODO c should be a char
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

	if (!result) {
		return NULL;
	} else {
		/* copy len characters and set terminating \0 */
		result = strncpy(result, start, len);
		*(result+len) = '\0';

		return result;
	}
}

/* removes trailing \n and non-alphanumeric characters from postcode line
   and copies string into new malloc'd pointer. */
char* postcode_get(char *postcode)
{
	char buf[ADDRESS_BUFFER];
	char *result;
	int c, n, len;

	n = len = 0;

	while ((c = postcode[n++]) != '\n' && n < ADDRESS_BUFFER - 1) {
		if( isalnum(c) )
			buf[len++] = c;
	}
	buf[len++] = '\0';

	result = malloc(len);
	if(!result)
		/* malloc failure */
		return NULL;

	return strcpy(result, buf);
}   

/* converts a string to lower case in place. */
char* strtolower(char *str)
{
	char* p;
	p = str;
	while ( *p ) {
		*p = (char) tolower((int) *p);
		p++;
	}
	return str;
}
