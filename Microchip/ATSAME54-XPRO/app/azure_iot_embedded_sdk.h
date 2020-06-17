/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef _AZURE_IOT_EMBEDDED_SDK_H
#define _AZURE_IOT_EMBEDDED_SDK_H

#include "nx_api.h"
#include "tx_api.h"

UINT azure_iot_embedded_sdk_entry(
    NX_IP* ip_ptr, NX_PACKET_POOL* pool_ptr, NX_DNS* dns_ptr, UINT (*unix_time_callback)(ULONG* unix_time));

#endif // _AZURE_IOT_EMBEDDED_SDK_H
