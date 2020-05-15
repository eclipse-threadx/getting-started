# Troubleshooting

## Console Output Errors

### DHCP
Confirm that you are running a DHCP server on your network (it is unusual to not have this running). The guides doesn't support manual network configuration.
### DNS
* If DNS fails to resolve, check the iot hub hostname is correct, try pinging the host from a command line to make sure it is responding
### SNTP
* Make sure you dont have a firewall in place blocking traffic.
### MQTT
* Check your device_id + primary_key values match against the IoT Hub instance. If these values are incorrect / missing then MQTT will fail to connect

## Serial Port
* garbled text on termite means baud rate is wrong, make sure it is set to 115200
* Check the flow control, some boards like the SAME54 required flow control to be setup to DTR/DSR
* restart Termite 

## Azure CLI

## Physical Setup

### USB
Try a different cable
Try a different port on your computer
Restart your computer to make sure drivers have installed correctly
Check in Device Manager and make sure all devices have are connected correctly
Reinstall drivers / flashing software and make sure all components are installed correctly

### Board specific
* Make sure all switches / DIP switches are setup correctly, as specified in the guide or in the associated devkit documentation
* Check the SV website / forums (provide links) for additional help on the boards.

## Connectivity

### Ethernet
Try a new ethernet cable
Confirm connections between your board and your router
Check the lights on the ethernet jack on the board, if there are no lights then it means the device isnt connected properly.

### WiFi
make sure you have 2.4GHz, 5GHz is not supported
Check your connection details
Make sure you are using none/wep/wpa2 personal with AES. TKIP + WPA are no supported

## Azure IoT

