---
page_type: sample
description: Connect an MXCHIP AZ3166 to Azure IoT using Azure RTOS
languages:
- c
products:
- azure-iot
- azure-iot-pnp
- azure-rtos
- azure-iot-central
---

# Connect an MXCHIP AZ3166 to Azure IoT

[![Quickstart article](../../docs/media/docs-link-buttons/azure-quickstart.svg)](https://docs.microsoft.com/azure/iot-develop/quickstart-devkit-mxchip-az3166-iot-hub)
[![Documentation](../../docs/media/docs-link-buttons/azure-documentation.svg)](https://docs.microsoft.com/azure/iot-develop/)

The **Quickstart** button above provides the complete steps for creating an IoT Hub application and then configuring, building and flashing the device.

For guidance on connecting additional devices, see [Getting started with IoT device development](https://learn.microsoft.com/azure/iot-develop/about-getting-started-device-development).

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
    
1. Build the binary image:

    *getting-started\MXChip\AZ3166\tools\rebuild.bat*

1. Flash the image to the device by copying the image file to the **AZ3166** drive:

    *getting-started\MXChip\AZ3166\build\app\mxchip_azure_iot.bin*

1. Configure a serial port app at baud rate **115,200** to monitor the device output.
