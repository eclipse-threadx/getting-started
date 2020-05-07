#ifndef _AZURE_CONFIG_H
#define _AZURE_CONFIG_H

typedef enum
{
    None         = 0, 
    WEP          = 1,
    WPA_Personal = 2,
} WiFi_Mode;

// ----------------------------------------------------------------------------
// WiFi connection information
// ----------------------------------------------------------------------------
extern const char* wifi_ssid;
extern const char* wifi_password;
extern const WiFi_Mode wifi_mode;

// ----------------------------------------------------------------------------
// Azure IoT Device information
// ----------------------------------------------------------------------------
extern const char* iot_hub_hostname;
extern const char* iot_device_id;
extern const char* iot_sas_key;

#endif // _AZURE_CONFIG_H