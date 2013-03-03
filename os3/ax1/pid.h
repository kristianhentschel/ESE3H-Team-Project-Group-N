#ifndef PID_HDR
#define PID_HDR

/*
 * Author:    Peter Dickman
 * Version:   1.3
 * Last edit: 2003-02-18
 *
 * This file is a component of the test harness and or sample
 * solution to the NetworkDriver exercise (re)developed in February 2003
 * for use in assessing:
 *    the OS3 module in undergraduate Computing Science at dcs.gla.ac.uk
 *
 * It tests the ability to develop a small but complex software system
 * using PThreads to provide concurrency in C.
 *
 *
 * Code quality:
 * Quick hack - no guarantees or liability accepted.
 *
 * Copyright:
 * (c) 2003 University of Glasgow and Dr Peter Dickman
 *
 * Licencing: 
 * this software may not be used except with the author's permission.
 *
 */

typedef unsigned int PID;
#define MAX_PID 10
/* A PID is used to distinguish between the different applications */
/* It is implemented as an unsigned int, but is guaranteed not to  */
/* exceed MAX_PID.                                                 */

#endif
