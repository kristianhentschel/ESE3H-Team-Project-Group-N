#ifndef _QUEUE_INCLUDED
#define _QUEUE_INCLUDED

typedef struct queue Queue;
typedef void *Item;

#define MIN_PRIO 10
#define MAX_PRIO 1

/*
 * create a Queue that holds Items
 * returns NULL if the create call failed (malloc failure)
 */
Queue *q_create(void);
/*
 * add Item to the Queue; 3rd arg is priority in MIN_PRIO..MAX_PRIO;
 * return 1/0 if successful/not-successful
 */
int q_add(Queue *q, Item i, int prio);
/*
 * remove next Item from queue; returns NULL if queue is empty
 */
Item q_remove(Queue *q);

#endif /* _QUEUE_INCLUDED */
