# Author: Kristian Hentschel / 1003734h
# Advanced Programming 3, Assessed Exercise AX1
# Makefile TODO may be replaced by version provided by Joe Sventek

CC = gcc
CFLAGS = -W -Wall -ggdb

all: finddupl findduplLL findduplRS

finddupl: finddupl.o mlist.o mentry.o
	$(CC) $(CFLAGS) -o finddupl finddupl.o mlist.o mentry.o

findduplRS: finddupl.o mlistRS.o mentry.o
	$(CC) $(CFLAGS) -o findduplRS finddupl.o mlistRS.o mentry.o

findduplLL: finddupl.o mlistLL.o mentry.o
	$(CC) $(CFLAGS) -o findduplLL finddupl.o mlistLL.o mentry.o

finddupl.o: finddupl.c mlist.h mentry.h

mentry.o: mentry.c mentry.h

mlist.o: mlist.c mlist.h mentry.h

mlistLL.o: mlistLL.c mlist.h mentry.h

mlistRS.o: mlistRS.c mlist.h mentry.h

drive_mentry: drive_mentry.o mentry.o

drive_mentry.o: mentry.h mentry.c

clean:
	rm -f *.o finddupl findduplLL findduplRS drive_mentry 
