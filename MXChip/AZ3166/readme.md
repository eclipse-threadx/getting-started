---
page_type: sample
description: Connect an MXCHIP AZ3166 to Azure IoT using Azure RTOS
languages:
- c
products:
- azure-iot
- azure-iot-pnp
- azure-rtos
- azure-iot-dps
- azure-iot-hub
---

# Connect an MXCHIP AZ3166 to Azure IoT

[![Documentation](../../docs/media/docs-link-buttons/azure-documentation.svg)](https://docs.microsoft.com/azure/iot-develop/)
[![Quickstart article](../../docs/media/docs-link-buttons/azure-quickstart.svg)](https://docs.microsoft.com/azure/iot-develop/quickstart-devkit-mxchip-az3166)

This Readme provides basic details to clone the repo and flash the device. The linked quickstart provides more details on how to build and flash the image, how to create an IoT Central application, and how to connect the device.

For guidance on connecting additional devices, see the series [Getting started with Azure RTOS](https://go.microsoft.com/fwlink/p/?linkid=2129824).

## What you need

* The [MXCHIP AZ3166 IoT DevKit](https://aka.ms/iot-devkit)
* Wi-Fi 2.4 GHz
* USB 2.0 A male to Micro USB male cable

## Steps

1. Recursively clone the repository:
    ```shell
    git clone --recursive https://github.com/azure-rtos/getting-started.git
    ```
1. Install the development tools:

    *getting-started\tools\get-toolchain.bat*
1. Add Wi-Fi and Azure IoT configuration to the config file:
    
    *getting-started\MXChip\AZ3166\app\azure_config.h*
    
    > Note: comment out *#define ENABLE_DPS* if you want to connect to a IoT Hub without using DPS.
1. Build the binary image:

    *getting-started\MXChip\AZ3166\tools\rebuild.bat*
1. Flash the image to the device by copying the image file to the **AZ3166** drive:

    *getting-started\MXChip\AZ3166\build\app\mxchip_azure_iot.bin*
1. Configure a serial port app at BAUD 115,200 to monitor the debug output from the device
