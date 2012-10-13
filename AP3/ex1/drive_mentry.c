#include <stdio.h>
#include "mentry.h"

int main(){
	printf("===\n%s===\n", me_get(stdin)->full_address);
	return 0;
}


