# Using Azure RTOS in the Getting started guide

The getting started guide in this repository uses Microsoft Azure RTOS for general operations and for connecting to [Azure IoT Hub](https://azure.microsoft.com/en-us/services/iot-hub). This article explains how the sample code implements Azure RTOS components for devices.

The sample code in the getting started guide uses the following Azure RTOS components:

* [Azure ThreadX](https://docs.microsoft.com/azure/rtos/threadx/overview-threadx). Provides the core real-time operating system components for devices.
* [Azure NetX Duo](https://docs.microsoft.com/azure/rtos/netx-duo/overview-netx-duo). Provides a full TCP/IP IPv4 and IPv6 network stack, and networking support integrated with ThreadX.

> Note: Azure RTOS provides OEMs with components to secure communication and to create code and data isolation using underlying MCU/MPU hardware protection mechanisms. However, each OEM is ultimately responsible for ensuring that their device meets evolving security requirements.

## Repository file structure

The getting started guide repository is arranged in the following folder and file structure. The remaining sections of this article describe the basic function of each of the following code files:

    |--- cmake/
    |   |--- arm-gcc-cortex-m4.cmake
    |   |--- arm-gcc-cortex-m7.cmake
    |   |--- arm_gcc_toolchain.cmake
    |   |--- CPM.cmake
    |
    |--- common/
    |   |--- azure_iot_mqtt/
    |   |   |--- azure_iot_mqtt.c
    |   |--- azure_iot_nx/
    |   |   |--- azure_iot_nx_client.c
    |   |--- azure_iot_cert.c
    |   |--- networking.c
    |   |--- newlib_nano.c
    |   |--- sntp_client.c
    |
    |--- {vendor}/{device}/
        |--- app/
        |   |--- main.c
        |   |--- azure_config.h
        |   |--- board_init.c
        |   |--- mqtt.c
        |   |--- nx_client.c
        |   |--- startup/
        |       |--- startup.s
        |       |--- linker.ld
        |--- lib/
            |--- threadx/
            |   |--- tx_user.h
            |--- netx_duo/
            |   |--- nx_user.h
            |--- netx_driver/
            |--- {board_bsp}/

## CMake

[CMake](https://cmake.org) is the tool used to generate the build files that then build the final binary. CMake is the preferred build tool for Azure RTOS because of its portability, simplicity, and scalability. CMake is also used by the core Azure RTOS components, which makes the process of integrating the components into the getting started guides as simple as an `add_subdirectory` command. The getting started guide tutorials use a 3rd party module, [CPM](https://github.com/TheLartians/CPM), to automatically pull in the Azure RTOS repository at build time.

### Toolchain

The *cmake* folder contains the build toolchain files for the project. It contains both a [Cortex-M4](../cmake/arm-gcc-cortex-m4.cmake) and a [Cortex-M7](../cmake/arm-gcc-cortex-m7.cmake) variant which enable the specific Gcc flags for building on the architecture. There is also a base level [arm-gcc-toolchain.cmake](../cmake/arm-gcc-toolchain.cmake) file that defines the specific build tools (arm-none-eabi-*), along with a set of compile and linker flags to optimize the build.

### Building

Building the project consists of two steps:

1. Generate the build system, specifying the build directory (```-B```), the build system generator (```-G```) and the appropriate toolchain for the MCU (```-DCMAKE_TOOLCHAIN_FILE```):

    ```
    cmake -Bbuild -GNinja -DCMAKE_TOOLCHAIN_FILE="../../cmake/arm-gcc-cortex-m4.cmake"
    ```
    
2. Build the project binary
    
    ```
    cmake --build build
    ```

### Additional Build Options

By default, the project builds a *Debug* release. This release type is suitable for debugging the application as optimizations are disabled (```-O0```), and the debug symbols are included in the resulting elf file.

To build a *Release* binary, append the following to step 1 above, while generating the build system:

```-DCMAKE_BUILD_TYPE=Release```

> Note: A *Release* build may have different run-time characteristics compared to the *Debug* build due to compiler optimizations. Read [Options That Control Optimization](https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html) for further information on GCC compiler optimizations.

## Application files

### Main.c

The *Main.c* file contains the application entry point `main()`. This function is responsible for the following:

* Entering the ThreadX kernel
* Initializing the board
* Start the main application thread
* Starting the network
* Starting the SNTP client
* Starting the IoT Hub thread

### Startup

The *startup* folder contains three components:

1. The linker script (the file with the **.ld* extension) describes how sections in the input files are mapped into the output file as well as control the memory layout of the output file. In this file, you can control where the different sections such as .data, .text and .stack are placed in the device's flash memory.

1. The startup assembly typically contains the interrupt vector table, and the execution entry point. The vector table is an array of pointer where execution begins when a hardware interrupt is triggered. The `Reset_Handler` entry point performs important memory initialization functions before the application main is called.

1. *Tx_initialize_low_level.S* is an assembly file that contains the low level initialization for ThreadX. The primary function of this file is to set up the System Tick handler, which controls the internal timing of ThreadX.

### Azure_config.h

The *azure_config.h* file contains configuration required to connect the device to IoT Hub. Primarily it  contains the IoT Hub connection information, which you can store in the constants `IOT_HUB_HOSTNAME`, `IOT_HUB_DEVICE_ID`, `IOT_DEVICE_SAS_KEY`. The file also contains Wi-Fi configuration details if the device requires a Wi-Fi connection.
> Note: In a production environment, we recommend that you not store connection details in code files.

### mqtt.c

This file is the primary location for all IoT Hub communication logic. The file starts the MQTT client and registers the following callbacks for the subscribed topics needed for IoT Hub communication:

|Callback |Description|
|---------|---------|
|Direct method |Handle direct methods initiated from IoT Hub|
|C2D message |Handle cloud-to-device messages initiated from IoT Hub|
|Device twin desired property |Handle device twin desired properties initiated from IoT Hub|
|Thread entry |Main thread loop. Device telemetry is implemented here on a regular interval|

### nx_client.c

This file is the primary location for all IoT Hub communication logic. The file starts the Azure Iot NX client and registers the following callbacks for the subscribed topics needed for IoT Hub communication

### Board_init.c

This file is responsible for initializing the different functions of the device. Typically the code sets up the clocks, pins, peripherals, and the debug console which redirects to the virtual serial port.

## Common Files

### azure_iot_mqtt/azure_iot_mqtt.c

Communication between the device and Azure IoT Hub is accomplished with the [MQTT](http://mqtt.org) protocol. The Azure RTOS NetX Duo library includes an MQTT client.

The *azure_iot_mqtt.c* file contains the build of the code required to interface with Azure IoT Hub using the Azure RTOS MQTT application. The functions in this file are responsible for:

1. Initializing the MQTT client
1. Establishing a secure connection using TLS
1. Subscribing to the IoT Hub topics
    1. Cloud-to-device messages
    1. Direct methods
    1. Device twin properties
    1. Device twin desired properties
1. Parsing the MQTT messages and calling the appropriate callback into the applications function.
1. Facilitating publishing messages to IoT Hub via a set of helper functions.

### Networking.c

The file initializes the main components of the networking stack using Azure RTOS NetX Duo. This includes allocating a packet pool, and creating the IP instance. The TCP, UDP and ICMP protocols are enabled.

Azure RTOS NetX Duo contains a couple of add-ons, DHCP and DNS, used to establish the network connection.

### Sntp_client.c

Accurate time is required for generation of the SAS token, which is used for authentication with Azure IoT Hub. The SNTP (Simple Network Time Protocol), is responsible for synchronizing time between the MCU and a timer server on the internet.

The SNTP implementation attempts to synchronize time at startup, and then maintains a thread that monitors for incoming updates, and resynchronizes. The monitor thread contains simple logic to reconnect if required.

## Library Files

### Threadx

Contains a single file, *tx_user.h*, that defines the varies preprocessor flags to customize ThreadX for the specific architecture and device for each of the tutorials.

### Netxduo

Contains a single file, *nx_user.h*, that defines the varies preprocessor flags to customize Azure RTOS NetX Duo for the specific architecture and device for each of the tutorials.

### Netx_driver

Contains the network driver to enable Azure RTOS NetX Duo to interface with the network module on each of the boards.
