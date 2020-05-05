#include "azure_config.h"

// ----------------------------------------------------------------------------
// WiFi connection information
// ----------------------------------------------------------------------------
const char* wifi_ssid			= "";
const char* wifi_password       = "";
const WiFi_Mode wifi_mode       = WPA_Personal; // None, WEP, WPA_Personal

// ----------------------------------------------------------------------------
// Azure IoT Device information
// ----------------------------------------------------------------------------
const char* iot_hub_hostname    = "";
const char* iot_device_id       = "";
const char* iot_sas_key         = "";
