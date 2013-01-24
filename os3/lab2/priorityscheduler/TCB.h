#ifndef __TCBheaders
#define __TCBheaders

/* Authors:  Peter Dickman and Joe Sventek
 *
 * Header file for a simple TCB package
 */

/* conceal exactly what a TCB is behind opaque struct */

typedef struct tcb TCB;


/* provide access to sole initial task */
TCB *get_idle_task (void);



/* functions for loading a virtual task into the CPU
 * and unloading it back out again, plus a function
 * for stashing blocked tasks somewhere
 */

void move_from_TCB_to_CPU(TCB *t);   /* copies TCB contents into CPU */
void move_from_CPU_to_TCB(TCB *t);   /* copies CPU contents into TCB */

void add_blocked_TCB(TCB *t);        /* hides the TCB away somewhere */





/* IGNORE STUFF BELOW ON PRIORITIES FOR EXERCISE 2 */

/* stuff to do with priorities.... MAX must exceed MIN, both must be ints
 *
 * All tasks have a static priority (determined when TCB is created) and
 * a dynamic priority, which can be adjusted at will. The scheduler may
 * read/modify/ignore the dynamic priorities at will. 
 */

#define MAX_PRIO  32
#define MIN_PRIO   0

int get_static_priority(TCB *t);

/* Only for use by the scheduler */
int  get_dynamic_priority(TCB *t);
void set_dynamic_priority(TCB *t, int pr); /* pr must be a legal priority */




#endif
