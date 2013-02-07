#include <stdio.h>
#include <stdlib.h>

#include "VM.h"
#include "SystemParameters.h"
#include "PhysicalMemory.h"
#include "BackingStorePager.h"

typedef unsigned long frame_t;
typedef unsigned long page_t;
typedef unsigned long process_t;
typedef unsigned long vm_t;
typedef unsigned long pm_t;

struct pt_entry {
	page_t	pagenumber;
	frame_t	framenumber;
	char dirty;
};



/* returns the physical memory address of the given virtual address in the given process's virtual address space.
 * will bring in the page from the backing store if required. */
static unsigned long get_page(process_t pid, vm_t vaddr);

/* selects a page and evicts it from physical memory, returns the now free frame number */
static unsigned long evict_page();

/* called once at start, used to initialise datastructures */
void  initialise_VM(void) {

}

/* takes a ProcessId, a VM address and a datum */
void  VMWrite(unsigned long pid, unsigned long vaddr, unsigned long datum) {
	
}

/* takes a ProcessId and a VM address, returns the datum */
unsigned long  VMRead(unsigned long pid, unsigned long vaddr) {
	return 0L;
}
