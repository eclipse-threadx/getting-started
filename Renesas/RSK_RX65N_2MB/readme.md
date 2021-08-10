---
page_type: sample
description: Connecting a Renesas Starter Kit+ for RX65N-2MB to Azure IoT using Azure RTOS
languages:
- c
products:
- azure-iot
- azure-iot-pnp
- azure-rtos
- azure-iot-central
---

# Connect an Renesas Starter Kit+ for RX65N-2MB to Azure IoT

[![Quickstart article](../../docs/media/docs-link-buttons/azure-quickstart.svg)](https://docs.microsoft.com/azure/iot-develop/quickstart-devkit-renesas-rx65n-2mb)
[![Documentation](../../docs/media/docs-link-buttons/azure-documentation.svg)](https://docs.microsoft.com/azure/iot-develop/)

The **Quickstart** button above provides the complete steps for creating an IoT Central application and then configuring, building and flashing the device.

For guidance on connecting additional devices, see the [Embedded device quickstarts](https://docs.microsoft.com/azure/iot-develop/quickstart-devkit-mxchip-az3166).

## What you need

* The [Renesas Starter Kit+ for RX65N-2MB](https://www.renesas.com/products/microcontrollers-microprocessors/rx-32-bit-performance-efficiency-mcus/rx65n-2mb-starter-kit-plus-renesas-starter-kit-rx65n-2mb)
* 2 * USB 2.0 A male to Mini USB male cable
* The included 5V power supply
* Ethernet cable
* Wired Ethernet access

## Steps

1. Recursively clone the repository:
    ```shell
    git clone --recursive https://github.com/azure-rtos/getting-started.git
    ```

1. Install the development tools:

    *getting-started\tools\get-toolchain-rx.bat*

1. Add Azure IoT configuration to the config file:
    
    *getting-started\Renesas\RSK_RX65N_2MB\app\azure_config.h*
    
1. Build the binary image:

    *getting-started\Renesas\RSK_RX65N_2MB\tools\rebuild.bat*

1. Connect the device:

    1. Connect the **5VDC** to an electrical outlet
    1. Connect the **Ethernet** to your router
    1. Connect **G1CUSB0** to your computer
    1. Connect the **E2Lite** to your computer
    1. Connect the **E2Lite** to the **RX54N RSK**
    1. Connect the **Ethernet** to your router

1. Flash the image to the device using the [Renesas Flash Programmer](https://www.renesas.com/software-tool/renesas-flash-programmer-programming-gui) and the following settings:
    * **Microcontroller**: RX65x
    * **Project Name**: RX65N
    * **Tool**: E2 emulator Lite
    * **Reset Settings**: Reset Pin as Hi-Z

    *getting-started\Renesas\RSK_RX65N_2MB\build\app\rx65n_azure_iot.hex*

1. Configure a serial port app at baud rate **115,200** to monitor the device output.
