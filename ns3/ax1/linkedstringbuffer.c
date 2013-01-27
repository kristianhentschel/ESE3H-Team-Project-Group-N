#include "linkedstringbuffer.h"
#include <stdlib.h>
#include <string.h>

struct lsb_node {
	char *s;
	struct lsb_node *next;
};

struct lsb_struct {
	long size;
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
		strlcat(result, node->s, l->size + 1);
	}

	return result;
}

/* adds another \0 delimited string to the buffer, copying it into a malloc'd string. */
void lsb_add(lsb l, const char **s) {
	int size;
	struct lsb_node *node;

	node = lsb_node_create();

	size = strlen(*s) + 1;
	node->s = malloc(size);
	strlcpy(node->s, *s, size);

	l->size += size - 1;
	l->tail->next = node;
	l->tail = node;	
}


