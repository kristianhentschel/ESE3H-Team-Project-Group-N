CFLAGS = -W -Wall -pedantic -g
CC = gcc

all: master_test scale_test 

master_test: master_test.o zb_packets_api.o zb_transport_tty.o master_requesthandlers.o
	gcc -o master_test -lpthread master_test.o zb_packets_api.o zb_transport_tty.o master_requesthandlers.o

scale_test: scale_test.o zb_packets_api.o zb_transport_tty.o
	gcc -o scale_test -lpthread scale_test.o zb_packets_api.o zb_transport_tty.o

clean:
	rm -f *.o master_test scale_test scale_test_tty
