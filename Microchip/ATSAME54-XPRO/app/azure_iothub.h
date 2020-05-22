/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */
   
#ifndef _AZURE_IOTHUB_H
#define _AZURE_IOTHUB_H

#include "tx_api.h"

UINT azure_iothub_run(CHAR *iot_hub_hostname, CHAR *iot_device_id, CHAR *iot_sas_key);

#endif // _AZURE_IOTHUB_H