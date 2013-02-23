#ifndef FREE_PACKET_DESCRIPTOR_STORE__FULL_HDR
#define FREE_PACKET_DESCRIPTOR_STORE__FULL_HDR

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

#include "freepacketdescriptorstore.h"

/* This header adds the create/destroy calls to the specified behaviour */

FreePacketDescriptorStore create_fpds(void);
void destroy_fpds(FreePacketDescriptorStore);

#endif
