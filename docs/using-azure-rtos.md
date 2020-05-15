# Using Azure RTOS in the Getting Started Guides

The getting started guides in this repository extensively use the Azure RTOS for general operation as well as connecting to Azure IoT Hub.

## ThreadX
### Threads
### Mutexs
## NetXDuo
### IP
### DNS
### DHCP
### MQTT

## Application Files

### Startup Folder

The startup folder contains three components.
http://www.scoberlin.de/content/media/http/informatik/gcc_docs/ld_3.html

1. The linker script (file ending in .ld) describes how sections of in the input files are mapped into the output file as well as control the memory layout of the output file. In here you can control where the different sections such as .data, .text, .stack are placed in the devices flash.

1. The startup assembly will typically contain the interrupt vector table, as well as the execution entry point. The vector table is a array of pointer where execution will begin when a hardware interrupt is triggered. The Reset_Handler entry point performs important memory initialization functions before the application main is called.

1. Tx_initialize_low_level.S is an assembly file that contains the low level initialization for ThreadX. The primary function of this file is to setup the System Tick handler, which is used to control the internal timing of ThreadX.

### Main.c

This file contains the application entry point main(). Main is responsible for the following functions:

1. Entering the ThreadX kernel
1. Initializing the board
1. Initializing the network
1. Initializing the SNTP client
1. Starting the IoT Hub thread

### Azure_config.h

This file contains configuration required to connect the device to IoT Hub. Primarily it contains the IoT Hub connection information hub (IOT_HUB_HOSTNAME, IOT_DEVICE_ID, IOT_PRIMARY_KEY) as well as WiFi configuration if the device utilizes a WiFi chip.

In a production environment it would not be recommended to hard code configuration in code, however for simplicity of the guide execution, this is the current setup.

### Azure_iothub.c

The main application location for all IoT Hub communication. This file performs the following functions:

1. Start the MQTT client and register the callbacks for the subscribed topics needed for IoT Hub communication.

### Board_init.c

This file is responsible for initializing the different functions of the board. Typically this will setup the clocks, pins, peripherals as well as the debug console.

## Common Files
### networking.c
### sntp_client.c
### azure/azure_mqtt.c
### azure/cert.c
### azure/sas_token.c


## Library Files
### threadx
### netxduo
### netx_driver

## CMake
