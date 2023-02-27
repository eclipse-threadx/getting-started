---
page_type: sample
description: Connecting an Microchip ATSAME54-XPro device to Azure IoT using Azure RTOS
languages:
- c
products:
- azure-iot
- azure-iot-pnp
- azure-rtos
- azure-iot-central
---

# Connect an Microchip ATSAME54-XPro evaluation kit to Azure IoT

[![Quickstart article](../../docs/media/docs-link-buttons/azure-quickstart.svg)](https://docs.microsoft.com/azure/iot-develop/quickstart-devkit-microchip-atsame54-xpro-iot-hub)
[![Documentation](../../docs/media/docs-link-buttons/azure-documentation.svg)](https://docs.microsoft.com/azure/iot-develop/)

The **Quickstart** button above provides the complete steps for creating an IoT Hub application and then configuring, building and flashing the device.

For guidance on connecting additional devices, see [Getting started with IoT device development](https://learn.microsoft.com/azure/iot-develop/about-getting-started-device-development).

## What you need

* The [Microchip ATSAME54-XPro](https://www.microchip.com/developmenttools/productdetails/atsame54-xpro)
* USB 2.0 A male to Micro USB male cable
* Wired Ethernet access
* Ethernet cable
* Optional (for temperature reporting):
    * [Weather Click](https://www.mikroe.com/weather-click) sensor. You can add this sensor to the device to monitor weather conditions. If you don't have this sensor, you can still complete this tutorial.
    * [mikroBUS Xplained Pro](https://www.microchip.com/development-tool/atmbusadapter-xpro) adapter. Use this adapter to attach the Weather Click sensor to the Microchip E54. If you don't have the sensor and this.

## Steps

1. Recursively clone the repository:
    ```shell
    git clone --recursive https://github.com/azure-rtos/getting-started.git
    ```

1. Install the development tools:

    *getting-started\tools\get-toolchain.bat*

1. Extract the latest [OpenOCD](https://gnutoolchains.com/arm-eabi/openocd) and add the `bin` folder to your path.

1. Add Azure IoT configuration to the config file:
    
    *getting-started\Microchip\ATSAME54-XPRO\app\azure_config.h*
    
1. Build the binary image:

    *getting-started\Microchip\ATSAME54-XPRO\tools\rebuild.bat*

1. Flash the image to the device:

    *getting-started\Microchip\ATSAME54-XPRO\tools\flash.bat*

1. Configure a serial port app the device output:
    * **Baud Rate**: 115,200
    * **Flow Control**: DTR/DSR
