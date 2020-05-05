#include "azure_config.h"

// ----------------------------------------------------------------------------
// WiFi connection information
// The connection requires a 2.4 GHz band
// ----------------------------------------------------------------------------
const char* wifi_ssid			= "ferret";
const char* wifi_password       = "i am a furry animal";
const WiFi_Mode wifi_mode       = WPA_Personal; // None, WEP, WPA_Personal

// ----------------------------------------------------------------------------
// Azure IoT Device information
// ----------------------------------------------------------------------------
const char* iot_hub_hostname    = "azurertos-iot.azure-devices.net";
const char* iot_device_id       = "renesas_ae_cloud2";
const char* iot_sas_key         = "bbatYjAS4QrmTd+2srQHFtGFfiSAzIzuB5YPIsnJkQ0=";
