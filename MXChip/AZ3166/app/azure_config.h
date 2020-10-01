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
// WiFi connection information
// ----------------------------------------------------------------------------
#define WIFI_SSID     "Spencer MSFT PC"
#define WIFI_PASSWORD "@azureiot"
#define WIFI_MODE     WPA2_PSK_AES

// ----------------------------------------------------------------------------
// Azure IoT Hub Connection Transport
// Define this to use the nx client, otherwise MQTT
// ----------------------------------------------------------------------------
#define USE_NX_CLIENT_PREVIEW

// ----------------------------------------------------------------------------
// Azure IoT Dynamic Provisioning Service
// Define this to use the DPS service, otherwise direct IoT Hub
// ----------------------------------------------------------------------------
#define ENABLE_DPS

// ----------------------------------------------------------------------------
// Azure IoT Hub config
// ----------------------------------------------------------------------------
#define IOT_HUB_HOSTNAME "gettingstarted.azure-devices.net"
#define IOT_DEVICE_ID    "gsg_test_device"
#define IOT_PRIMARY_KEY  "Chyz57v9QXOvinUBHbOviNsHnzmD5wc/+SfWa29d89yDdl5ZsnajEL7BrFDHeKYL5TmZMwlIphmTCtm6GgoSgg=="

// ----------------------------------------------------------------------------
// Azure IoT DPS config
// ----------------------------------------------------------------------------
#define IOT_DPS_ENDPOINT        "global.azure-devices-provisioning.net"
#define IOT_DPS_ID_SCOPE        "0ne00191969"
#define IOT_DPS_REGISTRATION_ID "global.azure-devices-provisioning.net"

// ----------------------------------------------------------------------------
// Azure IoT DPS X509 config
// ----------------------------------------------------------------------------
#define ENABLE_X509

#endif // _AZURE_CONFIG_H
