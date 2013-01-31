#include "scheduler.h"
#include "generic_queue.h"
#include "stdio.h"

static TCB *current_task;
static TCB *idle_task;

static GQueue *ready_queues[MAX_PRIO - MIN_PRIO + 1];

static enum {IDLE, RUNNING} state;


void dispatch_task(void);



/* initialize scheduler data structures */
void init(void) {
	int i;
	
	state = IDLE;

	for (i = 0; i <= MAX_PRIO - MIN_PRIO; i++) {
		if ((ready_queues[MIN_PRIO + i] = create_gqueue()) == NULL) {
			fprintf(stderr, "Could not create generic queue\n");
			/* TODO error handling */
		}
	}

	fprintf(stderr, "Scheduler: Initialised %d ready queues\n", i);

	idle_task = get_idle_task();
}

/* introduce a runnable TCB to the system.
 * store it in the queue corresponding to its priority */
void add_runnable_TCB(TCB *t) {
	int index;

	index = MIN_PRIO + get_static_priority(t);

	gqueue_enqueue(ready_queues[index], (GQueueElement) t);
	
	/* system is idle and the new task could  be scheduled immediately. */
	/* TODO task with higher priority should preempt running task. */
	if (state == IDLE || get_static_priority(t) > get_static_priority(current_task)) {
		scheduler();
	}
	fprintf(stderr, "Scheduler: Added runnable TCB to queue %d\n", index);
}

/* called when current_task is blocked.
 * unload task
 * add to blocked queue
 * schedule next task
 */
void block(void) {
	move_from_CPU_to_TCB(current_task);
	add_blocked_TCB(current_task);
	dispatch_task();
}
/* time slice expired. */
void scheduler(void) {
	switch(state) {
		case RUNNING:
			move_from_CPU_to_TCB(current_task);
			add_runnable_TCB(current_task);
			break;
		case IDLE:
			move_from_CPU_to_TCB(idle_task);
			break;
	}
	dispatch_task();
}

/* select highest priority task from queue or idle task.
 * move its TCB to the CPU.
 * assume no thread is currently loaded.
 */
void dispatch_task(void) {
	int i;

	for (i = 0; i <= MIN_PRIO + MAX_PRIO; i++) {
		int index = MIN_PRIO + i;
		if (gqueue_length(ready_queues[index]) > 0) {
			gqueue_dequeue(ready_queues[index], (GQueueElement *) &current_task);
			fprintf(stderr, "Scheduler: Dispatching task with priority %d\n", get_static_priority(current_task));
			break;
		}
	}

	if (i == MIN_PRIO + MAX_PRIO + 1) {
		current_task = idle_task;
		state = IDLE;
		fprintf(stderr, "Scheduler: Dispatching idle task\n");
	} else {
		state = RUNNING;
	}

	move_from_TCB_to_CPU( current_task );
}
