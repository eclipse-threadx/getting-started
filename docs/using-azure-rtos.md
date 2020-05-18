# Using Azure RTOS in the Getting Started Guides

The getting started guide in this repository uses Azure RTOS for general operations as well as connecting to [Azure IoT Hub](https://azure.microsoft.com/en-us/services/iot-hub). The sample code in the getting started guide uses the following Azure RTOS components:

* [Azure ThreadX](threadx/overview-threadx.md). Provides the core real-time operating system components for devices.
* [Azure NetX Duo](netx-duo/overview-netx-duo.md). Provides a full TCP/IP IPv4 and IPv6 network stack, and networking support integrated with ThreadX.

> Note: Azure RTOS provides OEMs with components to secure communication and to create code and data isolation using underlying MCU/MPU hardware protection mechanisms. However, each OEM is ultimately responsible for ensuring that their device meets evolving security requirements.

## Guide File Structure

Below is the basic file structure of the getting started repository. Each of the files will be discussed further in this document, to describe their function in the overall project.

    |--- cmake/
    |   |--- arm-gcc-cortex-m4.cmake
    |   |--- arm-gcc-cortex-m7.cmake
    |   |--- arm_gcc_toolchain.cmake
    |   |--- CPM.cmake
    |
    |--- common/
    |   |--- azure/
    |   |   |--- azure_mqtt.c
    |   |   |--- cert.c
    |   |   |--- sas_token.c
    |   |--- networking.c
    |   |--- sntp_client.c
    |
    |--- {vendor}/{device}/
        |--- app/
        |   |--- main.c
        |   |--- azure_config.h
        |   |--- azure_iothub.c
        |   |--- board_init.c
        |       |--- startup/
        |           |--- startup.s
        |           |--- linker.ld
        |--- lib/
            |--- threadx/
            |   |--- tx_user.h
            |--- netx_duo/
            |   |--- nx_user.h
            |--- netx_driver/
            |--- {board_bsp}/

## CMake

[CMake](https://cmake.org) is the mechanism used for to generate the build files that then build the final binary. CMake was chosen as the preferred system for for Azure RTOS because of it's portability, simplicity and scalability. CMake is used by the core Azure RTOS components, making integrating these into the getting started guides as simple as an `add_subdirectory` command. These guides utilizes an additional 3rd party module, [CPM](https://github.com/TheLartians/CPM), to automatically pull in the Azure RTOS repository at build time.

### Toolchain

The cmake folder contains te build toolchain files for the project. It contains both a [Cortex-M4](../cmake/arm-gcc-cortex-m4.cmake) and a [Cortex-M7](../cmake/arm-gcc-cortex-m7.cmake) variant which enable the specific Gcc flags for building on the architecture. There is also a base level [arm-gcc-toolchain.cmake](../cmake/arm-gcc-toolchain.cmake) file that defines the specific build tools (arm-none-eabi-*), along with the a set of compile and linker flags to optimally build the flags.

## Application Files

### Main.c

This file contains the application entry point main(). Main is responsible for the following functions:

* Entering the ThreadX kernel
* Initializing the board
* Start the main application thread
* Starting the network
* Starting the SNTP client
* Starting the IoT Hub thread

### Startup

The startup folder contains three components:

1. The linker script (file ending in .ld) describes how sections in the input files are mapped into the output file as well as control the memory layout of the output file. In here you can control where the different sections such as .data, .text, .stack are placed in the devices flash memory.

1. The startup assembly will typically contain the interrupt vector table, as well as the execution entry point. The vector table is a array of pointer where execution will begin when a hardware interrupt is triggered. The Reset_Handler entry point performs important memory initialization functions before the application main is called.

1. Tx_initialize_low_level.S is an assembly file that contains the low level initialization for ThreadX. The primary function of this file is to setup the System Tick handler, which is used to control the internal timing of ThreadX.

### Azure_config.h

This file contains configuration required to connect the device to IoT Hub. Primarily it contains the IoT Hub connection information (IOT_HUB_HOSTNAME, IOT_DEVICE_ID, IOT_PRIMARY_KEY) as well as WiFi configuration if the device utilizes a WiFi chip.

In a production environment it would not be recommended to hard code configuration in code, however for simplicity of the guide execution, this method was chosen.

### Azure_iothub.c

The main application location for all IoT Hub communication logic. This file starts the MQTT client and registers the following callbacks for the subscribed topics needed for IoT Hub communication:


|Callback |Description|
|---------|---------|
|Direct method |Handle direct methods initiated from IoT Hub|
|RowC2D message |Handle cloud to device messages initiated from IoT Hub|
|Device twin desired property |Handle device twin desired properties initiated from IoT Hub|
|Thread entry |Main thread loop. Device telemetry is implemented here on a regular interval|

### Board_init.c

This file is responsible for initializing the different functions of the board. Typically this will setup the clocks, pins, peripherals as well as the debug console which redirects to the virtual serial port.

## Common Files

### Azure/azure_mqtt.c

Communication between the device and Azure IoT Hub is accomplished using the [MQTT](http://mqtt.org) protocol. Azure IoT's NetXDuo library includes an MQTT client.

Contains the build of the code required to interface with Azure IoT Hub using the Azure RTOS MQTT Application. The functions in this file are responsible for:

1. Initializing the MQTT client
1. Establishing a secure connection using TLS
1. Subscribing to the IoT Hub topics
    1. Cloud to Device messages
    1. Direct methods
    1. Device twin properties
    1. Device twin desired properties
1. Parsing the MQTT messages and calling the appropriate callback into the applications azure_iothub.c function.
1. Facilitating publishing messages to IoT Hub via a set of helper functions.

### Azure/sas_token.c

Sas_token.c takes a IoT Hub hostname, the device id and the device's primary SAS key and generates a SAS token. The SAS token is used to authenticate with your Azure IoT Hub.

### Networking.c

Initialize the main components of the networking stack using NetXDuo. This includes allocating a packet pool, and creating the IP instance. The the TCP, UDP and ICMP protocols are enabled.

NetXDuo contains a couple of add-ons, DHCP and DNS, used to establish the network connection.

### Sntp_client.c

Accurate time is required for generation of the SAS token, used for authentication with Azure IoT Hub. The SNTP (Simple Network Time Protocol), is responsible for synchronizing time between the MCU and timer server on the internet.

The SNTP implementation will attempt to synchronize time at startup, and then will maintain a thread that will monitor for incoming updates and resynchronize. The monitor thread contains some simple logic to reconnect if a required.

## Library Files

### Threadx

Contains a single file, tx_user.h, that defines the varies preprocessor flags to customize ThreadX for the specific architecture and board for each of the guides.

### Netxduo

Contains a single file, nx_user.h, that defines the varies preprocessor flags to customize NetXDuo for the specific architecture and board for each of the guides.

### Netx_driver

Contains the network driver to enable NetXDuo to interface with the network module on each of the boards.
