/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */
   
#ifndef _RTOS_IOT_H
#define _RTOS_IOT_H

#include "tx_api.h"

UINT rtos_iot_run(CHAR *iot_hub_hostname, CHAR *iot_device_id, CHAR *iot_sas_key);

#endif // _RTOS_IOT_H