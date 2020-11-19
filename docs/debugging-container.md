# Container Debugging with Visual Studio Code

Building in a local container allows you to deploy the development environment with a couple of clicks, bypassing the need to manually install the required build tools. It does require the installation of Docker and Windows Subsystem for Linux, as well as having Virtualization enabled in the BIOS.

## Prerequisites

### Enable BIOS Virtualization

1. Reboot the computer and enter the BIOS setup.

1. Find the Virtualization option and set to `enabled`.

1. Save setting and restart Windows.

### Install Device Drivers

1. Download and unzip the latest [OpenOCD](https://gnutoolchains.com/arm-eabi/openocd) and add its `bin` folder to your environment path.

1. Install the relevant driver for the devkits on chip debugger:
    |Manufacturer |Devkit |Driver |
    |---------|---------|---------|
    |Microchip |ATSAME54-XPRO |None required |
    |MXCHIP |AZ3166 |[STLink](https://www.st.com/en/development-tools/stsw-link004.html) |
    |NXP |MIMXRT1050-EVKB |[JLink](https://www.segger.com/downloads/jlink#J-LinkSoftwareAndDocumentationPack) |
    |NXP |MIMXRT1060-EVK |[JLink](https://www.segger.com/downloads/jlink#J-LinkSoftwareAndDocumentationPack) |
    |STM |BL475E-IOT01 |[STLink](https://www.st.com/en/development-tools/stsw-link004.html) |
    |STM |BL4S5I-IOT01 |[STLink](https://www.st.com/en/development-tools/stsw-link004.html) |

### Install Development Tools

1. Install [Visual Studio Code](https://code.visualstudio.com).
1. Install [Docker Desktop](https://www.docker.com)

## Create the Local Container

1. Recursively clone the Getting Started Guide repository to your machine.
    > `git clone --recursive https://github.com/azure-rtos/getting-started.git`

1. Launch Visual Studio Code and install the following extension:
    * [Remote - Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers)

1. From the Command Palette `F1`, choose `Remote-Containers: Open Folder in Container`, and select the root folder of the cloned repository to build the Docker container

## Start Debugging

1. From a new command window run OpenOCD locally, using the appropriate command below:
    |Manufacturer |Devkit |Connection Command |
    |---------|---------|---------|
    |Microchip |ATSAME54-XPRO |openocd -f board/microchip_same54_xplained_pro.cfg -c "bindto 0.0.0.0" -c init -c "reset init" |
    |MXCHIP |AZ3166 |openocd -f board/stm32f4discovery.cfg -c "bindto 0.0.0.0" -c init -c "reset init" |
    |NXP |MIMXRT1050-EVKB |JLinkGDBServerCL -select USB -device MIMXRT1052xxx6A -speed auto -if SWD -noLocalhostOnly|
    |NXP |MIMXRT1060-EVK |JLinkGDBServerCL -select USB -device MIMXRT1062xxx6A -speed auto -if SWD -noLocalhostOnly|
    |STMicroelectronics |BL475E-IOT01 |openocd -f board/stm32l4discovery.cfg -c "bindto 0.0.0.0" -c init -c "reset init" |
    |STMicroelectronics |BL4S5I-IOT01 |openocd -f board/stm32l4discovery.cfg -c "bindto 0.0.0.0" -c init -c "reset init" |

1. Launch VS Code, and open the workspace in the directory for the target devkit. 
    > For example the STM32L4 devkit workspace is `STMicroelectronics/STM32L4_L4+/STM32L4plus.code-workspace`.

1. Press `F7` to build the application.
    > Choose the `ARM GCC Cortex` CMake kit if prompted.

1. Press `F5` to start debugging the application.

1. Enter `host.docker.internal:3333` when prompted for the local hostname.
    > The application will be flashed to the devKit and execution will break at main(). 

1. Press `F5` to resume execution.

1. Use a serial monitor such as [Termite](https://www.compuphase.com/software_termite.htm) to monitoring the device's output.
