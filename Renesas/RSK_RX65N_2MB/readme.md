---
page_type: sample
description: Connecting a Renesas Starter Kit+ for RX65N-2MB to Azure IoT
languages:
- c
products:
- azure-iot
- azure-iot-pnp
- azure-iot-central
---

# Connect an Renesas Starter Kit+ for RX65N-2MB to Azure IoT

This guide steps through the basic process to flash a device and connect to Azure IoT. 

## What you need

* The [Renesas Starter Kit+ for RX65N-2MB](https://www.renesas.com/products/microcontrollers-microprocessors/rx-32-bit-performance-efficiency-mcus/rx65n-2mb-starter-kit-plus-renesas-starter-kit-rx65n-2mb)
* 2 * USB 2.0 A male to Mini USB male cable
* The included 5V power supply
* Ethernet cable
* Wired Ethernet access

## Steps

1. Recursively clone the repository:
    ```shell
    git clone --recursive https://github.com/eclipse-threadx/getting-started.git
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
    1. Connect the **E2Lite** to the **RX65N RSK**
    1. Connect the **Ethernet** to your router

1. Flash the image to the device using the [Renesas Flash Programmer](https://www.renesas.com/software-tool/renesas-flash-programmer-programming-gui) and the following settings:
    * **Microcontroller**: RX65x
    * **Project Name**: RX65N
    * **Tool**: E2 emulator Lite
    * **Reset Settings**: Reset Pin as Hi-Z

    *getting-started\Renesas\RSK_RX65N_2MB\build\app\rx65n_azure_iot.hex*

1. Configure a serial port app at baud rate **115,200** to monitor the device output.
