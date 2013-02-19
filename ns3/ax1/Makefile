CFLAGS = -W -Wall -pedantic -g
CC = gcc

all: server

server: server.o linkedstringbuffer.o threadpool.o 
	${CC} -lpthread ${CFLAGS} server.o linkedstringbuffer.o threadpool.o -o server

server.o: server.c linkedstringbuffer.h networking.h threadpool.h

linkedstringbuffer.o: linkedstringbuffer.c linkedstringbuffer.h

threadpool.o: threadpool.c threadpool.c

clean:
	rm -f server *.exe *.o
