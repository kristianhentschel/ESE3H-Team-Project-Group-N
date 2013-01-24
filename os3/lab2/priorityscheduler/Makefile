CFLAGS=-W -Wall -pedantic -g
OBJS= diagnostics.o generic_collection.o generic_queue.o tcb_collection.o TCB.o testscheduler.o

all: staticpriorityscheduler dynamicpriorityscheduler

staticpriorityscheduler: staticpriorityscheduler.o ${OBJS}
	gcc -Wall -g -o staticpriorityscheduler staticpriorityscheduler.o ${OBJS}

dynamicpriorityscheduler: dynamicpriorityscheduler.o ${OBJS}
	gcc -Wall -g -o dynamicpriorityscheduler dynamicpriorityscheduler.o ${OBJS}

clean:
	rm -f staticpriorityscheduler dynamicpriorityscheduler *.o
