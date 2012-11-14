CFLAGS = -g -W -Wall
CC = gcc

all: primes

primes: primes.o isprime.o

primes.o: primes.c isprime.h

mtprimes: mtprimes.o isprime.o

mtprimes.o: mtprimes.c isprime.h

clean:
	rm -f primes mtprimes *.o
