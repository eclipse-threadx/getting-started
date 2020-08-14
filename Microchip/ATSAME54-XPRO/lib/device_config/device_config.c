/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "atmel_start.h"
#include "device_config.h"

/* A specific byte pattern stored at the begining of SmartEEPROM data area.
 * When the application comes from a reset, if it finds this signature,
 * the assumption is that the SmartEEPROM has some valid data.
 */
#define SMEE_CUSTOM_SIG			0x5a5a5a5a

/* This address must be within the total number of EEPROM data bytes 
 * ( Defined by SBLK and PSZ fuse values).
 */
#define SEEP_TEST_ADDR			32

/* This example assumes SBLK = 1 and PSZ = 3, thus total size is 4096 bytes */
#define SEEP_FINAL_BYTE_INDEX	4095

/* Address of FLASH, spaced away from the verify signature */
#define FLASH_ADDRESS			(SEEPROM_ADDR + 0x8)


/* Handler for a HardFault */
void HardFault_Handler(void);

/* Device specific flash access function prorotypes */
FLASH_Status_t save_to_flash_SAME54(char *writeData);
FLASH_Status_t read_flash_SAME54(char *readData);

// Helper functions
/**
 ** \brief Hardfault Handler
 * Used to identify when a location outside the reserved area
 * for SmartEEPROM is accessed.
 */
void HardFault_Handler(void)
{
	printf("\r\n!!!!!!!! In HardFault_Handler !!!!!!!!\r\n");

	while (1) {
		;
	}
}

/**
 * \brief Verify the custom data in FLASH
 *
 * Verify the custom data at initial 4 bytes of FLASH
 */
int8_t verify_seep_signature(void)
{
	uint32_t *ADDR = (uint32_t *)SEEPROM_ADDR;
	
	int8_t ret_val = ERR_INVALID_DATA;

	while (hri_nvmctrl_get_SEESTAT_BUSY_bit(NVMCTRL)) ;

	/* If SBLK fuse is not configured, inform the user and wait here */
	if (!(hri_nvmctrl_read_SEESTAT_SBLK_bf(NVMCTRL))) {
		printf("\r\nPlease configure SBLK fuse to allocate SmartEEPROM area\r\n");
		while (1) ;
	}

	if (SMEE_CUSTOM_SIG == ADDR[0]) {
		ret_val = ERR_NONE;
	}

	return ret_val;
}

/* SAME54 function to store into FLASH */
FLASH_Status_t save_to_flash_SAME54(char *writeData)
{
	// Pointer used to access flash
	uint8_t *FLASH_BUF = (uint8_t *)FLASH_ADDRESS;
	
	// Wait until Smart EEPROM is busy
	while(hri_nvmctrl_get_SEESTAT_BUSY_bit(NVMCTRL));
	
	// Store byte by byte into FLASH
	for(int i = 0 ; i < MAX_READ_BUFF; i++)
	{
		FLASH_BUF[i] = writeData[i];
	}
	// TODO find what the failure case here is
	
	return STATUS_OK;
}

/* SAME54 function to read into readData buffer */
FLASH_Status_t read_flash_SAME54(char *readData)
{
	FLASH_Status_t status = STATUS_OK;
	
	// Pointer to access the smart EEPROM
	uint8_t *FLASH_BUF = (uint8_t *)FLASH_ADDRESS;
	
	// Wait until SmartEEPROM is busy
	while(hri_nvmctrl_get_SEESTAT_BUSY_bit(NVMCTRL));
	
	// Read  one char at a time
	for(int i = 0 ; i < MAX_READ_BUFF; i++)
	{
		readData[i] = FLASH_BUF[i];
	}
	
	return status;
}


// Device Configuration interface functions

/**
 * \brief Verify the SmartEEPROM is setup properly 
 * 
 *	Verify the SmartEEPROM is setup properly 
 */
bool verify_mem_status(void)
{
	uint32_t *ADDR = (uint32_t *)SEEPROM_ADDR;
	
	/* Initializes MCU, drivers and middleware */
	if (ERR_NONE == verify_seep_signature()) {
		printf("\r\nSmartEEPROM contains valid data \r\n");
        return false;
    }
	else {
		printf("\r\nStoring signature to SmartEEPROM address 0x00 to 0x03\r\n");
		while (hri_nvmctrl_get_SEESTAT_BUSY_bit(NVMCTRL)) ;
		ADDR[0] = SMEE_CUSTOM_SIG;
	}
    
    return true;
}

/*
 * Verifies if the FLASH has been Azure IoT credentials*/
bool has_credentials(void)
{
	uint8_t *FLASH_BUF = (uint8_t *)FLASH_ADDRESS;
	
	int8_t ret_val = false;

	if (FLASH_BUF[0] != EMPTY_EEPROM_VAL) 
	{
		// Proper value stored in flash
		ret_val = true;
	}

	return ret_val;
}


/*
 * Format and store Azure IoT credentials in flash 
 **/
FLASH_Status_t save_to_flash(char *hostname, char *device_id, char* primary_key)
{
	FLASH_Status_t status = SAVE_STATUS_ERROR;
	
	const char *format = "hostname=%s device_id=%s primary_key=%s";
	
	char writeData[MAX_READ_BUFF] = { 0 };
	
	// Create credential string using format string
	if (sprintf(writeData, format, hostname, device_id, primary_key) < 0)
	{
		printf("Error parsing credentials to store. \n");
		return SAVE_STATUS_ERROR;
	}
	
	// Call device specific implementation of FLASH storage
	status = save_to_flash_SAME54(writeData);
	
	return status;
}


/*
 * Erase flash by setting buffer to 0xFF to clear
 **/
FLASH_Status_t erase_flash(void)
{
	uint8_t *FLASH_BUF = (uint8_t *)FLASH_ADDRESS;

	for (int i = 0; i < MAX_READ_BUFF; i++)
	{
		FLASH_BUF[i] = EMPTY_EEPROM_VAL;
	}
	
	printf("Flash erased\n");

	return STATUS_OK;
}



/*
 * Read credentials from flash memory and write info to DevConfig struct
 **/
FLASH_Status_t read_flash(DevConfig_IoT_Info_t* info)
{
	FLASH_Status_t status = READ_STATUS_FLASH_ERROR;
	
	char readData[MAX_READ_BUFF] = { 0 };

	const char *format = "hostname=%s device_id=%s primary_key=%s"; 
	
	// Call MCU specific flash reading function
	status = read_flash_SAME54(readData);
	
	// Parse credentials from string
	if(sscanf(readData, format, info->hostname, info->device_id, info->primary_key) < 0)
	{
		status = READ_STATUS_FLASH_ERROR;
	}

	return status;
}
