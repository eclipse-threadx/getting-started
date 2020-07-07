/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */
   
#ifndef _AZURE_CONFIG_H
#define _AZURE_CONFIG_H

#include "wwd_constants.h"

// ----------------------------------------------------------------------------
// WiFi connection information
// ----------------------------------------------------------------------------
#define WIFI_SSID           ""
#define WIFI_PASSWORD       ""

// WIFI Security as defined in wwd_constants.h
#define WIFI_SECURITY       WICED_SECURITY_WPA2_MIXED_PSK
// Country codes as defined in wwd_constants.h
#define WIFI_COUNTRY        WICED_COUNTRY_CHINA

// ----------------------------------------------------------------------------
// Azure IoT Hub Connection Transport
// Define this to use the nx client, otherwise MQTT
// ----------------------------------------------------------------------------
//#define USE_NX_CLIENT_PREVIEW

// ----------------------------------------------------------------------------
// Azure IoT Device information
// ----------------------------------------------------------------------------
#define IOT_HUB_HOSTNAME    ""
#define IOT_DEVICE_ID       ""
#define IOT_PRIMARY_KEY     ""

#endif // _AZURE_CONFIG_H
