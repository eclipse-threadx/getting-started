/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef _RX_NETWORKING_H
#define _RX_NETWORKING_H

#include "nx_api.h"
#include "nxd_dns.h"

#include "azure_config.h"

extern NX_IP          nx_ip;
extern NX_PACKET_POOL nx_pool;
extern NX_DNS         nx_dns_client;

int rx_network_init(CHAR* ssid, CHAR* password, WiFi_Mode mode);

#endif // _RX_NETWORKING_H