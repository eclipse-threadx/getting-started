/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef _MQTT_H
#define _MQTT_H

#include "nx_api.h"
#include "tx_api.h"

UINT azure_iot_embedded_sdk_entry(
    NX_IP* ip_ptr, NX_PACKET_POOL* pool_ptr, NX_DNS* dns_ptr, UINT (*unix_time_callback)(ULONG* unix_time));

#endif // _MQTT_H
