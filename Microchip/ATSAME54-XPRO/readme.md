<h1>Getting started with the Microchip ATSAME54-XPro evaluation kit</h1>

**Total completion time**: 35 minutes

In this tutorial you use Azure RTOS to connect the Microchip ATSAME54-XPro (hereafter, the Microchip E54) to Azure IoT.  The article is part of the series [Getting Started with Azure RTOS](https://go.microsoft.com/fwlink/p/?linkid=2129824). The series introduces device developers to Azure RTOS, and shows how to connect several device evaluation kits to Azure IoT.

You will complete the following tasks:

* Install a set of embedded development tools for programming the Microchip E54 in C
* Build an image and flash it onto the Microchip E54
* Create an Azure IoT hub and securely connect the Microchip E54 to it
* Use Azure CLI to view device telemetry, view properties, and invoke cloud-to-device methods

## Prerequisites

* A PC running Microsoft Windows (Windows 10 recommended)
* If you don't have an Azure subscription, [create one for free](https://azure.microsoft.com/free/?WT.mc_id=A261C142F) before you begin.
* [Git](https://git-scm.com/downloads)
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

```
git clone https://github.com/azure-rtos/getting-started
```

### Install the tools

The cloned repo contains a setup script that installs and configures the first set of required tools. After you run the setup script, you can install the remaining tools manually. If you installed these tools in another tutorial in the getting started guide, you don't need to do it again.

> Note: The setup script installs the following tools:
> * [GCC](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm): Compile
> * [CMake](https://cmake.org): Build
> * [Ninja](https://ninja-build.org): Build
> * [Termite](https://www.compuphase.com/software_termite.htm): Monitor

To run the setup script:

1. Open a console app with administrator privileges, go to the following path in the repo, and run the setup script named *get-toolchain.bat*. If you use File Explorer, right-click the file and select **Run As Administrator**.

    > *getting-started\tools\get-toolchain.bat*

1. After the installation, open a new console window to recognize the configuration changes made by the setup script. Use this console to complete the remaining programming tasks in the tutorial. You can use Windows CMD, Powershell, or Git Bash for Windows.
1. Run the following code to confirm that CMake version 3.14 or later is installed.

    ```
    cmake --version
    ```

To install the remaining tool:

1. Install [Atmel Studio 7](https://www.microchip.com/mplab/avr-support/atmel-studio-7). Atmel Studio is a device development environment that includes the tools to program and flash program the Microchip E54. For this tutorial, you use Atmel Studio only to flash the Microchip E54. The installation takes several minutes, and prompts you several times to approve installation of components.

## Prepare Azure resources

To prepare Azure cloud resources and connect a device to Azure, you can use Azure CLI. There are two ways to access the Azure CLI: by using the Azure Cloud Shell, or by installing Azure CLI locally.  Azure Cloud Shell lets you run the CLI in a browser so you don't have to install anything.

Use one of the following options to run Azure CLI.  

If you prefer to run Azure CLI locally:

1. If you already have Azure CLI installed locally, run `az --version` to check the version. This tutorial requires Azure CLI 2.5.1 or later.
1. To install or upgrade, see [Install Azure CLI](https://docs.microsoft.com/cli/azure/install-azure-cli?view=azure-cli-latest). If you install Azure CLI locally, you can run CLI commands in the **GCC Command Prompt**, Git Bash for Windows, or Powershell.

If you prefer to run Azure CLI in the browser-based Azure Cloud Shell:

1. Use your Azure account credentials to sign into the Azure Cloud shell at https://shell.azure.com/.
    > Note: If this is the first time you've used the Cloud Shell, it prompts you to create storage, which is required to use the Cloud Shell.  Select a subscription to create a storage account and Microsoft Azure Files share.
1. Select Bash or Powershell as your preferred CLI environment in the **Select environment** dropdown. If you plan to use Azure Cloud Shell, keep your browser open to run the Azure CLI commands in this tutorial.

    ![Select CLI environment](media/cloud-shell-environment.png)

### Create an IoT hub

You can use Azure CLI to create an IoT hub that handles events and messaging for your device.

To create an IoT hub:

1. In your CLI console, run the [az extension add](https://docs.microsoft.com/cli/azure/extension?view=azure-cli-latest#az-extension-add) command to add the Microsoft Azure IoT Extension for Azure CLI to your CLI shell. The IOT Extension adds IoT Hub, IoT Edge, and IoT Device Provisioning Service (DPS) specific commands to Azure CLI.

   ```azurecli
   az extension add --name azure-iot
   ```

1. Run the [az group create](https://docs.microsoft.com/cli/azure/group?view=azure-cli-latest#az-group-create) command to create a resource group. The following command creates a resource group named *MyResourceGroup* in the *eastus* location.
    > Note: To set an alternate `location`, run [az account list-locations](https://docs.microsoft.com/cli/azure/account?view=azure-cli-latest#az-account-list-locations) to see available locations. Specify the alternate location in the following command in place of *eastus*.

    ```azurecli
    az group create --name MyResourceGroup --location eastus
    ```

1. Run the [az iot hub create](https://docs.microsoft.com/cli/azure/iot/hub?view=azure-cli-latest#az-iot-hub-create) command to create an IoT hub. It might take a few minutes to create an IoT hub.

    *YourIotHubName*. Replace this placeholder below with the name you chose for your IoT hub. An IoT hub name must be globally unique in Azure. This placeholder is used in the rest of this tutorial to represent your unique IoT hub name.

    ```azurecli
    az iot hub create --resource-group MyResourceGroup --name {YourIoTHubName}
    ```

    > Note: The Basic tier is **not supported** by this guide as it requires cloud-to-device communication.

1. After the IoT hub is created, view the JSON output in the console, and copy the `hostName` value from the following named field to a safe place. You use this value in a later step.

### Register a device

In this section, you create a new device instance and register it with the Iot hub you created. You will use the connection information for the newly registered device to securely connect your physical device in a later section.

To register a device:

1. In your console, run the [az iot hub device-identity create](https://docs.microsoft.com/cli/azure/ext/azure-cli-iot-ext/iot/hub/device-identity?view=azure-cli-latest#ext-azure-cli-iot-ext-az-iot-hub-device-identity-create) command. This creates the simulated device identity.

    *YourIotHubName*. Replace this placeholder below with the name you chose for your IoT hub.

    *MyMicrochipDevice*. You can use this name directly for the device in CLI commands in this tutorial. Optionally, use a different name.

    ```azurecli
    az iot hub device-identity create --device-id MyMicrochipDevice --hub-name {YourIoTHubName}
    ```

1. After the device is created, view the JSON output in the console, and copy the `deviceId` and `primaryKey` values to use in a later step.

Confirm that you have the copied the following values from the JSON output to use in the next section:

> * `hostName`
> * `deviceId`
> * `primaryKey`

## Prepare the device

To connect the Microchip E54 to Azure, you'll modify a configuration file for Azure IoT settings, rebuild the image, and flash the image to the device.

### Add configuration

1. Open the following file in a text editor:

    > *getting-started\Microchip\ATSAME54-XPRO\app\azure_config.h*

1. Set the Azure IoT device information constants to the values that you saved after you created Azure resources.

    |Constant name|Value|
    |-------------|-----|
    |`IOT_HUB_HOSTNAME` |{*Your Iot hub hostName value*}|
    |`IOT_DEVICE_ID` |{*Your deviceID value*}|
    |`IOT_PRIMARY_KEY` |{*Your primaryKey value*}|

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

    ```
    atprogram --tool edbg --interface SWD --device ATSAME54P20A program --chiperase --file atsame54_azure_iot.bin --verify
    ```

    After the flashing process completes, the console confirms that programming was successful:

    ```
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

    ![Termite settings](media/termite-settings.png)
1. Select OK.
1. Press the **Reset** button on the device.
1. In the **Termite** console, check the following checkpoint values to confirm that the device is initialized and connected to Azure IoT.

    |Checkpoint name|Output value|
    |---------------|-----|
    |DHCP |SUCCESS: DHCP initialized|
    |DNS |SUCCESS: DNS client initialized|
    |SNTP |SUCCESS: SNTP initialized|
    |MQTT |SUCCESS: MQTT client initialized|

    The Termite terminal shows the details about the device, your connection, and the checkpoint values.

    ```
    Starting Azure thread
    Initializing DHCP
    	IP address: 192.168.1.132
    	Mask: 255.255.255.0
    	Gateway: 192.168.1.1
    SUCCESS: DHCP initialized
    
    Initializing DNS client
    	DNS address: 192.168.1.1
    SUCCESS: DNS client initialized
    
    Initializing SNTP client
    SNTP time update: May 15, 2020 15:43:42.951 UTC 
    SUCCESS: SNTP initialized
    
    Initializing MQTT client
    SUCCESS: MQTT client initialized
    
    Time 1589557423
    Starting MQTT thread
    Sending telemetry
    Sending message {"temperature(c)": 23.50}
    ```

Keep the terminal open to monitor device output in the following steps.

## View telemetry

You can use Azure CLI to inspect the flow of telemetry from the device to Azure IoT.

1. In your CLI console, run the [az iot hub monitor-events](https://docs.microsoft.com/cli/azure/ext/azure-cli-iot-ext/iot/hub?view=azure-cli-latest#ext-azure-cli-iot-ext-az-iot-hub-monitor-events) command to monitor telemetry from your device. Use the names that you created previously in Azure IoT for your device and IoT hub.

    > Note: The first time you run this command after installing, Azure CLI might prompt to install a *Dependency update (uamqp 1.2) required for IoT extension version: 0.9.1*. Select *y* to install the update. If the CLI prompts you to install another extension named *azure-cli-iot-ext*, do not install it. The current extension to use is the *azure-iot* extension that you installed previously.

    ```azurecli
    az iot hub monitor-events --device-id MyMicrochipDevice --hub-name {YourIoTHubName}
    ```

1. To force the Microchip E54 to reconnect and send telemetry, press **Reset**.

    View the telemetry in the CLI console's JSON output.

    ```json
    {
        "event": {
            "origin": "MyMicrochipDevice",
            "payload": "{\"temperature(c)\": 23.50}"
        }
    }
    ```

1. Select CTRL+C to end monitoring.

## View device properties

Using Azure CLI, you can inspect the properties on your Azure resources, including your connected device.

1. Run the [az iot hub device-identity list](https://docs.microsoft.com/cli/azure/ext/azure-cli-iot-ext/iot/hub/device-identity?view=azure-cli-latest#ext-azure-cli-iot-ext-az-iot-hub-device-identity-list) command to list devices attached to your Iot hub.

    ```azurecli
    az iot hub device-identity list --hub-name {YourIoTHubName}
    ```

    The following partial JSON output shows how the connected device is included in the device list.

    ```json
    {
    "authenticationType": "sas",
    "capabilities": {
        "iotEdge": false
    },
    "cloudToDeviceMessageCount": 0,
    "connectionState": "Connected",
    "deviceEtag": "Njc0NTAzODkw",
    "deviceId": "MyMicrochipDevice",
    ```

1. Run the [az iot hub device-identity show](https://docs.microsoft.com/cli/azure/ext/azure-cli-iot-ext/iot/hub/device-identity?view=azure-cli-latest#ext-azure-cli-iot-ext-az-iot-hub-device-identity-show) command to view the properties of your device.

    ```azurecli
    az iot hub device-identity show --device-id MyMicrochipDevice --hub-name {YourIoTHubName}
    ```

## Call a direct method on the device

You can use the Azure CLI to call a direct method that you have implemented on your device. Direct methods have a name, and can optionally have a JSON payload, configurable connection, and method timeout. In this section, you call a method that enables you to turn an LED on or off.

To call a method to turn the LED on:

1. Run the [az iot hub invoke-device-method](https://docs.microsoft.com/cli/azure/ext/azure-cli-iot-ext/iot/hub?view=azure-cli-latest#ext-azure-cli-iot-ext-az-iot-hub-invoke-device-method) command, and specify the method name and payload. For this method, setting `method-payload` to `1` turns the LED on, and setting it to `0` turns it off.

    <!-- Inline code tag and CSS to wrap long code lines. -->
    <code style="white-space : pre-wrap !important;">
    az iot hub invoke-device-method --device-id MyMicrochipDevice --method-name set_led_state --method-payload 1 --hub-name {YourIoTHubName}
    </code>

    The CLI console shows the status of your method call on the device, where `204` indicates success.

    ```json
    {
      "payload": {},
      "status": 204
    }
    ```

1. Check your device to confirm the LED state.

1. View the Termite terminal to confirm the output messages:

    ```json
    Received direct method=set_led_state, id=6, message=1
    LED0 is turned ON
    Sending device twin update with bool value
    Sending message {"led0State": 1}
    Direct method=set_led_state invoked
    ```

## Clean up resources

If you no longer need the Azure resources created in this tutorial, you can use the Azure CLI to delete them.

If you continue to another tutorial in this Getting Started guide, you can keep the resources you've already created and reuse them.

> **Important**: Deleting a resource group is irreversible. The resource group and all the resources contained in it are permanently deleted. Make sure that you do not accidentally delete the wrong resource group or resources.

To delete a resource group by name:
1. Run the [az group delete](https://docs.microsoft.com/cli/azure/group?view=azure-cli-latest#az-group-delete) command. This removes the resource group, the IoT Hub, and the device registration you created.

    ```azurecli
    az group delete --name MyResourceGroup
    ```

1. Run the [az group list](https://docs.microsoft.com/cli/azure/group?view=azure-cli-latest#az-group-list) command to confirm the resource group is deleted.  

    ```azurecli
    az group list
    ```

## Next Steps

In this tutorial you built a custom image that contains Azure RTOS sample code, and then flashed the image to the Microchip E54 device. You also used the Azure CLI to create Azure resources, connect the Microchip E54 securely to Azure, view telemetry, and send messages.

* For device developers, the suggested next step is to see the other tutorials in the series [Getting started with Azure RTOS](https://go.microsoft.com/fwlink/p/?linkid=2129824).
* If you have issues getting your device to initialize or connect after following the steps in this guide, see [Troubleshooting](../../docs/troubleshooting.md).
* To learn more about how Azure RTOS components are used in the sample code for this tutorial, see [Using Azure RTOS in the Getting Started Guides](../../docs/using-azure-rtos.md).
    >Note: Azure RTOS provides OEMs with components to secure communication and to create code and data isolation using underlying MCU/MPU hardware protection mechanisms. However, each OEM is ultimately responsible for ensuring that their device meets evolving security requirements.