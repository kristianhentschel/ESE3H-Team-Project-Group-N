CFLAGS=-W -Wall -pedantic
OBJECTS=BoundedBuffer.o diagnostics.o fakeapplications.o \
        freepacketdescriptorstore.o generic_queue.o \
        networkdevice.o networkdriver.o packetdescriptor.o \
        packetdescriptorcreator.o testharness.o

demo: $(OBJECTS)
	cc -o demo $(OBJECTS) -lpthread

clean:
	rm -f *.o demo
