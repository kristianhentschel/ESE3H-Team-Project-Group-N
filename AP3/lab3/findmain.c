#include <stdio.h>
#include <string.h>

#define MAXLINE 1000

char pattern[] = "ould";

int main (int argc, char *argv[])
{
	char line[MAXLINE];

	int found = 0;

	while (fgets(line, MAXLINE, stdin) != NULL) {
		if (strstr(line, pattern) != NULL) {
			printf("%s", line);
			found++;
		}
	}

	return found;
}
