/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

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

/* Status of flash access functions */
typedef enum
{
	STATUS_OK = 0,
	SAVE_STATUS_ERROR = 1,
	SAVE_STATUS_ERASE_ERROR = 2,
	READ_STATUS_FLASH_ERROR = 3
} FLASH_Status_t;

/* Struct to store Azure IoT device information */
typedef struct {
	char hostname[MAX_HOSTNAME_LEN];
	char device_id[MAX_DEVICEID_LEN];
	char primary_key[MAX_KEY_LEN];
} DevConfig_IoT_Info_t;

/* Checks if memory contains Azure IoT credentials*/
bool has_credentials(void);

/* Checks if flash memory is ready to use */
bool verify_mem_status(void);

/* Save Azure IoT credentials to flash */
FLASH_Status_t save_to_flash(char *hostname, char *device_id, char* primary_key);

/* Erase Azure IoT credentials from flash */ 
FLASH_Status_t erase_flash(void);

/* Read Azure IoT credentials from flash */
FLASH_Status_t read_flash(DevConfig_IoT_Info_t* info);
