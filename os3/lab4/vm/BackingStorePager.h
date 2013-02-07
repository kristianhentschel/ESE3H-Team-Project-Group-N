#ifndef __BSPagerDefns
#define __BSPagerDefns

/*
 * Author: Peter Dickman
 * Created: 15-11-95
 * Version 1.01
 *
 * This is the header for the backing store emulator
 */

/* takes a PM address and a process number and a virtual memory address */
/* moves the contents of the indicated PM page off to backing store assuming
 * it corresponds to the VM page beginning at the indicated address for the
 * process identified as the owner.
 * NB PM & VM addresses should be addresses corresponding to starts of pages
 */
void  PMtoBS(unsigned long, unsigned long, unsigned long);  

/* takes a PM address and a process number and a virtual memory address */
/* moves the contents of the indicated VM page for the indicated process from
 * backing store into the PM page given as first argument 
 * NB PM & VM addresses should be addresses corresponding to starts of pages
 */
void  BStoPM(unsigned long, unsigned long, unsigned long);  

#endif




