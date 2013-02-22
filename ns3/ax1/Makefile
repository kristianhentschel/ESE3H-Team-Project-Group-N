CFLAGS = -W -Wall -pedantic -g
CC = gcc

all: server buf_server

server: server.o linkedstringbuffer.o threadpool.o 
	${CC} -lpthread ${CFLAGS} server.o linkedstringbuffer.o threadpool.o -o server

buf_server: server.o linkedstringbuffer.o bufferthreadpool.o 
	${CC} -lpthread ${CFLAGS} server.o linkedstringbuffer.o bufferthreadpool.o -o buf_server

server.o: server.c linkedstringbuffer.h networking.h threadpool.h

linkedstringbuffer.o: linkedstringbuffer.c linkedstringbuffer.h

threadpool.o: threadpool.c threadpool.h

bufferthreadpol.o: bufferthreadpool.c bufferthreadpool.h

clean:
	rm -f server *.exe *.o
