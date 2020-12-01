# Debugging with CodeSpaces

> Developing in Codespaces requires **early access** to the Codespaces beta and also to expose a port on your computer the public internet.

[Codespaces](https://github.com/features/codespaces) sets up a cloud-hosted container and a customizable VS Code environment directory from a GitHub repository. 

This removes the need to install the compiler tools locally, however device drivers are still required to interface to the local devkit.

## Install Device Drivers

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

## Create Codespaces Container

1. Go to the [Getting Started GitHub repository](https://github.com/azure-rtos/getting-started).

1. Choose `Code | Open with Codespaces`, to create and launch your Codespaces environment.

1. Setup your router to forward outside connections to port 3333 to your local machine. This allows the Codespaces GDB service to communicate with the devkit via the local OpenOCD instance.
    > Warning: This will expose the OpenOCD/JLink service to the public network which may result in an external compromise.

### Start Debugging

1. From a new command window run OpenOCD locally, using the appropriate command below:
    |Manufacturer |Devkit  |Connection Command |
    |---------|---------|---------|
    |Microchip |ATSAME54-XPRO |openocd -f board/microchip_same54_xplained_pro.cfg -c "bindto 0.0.0.0" -c init -c "reset init" |
    |MXCHIP |AZ3166 |openocd -f board/stm32f4discovery.cfg -c "bindto 0.0.0.0" -c init -c "reset init" |
    |NXP |MIMXRT1050-EVKB |JLinkGDBServerCL -select USB -device MIMXRT1052xxx6A -speed auto -if SWD -noLocalhostOnly|
    |NXP |MIMXRT1060-EVK |JLinkGDBServerCL -select USB -device MIMXRT1062xxx6A -speed auto -if SWD -noLocalhostOnly|
    |STMicroelectronics |BL475E-IOT01 |openocd -f board/stm32l4discovery.cfg -c "bindto 0.0.0.0" -c init -c "reset init" |
    |STMicroelectronics |BL4S5I-IOT01 |openocd -f board/stm32l4discovery.cfg -c "bindto 0.0.0.0" -c init -c "reset init" |

1. Open the workspace in the directory for the target devkit. 
    > For example the STM32L4 devkit workspace is `STMicroelectronics/STM32L4_L4+/STM32L4plus.code-workspace`.

1. Press `F7` to build the application.
    > Choose the `ARM GCC Cortex` CMake kit if prompted.

1. Press `F5` to start debugging the application.

1. Enter `{PUBLIC_IP_ADDRESS}:3333` when prompted for the local hostname.
    > The application will be flashed to the devKit and execution will break at main(). 

1. Press `F5` to resume execution.

1. Use a serial monitor such as [Termite](https://www.compuphase.com/software_termite.htm) to monitoring the device's output.
