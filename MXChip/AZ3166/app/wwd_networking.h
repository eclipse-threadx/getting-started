/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef _WWD_NETWORKING_H
#define _WWD_NETWORKING_H

#include "nx_api.h"
#include "nxd_dns.h"

#include "azure_config.h"

extern NX_PACKET_POOL nx_pool[2]; /* 0=TX, 1=RX. */
extern NX_IP nx_ip;
extern NX_DNS nx_dns_client;

int platform_init(CHAR* ssid, CHAR* password, WiFi_Mode mode);

#endif // _WWD_NETWORKING_H
