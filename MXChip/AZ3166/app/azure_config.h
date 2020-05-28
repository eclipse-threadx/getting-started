/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */
   
#ifndef _AZURE_CONFIG_H
#define _AZURE_CONFIG_H

#include "wwd_constants.h"

// ----------------------------------------------------------------------------
// WiFi connection information
// ----------------------------------------------------------------------------
#define WIFI_SSID           "graceyou"

#define WIFI_PASSWORD       "yjy64161551"

/* WIFI Security, the security types are defined in wwwd_constants.h.  */
#define WIFI_SECURITY       WICED_SECURITY_WPA2_MIXED_PSK
/* Country codes are defined in wwwd_constants.h.  */
#define WIFI_COUNTRY        WICED_COUNTRY_CHINA

// ----------------------------------------------------------------------------
// Azure IoT Device information
// ----------------------------------------------------------------------------
#define IOT_HUB_HOSTNAME    "threadx-adu.azure-devices.net"
#define IOT_DEVICE_ID       "liydu-testdevice"
#define IOT_PRIMARY_KEY     "isgOAjBeclXpxLiVG5UeTH5boCIpJBdsfDR5192KGyg="

#endif // _AZURE_CONFIG_H
