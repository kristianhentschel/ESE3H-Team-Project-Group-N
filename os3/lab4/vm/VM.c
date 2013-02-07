#include <stdio.h>
#include <stdlib.h>

#include "VM.h"
#include "SystemParameters.h"
#include "PhysicalMemory.h"
#include "BackingStorePager.h"

typedef unsigned long frame_t;
typedef unsigned long process_t;
typedef unsigned long vm_t;
typedef unsigned long pm_t;

/* inverse page table maps from virtual base address and process id
 * to page number in global page space. */
struct ipt_entry {
	process_t	process;
	vm_t		virtual_base_address;
	frame_t		frame_number;
	char		dirty;
}

/* returns the physical memory address of the given virtual address in the given process's virtual address space.
 * will bring in the page from the backing store if required. */
static frame_t get_page(process_t pid, vm_t vaddr);

/* selects a page and evicts it from physical memory, returns the now free frame number */
static frame_t evict_page();

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



static frame_t get_page(process_t pid, vm_t vaddr) {
	/* find entry in inverted page table, using top bits of virtual address */

	if (found) {
		return frame number;
	}

		
	/* get a free page */
	if (!free_frames) {
		frame = evict_page();
	} else {
		free_frames--;
		frame = first_free_frame;
	}
	
	/* bring the page in from the backing store */
	BStoPM(free frame start address, pid, vaddr);
	

}
