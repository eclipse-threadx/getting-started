---
page_type: sample
description: Connecting STMicroelectronics B-L475E-IOT01A and B-L4S5I-IOT01A devices to Azure IoT using Azure RTOS
languages:
- c
products:
- azure-iot
- azure-iot-pnp
- azure-rtos
- azure-iot-central
---

# Connect an STMicroelectronics B-L475E-IOT01A / B-L4S5I-IOTOA1 Discovery kit to Azure IoT

[![Quickstart article](../../docs/media/docs-link-buttons/azure-quickstart.svg)](https://docs.microsoft.com/azure/iot-develop/quickstart-devkit-stm-b-l475e)
[![Documentation](../../docs/media/docs-link-buttons/azure-documentation.svg)](https://docs.microsoft.com/azure/iot-develop/)

The **Quickstart** button above provides the complete steps for creating an IoT Central application and then configuring, building and flashing the device.

For guidance on connecting additional devices, see the [Embedded device quickstarts](https://docs.microsoft.com/azure/iot-develop/quickstart-devkit-mxchip-az3166).

## What you need

* One of the following DevKits:
    * [B-L475E-IOT01A](https://www.st.com/en/evaluation-tools/b-l475e-iot01a.html)
    * [B-L4S5I-IOT01A](https://www.st.com/en/evaluation-tools/b-l4s5i-iot01a.html)
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
    
    *getting-started\STMicroelectronics\STM32L4_L4+\app\azure_config.h*
    
1. Build the binary image:

    *getting-started\STMicroelectronics\STM32L4_L4+\tools\rebuild.bat*

1. Flash the image to the device by copying the image file to the **DIS_L4IOT** drive:

    *getting-started\STMicroelectronics\STM32L4_L4+\build\app\stm32l475_azure_iot.bin*; or

    *getting-started\STMicroelectronics\STM32L4_L4+\build\app\stm32l4S5_azure_iot.bin*

1. Configure a serial port app at baud rate **115,200** to monitor the device output.
