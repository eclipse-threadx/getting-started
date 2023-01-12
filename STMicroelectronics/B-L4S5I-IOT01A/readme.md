---
page_type: sample
description: Connecting an STMicroelectronics L4S5I-IOT01A device to Azure IoT using Azure RTOS
languages:
- c
products:
- azure-iot
- azure-iot-pnp
- azure-rtos
- azure-iot-central
---

# Connect an STMicroelectronics B-L4S5I-IOTOA1 Discovery kit to Azure IoT

[![Quickstart article](../../docs/media/docs-link-buttons/azure-quickstart.svg)](https://docs.microsoft.com/azure/iot-develop/quickstart-devkit-stm-b-l4s5i-iot-hub)
[![Documentation](../../docs/media/docs-link-buttons/azure-documentation.svg)](https://docs.microsoft.com/azure/iot-develop/)

The **Quickstart** button above provides the complete steps for creating an IoT Hub application and then configuring, building and flashing the device.

For guidance on connecting additional devices, see [Getting started with IoT device development](https://learn.microsoft.com/azure/iot-develop/about-getting-started-device-development).

## What you need

* The [B-L4S5I-IOT01A devkit](https://www.st.com/en/evaluation-tools/b-l4s5i-iot01a.html)
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

    *getting-started\STMicroelectronics\B-L4S5I-IOT0A1\app\azure_config.h*

1. Build the binary image:

    *getting-started\STMicroelectronics\B-L4S5I-IOT0A1\tools\rebuild.bat*

1. Flash the image to the device by copying the image file to the **DIS_L4IOT** drive:

    *getting-started\STMicroelectronics\B-L4S5I-IOT0A1\build\app\stm32l4S5_azure_iot.bin*

1. Configure a serial port app at baud rate **115,200** to monitor the device output.

## Debugging with VSCode or Visual Studio

The following guides are available on how to build, flash and debug the devkit using some popular development environments:

* [Debugging with VSCode](vscode.md)
* [Debugging with Visual Studio 2022](VS.md)

## STM32CubeMX configuration

The board specific code was creating using [STM32CubeMX](https://www.st.com/en/development-tools/stm32cubemx.html). The project file can be found in the [lib/stmcubel4/stmcubemx]() directory if further customization is needed.

