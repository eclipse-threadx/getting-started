/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef _DEVICE_CONFIG_H
#define _DEVICE_CONFIG_H

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

// Max length of the data buffer read from flash
#define MAX_READ_BUFF 380

// Max length of the Azure IoTHub hostname
#define MAX_HOSTNAME_LEN 100

// Max length of the device ID
#define MAX_DEVICEID_LEN 130

// Max length of the device primary key
#define MAX_KEY_LEN 150

// Value of empty EEPROM byte
#define EMPTY_EEPROM_VAL 0xFF

// Using an STM32L4 board with wifi capabilities
#define STM32L4

#define WIFI_MAX_SSID_NAME_SIZE	32
#define WIFI_MAX_PSWD_NAME_SIZE 32

typedef enum 
{
	STATUS_OK = 0,
	SAVE_STATUS_ERROR = 1,
	SAVE_STATUS_ERASE_ERROR = 2,
	READ_STATUS_FLASH_ERROR = 3,
	ERASE_STATUS_ERROR = 4
} FLASH_Status_t;

typedef enum
{
	None         = 0, 
	WEP          = 1,
	WPA_PSK_TKIP = 2,
	WPA2_PSK_AES = 3
} WiFi_Mode;

typedef struct 
{
	char   SSID[WIFI_MAX_SSID_NAME_SIZE + 1];
	char   PSWD[WIFI_MAX_PSWD_NAME_SIZE + 1];
	WiFi_Mode Security;
} SoftAP_WiFi_Info_t;

typedef struct 
{
	char hostname[MAX_HOSTNAME_LEN];
	char device_id[MAX_DEVICEID_LEN];
	char primary_key[MAX_KEY_LEN];
#ifdef STM32L4
	char   ssid[WIFI_MAX_SSID_NAME_SIZE + 1];
	char   pswd[WIFI_MAX_PSWD_NAME_SIZE + 1];
	WiFi_Mode security;
#endif
} DevConfig_IoT_Info_t;



/* Checks if memory contains Azure IoT credentials*/
bool has_credentials(void);

/* Checks if memory is ready to use */
bool verify_mem_status(void);

/* Save Azure IoT credentials to flash */
//FLASH_Status_t save_to_flash(char *hostname, char *device_id, char* primary_key);
FLASH_Status_t save_to_flash(DevConfig_IoT_Info_t* info);

/* Erase Azure IoT credentials from flash */ 
FLASH_Status_t erase_flash(void);

/* Read Azure IoT credentials from flash */
// FLASH_Status_t read_flash(char* hostname, char* device_id, char* primary_key);
FLASH_Status_t read_flash(DevConfig_IoT_Info_t* info);



#endif // _DEVICE_CONFIG_H
