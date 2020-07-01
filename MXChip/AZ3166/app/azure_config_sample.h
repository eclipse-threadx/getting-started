/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */
   
#ifndef _AZURE_CONFIG_H
#define _AZURE_CONFIG_H

#include "wwd_constants.h"

// ----------------------------------------------------------------------------
// WiFi connection information
// ----------------------------------------------------------------------------
#define WIFI_SSID           "iot_test"
#define WIFI_PASSWORD       "iot_rulez"

// WIFI Security, the security types are defined in wwd_constants.h
#define WIFI_SECURITY       WICED_SECURITY_WPA2_MIXED_PSK
// Country codes are defined in wwd_constants.h
#define WIFI_COUNTRY        WICED_COUNTRY_CHINA

// ----------------------------------------------------------------------------
// Azure IoT Device information
// ----------------------------------------------------------------------------
#define IOT_HUB_HOSTNAME    "ryangsg.azure-devices.net"
#define IOT_DEVICE_ID       "mxchip"
#define IOT_PRIMARY_KEY     "6v4WRLRV0od3oJo+So+PCP54TxoS12ZGiM/myyRm6PY="

#endif // _AZURE_CONFIG_H
