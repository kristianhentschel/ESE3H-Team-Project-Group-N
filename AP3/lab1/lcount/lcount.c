#include <stdio.h>
#include <string.h>
#include "lines.h"

#define MAXLINE 1000

int main()
{
	char line[MAXLINE];
	int n;

	for (n = 0; readline(line, MAXLINE); n++)
			;

	printf("%d\n", n);

	return 0;
}
