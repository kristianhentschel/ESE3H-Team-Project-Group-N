#ifndef __PMHEADERS
#define __PMHEADERS

/*
 * Author: Peter Dickman
 * Edited: 15-11-95
 * Version 1.01
 *
 * This is the header for the physical memory emulator
 */

/* takes a PM address and a datum */
void  Write(unsigned long, unsigned long);  

/* takes a PM address, returns the datum */
unsigned long  Read(unsigned long);  

/* takes a PM address that's the start of a page, returns 1 if page is dirty */
int is_dirty(unsigned long);

#endif




