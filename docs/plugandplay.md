# Plug and Play
* This will plug into the existing guides as an alternative path
* The user needs to deploy their hub into US-Central as this contains the PNP code.

## Introduction
* Provide context: Brief intro on why we are implementing PNP
* Pointer to PNP docs for more details
   * https://docs.microsoft.com/en-us/azure/iot-pnp/
* This is a companion document to the GSG

## Prerequisites
Link to top level index page that links to the 4 GSG's as they need to complete many steps there first in case they land here.

## Setup
I chatted with Stefan and he has added the model to his private repo, and he said he would look into making this a public model after his review with Cameron, however they are still deciding how to stop the model repository being polluted with sample models.

1. Make sure the device is flashed and booted up and connected to Azure IoT Hub
1. Choose your hub and device
1. Select "IoT Plug and Play components"
1. If explorer is unable to resolve the model, if will provide a "Configure" button so you can choose a local folder containing the model
1. Click "Configure", then "Add | Local folder | Pick a folder".
1. Browse using the horrible folder picker to the where the GSG's where cloned and choose the directory "gsg/core/model" and click "Select"
1. Click "Save"

## Accessing the PNP components
This is accessed by going to "IoT Plug and Play components" for the device, and choosing "Default component"
There are 5 tabs in here corresponding to the next sections
* Interface
* Properties (read-only)
* Properties (writeable)
* Commands
* Telemetry

## Plug and Play properties (Interface tab)
Break down the "model content" to the user so they can understand how the UI is being generated.
This is a view of the 4 sections of the model.

| Type | Name | Description |
|---|---|---|
| Telemetry | temperature | Temperature in celsius |
| Property (writeable) | telemetryInterval | The interval telemetry is sent |
| Property (read-only) | ledState | The current state of the LED |
| Command | setLedState | Change the state of the LED |

## View Telemetry
1. Click the "Telemetry" tab from within the plug and play section of Azure IoT Explorer
1. Press the "Start" button
1. Select "Show modeled events" to display the parsed Plug and Play telemetry
1. Confirm that temperature telemetry is correctly being parsed

## Call a direct method on the device
1. Click on the "Commands" tab
1. Set the "state" as "true" and press "Send command"
1. Observe the LED turns off (or look at the serial console for device without an LED)
1. Click on "Properties (read-only)" tab
1. Select the "Refresh" button"
1. Confirm the value for "ledState" is "true"
1. Click on the "Commands" tab
1. Set the "state" as "false" and press "Send command"
1. Click on "Properties (read-only)" tab
1. Select the "Refresh" button"
1. Confirm the value for "ledState" is "false" (There is a bug right now, and false shows up as blank, but I believe this will be fixed soon)

## [New] Modify Writeable Property
1. Click on the "Properties (writeable)" tab
1. Type "5" into the telemetryInterval box
1. Click Update desired property
1. Observe in the serial console that the telemetryInterval is updated, the telemetry sending will now be every 5 seconds
