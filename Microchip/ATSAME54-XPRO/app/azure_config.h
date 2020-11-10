/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef _AZURE_CONFIG_H
#define _AZURE_CONFIG_H

// ----------------------------------------------------------------------------
// 0 - BME280 sensor is not present
// 1 - BME280 sensor is present
// ----------------------------------------------------------------------------
#define __SENSOR_BME280__ 0

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
// Azure IoT DPS Self-Signed X509Certificate
// Define this to connect to DPS or Iot Hub using a self-signed X509 certificate
// ----------------------------------------------------------------------------
// #define ENABLE_X509

// ----------------------------------------------------------------------------
// Azure IoT Hub config
// ----------------------------------------------------------------------------
#define IOT_HUB_HOSTNAME ""
#define IOT_DEVICE_ID    ""
#define IOT_PRIMARY_KEY  ""

// ----------------------------------------------------------------------------
// Azure IoT DPS config
// ----------------------------------------------------------------------------
#define IOT_DPS_ENDPOINT        "global.azure-devices-provisioning.net"
#define IOT_DPS_ID_SCOPE        ""
#define IOT_DPS_REGISTRATION_ID ""

#endif // _AZURE_CONFIG_H
