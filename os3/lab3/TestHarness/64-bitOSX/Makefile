CFLAGS=-W -Wall -pedantic
OBJS= diagnostics.o FSM.o

testFSM: testFSM.o ${OBJS}
	gcc -Wall -g -o testFSM testFSM.o ${OBJS}

clean:
	rm -f testFSM *.o
