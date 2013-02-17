CFLAGS = -W -Wall -pedantic -lpthread -g
CC = gcc

all: server

server: server.o linkedstringbuffer.o workerthreadpool.o 
	${CC} ${CFLAGS} server.o linkedstringbuffer.o workerthreadpool.o -o server

server.o: server.c linkedstringbuffer.h networking.h workerthreadpool.h

linkedstringbuffer.o: linkedstringbuffer.c linkedstringbuffer.h

workerthreadpool.o: workerthreadpool.c workerthreadpool.c

clean:
	rm -f server *.exe *.o
