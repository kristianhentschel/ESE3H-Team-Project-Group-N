CFLAGS = -W -Wextra -Wall -Wconversion -pedantic -g
CC = gcc

all: server 

server: server.o linkedstringbuffer.o bufferthreadpool.o 
	${CC} -lpthread ${CFLAGS} server.o linkedstringbuffer.o bufferthreadpool.o -o server

server.o: server.c linkedstringbuffer.h networking.h threadpool.h

linkedstringbuffer.o: linkedstringbuffer.c linkedstringbuffer.h

bufferthreadpol.o: bufferthreadpool.c threadpool.h

clean:
	rm -f server *.o
