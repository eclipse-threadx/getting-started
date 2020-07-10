/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef _AZURE_IOT_DPS_MQTT_H
#define _AZURE_IOT_DPS_MQTT_H

#include <stdbool.h>

#include "tx_api.h"

#include "nx_api.h"
#include "nxd_dns.h"
#include "nxd_mqtt_client.h"

typedef struct AZURE_IOT_DPS_MQTT_STRUCT
{
    NXD_MQTT_CLIENT nxd_mqtt_client;
    TX_MUTEX mqtt_mutex;
    
    UCHAR* mqtt_stack;

    NX_DNS* nx_dns;
    CHAR* dps_id_scope;
    CHAR* dps_endpoint;
    CHAR* device_registration_id;
    CHAR* device_sas_key;

} AZURE_IOT_DPS_MQTT;

UINT azure_iot_dps_create(AZURE_IOT_DPS_MQTT* dps_client_ptr,
    UCHAR* mqtt_stack,
    UINT mqtt_stack_size,
    NX_IP* nx_ip,
    NX_PACKET_POOL* nx_pool,
    NX_DNS* nx_dns,
    CHAR* endpoint,
    CHAR* id_scope,
    CHAR* registration_id);

UINT azure_iot_dps_delete(AZURE_IOT_DPS_MQTT* dps_client_ptr);

UINT azure_iot_dps_symmetric_key_set(AZURE_IOT_DPS_MQTT* dps_client_ptr, CHAR* symmetric_key);

UINT azure_iot_dps_register(AZURE_IOT_DPS_MQTT* dps_client_ptr, UINT wait);

UINT azure_iot_dps_device_info_get(AZURE_IOT_DPS_MQTT* dps_client_ptr, UCHAR* iothub_hostname, UCHAR* device_id);

#endif