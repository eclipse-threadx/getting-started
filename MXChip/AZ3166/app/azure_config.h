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
#define WIFI_SSID     "Daddy Leroy"
#define WIFI_PASSWORD "leroyjenkins"
#define WIFI_MODE     WPA2_PSK_AES

// ----------------------------------------------------------------------------
// Azure IoT Hub Connection Transport
// Define to use the legacy MQTT connection, else Azure RTOS SDK for Azure IoT
// ----------------------------------------------------------------------------
//#define ENABLE_LEGACY_MQTT

// ----------------------------------------------------------------------------
// Azure IoT Dynamic Provisioning Service
// Define this to use the DPS service, otherwise direct IoT Hub
// ----------------------------------------------------------------------------
// #define ENABLE_DPS

// ----------------------------------------------------------------------------
// Azure IoT DPS X509 config
// Define this to connect to DPS or Iot Hub using a self-signed X509 certificate
// ----------------------------------------------------------------------------
#define ENABLE_X509

// ----------------------------------------------------------------------------
// Azure IoT Hub config
// ----------------------------------------------------------------------------
#define IOT_HUB_HOSTNAME "spmcdono-iothub.azure-devices.net"
#define IOT_DEVICE_ID    "spmcdono-device-x509-iothub"
#define IOT_PRIMARY_KEY  "KFia2aLcnmICm+hT+O3mX2iLjvdtZvV9s6N9XBiYLTY="

// ----------------------------------------------------------------------------
// Azure IoT DPS config
// ----------------------------------------------------------------------------
#define IOT_DPS_ENDPOINT        "global.azure-devices-provisioning.net"
#define IOT_DPS_ID_SCOPE        "0ne0019BD5E"
#define IOT_DPS_REGISTRATION_ID "global.azure-devices-provisioning.net"

#endif // _AZURE_CONFIG_H
