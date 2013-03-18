/*
 * this file was part of the network systems 3 coursework, and has not been changed for this project
 */

#ifndef __LINKEDSTRINGBUFFER_H__
#define __LINKEDSTRINGBUFFER_H__

typedef struct lsb_struct *lsb;

/* creates an empty linked string buffer */
lsb lsb_create(void);

/* destroys a linked string buffer and frees all nodes and strings stored therein. */
void lsb_destroy(lsb l);

/* concatenates all strings contained in the buffer. resulting string must be freed outside of the lsb interface. */
char *lsb_string(lsb l);

/* adds another \0 delimited string to the buffer, copying it into a malloc'd string. */
void lsb_add(lsb l, const char *s);

#endif /* __LINKEDSTRINGBUFFER_H__ */

