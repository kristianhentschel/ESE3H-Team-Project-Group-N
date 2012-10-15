#include <stdio.h>
#include <string.h>
#include <ctype.h>

int main()
{
	char c;
	int in_space = 0;
	int in_string = 0;
	
	while (( c = getchar()) != EOF) {
		if (in_string) {
			putchar(c);
			if(c == '\'' || c == '\"')
				in_string = 0;
		} else if (isspace(c)) {
			in_space = 1;
		} else {
			if (in_space) {
				in_space = 0;
				putchar(' ');	
			}
			if(c == '\'' || c == '\"')
				in_string = 1;
			putchar(c);
		}
	}

	putchar('\n');
	
	return 0;
}
