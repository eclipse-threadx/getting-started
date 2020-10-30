/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef _AZURE_CONFIG_H
#define _AZURE_CONFIG_H

// ----------------------------------------------------------------------------
// 0 - BME280 sensor is not present
// 1 - BME280 sensor is present
// ----------------------------------------------------------------------------
#define __SENSOR_BME280__ 1

// ----------------------------------------------------------------------------
// Azure IoT Hub Connection Transport
// Define to use the legacy MQTT connection, else Azure RTOS SDK for Azure IoT
// ----------------------------------------------------------------------------
//#define ENABLE_LEGACY_MQTT

// ----------------------------------------------------------------------------
// Azure IoT Dynamic Provisioning Service
// Define this to use the DPS service, otherwise direct IoT Hub
// ----------------------------------------------------------------------------
#define ENABLE_DPS

// ----------------------------------------------------------------------------
// Azure IoT Hub config
// ----------------------------------------------------------------------------
#define IOT_HUB_HOSTNAME "getting-started-guide-iot-hub.azure-devices.net"
#define IOT_DEVICE_ID    "spmcdono-device"
#define IOT_PRIMARY_KEY  "v6Ht1YZZeB9VOC6UWQFkkiSmzYTRQNBzWptIdSmM/OAiRu+jIUKdHKb0xt9nuQsumFf4WUlOqNqhDPTiEmRjrg=="
// #define IOT_PRIMARY_KEY  ""

// ----------------------------------------------------------------------------
// Azure IoT DPS config
// ----------------------------------------------------------------------------
#define IOT_DPS_ENDPOINT        "global.azure-devices-provisioning.net"
#define IOT_DPS_ID_SCOPE        "0ne0019BD5E"
#define IOT_DPS_REGISTRATION_ID "global.azure-devices-provisioning.net"

// ----------------------------------------------------------------------------
// Azure IoT DPS X509 config
// ----------------------------------------------------------------------------
#define ENABLE_X509

#endif // _AZURE_CONFIG_H
