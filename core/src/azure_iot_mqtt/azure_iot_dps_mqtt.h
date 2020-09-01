/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef _AZURE_IOT_DPS_MQTT_H
#define _AZURE_IOT_DPS_MQTT_H

#include <stdbool.h>

#include "tx_api.h"

#include "nx_api.h"
#include "nxd_dns.h"
#include "nxd_mqtt_client.h"

#include "azure_iot_mqtt.h"

UINT azure_iot_dps_create(AZURE_IOT_MQTT* azure_iot_mqtt,
    NX_IP* nx_ip,
    NX_PACKET_POOL* nx_pool,
    NX_DNS* nx_dns,
    func_ptr_unix_time_get unix_time_get,
    CHAR* endpoint,
    CHAR* id_scope,
    CHAR* registration_id);

UINT azure_iot_dps_delete(AZURE_IOT_MQTT* azure_iot_mqtt);

UINT azure_iot_dps_symmetric_key_set(AZURE_IOT_MQTT* azure_iot_mqtt, CHAR* symmetric_key);

UINT azure_iot_dps_register(AZURE_IOT_MQTT* azure_iot_mqtt, UINT wait);

UINT azure_iot_dps_device_info_get(AZURE_IOT_MQTT* azure_iot_mqtt, CHAR* iothub_hostname, CHAR* device_id);

#endif