/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "device_config.h"
#include "stm32l4xx_hal.h"

// Page size calculation, justification from datasheet needed here ()
#define page_size 0x800

// format string for parsing data
const char *format = "hostname=%s deviceid=%s sas=%s idscope=%s registrationid=%s ssid=%s pw=%s sec=%d";

// Specific helper function for writing to flash for STM32L4
Flash_Status_t save_to_flash_ST(uint8_t *data);

// Specific helper function for reading from flash for STM32L4
Flash_Status_t read_flash_ST(uint8_t* data);

// Specific helper function for erasing flash for STM32L4
HAL_StatusTypeDef erase_flash_ST();

// Symbol for section of flash to store credentials
extern uint32_t __DEVICEINFO;

// Helper functions

// Specific helper function for saving to flash for STM32L4
Flash_Status_t save_to_flash_ST(uint8_t *data)
{	
    volatile uint64_t data_to_flash[(strlen((char*)data) / 8) + (int)((strlen((char*)data) % 8) != 0)];
    memset((uint8_t*)data_to_flash, 0, strlen((char*)data_to_flash));
    strcpy((char*)data_to_flash, (char*)data);

    volatile uint32_t data_length = (strlen((char*)data_to_flash) / 8) + (int)((strlen((char*)data_to_flash) % 8) != 0);
    volatile uint32_t write_cnt = 0;
    volatile uint32_t index = 0;
    volatile HAL_StatusTypeDef status;

    status = erase_flash_ST();

    // Unlock flash
    HAL_FLASH_Unlock();
    HAL_FLASH_OB_Unlock();
    
    // Program flash
    while (index < data_length)
    {
        if (status == HAL_OK)
        {
            status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, ((uint32_t)(&__DEVICEINFO) + write_cnt), data_to_flash[index]); // FAST
            if (status ==  HAL_OK)
            {
                status = HAL_FLASH_GetError();
                if (status == HAL_OK)
                {
                    write_cnt += 8;
                    index++;
                }
                else
                {
                    printf("HAL Error in saving to FLASH.\n");
                    // lock flash
                    HAL_FLASH_OB_Lock();
                    HAL_FLASH_Lock();
                    return SAVE_STATUS_ERROR;
                }
            }
        }
    }
    
    // Lock flash
    HAL_FLASH_OB_Lock();
    HAL_FLASH_Lock();		
        
    return STATUS_OK;
}

// Specific helper function for reading from flash for STM32L4
Flash_Status_t read_flash_ST(uint8_t* data)
{
    volatile uint32_t read_data;
    volatile uint32_t read_cnt = 0;
    
    do
    {
        read_data = *(uint32_t*)(((uint32_t)&__DEVICEINFO) + read_cnt);
        if (read_data != 0xFFFFFFFF)
        {
            data[read_cnt] = (uint8_t)read_data;
            data[read_cnt + 1] = (uint8_t)(read_data >> 0x8);
            data[read_cnt + 2] = (uint8_t)(read_data >> 0x10);
            data[read_cnt + 3] = (uint8_t)(read_data >> 0x18);
            read_cnt += 4;
        }
        
    } while (read_data != 0xFFFFFFFF); // end of flash content

    return STATUS_OK;
}

// Specific helper function for erasing flash for STM32L4
HAL_StatusTypeDef erase_flash_ST()
{
	// unlock flash
	HAL_FLASH_Unlock();
	HAL_FLASH_OB_Unlock();
	
	FLASH_EraseInitTypeDef EraseInitStruct;
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.Banks = FLASH_BANK_2;
	EraseInitStruct.Page = 255; // bank 2 page 0 is 0x80800000, using last page
	EraseInitStruct.NbPages = 0x1;
        
	uint32_t PageError;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) == HAL_ERROR) {
        if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) == HAL_ERROR) {
            // HAL_FLASHEx_Erase() requires two calls to work, if fails a second time then exit
            printf("Erase PageError: %lu\n", PageError);
            // lock flash
            HAL_FLASH_OB_Lock();
            HAL_FLASH_Lock();
            return PageError;
        }
    }
	
	// Lock flash
	HAL_FLASH_OB_Lock();
	HAL_FLASH_Lock();		
	
	return HAL_OK;
}

// Device Configuration Interface functions

Flash_Status_t save_to_flash(Device_Config_Info_t* info)
{
    Flash_Status_t status = SAVE_STATUS_ERROR;
    
    char writeData[MAX_READ_BUFF] = { 0 };
    
    // Create credential string using format string
    if (sprintf(writeData, format, info->hostname, info->deviceid, info->sas, info->idscope, info->registrationid, info->ssid, info->pswd, info->security) < 0) 
    {
        printf("Error parsing credentials to store. \n");
        return SAVE_STATUS_ERROR;
    }

    // Call device specific implementation of FLASH storage
    status = save_to_flash_ST((uint8_t *)(writeData));
    
    return status;
}


Flash_Status_t read_flash(Device_Config_Info_t* info)
{
    Flash_Status_t status = READ_STATUS_FLASH_ERROR;

    char readData[MAX_READ_BUFF] = { 0 };

    // Call MCU specific flash reading function
    status = read_flash_ST((uint8_t*)(readData));
    
    // Write to readData buffer
    if(sscanf(readData, format, info->hostname, info->deviceid, info->sas, info->idscope, info->registrationid, info->ssid, info->pswd, info->security) < 0)
    {
        status = READ_STATUS_FLASH_ERROR;
    }

    return status;
}

bool verify_mem_status(void)
{
    // how to verify the memory is valid
    return true; // TODO
}

bool has_credentials(void)
{
    bool ret_val = true;
    volatile uint32_t read_data;
    
    read_data = *((uint32_t*)(&__DEVICEINFO));
    
    if (read_data == 0xFFFFFFFF)
    {
        ret_val = false;
    }
    
    return ret_val;
}

Flash_Status_t erase_flash()
{
    if(erase_flash_ST() != HAL_OK)
    {
        printf("HAL Error in erasing flash. \n");
        return ERASE_STATUS_ERROR;
    }

    return STATUS_OK;
}


