---
page_type: sample
description: Connecting an STMicroelectronics B-U585I-IOT02A device to Azure IoT using Azure RTOS
languages:
- c
products:
- azure-iot
- azure-iot-pnp
- azure-rtos
---

# Connect an STMicroelectronics B-U585I-IOT02A Discovery kit to Azure IoT

[![Quickstart article](../../docs/media/docs-link-buttons/azure-quickstart.svg)](https://docs.microsoft.com/azure/iot-develop/quickstart-devkit-stm-b-l4s5i)
[![Documentation](../../docs/media/docs-link-buttons/azure-documentation.svg)](https://docs.microsoft.com/azure/iot-develop/)

The **Quickstart** button above provides the complete steps for creating an IoT Central application and then configuring, building and flashing the device.

For guidance on connecting additional devices, see the [Embedded device quickstarts](https://docs.microsoft.com/azure/iot-develop/quickstart-devkit-mxchip-az3166).

## What you need

* The [B-U585I-IOT02A devkit](https://www.st.com/en/evaluation-tools/b-u585i-iot02a.html)
* Wi-Fi 2.4 GHz
* USB 2.0 A male to Micro USB male cable

## Get the source code

1. Clone the repository:
    ```shell
    git clone https://github.com/azure-rtos/getting-started.git
    ```

1. Add Wi-Fi and Azure IoT configuration to the config file:

    *STMicroelectronics\B-U585I-IOT02A\app\azure_config.h*

1. Configure a serial port app at baud rate **115,200** to monitor the device output.

## STM32CubeMX configuration

The board specific code was creating using [STM32CubeMX](https://www.st.com/en/development-tools/stm32cubemx.html). The project file can be found in the [lib/stmcubeu5/stmcubemx]() directory if further customization is needed.

