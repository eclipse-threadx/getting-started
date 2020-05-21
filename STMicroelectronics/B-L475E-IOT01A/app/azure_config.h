/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */
   
#ifndef _AZURE_CONFIG_H
#define _AZURE_CONFIG_H

typedef enum
{
    None          = 0, 
    WEP           = 1,
    WPA_PSK_TKIP  = 2,
    WPA2_PSK_AES  = 3
} WiFi_Mode;

// ----------------------------------------------------------------------------
// WiFi connection information
// ----------------------------------------------------------------------------
#define WIFI_SSID           "ferret"
#define WIFI_PASSWORD       "i am a furry animal"
#define WIFI_MODE           WPA2_PSK_AES

// ----------------------------------------------------------------------------
// Azure IoT Device information
// ----------------------------------------------------------------------------
#define IOT_HUB_HOSTNAME    "ryantest2.azure-devices.net"
#define IOT_DEVICE_ID       "stm"
#define IOT_PRIMARY_KEY     "eNVewYP9j627+BPsoqxh9NAH+0zyePBXRW84TS10YpE="

#endif // _AZURE_CONFIG_H
