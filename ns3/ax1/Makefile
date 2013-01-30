CFLAGS = -W -Wall -g
CC = gcc

all: server

server: server.o linkedstringbuffer.o networking.h
	${CC} ${CFLAGS} server.o linkedstringbuffer.o -o server

linkedstringbuffer.o: linkedstringbuffer.c linkedstringbuffer.h

clean:
	rm -f server *.exe *.o
