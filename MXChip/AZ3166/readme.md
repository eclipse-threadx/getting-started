---
page_type: sample
description: Connect an MXCHIP AZ3166 to Azure IoT
languages:
- c
products:
- azure-iot
- azure-iot-pnp
- azure-iot-central
---

# Connect an MXCHIP AZ3166 to Azure IoT

This guide steps through the basic process to flash a device and connect to Azure IoT. 

## What you need

* The [MXCHIP AZ3166 IoT DevKit](https://aka.ms/iot-devkit)
* Wi-Fi 2.4 GHz
* USB 2.0 A male to Micro USB male cable

## Steps

1. Recursively clone the repository:
    ```shell
    git clone --recursive https://github.com/eclipse-threadx/getting-started.git
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
