#ifndef PACKET_DESCRIPTOR_HDR
#define PACKET_DESCRIPTOR_HDR

/*
 * Author:    Peter Dickman
 * Version:   1.3
 * Last edit: 2003-02-25
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

#include "destination.h"
#include "pid.h"

typedef void * PacketDescriptor;

#define SIZEOF_PacketDescriptor_target   64
/* So a packet descriptor should be a pointer to 64 bytes of memory */

void init_packet_descriptor(PacketDescriptor *);
/* Resets the packet descriptor to be empty.        */
/* Should be used before registering a descriptor   */
/* with the NetworkDevice.                          */

/* The next few functions are used to set and get the destination info     */
/* Packets are routed to a Destination and then to the indicated PID there */

void packet_descriptor_set_pid(PacketDescriptor *, PID);
void packet_descriptor_set_destination(PacketDescriptor *, Destination);

PID         packet_descriptor_get_pid(PacketDescriptor *);
Destination packet_descriptor_get_destination(PacketDescriptor *);

/* Routines for manipulating the actual data are omitted, we don't bother */
/* sending actual data in this testharness.........                       */

#endif
