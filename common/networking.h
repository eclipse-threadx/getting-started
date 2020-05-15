/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */
   
#ifndef _NETWORKING_C
#define _NETWORKING_C

#include <stdbool.h>

#include "nx_api.h"
#include "nxd_dns.h"

extern NX_PACKET_POOL main_pool;
extern NX_IP ip_0;
extern NX_DNS dns_client;

bool network_init(VOID (*ip_link_driver)(struct NX_IP_DRIVER_STRUCT *));

#endif // _NETWORKING_C