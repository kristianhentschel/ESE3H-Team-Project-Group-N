#ifndef FREE_PACKET_DESCRIPTOR_STORE_HDR
#define FREE_PACKET_DESCRIPTOR_STORE_HDR

/*
 * Author:    Peter Dickman
 * Version:   1.3
 * Last edit: 2003-02-18
 *
 * This file is a component of the test harness and or sample
 * solution to the NetworkDriver exercise (re)developed in February 2003
 * for use in assessing:
 *    the OS3 module in undergraduate Computing Science at dcs.gla.ac.uk
 *
 * It tests the ability to develop a small but complex software system
 * using PThreads to provide concurrency in C.
 *
 *
 * Code quality:
 * Quick hack - no guarantees or liability accepted.
 *
 * Copyright:
 * (c) 2003 University of Glasgow and Dr Peter Dickman
 *
 * Licencing: 
 * this software may not be used except with the author's permission.
 *
 */

#include "packetdescriptor.h"

typedef void * FreePacketDescriptorStore;

void blocking_get_pd(FreePacketDescriptorStore, PacketDescriptor *);
int  nonblocking_get_pd(FreePacketDescriptorStore, PacketDescriptor *);

void blocking_put_pd(FreePacketDescriptorStore, PacketDescriptor);
int  nonblocking_put_pd(FreePacketDescriptorStore, PacketDescriptor);

/* As usual, the blocking versions only return when they succeed. */
/* The nonblocking versions return 1 if they worked, 0 otherwise. */
/* The _get_ functions set their final arg if they succeed.       */

#endif
