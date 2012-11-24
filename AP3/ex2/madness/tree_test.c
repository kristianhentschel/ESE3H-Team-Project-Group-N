#include "tree.h"
#include "iterator.h"
#include <stdio.h>
#include <string.h>

int scmp(void *s1, void *s2){
	return strcmp((char *) s1, (char *) s2);
}
/*
 * tests the tree implementation by letting the user input strings.
 * then prints out the in-order iteration
 */
int main(int argc, char *argv[]) {
	tree *t;
	iterator *it;
	char buf[1024];
	char c;
	unsigned int n;
	char *p;

	t = tree_create( &scmp );

	n = 0;
	while(n < sizeof(buf) && (c = getchar()) != EOF) {
		if (c == '\n') {
			buf[n] = '\0';
			tree_insert(t, (void*) strdup(buf));
			n = 0;
		} else {
			buf[n++] = c;
		}
	}

	printf("end of input\n");

	it = tree_iterator_inorder(t);

	while((p = (char *) iterator_get_next(it)) != NULL){
		printf("%s\n", p);
	}

	tree_iterator_destroy(it, t);

	return 0;
}
