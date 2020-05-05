<h1>Getting started with the STMicroelectronics B-L475E-IOT01</h1>

[[_TOC_]]

**Total completion time**:  45-60 minutes

In this tutorial you use Azure RTOS to connect the STMicroelectronics B-L475E-IOT01 (hereafter, the STM  DevKit) to Azure IoT.  The article is part of the series [Getting Started with Azure RTOS](https://review.docs.microsoft.com/azure/rtos/getting-started?branch=master). The series introduces device developers to Azure RTOS, and shows how to connect several micro-controller units (MCU) to Azure IoT.

You will complete the following tasks:

* Install a set of embedded development tools for programming the STM DevKit in C
* Build an image and flash it onto the STM MCU
* Create an Azure IoT hub and securely connect the STM DevKit to it
* Use Azure CLI to view device telemetry, view properties, and invoke cloud-to-device methods

## Prerequisites

* A PC running Microsoft Windows (Windows 10 recommended)
* If you don't have an Azure subscription, [create one for free](https://azure.microsoft.com/free/?WT.mc_id=A261C142F) before you begin.
* [Git](https://git-scm.com/downloads)
* Hardware

    > * The [STMicroelectronics B-L475E-IOT01](https://www.st.com/content/st_com/en/products/evaluation-tools/product-evaluation-tools/mcu-mpu-eval-tools/stm32-mcu-mpu-eval-tools/stm32-discovery-kits/b-l475e-iot01a.html) (STM DevKit)
    > * Wi-Fi access
    > * USB Cable (data capable)

## Prepare the development environment

To set up your development environment, first you clone a GitHub repo that contains all the assets you need for the tutorial. Then you install a set of programming tools.

### Clone the repo for the tutorial

Clone the following repo to download all sample device code, setup scripts, and offline versions of the documentation for the series [Getting Started with Azure RTOS](https://review.docs.microsoft.com/azure/rtos/getting-started?branch=master). If you previously cloned this repo, you don't need to do it again.

To clone the repo, run the following command:

    ```
    git clone https://github.com/AzureRTOS/getting-started
    ```

### Install the tools

The cloned repo contains a setup script that installs and configures the first set of required tools. After you run the setup script, you can install the remaining tools manually.

> Note: The script uses the Visual Studio 2019 installer to install the following set of tools:
> * [GCC](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm): Compile
> * [CMake](https://cmake.org): Build
> * [Ninja](https://ninja-build.org): Build

To run the setup script:

1. In the cloned repo folder, run the setup script found at `tools\get-toolchain.bat`. If the script prompts for elevated access during the installation, enable it.
1. After the installation, open the console app found at **Windows Start > Visual Studio 2019 > Developer Command Prompt for VS 2019**. You must use this console to use the installed programming environment. Keep the **Developer Command Prompt** open for use later in the tutorial.
1. Run the following code to confirm proper installation and version of CMake.

    ```
    cmake --version
    ```

To install the remaining tools:

1. Install [ST-LINK](https://www.st.com/en/development-tools/stsw-link004.html). You use this utility to flash the device.

    > Note: Before you download ST-LINK, register for a free user account.

1. Install [Termite](https://www.compuphase.com/software_termite.htm). You use this utility to monitor your device.

## Prepare Azure resources

To prepare Azure cloud resources and connect a device to Azure, you can use Azure CLI. There are two ways to access the Azure CLI: by using the Azure Cloud Shell, or by installing Azure CLI locally.  Azure Cloud Shell lets you run the CLI in a browser so you don't have to install anything.

Use one of the following options to run Azure CLI.  

If you prefer to run Azure CLI locally:

1. If you already have Azure CLI installed locally, run `az --version` to check the version. This tutorial uses Azure CLI 2.4.0 or later.
1. To install or upgrade, see [Install Azure CLI](https://docs.microsoft.com/cli/azure/install-azure-cli?view=azure-cli-latest). If you install Azure CLI locally, you can run CLI commands in the **Developer Command Prompt**.

If you prefer to run Azure CLI in the browser-based Azure Cloud Shell:

1. Use your Azure account credentials to sign into the Azure Cloud shell at https://shell.azure.com/.
    > Note: If this is the first time you've used the Cloud Shell, it prompts you to create storage, which is required to use the Cloud Shell.  Select a subscription to create a storage account and Microsoft Azure Files share.
1. Select your preferred CLI environment in the **Select environment** dropdown. This tutorial uses the **Bash** environment. Azure CLI commands work in the Powershell environment too.

    ![Select CLI environment](images/cloud-shell-environment.png)
1. Keep the browser open to run the CLI commands in later steps.

### Create an IoT hub

You can use Azure CLI to create an IoT hub that handles events and messaging for your device.

To create an IoT hub:

1. In your console, run the [az extension add](https://docs.microsoft.com/cli/azure/extension?view=azure-cli-latest#az-extension-add) command to add the Microsoft Azure IoT Extension for Azure CLI to your CLI shell. The IOT Extension adds IoT Hub, IoT Edge, and IoT Device Provisioning Service (DPS) specific commands to Azure CLI.

   ```azurecli
   az extension add --name azure-iot
   ```

1. Run the [az group create](https://docs.microsoft.com/cli/azure/group?view=azure-cli-latest#az-group-create) command to create a resource group. The following command creates a resource group named *MyResourceGroup* in the *eastus* location.

    ```azurecli
    az group create --name MyResourceGroup --location eastus
    ```

1. Run the [az iot hub create](https://docs.microsoft.com/cli/azure/iot/hub?view=azure-cli-latest#az-iot-hub-create) command to create an IoT hub. It might take a few minutes to create an IoT hub.

    *YourIotHubName*. Replace this placeholder below with the name you chose for your IoT hub. An IoT hub name must be globally unique in Azure. This placeholder is used in the rest of this tutorial to represent your unique IoT hub name.

    ```azurecli
    az iot hub create --resource-group MyResourceGroup --name {YourIoTHubName}
    ```

1. After the IoT hub is created, view the JSON output in the console, and copy the `hostName` value from the following named field to a safe place. You use this value in a later step.

### Register a device

In this section, you create a new device instance and register it with the Iot hub you created. You will use the connection information for the newly registered device to securely connect your physical device in a later section.

To register a device:

1. In your console, run the [az iot hub device-identity create](https://docs.microsoft.com/cli/azure/ext/azure-cli-iot-ext/iot/hub/device-identity?view=azure-cli-latest#ext-azure-cli-iot-ext-az-iot-hub-device-identity-create) command. This creates the simulated device identity.

    *YourIotHubName*. Replace this placeholder below with the name you chose for your IoT hub.

    *MySTMDevice*. You can use this name directly for the device in the rest of this tutorial. Optionally, use a different name.

    ```azurecli
    az iot hub device-identity create --device-id MySTMDevice --hub-name {YourIoTHubName}
    ```

1. After the device is created, view the JSON output in the console, and copy the `deviceId` and `primaryKey` values to use in a later step.

Confirm that you have the copied the following values from the JSON output to use in the next section:

> * `hostName`
> * `deviceId`
> * `primaryKey`

## Prepare the device

To connect the STM DevKit to Azure, you'll modify a configuration file for WiFi and Azure IoT settings, rebuild the image, and flash the image to the device.

### Add configuration

1. In a text editor, edit the file `STM-B-L475E-IOT01\app\azure_config.c` to set the WiFi constants to the following values from your local environment.

    |Constant name|Value|
    |-------------|-----|
    |`wifi_ssid` |{*Your WiFi ssid*}|
    |`wifi_password` |{*Your WiFi password*}|
    |`wifi_mode` |{*One of the enumerated WiFi mode values in the file*}|

1. Edit the same file to set the Azure IoT device information constants to the values that you saved after you created Azure resources.

    |Constant name|Value|
    |-------------|-----|
    |`iot_hub_hostname` |{*Your Iot hub hostName value*}|
    |`iot_device_id` |{*Your deviceID value*}|
    |`iot_sas_key` |{*Your primaryKey value*}|

### Build the image

In **Developer Command Prompt**, go to the `getting-started\STM-B-L475E-IOT01` folder and run the script at `tools\rebuild.bat` to build the image.
```
tools\rebuild.bat
```

After the build is complete, the binary file is in the `_build\app` folder.

### Flash the image

1.  Launch STM32 ST-LINK Utility
2.  Select `Target` -> `Program...` and select the azure_iot_sample.bin produced from the [Building the Image](##Building-the-Image) step. It should be located in the `STM-B-L475E-IOT01\_build\app` folder
3.  Click on `Start` in `Download` diaglog while keeping the fields as is
4.  The flashing process should be quick. The console window in the STM32 ST-LINK Utility tool should display a message similar to ` Memory programmed in 18s and 375ms.` once the flashing process is complete

### Confirm device connection details

**TODO**: Run termite, and confirm that the device starts up and connects to hub correctly.
List the checkpoint output strings:
1. Wifi
2. DHCP - Ip address
3. SNTP - Simple Network Time Protocol
4. MQTT Client

Refer to troubleshooting to help resolve this.

## View telemetry

You can use [Azure IoT Explorer](https://github.com/Azure/azure-iot-explorer) to inspect the flow of telemetry from the device to Azure. 
1.  Launch Azure IoT Explorer
2.  Paste in the connection string for the IoT Hub you created earlier in this guide
3.  Click on `Connect`
4.  Select the device you wish to interact with
5.  Select `Telemetry` from the left menu
6.  Click `Start` to view the telemetry flowing through the hub

## View device properties

If you connected the device using IoT Hub:
1.  On the [Azure Portal](https://portal.azure.com/), navigate to the IoT Hub instance you created earlier in step 2
2.  Select *"IoT devices"* from the navigation menu for the hub instance
3.  Select the device you would like to examine the properties for
4.  On the device blade, click on the *"Device Twin"* button on the top menu
5. You can now view the device properties in the `properties` element in the JSON payload

## Invoke a direct method on the device
You can use the [Azure IoT Explorer](https://github.com/Azure/azure-iot-explorer) or the [Azure Portal](https://portal.azure.com/) to communicate with your board. Whether you are using the IoT explorer or the Azure portal, the experience of communicating with the board is similar. 

If you plan to use the Azure IoT Explorer:
1.  Launch the tool
2.  Paste in the connection string for the IoT Hub you created earlier in this guide
3.  Click on `Connect`
4.  Select the device you plan to communicate with

If you plan to use the Azure Portal:
1.  Navigate to [Azure Portal](https://portal.azure.com/)
2.  Navigate to the IoT Hub instance you created earlier in this guide
3.  Select *"IoT devices"* from the navigation menu for the hub instance
4.  Select the device you plan to communicate with

### Messages
Messages have both a body and optional properties organized as a collection of key/value string pairs. Enter a message body and a set of properties if you desire. 

### Direct Methods
Direct methods have a name, payload, and configurable connection and method timeouts. Fill in the method name to invoke on the device. If the method takes in a payload, fill in the payload text box. You can configure a connection or a response timeout in seconds before invoking the method.

## Clean up resources

If you no longer need the Azure resources created in this tutorial, you can use the Azure CLI to delete them.

If you continue to another tutorial in the series [Getting Started with Azure RTOS](https://review.docs.microsoft.com/azure/rtos/getting-started?branch=master), you can keep the resources you've already created and reuse them.

> [!IMPORTANT]
> Deleting a resource group is irreversible. The resource group and all the resources contained in it are permanently deleted. Make sure that you do not accidentally delete the wrong resource group or resources.

To delete a resource group by name:
1. Run the [az group delete](https://docs.microsoft.com/cli/azure/group?view=azure-cli-latest#az-group-delete) command. This removes the resource group, the IoT Hub, and the device registration you created.

    ```azurecli
    az group delete --name MyResourceGroup
    ```

1. Run the [az group list](https://docs.microsoft.com/cli/azure/group?view=azure-cli-latest#az-group-list) command to confirm the resource group is deleted.  

    ```azurecli
    az group list
    ```

## Troubleshooting

**TODO**

[We will temporarily include this information here, but move into separate article in the Docs RTOS repo for GA. ]

List of different troubleshooting to deal with edge cases arising from the steps within this guide. This would include device connection errors, Wi-Fi connection errors, Central connection errors, etc. 

Provide a pre-built binary that is known to work so they can test that they can use to test flashing and monitoring. 

## Next Steps 
**Ryan TODO**

Describe the shape of the application and how to customize for their own need 
1. List the cmake structure
1. List the app + lib folders
1. List common folder
