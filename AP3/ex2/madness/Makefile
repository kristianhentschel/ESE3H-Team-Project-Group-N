CFLAGS = -W -Wall

file_crawler: file_crawler.o ts_fifo.o re.o
	gcc -o file_crawler ts_fifo.o file_crawler.o re.o ${CFLAGS}

ts_fifo_test: ts_fifo.o ts_fifo_test.o
	gcc -o ts_fifo_test ts_fifo.o ts_fifo_test.o ${CFLAGS}

file_crawler.o: file_crawler.c ts_fifo.h re.h

ts_fifo_test.o: ts_fifo_test.c ts_fifo.h

ts_fifo.o: ts_fifo.c


clean:
	rm -f *.o *.exe file_crawler ts_fifo_test
