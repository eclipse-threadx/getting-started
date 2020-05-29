/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef _AZURE_CONFIG_H
#define _AZURE_CONFIG_H

/*************** Azure IoT Hub Connection Configuration ******************/
#define IOT_HUB_HOSTNAME    "azurertos-iot.azure-devices.net"
#define IOT_DEVICE_ID       "MIMXRT1060"
#define IOT_PRIMARY_KEY     "bZ5nAluSJlroefdTlSKpEiTg6SxtMgzTw9q7s1Pf1Tc="

// 0 - BME280 sensor is not present
// 1 - BME280 sensor is present
#define __SENSOR_BME280__ 1

#endif // _AZURE_CONFIG_H