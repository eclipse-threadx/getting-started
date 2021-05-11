/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef _DEVICE_CONFIG_H
#define _DEVICE_CONFIG_H

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

// Max length of the data buffer read from flash
#define MAX_READ_BUFF 1024
#define MAX_WIFI_SSID_SIZE	32
#define MAX_WIFI_PSWD_SIZE 64
#define MAX_DPS_ID_SCOPE_SIZE 12
#define MAX_DPS_REGISTRATION_ID_SIZE 256
#define MAX_HUB_HOSTNAME_SIZE 68
#define MAX_HUB_DEVICE_ID_SIZE 256
#define MAX_DEVICE_SAS_KEY_SIZE 45

// Value of empty EEPROM byte
#define EMPTY_EEPROM_VAL 0xFF

typedef enum 
{
	STATUS_OK = 0,
	SAVE_STATUS_ERROR = 1,
	SAVE_STATUS_ERASE_ERROR = 2,
	READ_STATUS_FLASH_ERROR = 3,
	ERASE_STATUS_ERROR = 4
} Flash_Status_t;

typedef enum
{
	None         = 0, 
	WEP          = 1,
	WPA_PSK_TKIP = 2,
	WPA2_PSK_AES = 3
} WiFi_Mode;

typedef struct 
{
	char hostname[MAX_HUB_HOSTNAME_SIZE];
	char deviceid[MAX_HUB_DEVICE_ID_SIZE];
	char sas[MAX_DEVICE_SAS_KEY_SIZE];
	char idscope[MAX_DPS_ID_SCOPE_SIZE];
	char registrationid[MAX_DPS_REGISTRATION_ID_SIZE];
	char ssid[MAX_WIFI_SSID_SIZE];
	char pswd[MAX_WIFI_PSWD_SIZE];
	WiFi_Mode security;
} Device_Config_Info_t;



/* Checks if memory contains Azure IoT credentials*/
bool has_credentials(void);

/* Checks if memory is ready to use */
bool verify_mem_status(void);

/* Save Azure IoT credentials to flash */
Flash_Status_t save_to_flash(Device_Config_Info_t* info);

/* Erase Azure IoT credentials from flash */ 
Flash_Status_t erase_flash(void);

/* Read Azure IoT credentials from flash */
Flash_Status_t read_flash(Device_Config_Info_t* info);

#endif // _DEVICE_CONFIG_H
