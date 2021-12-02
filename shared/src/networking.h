/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef _NETWORKING_H
#define _NETWORKING_H

#include "nx_api.h"
#include "nxd_dns.h"

extern NX_IP nx_ip;
extern NX_PACKET_POOL nx_pool;
extern NX_DNS nx_dns_client;

UINT network_init(VOID (*ip_link_driver)(struct NX_IP_DRIVER_STRUCT*));
UINT network_connect();

#endif // _NETWORKING_H