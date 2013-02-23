#ifndef NETWORK_DEVICE_HDRS
#define NETWORK_DEVICE_HDRS

/*
 * Author:    Peter Dickman
 * Version:   1.4
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


#include "packetdescriptor.h"
#include "destination.h"
#include "pid.h"

typedef  void * NetworkDevice;

int send_packet(NetworkDevice, PacketDescriptor);
/* Returns 1 if successful, 0 if unsuccessful */
/* May take a substantial time to return      */
/* If unsuccessful you can try again, but if you fail repeatedly give */
/* up and just accept that this packet cannot be sent for some reason */

void register_receiving_packetdescriptor(NetworkDevice, PacketDescriptor*);
/* tell the network device to use the indicated PacketDescriptor     */
/* for next incoming data packet; once a descriptor is used it won't */
/* be reused for a further incoming data packet; you must register   */
/* another PacketDescriptor before the next packet arrives           */

void await_incoming_packet(NetworkDevice);
/* The thread blocks until the registered PacketDescriptor has been   */
/* filled with an incoming data packet. The PId field of the packet   */
/* indicates the local application process which should be given it.  */
/* This should be called as soon as possible after the previous       */
/* thread to wait for a packet returns. Only 1 thread may be waiting. */

#endif
