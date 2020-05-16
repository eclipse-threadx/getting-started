# Using Azure RTOS in the Getting Started Guides

The getting started guides in this repository extensively use the Azure RTOS for general operation as well as connecting to Azure IoT Hub.

## CMake
[CMake](https://cmake.org) is the mechanism used for to generate the build files that then build the final flashable binary. CMake was chosen as the preferred system for for Azure RTOS because of it's portability, simplicity and scalability. CMake is used by the core Azure RTOS components and 

### Toolchain

## ThreadX
### Threads
### Mutexs
## NetXDuo
### IP
### DNS
### DHCP
### MQTT

## Application Files

### Startup

The startup folder contains three components.

1. The linker script (file ending in .ld) describes how sections in the input files are mapped into the output file as well as control the memory layout of the output file. In here you can control where the different sections such as .data, .text, .stack are placed in the devices flash.

1. The startup assembly will typically contain the interrupt vector table, as well as the execution entry point. The vector table is a array of pointer where execution will begin when a hardware interrupt is triggered. The Reset_Handler entry point performs important memory initialization functions before the application main is called.

1. Tx_initialize_low_level.S is an assembly file that contains the low level initialization for ThreadX. The primary function of this file is to setup the System Tick handler, which is used to control the internal timing of ThreadX.

### Main.c

This file contains the application entry point main(). Main is responsible for the following functions:

1. Entering the ThreadX kernel
1. Initializing the board
1. Starting the network
1. Starting the SNTP client
1. Starting the IoT Hub thread

### Azure_config.h

This file contains configuration required to connect the device to IoT Hub. Primarily it contains the IoT Hub connection information hub (IOT_HUB_HOSTNAME, IOT_DEVICE_ID, IOT_PRIMARY_KEY) as well as WiFi configuration if the device utilizes a WiFi chip.

In a production environment it would not be recommended to hard code configuration in code, however for simplicity of the guide execution, this is the current setup.

### Azure_iothub.c

The main application location for all IoT Hub communication logic. This file starts the MQTT client and register the following callbacks for the subscribed topics needed for IoT Hub communication:


|Callback |Description|
|---------|---------|
|Direct method |Handle direct methods initiated from IoT Hub|
|RowC2D message |Handle cloud to device messages initiated from IoT Hub|
|Device twin desired property |Handle device twin desired properties initiated from IoT Hub|
|Thread entry |Main thread loop. Device telemetry is implemented here on a regular interval|

### Board_init.c

This file is responsible for initializing the different functions of the board. Typically this will setup the clocks, pins, peripherals as well as the debug console.

## Common Files

### Networking.c
Initialize the main components of the networking stack. 

### sntp_client.c
### azure/azure_mqtt.c

### Azure/sas_token.c

Sas_token.c takes a IoT Hub hostname, the device id and the device sas key and generates a SAS token. The SAS token is used to authenticate with your Azure IoT Hub.

A Sas token consist of a 

## Library Files

### Threadx

### Netxduo

### Netx_driver

