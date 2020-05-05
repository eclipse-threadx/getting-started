[[_TOC_]]

## Introduction
This article describes how to connect a STM32L475 development kit to an Azure IoT Central application. 

An IAR Embedded Workbench project, containing Azure IoT Plug and Play and Azure RTOS, will be used to build and deploy the image.

Visit the [Troubleshooting](#Troubleshooting) section below for help resolving issues with this guide.

## Before you begin
This following is a list of resources you will need for this project.

### Hardware
- STMicroelectronics [STM32L475](https://www.st.com/en/microcontrollers-microprocessors/stm32l4x5.html) development kit
- USB to Micro USB cable
- 2.4GHz WiFi with internet access

### Software
- [STM32 ST-Link Utility](https://www.st.com/en/development-tools/stsw-link004.html)
- [IAR Embedded Workbench for Arm](https://www.iar.com/iar-embedded-workbench/#!?currentTab=free-trials), version 8.40 or later
- [Tera Term](https://ttssh2.osdn.jp/index.html.en) - Serial Terminal
- [Git](https://git-scm.com/downloads)

## Getting the Source Code
1. From the command line, clone the repository to your top level folder of your hard disk:

       git clone -b Embedded_World/2020 https://ExpressLogic@dev.azure.com/ExpressLogic/X-Ware/_git/project_test
    **WARNING**: Place the folder in the root of your hard drive to avoid long file path compilation errors
2. Throughout this document **<STM32L475>** will be used as refer to this directory location **\project_test\STM32L475_DISCO_AzureRTOS**

## Setup Azure IoT Central
1. If you don't have an Azure IoT Central application, then follow the instructions on [How to create an Azure IoT Central application](https://docs.microsoft.com/en-us/azure/iot-central/quick-deploy-iot-central-pnp) before continuing.


1. From within the Central application, select *Device templates* from the sidebar

    ![central](images/IoT_central_004.png =400x)
1. Click on *"+ New"* to create a new template

    ![central](images/IoT_central_005.png =400x)
1. Click on the *"IoT device"* tile, and then click *"Next: Customize"*

    ![central](images/IoT_central_006.png =600x)
1. Confirm the *"Gateway device"* checkbox is **unchecked**, then click *"Next: Review"*.
1. Click *"Create"*
1. Enter a name for your device template (for instance "STM32L475") and press Enter
1. Click on the *Import capability model* tile

    The **IoTNode.iotcentral.json** file with the Device capability model is stored under \<STM32L475\>\Source\Projects\B-L475E-IOT01\Applications\Azure1\Src\AzureIoTPnp\dcm\ folder
   
    ![central](images/IoT_central_007.png =600x)
1. To create a dashboard for devices, click on *"Views"* and then click *"Generate default views*" (If you wish to create your own view instead, click on *"Visualizing the device*")

    ![central](images/IoT_central_004.png =400x)
1. Click *"Generate default dashboard view(s)"* to generate a default view for this device type

      **TODO**: Add screenshot here
1. Click *"Publish"*, to publish the device template to the application. When prompted, please click *"Publish"* again

    ![central](images/IoT_central_008.png =1000x)
13. From the side navigation, click *"Devices"*, then click *"STM32L475"* (or use the device template name chosen in step 7) and then click *"New"* to create a device
    
    ![central](images/IoT_central_009.png =600x)
14. You may personalize the Device ID and Device name if desired, then click *"Create"*

    ![central](images/IoT_central_010.png =400x)
15. Click on the device name that you just created and click *"Connect"*

    ![central](images/IoT_central_012.png =1000x)
16. Take a note of the following 3 pieces of information; this will be used later to configure your device's connection to your IoT Central application

      | Information     | Description |
      |-----------------|-------------|
      | *"ID Scope"*    | The Azure IoT Hub identifier |
      | *"Device ID"*   | The device's unique identifier |
      | *"Primary Key"* | The device's connection key |

    ![central](images/IoT_central_013.png =400x)

## Building and Flashing the Image
1. Connect the STM32L475 device to the development machine via the ST-LINK USB port

    ![STM32L4](images/STM32L4_DebugPort.jpg =600x)
2. Within IAR Embedded Workbench, select *"File > Open Workspace"* from the menubar
3. Navigate to \<STM32L475\>\Source\Projects\B-L475E-IOT01\Applications\Azure1\EWARM\B-L475E-IOT01\ and open the **"Project.eww"**

    **NOTE**: If you get a dialog box stating that the project file is in an old format, click *"Yes"* to convert each project
4. Right click *"Project - B-L475E-IOT01"* from the Workspace window and click *"Set as Active"*

      **TODO**: This step should be updated in the project itself, and them removed from this walkthrough
5. Select *"Project > Batch build..."* from the menubar, and then click the *"Make"* button to build the entire project

6. From the menubar, click on *"Project > Download > Download active application"*

    **TODO**: Image here
7. Open Tera Term and select *"Serial"* and the port with the STMicroelectronics description.

    ![Terminal](images/TeraTerm_001.png =600x)
8. From the menubar, click on *"Setup"* then *"Serial port..."* to change the serial connection settings

    ![Terminal](images/TeraTerm_002.png =600x)
9. Change the Speed (or Baudrate) setting to **115200** and change Transmit delay to **1 msec/char**, then click *"New setting"*

    ![Terminal](images/TeraTerm_003.png =400x)
10. Press the **Black Reset** button on the board to reset the device
    - You should now see information populating in the Tera Term terminal window
11. Follow the screen prompts to setup a WiFi connection
    **NOTE**: Press the **Blue User** button on the device within 10 seconds to change the WiFi settings

    ![Terminal](images/TeraTerm_004.png =600x)
12. Follow the screen prompts to enter the Scope ID, Device ID and Primary Key collected from step 16 in the [Setup Azure IoT Central Application](#setup-azure-iot-central-application)
    **NOTE** Press the **Blue User** button on the device within 10 seconds to change the Azure IoT connection settings
13. The logs should now output in the terminal
    - **TODO**: Need to add an image of a "Success" output (having troubles with my remote connection so will upload it as soon as I can)

## Visualizing in Azure IoT Central
1. Go to [Azure IoT Central](https://apps.azureiotcentral.com)
2. Go to *"My apps"* and select the IoT Central App created in [Setup Azure IoT Central Application](#setup-azure-iot-central-application) above

    ![Central](images/ViewDeviceData_001.png =400x)
3. Click *"Devices*", click *"STM32L475"* and then click the previously created device

    ![Central](images/ViewDeviceData_002.png =800x)
4. Click the *"Overview"* tab to view the incoming telemetry in a set of time series charts
      - **TODO**: Snapshot of overview tab

## Final Steps
After the device is running correctly and upload telemetry to Azure IoT Central, it may be disconnected from the computer and connected to an external power source via the same ST-LINK USB port.

## Troubleshooting
1. Device connection to PC errors
      * Confirm ST-LINK is installed
      * Check in Device Manager to make sure the ST-LINK port is present
2. Wifi connection errors
      * Check the encryption type matches your wifi setup
      * Make sure the Wifi is 2.4GHz
3. Central connection errors