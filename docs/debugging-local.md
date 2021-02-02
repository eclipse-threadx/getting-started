# Local Debugging with Visual Studio Code

The Getting Started Guide include the configuration required to build and debug your application locally within VS Code. 

Local Debugging assumes you have already run through one of the existing [Getting Started Guides](https://github.com/azure-rtos/getting-started#getting-started-guides) and have already cloned the repository and installed the required prerequisites.

## Install Prerequisities

### Install Device Drivers

1. Download and unzip the latest [OpenOCD](https://gnutoolchains.com/arm-eabi/openocd) and add its `bin` folder to your environment path.

1. Install the relevant driver for the devkits on chip debugger:
    |Manufacturer | Devkit |Driver |
    |---------|---------|---------|
    |Microchip |ATSAME54-XPRO |None required |
    |MXCHIP |AZ3166 |[STLink](https://www.st.com/en/development-tools/stsw-link004.html) |
    |NXP |MIMXRT1050-EVKB |[JLink](https://www.segger.com/downloads/jlink#J-LinkSoftwareAndDocumentationPack) |
    |NXP |MIMXRT1060-EVK |[JLink](https://www.segger.com/downloads/jlink#J-LinkSoftwareAndDocumentationPack) |
    |STM |BL475E-IOT01 |[STLink](https://www.st.com/en/development-tools/stsw-link004.html) |
    |STM |BL4S5I-IOT01 |[STLink](https://www.st.com/en/development-tools/stsw-link004.html) |

### Install Visual Studio Code

1. Install [Visual Studio Code](https://code.visualstudio.com).

1. Launch Visual Studio Code and install the following extensions:
    * Required
      * [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)
      * [Cortex Debug](https://marketplace.visualstudio.com/items?itemName=marus25.cortex-debug)
    * Recommended
      * [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)
      * [CMake](https://marketplace.visualstudio.com/items?itemName=twxs.cmake)

### Start Debugging

1. Launch VSCode, and open the workspace in the directory for the target devkit. 
    > For example the STM32L4 devkit workspace is `STMicroelectronics/STM32L4_L4+/STM32L4plus.code-workspace`.

1. Press `F7` to build the application
    > Choose the `ARM GCC Cortex` CMake kit if prompted.

1. Press `F5` to start debugging the application.
    > The application will be flashed to the devKit and execution will break at main(). 

1. Press `F5` to resume execution.

1. Use a serial monitor such as [Termite](https://www.compuphase.com/software_termite.htm) to monitoring the device's output.
