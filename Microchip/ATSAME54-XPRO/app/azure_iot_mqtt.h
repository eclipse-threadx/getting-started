/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */
   
#ifndef _AZURE_IOT_MQTT_H
#define _AZURE_IOT_MQTT_H

#include "tx_api.h"

UINT azure_iot_mqtt_run(CHAR *iot_hub_hostname, CHAR *iot_device_id, CHAR *iot_sas_key);

#endif // _AZURE_IOT_MQTT_H