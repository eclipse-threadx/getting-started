/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef _NX_CLIENT_H
#define _NX_CLIENT_H

#include "tx_api.h"
#include "nx_api.h"
#include "nxd_dns.h"
#include "device_config.h"

UINT azure_iot_nx_client_entry(
    NX_IP* ip_ptr, NX_PACKET_POOL* pool_ptr, NX_DNS* dns_ptr, UINT (*unix_time_callback)(ULONG* unix_time), Device_Config_Info_t* device_info);

#endif // _NX_CLIENT_H
