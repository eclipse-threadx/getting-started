#include "azure_config.h"

// ----------------------------------------------------------------------------
// WiFi connection information
// ----------------------------------------------------------------------------
const char* wifi_ssid			= "ferret";
const char* wifi_password       = "i am a furry animal";
const WiFi_Mode wifi_mode       = WPA_Personal; // None, WEP, WPA_Personal

// ----------------------------------------------------------------------------
// Azure IoT Device information
// ----------------------------------------------------------------------------
// The IoT Hub hostname
const char* iot_hub_hostname    = "azurertos-iot.azure-devices.net";

// The device ID
const char* iot_device_id       = "renesas_ae_cloud2";

// The devices base64 SAS key
const char* iot_sas_key         = "bbatYjAS4QrmTd+2srQHFtGFfiSAzIzuB5YPIsnJkQ0=";
