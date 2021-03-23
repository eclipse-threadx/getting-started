---
page_type: sample
description: Connecting an MXChip AZ3166 device to Azure IoT using Azure RTOS
languages:
- c
products:
- azure-iot
- azure-iot-pnp
- azure-rtos
- azure-iot-dps
- azure-iot-hub
---

# Getting started with the MXCHIP AZ3166 IoT DevKit

This tutorial covers the basic steps required to use Azure RTOS to connect the MXCHIP AZ3166 IoT DevKit to Azure IoT. The series introduces device developers to Azure RTOS, and shows how to connect several device evaluation kits to Azure IoT.

For detailed instructions [Connect an MXCHIP AZ3166 to Azure IoT Central
](https://docs.microsoft.com/azure/iot-develop/quickstart-devkit-mxchip-az3166).

See other tutorials in the series [Getting started with Azure RTOS](https://go.microsoft.com/fwlink/p/?linkid=2129824).

## What you need

* The [MXCHIP AZ3166 IoT DevKit](https://aka.ms/iot-devkit) (MXCHIP DevKit)
* Wi-Fi 2.4 GHz
* USB 2.0 A male to Micro USB male cable

## Steps

1. Recursively clone the repository to your machine
    ```shell
    git clone --recursive https://github.com/azure-rtos/getting-started.git
    ```
1. Install the development tools
    
    *getting-started\tools\get-toolchain.bat*
1. Add WiFi and Azure IoT configuration to the following file
    
    *getting-started\MXChip\AZ3166\app\azure_config.h*
    
    > Note: comment out *#define ENABLE_DPS* if you want to connect to a IoT Hub without using DPS.
1. Build the binary image

    *getting-started\MXChip\AZ3166\tools\rebuild.bat*
1. Flash the image to the device by copying the image file to the **AZ3166* drive

    *getting-started\MXChip\AZ3166\build\app\mxchip_azure_iot.bin*
1. Configure a serial port monitor at baud 115200 to monitor the debug output
