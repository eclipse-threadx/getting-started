---
page_type: sample
description: Connecting a Renesas RX65N Cloud Kit to Azure IoT using Azure RTOS
languages:
- c
products:
- azure-iot
- azure-iot-pnp
- azure-rtos
- azure-iot-central
---

---
page_type: sample
description: Connecting an Microchip ATSAME54-XPro device to Azure IoT using Azure RTOS
languages:
- c
products:
- azure-iot
- azure-iot-pnp
- azure-rtos
- azure-iot-dps
- azure-iot-hub
---

# Connect an Renesas RX65N Cloud Kit to Azure IoT

[![Quickstart article](../../docs/media/docs-link-buttons/azure-quickstart.svg)](https://docs.microsoft.com/azure/iot-develop/quickstart-devkit-renesas-rx65n-cloud-kit)
[![Documentation](../../docs/media/docs-link-buttons/azure-documentation.svg)](https://docs.microsoft.com/azure/iot-develop/)

The **Quickstart** button above provides the complete steps for creating an IoT Central application and then configuring, building and flashing the device.

For guidance on connecting additional devices, see the [Embedded device quickstarts](https://docs.microsoft.com/azure/iot-develop/quickstart-devkit-mxchip-az3166).

## What you need

* The [Renesas RX65N Cloud Kit](https://www.renesas.com/products/microcontrollers-microprocessors/rx-32-bit-performance-efficiency-mcus/rx65n-cloud-kit-renesas-rx65n-cloud-kit)
* 2 * USB 2.0 A male to Mini USB male cable
* WiFi 2.4 GHz

## Steps

1. Recursively clone the repository:
    ```shell
    git clone --recursive https://github.com/azure-rtos/getting-started.git
    ```

1. Install the development tools:

    *getting-started\tools\get-toolchain-rx.bat*

1. Add Azure IoT configuration to the config file:
    
    *getting-started\Renesas\RX65N_Cloud_Kit\app\azure_config.h*
    
1. Build the binary image:

    *getting-started\Renesas\RX65N_Cloud_Kit\tools\rebuild.bat*

1. Connect the device:
    1. Connect the **WiFi module** to the **Cloud Option Board**
    1. Connect **CN18** on the **Cloud Option Board** to your computer
    1. Connect **ECN1** on the **Target Board** to your computer
    1. Remove the **EJ2 link** from the **Target Board**

1. Flash the image to the device using the [Renesas Flash Programmer](https://www.renesas.com/software-tool/renesas-flash-programmer-programming-gui) and the following settings:
    * **Microcontroller**: RX65x
    * **Project Name**: RX65N
    * **Tool**: E2 emulator Lite
    * **Interface**: FINE
    * **Reset Settings**: Reset Pin as Hi-Z

    *getting-started\Renesas\RX65N_Cloud_kit\build\app\rx65n_azure_iot.hex*

1. Configure a serial port app at baud rate **115,200** to monitor the device output.
