# Debugging with Visual Studio Code + OpenOCD (beta)

This article on debugging is under construction. It refers to products in private beta, and also uses techniques that could expose your internet network to external attack. Care should be taken when following the steps below.

## Install Open OCD

1. Install the relevant driver for the devkits on chip debugger:
    |Devkit |Driver |
    |---------|---------|
    |STM BL475E-IOT01 |[STLink](https://www.st.com/en/development-tools/stsw-link004.html) |
    |STM BL4S5I-IOT01 |[STLink](https://www.st.com/en/development-tools/stsw-link004.html) |

1. Download and unzip [OpenOCD](https://gnutoolchains.com/arm-eabi/openocd) onto your computer and add it to your environment path.

## Setup your Project

You can choose to develop locally or develop in [Codespaces](https://github.com/features/codespaces).

* Developing locally requires the installation of the build/debug tools on your local machine to build the application.
* Developing in Codespaces doesn't require additional installation, however it requires early access to the Codespaces beta and also to expose a port on your computer the public internet.

### Setup Local Development

1. Go to the [Getting Started GitHub repository](https://github.com/azure-rtos/getting-started)
1. Recursively clone the repository to your machine.
1. Run as Administrator "tools\get-toolchain.bat" from the repository clone to install the required build tools.
1. Install [Visual Studio Code](https://code.visualstudio.com).
1. Open Visual Studio Code and install the required extensions:
    * Required
      * [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)
    * Recommended
      * [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)
      * [CMake](https://marketplace.visualstudio.com/items?itemName=twxs.cmake)

### Setup Codespaces Development

1. Go to the [Getting Started GitHub repository](https://github.com/azure-rtos/getting-started).
1. Choose "Code | Open with Codespaces", to create and launch your Codespaces environment.
1. Setup your router to forward outside connections to port 3333 to your local machine. This allows the Codespaces GDB service to communicate with the devkit via the local OpenOCD instance.
    > Warning: This will expose the OpenOCD service to the public network which may result in an external compromise.

1. Install [Termite](https://www.compuphase.com/software_termite.htm) for serial port monitoring if desired.

## Start Debugging

1. From a new command window run OpenOCD locally, using the appropriate command below:
    |Devkit  |OpenOCD Command |
    |---------|---------|
    |STM BL475E-IOT01 |openocd.exe -f "board/stm32l4discovery.cfg" -c "bindto 0.0.0.0" -c init -c "reset init"

1. Select "Add a Folder to your Workspace...", and choose the folder for the devkit of your choice.
    > For example: choose the "STMicroelectronics/STM32L4_L4+" folder for the B-L475-IOT01 devkit.

1. Build the application by pressing "F7"
    1. Choose the "ARM GCC Cortex" CMake kit when prompted

1. Start the debugger by pressing "F5".
    1. For debugging locally, use "localhost:3333" for the OpenOCD hostname.
    1. For debugging via Codespaces, specify {PUBLIC_IP_ADDRESS}:3333
