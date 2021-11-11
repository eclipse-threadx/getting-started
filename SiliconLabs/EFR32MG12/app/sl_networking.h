/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef SL_NETWORKING_H
#define SL_NETWORKING_H

#include "nx_api.h"
#include "nxd_dns.h"

#include "azure_config.h"

extern NX_IP nx_ip;
extern NX_PACKET_POOL nx_pool;
extern NX_DNS nx_dns_client;

UINT network_init(VOID (*ip_link_driver)(struct NX_IP_DRIVER_STRUCT*));

#endif
