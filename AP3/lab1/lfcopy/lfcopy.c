#include <stdio.h>
#include <string.h>
#include "lines.h"

#define MAXLINE 1000

int main()
{
	char line[MAXLINE];

	while (readline(line, MAXLINE))
	{
			writeline(line);
	}

	return 0;
}
