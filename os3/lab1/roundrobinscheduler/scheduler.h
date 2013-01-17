#ifndef __SCHEDULER
#define __SCHEDULER

/*
 * Authors: Peter Dickman and Joe Sventek
 *
 * This is the header to which the students have to supply an implementation
 * It's meant to be the interface to a simple scheduler
 */

#include "TCB.h"

void  init(void);               /* called once, before any of the others */
void  add_runnable_TCB(TCB *t); /* introduces a runnable TCB into system */
void  block(void);              /* current task has blocked */
void  scheduler(void);          /* timeslice has expired */

#endif
