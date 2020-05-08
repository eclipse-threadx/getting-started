#include "azure_config.h"

// ----------------------------------------------------------------------------
// WiFi connection information
// ----------------------------------------------------------------------------
const char* wifi_ssid           = "";
const char* wifi_password       = "";
const WiFi_Mode wifi_mode       = WPA_Personal; // None, WEP, WPA_Personal

// ----------------------------------------------------------------------------
// Azure IoT Device information
// ----------------------------------------------------------------------------
// The IoT Hub hostname
const char* iot_hub_hostname    = "";

// The device ID
const char* iot_device_id       = "";

// The devices base64 SAS key
const char* iot_sas_key         = "";
