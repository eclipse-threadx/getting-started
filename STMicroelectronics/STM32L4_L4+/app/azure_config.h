/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef _AZURE_CONFIG_H
#define _AZURE_CONFIG_H

typedef enum
{
    None         = 0,
    WEP          = 1,
    WPA_PSK_TKIP = 2,
    WPA2_PSK_AES = 3
} WiFi_Mode;

// ----------------------------------------------------------------------------
// WiFi connection config
// ----------------------------------------------------------------------------
#define WIFI_SSID     "iot_test"
#define WIFI_PASSWORD "iot_rulez"
#define WIFI_MODE     WPA2_PSK_AES

// ----------------------------------------------------------------------------
// Azure IoT Hub Connection Transport
// Define this to use the nx client, otherwise MQTT
// ----------------------------------------------------------------------------
//#define USE_NX_CLIENT_PREVIEW

// ----------------------------------------------------------------------------
// Azure IoT Dynamic Provisioning Service
// Define this to use the DPS service, otherwise direct IoT Hub
// ----------------------------------------------------------------------------
//#define ENABLE_DPS

// ----------------------------------------------------------------------------
// Azure IoT Hub config
// ----------------------------------------------------------------------------
#define IOT_HUB_HOSTNAME "ryansgsg.azure-devices.net"
#define IOT_DEVICE_ID    "stm"

// ----------------------------------------------------------------------------
// Azure IoT DPS config
// ----------------------------------------------------------------------------
#define IOT_DPS_ENDPOINT        "global.azure-devices-provisioning.net"
#define IOT_DPS_ID_SCOPE        "0ne00165610"
#define IOT_DPS_REGISTRATION_ID "stm"

// ----------------------------------------------------------------------------
// Azure IoT device config
// ----------------------------------------------------------------------------
#define IOT_PRIMARY_KEY "ZPR7kKgw/+KCuS/CQN1er7Etg7AZ8UEfKOv/5itGRKk="

#endif // _AZURE_CONFIG_H
