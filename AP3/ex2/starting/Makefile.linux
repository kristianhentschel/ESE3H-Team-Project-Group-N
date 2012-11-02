CFLAGS=-W -Wall -g -pedantic

fileCrawler: fileCrawler.o libADTS.a
	gcc $(CFLAGS) -o fileCrawler fileCrawler.o libADTS.a

libADTS.a: iterator.o linkedlist.o treeset.o re.o
	rm -f libADTS.a
	ar r libADTS.a iterator.o linkedlist.o treeset.o re.o
	ranlib libADTS.a

iterator.o: iterator.c iterator.h
linkedlist.o: linkedlist.c linkedlist.h iterator.h
treeset.o: treeset.c treeset.h iterator.h
re.o: re.c re.h
fileCrawler.o: fileCrawler.c iterator.h linkedlist.h treeset.h re.h
