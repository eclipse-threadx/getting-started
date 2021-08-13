---
page_type: sample
description: Connecting an NXP MIMXRT1050-EVKB device to Azure IoT using Azure RTOS
languages:
- c
products:
- azure-iot
- azure-iot-pnp
- azure-rtos
- azure-iot-central
---

# Connect an NXP MIMXRT1050-EVKB Evaluation kit to Azure IoT

[![Quickstart article](../../docs/media/docs-link-buttons/azure-quickstart.svg)](https://docs.microsoft.com/azure/iot-develop/quickstart-devkit-nxp-mimxrt1050-evkb)
[![Documentation](../../docs/media/docs-link-buttons/azure-documentation.svg)](https://docs.microsoft.com/azure/iot-develop/)

The **Quickstart** button above provides the complete steps for creating an IoT Central application and then configuring, building and flashing the device.

For guidance on connecting additional devices, see the [Embedded device quickstarts](https://docs.microsoft.com/azure/iot-develop/quickstart-devkit-mxchip-az3166).

## What you need

* The [NXP MIMXRT1060-EVK Evaluation kit](https://www.nxp.com/design/development-boards/i-mx-evaluation-and-development-boards/i-mx-rt1050-evaluation-kit:MIMXRT1050-EVK)
* USB 2.0 A male to Micro USB male cable
* Wired Ethernet access
* Ethernet cable

## Steps

1. Recursively clone the repository:
    ```shell
    git clone --recursive https://github.com/azure-rtos/getting-started.git
    ```

1. Install the development tools:

    *getting-started\tools\get-toolchain.bat*

1. Add Azure IoT configuration to the config file:
    
    *getting-started\NXP\MIMXRT1050-EVKB\app\azure_config.h*
    
1. Build the binary image:

    *getting-started\NXP\MIMXRT1050-EVKB\tools\rebuild.bat*

1. Flash the image to the device by copying the image file to the **RT1050-EVK** drive:

    *getting-started\NXP\MIMXRT1050-EVKB\build\app\mimxrt1050_azure_iot.bin*

1. Configure a serial port app at baud rate **115,200** to monitor the device output.
