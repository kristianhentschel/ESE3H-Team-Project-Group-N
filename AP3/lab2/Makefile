CFLAGS=-W -Wall -ggdb
CC=gcc

all: qtestPrioHeap qtestLIFO qtestFIFO

qtestPrioHeap: qtest.o queuePrioHeap.o
	gcc -o qtestPrioHeap qtest.o queuePrioHeap.o

queuePrioHeap.o: queue.h queuePrioHeap.c

qtestPrioMultiLL: qtest.o queuePrioMultiLL.o
	gcc -o qtestPrioMultiLL qtest.o queuePrioMultiLL.o

queuePrioLL.o: queue.h queuePrioLL.c

qtestPrioLL: qtest.o queuePrioLL.o
	gcc -o qtestPrioLL qtest.o queuePrioLL.o

queuePrioLL.o: queue.h queuePrioLL.c

qtestLIFO: qtest.o queueLIFO.o
	gcc -o qtestLIFO qtest.o queueLIFO.o

queueLIFO.o: queue.h queueLIFO.c

qtestFIFO: qtest.o queueFIFO.o
	gcc -o qtestFIFO qtest.o queueFIFO.o

queueFIFO.o: queue.h queueFIFO.c

qtest.o: queue.h qtest.c

clean:
	rm -rf *.o qtestFIFO qtestLIFO qtestPrioHeap qtestPrioLL
