#ifndef _NETWORKING_CONFIG_H
#define _NETWORKING_CONFIG_H

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

#endif // _NETWORKING_CONFIG_H