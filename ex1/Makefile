# Author: Kristian Hentschel / 1003734h
# Advanced Programming 3, Assessed Exercise AX1
# Makefile TODO may be replaced by version provided by Joe Sventek

CC = gcc
CFLAGS = -W -Wall

all: finddupl findduplLL

finddupl: finddupl.o mlist.o mentry.o

findduplLL: finddupl.o mlistLL.o mentry.o

finddupl.o: finddupl.c mlist.h mentry.h

mentry.o: mentry.c mentry.h

mlist.o: mlist.c mlist.h mentry.h

mlistLL.o: mlistLL.c mlist.h mentry.h

clean:
	rm -f *.o finddupl findduplLL
