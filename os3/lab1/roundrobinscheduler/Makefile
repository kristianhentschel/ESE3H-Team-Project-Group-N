CFLAGS=-W -Wall -pedantic
OBJS= diagnostics.o generic_collection.o generic_queue.o tcb_collection.o TCB.o testscheduler.o

scheduler: scheduler.o ${OBJS}
	gcc -Wall -g -o scheduler scheduler.o ${OBJS}

clean:
	rm -f scheduler *.o
