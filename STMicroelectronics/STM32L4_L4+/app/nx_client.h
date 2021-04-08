/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef _NX_CLIENT_H
#define _NX_CLIENT_H

#include "tx_api.h"
#include "nx_api.h"
#include "nxd_dns.h"
#include "az_ulib_result.h"

UINT azure_iot_nx_client_entry(
    NX_IP* ip_ptr, NX_PACKET_POOL* pool_ptr, NX_DNS* dns_ptr, UINT (*unix_time_callback)(ULONG* unix_time));

az_result dcf_ip_gateway_client_entry(
    NX_IP* ip_ptr, NX_PACKET_POOL* pool_ptr, NX_DNS* dns_ptr, UINT (*unix_time_callback)(ULONG* unix_time));

#endif // _NX_CLIENT_H
