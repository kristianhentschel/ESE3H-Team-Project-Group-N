CFLAGS = -g -W -Wall

all: zb_test

zb_test: zb_test.o zigbee.o
	gcc ${CFLAGS} -o zb_test -lpthread zb_test.o zigbee.o

zb_test.o: zb_test.c

zigbee.o: zigbee.c

clean:
	rm -f zb_test *.o
