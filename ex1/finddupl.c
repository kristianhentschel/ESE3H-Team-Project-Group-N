#include <stdio.h>
#include "mentry.h"
#include "mlist.h"

static void usage(void) {
	fprintf(stderr, "usage: finddupl [-v]\n");
}

int main(int argc, char *argv[]) {
	MEntry *mep, *meq;
	MList *ml;
	char *p;
	int c;
	int varg = 0;

	if (argc > 2) {
		usage(); return -1;
	}
	if (argc == 2) {
		p = argv[1];
		if (*p++ != '-') {
			usage(); return -1;
		}
		while ((c = *p++) != '\0') {
			if (c =='v' || c == 'V')
				varg++;
			else {
				fprintf(stderr, "Illegal flag: %c\n", c);
				usage(); return -1;
			}
		}
	}
	ml_verbose = varg;
		
	ml = ml_create();
	while ((mep = me_get(stdin)) != NULL) {
		meq = ml_lookup(ml, mep);
		if (meq == NULL)
			(void) ml_add(&ml, mep);
		else {
			printf("Potential duplicate\n");
			printf("===================\n");
			me_print(mep, stdout);
			printf("==========\n");
			me_print(meq, stdout);
			printf("\n");
			me_destroy(mep);
		}
	}
	ml_destroy(ml);
	return 0;
}
