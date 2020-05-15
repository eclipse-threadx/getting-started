#ifndef _AZURE_CONFIG_H
#define _AZURE_CONFIG_H

/*************** Azure IoT Hub Connection Configuration ******************/
#define IOT_HUB_HOSTNAME    "azurertos-iot.azure-devices.net"
#define IOT_DEVICE_ID       "SAME54"
#define IOT_PRIMARY_KEY     "QhTpPmqJzkKjyDYTanabbnEl4UYwyP0CCqdu8JcUaow="

// 0 - BME280 sensor is not present
// 1 - BME280 sensor is present
#define __SENSOR_BME280__ 0

#endif // _AZURE_CONFIG_H