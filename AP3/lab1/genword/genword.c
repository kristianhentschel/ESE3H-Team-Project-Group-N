#include <stdio.h>
#include <string.h>
#include <ctype.h>

int main()
{
	char c;
	int wc = 0;
	int in_word = 0;

	while( ( c = getchar()) != EOF ) {
		if(in_word && !isalpha(c)) {
			in_word = 0;
			wc++;
			putchar('\n');
		} else if (isalpha(c)) {
			in_word = 1;
			putchar(c);
		}
	}
	
	return 0;
}
