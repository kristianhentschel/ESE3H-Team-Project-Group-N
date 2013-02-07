CFLAGS=-W -Wall -pedantic
OBJS=BackingStoreAndPhysicalMemoryEmulator.o diagnostics.o

all: testSimple testVM

testSimple: testVM.o simpleVM.o ${OBJS}
	gcc -Wall -g -o testSimple testVM.o simpleVM.o ${OBJS}

testVM: testVM.o VM.o ${OBJS}
	gcc -Wall -g -o testVM testVM.o VM.o ${OBJS}

clean:
	rm -f testSimple testVM *.o
