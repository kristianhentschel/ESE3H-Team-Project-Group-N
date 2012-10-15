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
		} else if (isalpha(c)) {
			in_word = 1;
		}
	}
	
	printf("%d\n", wc);

	return 0;
}
