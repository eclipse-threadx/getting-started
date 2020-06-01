# MXChip AZ3166 IoT DevKit with Azure RTOS

## Prerequistes

* [Ubuntu 18.04 installed in WSL2](https://docs.microsoft.com/windows/wsl/wsl2-install) on Windows 10. You will set up your development environment in it.
* [Visual Studio Code](https://code.visualstudio.com/) for Windows with [Remote - WSL](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-wsl) extension installed. This is for code editing and debugging with UI supported. Learn [Develop in WSL](https://code.visualstudio.com/docs/remote/wsl) if you are new to it.
* [OpenOCD for Windows](https://gnutoolchains.com/arm-eabi/openocd/) for flashing and debugging the firmware.
* [Termite](https://www.compuphase.com/software_termite.htm) terminal for viewing log output from COM port.
* [Azure IoT Explorer](https://github.com/Azure/azure-iot-explorer/releases) is a cross platform IoT Hub explorer for retrieving the device connection string and monitoring data communication between device and IoT Hub.

## Preparation

1. Launch Ubuntu 18.04 bash command line.

    ![Ubuntu](./media/ubuntu.png)

1. Download and untar [GCC ARM Toolchain in Linux](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads).

    ```bash
    wget -c "https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-rm/9-2019q4/gcc-arm-none-eabi-9-2019-q4-major-x86_64-linux.tar.bz2"

    untar xvjf ./gcc-arm-none-eabi-9-2019-q4-major-x86_64-linux.tar.bz2 ~/tools/.
    ```

1. Install CMake and Ninja build system.

    ```bash
    sudo apt update && apt install -y cmake ninja-build

    cmake --version
    ```
    Make sure the CMake version is above 3.14.0. If not, you can follow [this guide](https://apt.kitware.com/) to install the latest CMake.

1. Launch Visual Studio Code, click the left bottom corner to launch Remote command palette and select **Remote-WSL: New Window**.

    ![VSCode WSL](./media/vscode-wsl.png)

1. In Extensions tab (`Ctrl+Shift+X`), search and install the following extensions in the WSL.

    * C/C++
    * CMake
    * Cortex-Debug

    ![VSCode Install Extension](./media/vscode-install-ext.png)

## Configure Azure IoT Hub

1. Follow [the guide](https://docs.microsoft.com/azure/iot-hub/iot-hub-create-through-portal) to create an Azure IoT Hub.

1. Save the IoT Hub Connection String in **IoT Hub -> Setting -> Shared access policies -> iothubowner -> Connection string** for later use.

1. Launch Azure IoT Explorer, paste the IoT Hub connection string you just
got and select **Connect**.

    ![IoT Hub Connection String](./media/iothub-conn-string.png =1200x)

1. Select **New** to create a new IoT device. Enter a device ID for your device
and keep the rest options as default. Then select **Create**.

    ![New device](./media/new-device.png =1200x)

1. Select from the list for the device you just created. In the **Device identity**
tab, copy **Device ID** and **Primary Key** to Notepad for later use.

1. Also copy the **IoT Hub host name** from the highlighted area to the
Notepad.

    ![Host name](./media/host-name.png =1200x)

1. You need all these three device credentials for the sample code
to connect to the IoT Hub.
    * IoT Hub Host Name
    * IoT Hub Device ID
    * IoT Hub Device Primary Key

## Build

We develop and build everything within WSL2.

1. Launch Ubuntu 18.04 bash command line.

1. Clone or download the source code as zip file.

    ```bash
    git clone https://github.com/azure-rtos/getting-started.git
    ```

1. Launch VSCode, update the `WIFI_SSID` and `WIFI_PASSWORD` for the WiFi that you will connect the MXChip IoT DevKit to. And `IOT_HUB_HOSTNAME`, `IOT_DEVICE_ID` and `IOT_PRIMARY_KEY` you noted in the previous step in `~/getting-started/MXChip/AZ3166/app/azure_config.h`.

    ```c
    // ----------------------------------------------------------------------------
    // WiFi connection information
    // ----------------------------------------------------------------------------
    #define WIFI_SSID           ""

    #define WIFI_PASSWORD       ""

    /* WIFI Security, the security types are defined in wwwd_constants.h.  */
    #define WIFI_SECURITY       WICED_SECURITY_WPA2_MIXED_PSK
    /* Country codes are defined in wwd_constants.h. */
    #define WIFI_COUNTRY        WICED_COUNTRY_CHINA

    // ----------------------------------------------------------------------------
    // Azure IoT Device information
    // ----------------------------------------------------------------------------
    #define IOT_HUB_HOSTNAME    ""
    #define IOT_DEVICE_ID       ""
    #define IOT_PRIMARY_KEY     ""
    ```

    The following shows example values:

    ```c
    #define IOT_HUB_HOSTNAME    "azurertos.azure-devices.net"

    #define IOT_DEVICE_ID       "az3166"

    #define IOT_PRIMARY_KEY     “hnCCA9Xq...l3izgg=”
    ```

1. Build the project with CMake and Ninja.

    ```bash
    cd ./getting-started/MXChip/AZ3166/

    ./rebuild.sh
    ```

1. The compiled binary files are located in the `build/app` folder.

## Run

1. Download and unzip [OpenOCD for Windows](https://gnutoolchains.com/arm-eabi/openocd/) to `C:\Program Files(x86)\OpenOCD`. Add `openocd.exe` path in Windows Path Environment Variables.

1. Install ST-Link driver within unzipped OpenOCD folder by running `stlink_winusb_install.bat`.

1. In Windows command line or PowerShell, launch **openocd** server:

    ```cmd
    openocd -c "bindto 0.0.0.0" -s "C:\Program Files(x86)\OpenOCD\share\openocd\scripts" -f interface/stlink.cfg -f target/stm32f4x.cfg
    ```

1. Plugin your MXChip IoT DevKit to your computer.

1. Launch Termite. Make sure you select the right COM port for the DevKit and baud rate is set to **115200**.

1. Launch Ubuntu 18.04 bash command line. Get Windows host IP address:

    ```bash
    cat /etc/resolv.conf
    ```

1. Replace the IP address in `./getting-started/MXChip/AZ3166/.vscode/launch.json`.

    ```json
    // Get WSL2 IP from "cat /etc/resolv.conf"
    "gdbTarget": "{Your Windows host IP}:3333",
    ```

1. In VSCode, press `F5` or launch debug Run tab. Then select `mxchip_azure_iot`.

    ![VSCode debug](./media/launch.png)

1. It will first flash the firmware onto the DevKit and start running it and stopped at `main()`. Press `F5` again or select continue to run the app.

1. Open Device Manager and find the COM port for the MXChip IoT DevKit.

    ![COM Port](./media/com_port.png)

1. Open Termite and configure the COM port and settings as:

    * Baud rate: 115200
    * Data bits: 8
    * Stop bits: 1

    Now you can view the DevKit is publising temperature telemetry to IoT Hub in every a few seconds.

    ![Termite](./media/termite.png)

## Debug

View [Debug C++ in Visual Studio Code](https://code.visualstudio.com/docs/cpp/cpp-debug) to learn debugging in VS Code.

![Debugging](./media/debugging.png)

## Clean

* Delete the entire build folder to clean things up.

    ```bash
    rm -rf ./build
    ```