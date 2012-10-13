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

extern int ml_verbose;

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
	while ( state != DONE && n < (ADDRESS_BUFFER - 1) && (c = fgetc(fd)) != EOF ) {
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

	if (c == EOF){
		me_destroy(e);
		return NULL;
	}

	address[n++] = '\0';

	e->full_address = (char *) malloc(n);

	if (e->full_address == NULL || e->surname == NULL || e->postcode == NULL) {
		if(ml_verbose) fprintf(stderr, "me_get: returning NULL\n");
		me_destroy(e);
		return NULL;
	} else {
		strcpy(e->full_address, address);
		return e;
	}
}

/* me_hash computes a hash of the MEntry, mod size */
unsigned long me_hash(MEntry *me, unsigned long size)
{
	/* TODO build a proper hash function. this is based on K&R book example. */
	unsigned long hash;
	char *s;
   
	hash = 0;

	s = me->surname;

	while (*s != '\0')
		hash = *(s++) + 31 * hash;

	hash += me->house_number + 31*hash; 

	s = me->postcode;
	while (*s != '\0')
		hash = *(s++) + 31 * hash;

	return hash % size;
}

/* me_print prints the full address on fd */
void me_print(MEntry *me, FILE *fd)
{
	
	fprintf(fd, "%s", me->full_address);
	/*char *c = me->full_address;
	while (*(c++) != '\0')
		fputc(*c, fd);*/
}

/* me_compare compares two mail entries, returning <0, 0, >0 if
 * me1<me2, me1==me2, me1>me2
 */
int me_compare(MEntry *me1, MEntry *me2)
{
	/*
	static unsigned long count = 0;
	count++;
	if(ml_verbose) fprintf(stderr, "%ld\n", count);
	*/

	int result;
	result = strcmp(me1->surname, me2->surname);
	
	//fprintf(stderr, "0x%x\n", me1->surname[0]);
	
	if (result == 0)
		result = (me1->house_number != me2->house_number);

	if (result == 0)
		result = strcmp(me1->postcode, me2->postcode);

	return result;
}

/* me_destroy destroys the mail entry
 */
void me_destroy(MEntry *me)
{
	free(me->surname);
	free(me->postcode);
	free(me->full_address);
	free(me);	
}

/* extracts the surname (first alphabetic token) from given string and returns
 * malloc'd char pointer to surname.
 * FIXME this function works according to the format of sample input files.
 */
char* surname_get(char *name)
{
	char buf[ADDRESS_BUFFER];
	char *result;
	int c, n, len;

	n = len = 0;

	while ( isalpha(c = name[n++]) && n < ADDRESS_BUFFER - 1 )
		buf[len++] = c;
	
	buf[len++] = '\0';

	result = (char *) malloc(len);
	if (!result)
		return NULL;

	return strcpy(result, buf);
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

	result = (char *) malloc(len);
	if(!result)
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
