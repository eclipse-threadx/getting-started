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
#define IOT_HUB_HOSTNAME    ""
#define IOT_DEVICE_ID       ""
#define IOT_PRIMARY_KEY     ""

#endif // _AZURE_CONFIG_H