CFLAGS = -W -Wall -ggdb
CC = gcc

file_crawler: file_crawler.o ts_fifo.o re.o
	${CC} -o file_crawler ts_fifo.o file_crawler.o re.o ${CFLAGS}

ts_fifo_test: ts_fifo.o ts_fifo_test.o
	${CC} -o ts_fifo_test ts_fifo.o ts_fifo_test.o ${CFLAGS}

file_crawler.o: file_crawler.c ts_fifo.h re.h

ts_fifo_test.o: ts_fifo_test.c ts_fifo.h

ts_fifo.o: ts_fifo.c

tree.o: tree.c tree.h

tree_test.o: tree_test.c tree.h iterator.h

tree_test: tree_test.o tree.o iterator.o
	${CC} -o tree_test tree_test.o tree.o iterator.o ${CFLAGS}

iterator.o: iterator.c iterator.h

clean:
	rm -f *.o *.exe file_crawler ts_fifo_test tree_test
