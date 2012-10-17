CFLAGS=-W -Wall -g
CC=gcc

APPS = echo find

all: ${APPS}

echo: echo.o
	${CC} ${CFLAGS} -o echo echo.o

find: findmain.o
	${CC} ${CFLAGS} -o find findmain.o

findmain.o: findmain.c

echo.o: echo.c

clean:
	rm -rf *.o ${APPS}

