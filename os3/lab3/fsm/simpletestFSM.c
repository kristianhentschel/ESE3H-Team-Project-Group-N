#include "FSM.h"
#include <assert.h>

int main(void) {
	FSM f = createFSM(0, 100);
	unsigned long a;

	assert(allocate(f, 10, &a));
	
	deallocate(f, a+1, 2);
	deallocate(f, a, 1);
	deallocate(f, a+3, 2);
	
	assert(allocate(f, 10, &a));


	destroyFSM(f);
	return 0;
}
