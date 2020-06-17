/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */
   
#ifndef _AZURE_CONFIG_H
#define _AZURE_CONFIG_H

#include "wwd_constants.h"

// ----------------------------------------------------------------------------
// WiFi connection information
// ----------------------------------------------------------------------------
#define WIFI_SSID           "FRITZ!Box 6490 Cable"

#define WIFI_PASSWORD       "42732488687857862742"

/* WIFI Security, the security types are defined in wwwd_constants.h.  */
#define WIFI_SECURITY       WICED_SECURITY_WPA2_MIXED_PSK
/* Country codes are defined in wwd_constants.h.  */
#define WIFI_COUNTRY        WICED_COUNTRY_GERMANY

// ----------------------------------------------------------------------------
// Azure IoT Device information
// ----------------------------------------------------------------------------
#define IOT_HUB_HOSTNAME    "azurertos-test.azure-devices.net"
#define IOT_DEVICE_ID       "bhnaphad-test"
#define IOT_PRIMARY_KEY     "gbH78eY+2ovz9BeGGt308bpJ0WS7+jG3HGJR6NMNjPU="

#endif // _AZURE_CONFIG_H
