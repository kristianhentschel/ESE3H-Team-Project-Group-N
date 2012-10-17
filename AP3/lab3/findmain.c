#include <stdio.h>
#include <string.h>

#define MAXLINE 1000

void usage(char *bin) {
	fprintf(stderr, "Usage: %s pattern\n", bin);
}

int main (int argc, char *argv[])
{
	char line[MAXLINE];
	char *pattern;
	int found = 0;
	int invert = 0;

	if (argc < 2 || argc > 3) {
		usage(argv[0]);
		return 1;
	}
	
	if (argc > 2) {
		if (*argv[1] == '-'){
			if(strstr(argv[1], "x")){
				invert = 1;
			}
		}
	}

	pattern = argv[i];

	while (fgets(line, MAXLINE, stdin) != NULL) {
		if (strstr(line, pattern) != NULL ) {
			printf("%s", line);
			found++;
		} else if ( invert ) {
			printf("%s", line);
			found++;
		}
	}

	return 0;
}
