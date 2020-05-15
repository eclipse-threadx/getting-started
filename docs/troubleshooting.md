# Troubleshooting

## IoT connection Hub is failing
Console output = "Unable to resolve DNS for MQTT Server"

* Check your spelling/case for IOT config in azure_config.h (hostname, device_id, primary_key)

## Flashing the board fails
Unable to successfully complete 'Flash the image' section

* Try and a different USB cable
* Try and a different USB port on your computer
* Restart your computer.

## Serial Port garbled text
Console output = garbled text

* Check termite's 'baud rate' to 115200

## Serial port no text
Console output = nothing

* Make sure your termite setting match the devices tutorial
* Confirm the the chosen com port is correct, check device manager
* Restart termite

## Generic not working
Something just isn't working after reviewing above.

* Refer to the SV website for further diagnostic information.
* List links to their sites.
