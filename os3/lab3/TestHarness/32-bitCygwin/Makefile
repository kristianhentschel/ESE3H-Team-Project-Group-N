CFLAGS=-W -Wall -pedantic
OBJS= diagnostics.o FSM.o

testFSM.exe: testFSM.o ${OBJS}
	gcc -Wall -g -o testFSM.exe testFSM.o ${OBJS}

clean:
	rm -f testFSM.exe *.o
