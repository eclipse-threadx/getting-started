/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef _AZURE_CONFIG_H
#define _AZURE_CONFIG_H

/*************** Azure IoT Hub Connection Transport ******************/
// Define this variable to use the MQTT layer directly, otherwise the Azure RTOS Embedded SDK is used.
#define USE_MQTT

/*************** Azure IoT Hub Connection Configuration ******************/
#define IOT_HUB_HOSTNAME    "ryangsg.azure-devices.net"
#define IOT_DEVICE_ID       "NXP"
#define IOT_PRIMARY_KEY     "AoyKIczyIkjIYOH13db61HQQzHgu2KkW6zEZhSkDlB4="

#endif // _AZURE_CONFIG_H