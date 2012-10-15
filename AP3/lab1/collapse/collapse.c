#include <stdio.h>
#include <string.h>
#include <ctype.h>

int main()
{
	char c;
	int in_space = 0;

	while (( c = getchar()) != EOF) {
		if (isspace(c)) {
			in_space = 1;
		} else {
			if (in_space) {
				in_space = 0;
				putchar(' ');	
			}
			putchar(c);
		}
	}

	putchar('\n');
	
	return 0;
}
