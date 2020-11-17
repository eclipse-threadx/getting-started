# Debugging with Visual Studio Code

This article on debugging is under construction. It refers to products in private beta, and also uses techniques that could expose your internet network to external attack. Care should be taken when following the steps below.

## Debug Options

You can choose to develop locally or develop in [Codespaces](https://github.com/features/codespaces).

* Developing locally requires the installation of the build/debug tools on your local machine to build the application.
* Develop locally in a Docker container. The development envrionment will be built automatically on your local machine.
* Developing in Codespaces doesn't require additional installation, however it requires early access to the Codespaces beta and also to expose a port on your computer the public internet.


## Install Drivers & Software

OpenOCD or the J-Link GDB Server can be used to interface your computer with your devkit. Only one is required:
* OpenOCD can be used for a variety of devkits without any additional hardware. Most devkits have an on-chip debugger supported by OpenOCD.
* J-Link requires an additional debugging hardware, however this gives you so more advanced features, as well as access to a larger set of devkits.

### OpenOCD

1. Install the relevant driver for the devkits on chip debugger:
    |Devkit |Driver |
    |---------|---------|
    |STM BL475E-IOT01 |[STLink](https://www.st.com/en/development-tools/stsw-link004.html) |
    |STM BL4S5I-IOT01 |[STLink](https://www.st.com/en/development-tools/stsw-link004.html) |
    |MXCHIP AZ3166 |[STLink](https://www.st.com/en/development-tools/stsw-link004.html) |
1. Download and unzip [OpenOCD](https://gnutoolchains.com/arm-eabi/openocd) to your computer and add its `bin` folder to your environment path.

### J-Link

1. Install the [JLink Software and Documentation Pack](https://www.segger.com/downloads/jlink#J-LinkSoftwareAndDocumentationPack). 

    > This includes the GDB Server as well as the neccessary J-Link drivers.

## Local Debugging

Local Debugging assumes you have already run through one of the existing [Getting Started Guides](https://github.com/azure-rtos/getting-started#getting-started-guides) and have already cloned the repository and installed the required prerequisites.

### Install Prerequisites

1. Install [Visual Studio Code](https://code.visualstudio.com).
1. Open Visual Studio Code and install the following extensions extensions:
    * Required
      * [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)
      * [Cortex Debug](https://marketplace.visualstudio.com/items?itemName=marus25.cortex-debug)
    * Recommended
      * [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)
      * [CMake](https://marketplace.visualstudio.com/items?itemName=twxs.cmake)

### Start Debugging

1. Open the VSCode workspace in the directory for the devkit you would like to debug. For example the STM32L4 devkit workspace is `STMicroelectronics/STM32L4_L4+/STM32L4plus.code-workspace`.
1. Build the application by pressing `F7`

    > Choose the `ARM GCC Cortex` CMake kit when prompted.
1. Start the debugger by pressing `F5`.

    > The application will be flashed to the DevKit and execution will break at main(). 
1. Press `F5` again or select continue to resume execution.
1. Use a serial monitor such as [Termite](https://www.compuphase.com/software_termite.htm) for monitoring the device's output.


## Local Container Debugging

### Install Prerequisites

1. Go to the [Getting Started GitHub repository](https://github.com/azure-rtos/getting-started)
1. Recursively clone the repository to your machine.
1. Install [Visual Studio Code](https://code.visualstudio.com).
1. Install [Docker Desktop](https://www.docker.com)
1. Open Visual Studio Code and install the required extensions:
    * Required
        * [Remote Development](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.vscode-remote-extensionpack)
1. From the Command Palette (F1), choose `Remote-Containers: Open Folder in Container`, and select the root folder of the cloned repository to build the Docker container

### Start Debugging

1. From a new command window run OpenOCD locally, using the appropriate command below:
    |Devkit  |Connection Command |
    |---------|---------|
    |STM BL475E-IOT01 |openocd -f board/stm32l4discovery.cfg -c "bindto 0.0.0.0" -c init -c "reset init" |
    |STM BL4S5I-IOT01 |openocd -f board/stm32l4discovery.cfg -c "bindto 0.0.0.0" -c init -c "reset init" |
    |Microchip ATSAME54-XPRO |openocd -f board/microchip_same54_xplained_pro.cfg -c "bindto 0.0.0.0" -c init -c "reset init" |
    |MXCHIP AZ3166 |openocd -f board/stm32f4discovery.cfg -c "bindto 0.0.0.0" -c init -c "reset init" |
    |NXP MIMXRT1060-EVK |JLinkGDBServerCL -select USB -device MIMXRT1062xxx6A -speed auto -if SWD -noLocalhostOnly|

1. Open the VSCode workspace in the directory for the devkit you would like to debug. For example the STM32L4 devkit workspace is `STMicroelectronics/STM32L4_L4+/STM32L4plus.code-workspace`.

1. Build the application by pressing `F7`
    
    > Choose the `ARM GCC Cortex` CMake kit when prompted

1. Start the debugger by pressing `F5`, 

1. Enter `host.docker.internal:3333` when prompted for the local hostname.

1. Use a serial monitor such as [Termite](https://www.compuphase.com/software_termite.htm) for monitoring the device's output.


## CodeSpaces Debugging

### Install Prerequisites

1. Go to the [Getting Started GitHub repository](https://github.com/azure-rtos/getting-started).
1. Choose `Code | Open with Codespaces`, to create and launch your Codespaces environment.
1. Setup your router to forward outside connections to port 3333 to your local machine. This allows the Codespaces GDB service to communicate with the devkit via the local OpenOCD instance.
    
    > Warning: This will expose the OpenOCD service to the public network which may result in an external compromise.


### Start Debugging

1. From a new command window run OpenOCD locally, using the appropriate command below:
    |Devkit  |Connection Command |
    |---------|---------|
    |STM BL475E-IOT01 |openocd -f board/stm32l4discovery.cfg -c "bindto 0.0.0.0" -c init -c "reset init" |
    |STM BL4S5I-IOT01 |openocd -f board/stm32l4discovery.cfg -c "bindto 0.0.0.0" -c init -c "reset init" |
    |Microchip ATSAME54-XPRO |openocd -f board/microchip_same54_xplained_pro.cfg -c "bindto 0.0.0.0" -c init -c "reset init" |
    |MXCHIP AZ3166 |openocd -f board/stm32f4discovery.cfg -c "bindto 0.0.0.0" -c init -c "reset init" |
    |NXP MIMXRT1060-EVK |JLinkGDBServerCL -select USB -device MIMXRT1062xxx6A -speed auto -if SWD -noLocalhostOnly|

1. Open the VSCode workspace in the directory for the devkit you would like to debug. For example the STM32L4 devkit workspace is `STMicroelectronics/STM32L4_L4+/STM32L4plus.code-workspace`.

1. Build the application by pressing `F7`
    
    > Choose the `ARM GCC Cortex` CMake kit when prompted

1. Start the debugger by pressing `F5`, 

1. Enter `{PUBLIC_IP_ADDRESS}:3333`  when prompted for the local hostname.

1. Use a serial monitor such as [Termite](https://www.compuphase.com/software_termite.htm) for monitoring the device's output.
