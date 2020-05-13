#ifndef _AZURE_IOTHUB_H
#define _AZURE_IOTHUB_H

#include <stdio.h>

#include "tx_api.h"

#include "azure/azure_mqtt.h"
#include "board_init.h"

bool azure_mqtt_init(CHAR *iot_hub_hostname, CHAR *iot_device_id, CHAR *iot_sas_key);

#endif // _AZURE_IOTHUB_H