CFLAGS = -W -Wextra -Wall -Wconversion -pedantic -g
CC = gcc

all: udp_hello_server udp_hello_client 

udp_hello_server: udp_hello_server.o

udp_hello_client: udp_hello_client.o

clean:
	rm -f udp_hello_server udp_hello_client *.o
