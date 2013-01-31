#ifndef __FSM
#define __FSM

/*
 * Author: Peter Dickman and Joe Sventek
 * Edited: 15-11-96
 * Version 1.01
 *
 * This is the header for the free space manager exercise
 */

typedef void *FSM;

FSM createFSM(unsigned long first, unsigned long last);
/* arguments indicate lower & upper limits of indices to be managed */
/* it will be normal, but not required, for 'first' to be 0       */
/* assume that initially all indices are free */

void destroyFSM(FSM f);
/* discards FSM data structure regardless of current status of indices */

int allocate(FSM f, unsigned long number, unsigned long *first);
/* request that 'number' adjoining free locations be marked as in use */
/* returns 0 if this is not possible */
/* returns 1 and sets *first to contain the first index, if it is possible */

void deallocate(FSM f, unsigned long first, unsigned long number);
/* 'number' locations starting at index 'first' are no longer in use */


#endif
