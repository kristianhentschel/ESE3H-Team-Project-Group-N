CFLAGS = -W -Wall
CC = gcc

file_crawler: file_crawler.o ts_fifo.o re.o
	${CC} -o file_crawler ts_fifo.o file_crawler.o re.o ${CFLAGS}

ts_fifo_test: ts_fifo.o ts_fifo_test.o
	${CC} -o ts_fifo_test ts_fifo.o ts_fifo_test.o ${CFLAGS}

file_crawler.o: file_crawler.c ts_fifo.h re.h

ts_fifo_test.o: ts_fifo_test.c ts_fifo.h

ts_fifo.o: ts_fifo.c


clean:
	rm -f *.o *.exe file_crawler ts_fifo_test
