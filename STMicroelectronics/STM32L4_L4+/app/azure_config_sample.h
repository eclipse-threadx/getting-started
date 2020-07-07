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
// Define this variable to use the MQTT layer directly, 
// otherwise the Azure RTOS Embedded SDK is used.
// ----------------------------------------------------------------------------
//#define USE_MQTT

// ----------------------------------------------------------------------------
// Azure IoT Device information
// ----------------------------------------------------------------------------
#define IOT_HUB_HOSTNAME    "ryangsg.azure-devices.net"
#define IOT_DEVICE_ID       "stm32"
#define IOT_PRIMARY_KEY     "Df4x9mctXS0G3iqnC5xmQwOBeqV5mTIwN2TyffQhWzU="

#endif // _AZURE_CONFIG_H