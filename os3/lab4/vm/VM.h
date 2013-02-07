#ifndef __VMHEADERS
#define __VMHEADERS

/*
 * Author: Peter Dickman
 * Created: 30-10-95
 * Version 1.00
 *
 * This is the header for the simple VM layer the students must implement
 */

/* called once at start, used to initialise datastructures */
void  initialise_VM(void);

/* takes a ProcessId, a VM address and a datum */
void  VMWrite(unsigned long, unsigned long, unsigned long);  

/* takes a ProcessId and a VM address, returns the datum */
unsigned long  VMRead(unsigned long, unsigned long);  

#endif
