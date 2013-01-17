#include "scheduler.h"
#include "generic_queue.h"
#include "stdio.h"

#define IDLE 1
#define RUNNING 2
#define INIT 3

/* holds TCBs of tasks ready to run */
static GQueue *ready_queue;

/* holds TCB of currently running task */
static TCB *current_task;

static int scheduler_state;

/* utility methods */
static void schedule_task(void);


/* initialise ready queue data structure and idle task */
void init(void) {
	if((ready_queue = create_gqueue()) == NULL) {
		fprintf(stderr, "Could not create generic queue\n");
		/* TODO error handling */
	}
	scheduler_state = INIT;
}

/* add new tcb to ready queue
 * if idle, immediately call the scheduler. */
void add_runnable_TCB(TCB *t) {
	gqueue_enqueue(ready_queue, (GQueueElement) t);
	if (scheduler_state == IDLE) {
		scheduler();
	}
}

/* current task blocked
 * unload current task
 * add current task to blocked queue 
 * call schedule_task to schedule next one */
void block(void) {
	move_from_CPU_to_TCB(current_task);
	add_blocked_TCB(current_task);
	schedule_task();
}

/* timeslice expired.
 * unload current task and schedule next one off the ready queue */
void scheduler(void) {
	if (scheduler_state == RUNNING) {
		move_from_CPU_to_TCB(current_task);
		gqueue_enqueue(ready_queue, (GQueueElement) current_task);
	}
	schedule_task();
}

/* schedule next task from the run queue.
 * assume no task currently running
 * if no task is ready, schedule the idle task. */
static void schedule_task(void) {
	if(gqueue_length(ready_queue) == 0) {
		scheduler_state = IDLE;
		current_task = get_idle_task();
	} else {
		scheduler_state = RUNNING;
		gqueue_dequeue(ready_queue, (GQueueElement *) &current_task);
	}

	move_from_TCB_to_CPU(current_task);
}

