---
page_type: sample
description: Connecting an Microchip ATSAME54-XPro device to Azure IoT using Azure RTOS
languages:
- c
products:
- azure-iot
- azure-iot-pnp
- azure-rtos
- azure-iot-dps
- azure-iot-hub
---

# Getting started with the Microchip ATSAME54-XPro evaluation kit

**Total completion time**: 45 minutes

In this tutorial you use Azure RTOS to connect the Microchip ATSAME54-XPro (hereafter, the Microchip E54) to Azure IoT.  The article is part of the series [Getting Started with Azure RTOS](https://go.microsoft.com/fwlink/p/?linkid=2129824). The series introduces device developers to Azure RTOS, and shows how to connect several device evaluation kits to Azure IoT.

You will complete the following tasks:

* Install a set of embedded development tools for programming the Microchip E54 in C
* Build an image and flash it onto the Microchip E54
* Use Azure IoT Central to create cloud components, view properties, view device telemetry, and call direct commands.

## Prerequisites

* A PC running Microsoft Windows 10
* [Git](https://git-scm.com/downloads) for cloning the repository
* Hardware

    > * The [Microchip ATSAME54-XPro](https://www.microchip.com/developmenttools/productdetails/atsame54-xpro) (Microchip E54)
    > * USB 2.0 A male to Micro USB male cable
    > * Wired Ethernet access
    > * Ethernet cable
    > * Optional: [Weather Click](https://www.mikroe.com/weather-click) sensor. You can add this sensor to the device to monitor weather conditions. If you don't have this sensor, you can still complete this tutorial.
    > * Optional: [mikroBUS Xplained Pro](https://www.microchip.com/Developmenttools/ProductDetails/ATMBUSADAPTER-XPRO) adapter. Use this adapter to attach the Weather Click sensor to the Microchip E54. If you don't have the sensor and this adapter, you can still complete this tutorial.

## Prepare the development environment

To set up your development environment, first you clone a GitHub repo that contains all the assets you need for the tutorial. Then you install a set of programming tools.

### Clone the repo for the tutorial

Clone the following repo to download all sample device code, setup scripts, and offline versions of the documentation. If you previously cloned this repo in another tutorial, you don't need to do it again.

To clone the repo, run the following command:

```shell
git clone --recursive https://github.com/azure-rtos/getting-started.git
```

### Install the tools

The cloned repo contains a setup script that installs and configures the required tools. If you installed these tools in another tutorial in the getting started guide, you don't need to do it again.

> Note: The setup script installs the following tools:

> * [GCC](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm): Compile
> * [CMake](https://cmake.org): Build
> * [Termite](https://www.compuphase.com/software_termite.htm): Monitor serial port output for connected devices

To install the tools:

1. From File Explorer, navigate to the following path in the repo and run the setup script named *get-toolchain.bat*:

    > *getting-started\tools\get-toolchain.bat*

1. After the installation, open a new console window to recognize the configuration changes made by the setup script. Use this console to complete the remaining programming tasks in the tutorial. You can use Windows CMD, PowerShell, or Git Bash for Windows.
1. Run the following code to confirm that CMake version 3.14 or later is installed.

    ```shell
    cmake --version
    ```

To install the remaining tools:

1. Install [Atmel Studio 7](https://www.microchip.com/mplab/avr-support/atmel-studio-7). Atmel Studio is a device development environment that includes the tools to program and flash the Microchip E54. For this tutorial, you use Atmel Studio only to flash the Microchip E54. The installation takes several minutes, and prompts you several times to approve the installation of components.

## Create the cloud components

### Create the IoT Central Application

There are several ways to connect devices to Azure IoT. In this section, you learn how to connect a device by using Azure IoT Central. IoT Central is an IoT application platform that reduces the cost and complexity of creating and managing IoT solutions.

To create a new application:
1. From [Azure IoT Central portal](https://apps.azureiotcentral.com/), select **My apps** on the side navigation menu.
1. Select **+ New application**.
1. Select **Custom apps**.
1. Add Application Name and a URL.
1. Choose the **Free** Pricing plan to activate a 7-day trial.

    ![IoT Central create an application](media/iotcentralcreate-custom.png)

1. Select **Create**.
1. After IoT Central provisions the application, it redirects you automatically to the new application dashboard.

    > Note: If you have an existing IoT Central application, you can use it to complete the steps in this article rather than create a new application.

### Create a new device

In this section, you use the IoT Central application dashboard to create a new device. You will use the connection information for the newly created device to securely connect your physical device in a later section.

To create a device:
1. From the application dashboard, select **Devices** on the side navigation menu.
1. Select **+ New** to bring up the **Create a new device** window.
1. Leave Device template as **Unassigned**.
1. Fill in the desired Device name and Device ID.

    ![IoT Central create a device](media/iotcentralcreate-device.png)

1. Select the **Create** button.
1. The newly created device will appear in the **All devices** list.  Select on the device name to show details.
1. Select **Connect** in the top right menu bar to display the connection information used to configure the device in the next section.

    ![IoT Central create a device](media/iotcentraldevice-connection-info.png)

1. Note the connection values for the following connection string parameters displayed in **Connect** dialog. You'll add these values to a configuration file in the next step:

> * `ID scope`
> * `Device ID`
> * `Primary key`

## Prepare the device

To connect the Microchip E54 to Azure, you'll modify a configuration file for Azure IoT settings, rebuild the image, and flash the image to the device.

### Add configuration

1. Open the following file in a text editor:

    > *getting-started\Microchip\ATSAME54-XPRO\app\azure_config.h*

1. Set the Azure IoT device information constants to the values that you saved after you created Azure resources.

    |Constant name|Value|
    |-------------|-----|
    |`IOT_DPS_ID_SCOPE` |{*Your ID scope value*}|
    |`IOT_DPS_REGISTRATION_ID` |{*Your Device ID value*}|
    |`IOT_DEVICE_SAS_KEY` |{*Your Primary key value*}|

1. Save and close the file.

### Connect the device

1. On the Microchip E54, locate the **Reset** button, the **Ethernet** port, and the Micro USB port which is labeled **Debug USB**. Each component is highlighted in the following picture:

    ![Microchip E54 reset button and micro usb port](media/microchip-xpro-board.png)

1. Connect the Micro USB cable to the **Debug USB** port on the Microchip E54, and then connect it to your computer.
    > Note: Optionally, for more details about setting up and getting started with the Microchip E54, see [SAM E54 Xplained Pro User's Guide](http://ww1.microchip.com/downloads/en/DeviceDoc/70005321A.pdf).

1. Use the Ethernet cable to connect the Microchip E54 to an Ethernet port.

### Optional: Install a weather sensor

If you have the Weather Click sensor and the mikroBUS Xplained Pro adapter, follow the steps in this section.  If you don't have them, skip to [Build the image](#build-the-image). You can complete this tutorial even if you don't have a sensor. The sample code for the device returns simulated data if a real sensor is not present.

1. If you have the Weather Click sensor and the mikroBUS Xplained Pro adapter, install them on the Microchip E54 as shown in the following photo:

    ![Microchip E54 with Weather click sensor](media/sam-e54-sensor.png)

1. Reopen the configuration file you edited previously:

    > *getting-started\Microchip\ATSAME54-XPRO\app\azure_config.h*

1. Set the value of the constant `__SENSOR_BME280__` to *1* as shown in the following code from the header file. Setting this value enables the device to use real sensor data from the Weather Click sensor.

    > `#define __SENSOR_BME280__ 1`

1. Save and close the file.

### Build the image

In your console or in File Explorer, run the script *rebuild.bat* at the following path to build the image:

> *getting-started\Microchip\ATSAME54-XPRO\tools\rebuild.bat*

After the build completes, confirm that a binary file was created in the following path:

> *getting-started\Microchip\ATSAME54-XPRO\build\app\atsame54_azure_iot.bin*

### Flash the image

1. Open the **Windows Start > Atmel Studio 7.0 Command Prompt** console, and go to the folder of the Microchip E54 binary file that you built.

    > *getting-started\Microchip\ATSAME54-XPRO\build\app*

1. Use the *atprogram* utility to flash the Microchip E54 with the binary image:
    > Note: For more details about using the Atmel-ICE and *atprogram* tools with the Microchip E54, see [Using Atmel-ICE for AVR Programming In Mass Production](http://ww1.microchip.com/downloads/en/AppNotes/00002466A.pdf).

    ```shell
    atprogram --tool edbg --interface SWD --device ATSAME54P20A program --chiperase --file atsame54_azure_iot.bin --verify
    ```

    After the flashing process completes, the console confirms that programming was successful:

    ```output
    Firmware check OK
    Programming and verification completed successfully.
    ```

### Confirm device connection details

You can use the **Termite** utility to monitor communication and confirm that your device is set up correctly.
> Note: If you have issues getting your device to initialize or connect after flashing, see [Troubleshooting](../../docs/troubleshooting.md).

1. Start **Termite**.
1. Select **Settings**.
1. In the **Serial port settings** dialog, check the following settings and update if needed:
    * **Baud rate**: 115,200
    * **Port**: The port that your Microchip E54 is connected to. If there are multiple port options in the dropdown, you can find the correct port to use. Open Windows **Device Manager**, and view **Ports** to identify which port to use.
    * **Flow control**: DTR/DSR

    ![Termite](media/termite-settings.png)
1. Select OK.
1. Press the **Reset** button on the device.
1. In the **Termite** console, check the following checkpoint values to confirm that the device is initialized and connected to Azure IoT.

    ```output
    Starting Azure thread

    Initializing DHCP
    	IP address: 10.0.0.123
    	Mask: 255.255.255.0
    	Gateway: 10.0.0.1
    SUCCESS: DHCP initialized

    Initializing DNS client
    	DNS address: 10.0.0.1
    SUCCESS: DNS client initialized

    Initializing SNTP client
    	SNTP server 0.pool.ntp.org
    	SNTP IP address: 185.242.56.3
    	SNTP time update: Nov 16, 2020 23:47:35.385 UTC 
    SUCCESS: SNTP initialized

    Initializing Azure IoT DPS client
    	DPS endpoint: global.azure-devices-provisioning.net
    	DPS ID scope: ***
    	Registration ID: ***
    SUCCESS: Azure IoT DPS client initialized

    Initializing Azure IoT Hub client
    	Hub hostname: ***
    	Device id: ***
    	Model id: dtmi:azurertos:devkit:gsg;1
    Connected to IoTHub
    SUCCESS: Azure IoT Hub client initialized

    Starting Main loop
    ```

Keep Termite open to monitor device output in the following steps.

## Verify the device status

To view the device status in IoT Central portal:
1. From the application dashboard, select **Devices** on the side navigation menu.
1. Check the **Device status** of the device is updated to **Provisioned**.
1. Check the **Device template** of the device has updated to **Getting Stared Guide**.

    ![IoT Central device status](media/azure-iot-central-device-view-status.png)

> Note: If the **Device template** of the device remains as **Unassigned**, select the device and select **Migrate** to the "Getting Started Guide" template in order to see the graphic visual data representation.

![IoT Central device template migrate](media/azure-iot-central-device-template-migrate.png)

## View telemetry

With IoT Central, you can view the flow of telemetry from your device to the cloud.

To view telemetry in IoT Central portal:

1. From the application dashboard, select **Devices** on the side navigation menu.
1. Select the device from the device list.
1. View the telemetry as the device sends messages to the cloud in the **Overview** tab.

    ![IoT Central device telemetry](media/azure-iot-central-device-telemetry.png)

> Note: You can also monitor telemetry from the device by using the Termite terminal.

## Call a direct method on the device

You can also use IoT Central to call a direct method that you have implemented on your device. Direct methods have a name, and can optionally have a JSON payload, configurable connection, and method timeout. In this section, you call a method that enables you to turn an LED on or off.

To call a method in IoT Central portal:

1. Select **Command** tab from the device page.
1. Select **State** and select **Run**.  The LED light should turn on.

    ![IoT Central invoke method](media/azure-iot-central-invoke-method.png)
1. Unselect **State** and select **Run**. The LED light should turn off.

## View device information

You can view the device information from IoT Central.

Select **About** tab from the device page.
![IoT Central device info](media/azure-iot-central-device-about.png)

## Debugging

For debugging the application, see [Debugging with Visual Studio Code](../../docs/debugging.md).

## Clean up resources

If you no longer need the Azure resources created in this tutorial, you can delete them from the IoT Central portal. Optionally, if you continue to another tutorial in this Getting Started guide, you can keep the resources you've already created and reuse them.

To keep the Azure IoT Central sample application but remove only specific devices:
1. Select the **Devices** tab for your application.
1. Select the device from the device list.
1. Select **Delete**.

To remove the entire Azure IoT Central sample application and all its devices and resources:
1. Select **Administration** > **Your application**.
1. Select **Delete**.

## Next Steps

In this tutorial you built a custom image that contains Azure RTOS sample code, and then flashed the image to the Microchip E54 device. You also used the IoT Central portal to create Azure resources, connect the Microchip E54 securely to Azure, view telemetry, and send messages.

* For device developers, the suggested next step is to see the other tutorials in the series [Getting started with Azure RTOS](https://go.microsoft.com/fwlink/p/?linkid=2129824).
* If you have issues getting your device to initialize or connect after following the steps in this guide, see [Troubleshooting](../../docs/troubleshooting.md).
* To learn more about how Azure RTOS components are used in the sample code for this tutorial, see [Using Azure RTOS in the Getting Started Guides](../../docs/using-azure-rtos.md).
    >Note: Azure RTOS provides OEMs with components to secure communication and to create code and data isolation using underlying MCU/MPU hardware protection mechanisms. However, each OEM is ultimately responsible for ensuring that their device meets evolving security requirements.
