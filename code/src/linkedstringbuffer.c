/*
 * Author: Kristian Hentschel
 * Matric: 1003734h
 * Submission: Networked Systems 3 Assessed Exercise 1
 *
 * This file is my own work.
 *
 * Implements a dynamically allocated string buffer that can grow to almost any size as required, without copying previously allocated memory around too much.
 */
#include "linkedstringbuffer.h"
#include <stdlib.h>
#include <string.h>

struct lsb_node {
	char *s;
	struct lsb_node *next;
};

struct lsb_struct {
	size_t size;
	struct lsb_node *head;
	struct lsb_node *tail;
};

void lsb_node_destroy(struct lsb_node *node){
	if (node == NULL) {
		return;
	}
	 
	free(node->s);
	free(node);
}

struct lsb_node *lsb_node_create() {
	struct lsb_node *node;

	node = (struct lsb_node *) malloc(sizeof(struct lsb_node));
	node->s = NULL;
	node->next = NULL;
	return node;
}

/* creates an empty linked string buffer */
lsb lsb_create(void){
	lsb result = malloc(sizeof(struct lsb_struct));
	result->size = 0;
	result->head = NULL;
	result->tail = NULL;
	return result;
}

/* destroys a linked string buffer and frees all nodes and strings stored therein. */
void lsb_destroy(lsb l) {
	struct lsb_node *prev;
	struct lsb_node *node = l->head;
	
	if (l == NULL) {
		return;
	}

	while (node != NULL) {
		prev = node;
		node = node->next;
		lsb_node_destroy(prev);
	}
	free(l);
}

/* concatenates all strings contained in the buffer. resulting string must be freed outside of the lsb interface. */
char *lsb_string(lsb l) {
	char *result;
	struct lsb_node *node;

	result = (char *) malloc(l->size + 1);
	result[0] = '\0';
	
	for (node = l->head; node != NULL; node = node->next) {
		strcat(result, node->s);
	}

	return result;
}

/* adds another \0 delimited string to the buffer, copying it into a malloc'd string. */
void lsb_add(lsb l, const char *s) {
	size_t size;
	struct lsb_node *node;

	node = lsb_node_create();

	size = strlen(s) + 1;
	node->s = malloc(size);
	strcpy(node->s, s);

	l->size += size - 1;
	
	if (l->tail == NULL) {
		l->head = node;
	} else {
		l->tail->next = node;
	}
	
	l->tail = node;
}


