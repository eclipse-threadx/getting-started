/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "device_config.h"
#include "stm32l4xx_hal.h"

// Address of FLASH storage, storing at bank 2 page 0 is 0x80800000
#define FLASH_STORAGE 0x08080000

// Page size calculation, justification from datasheet needed here ()
#define page_size 0x800

// Specific helper function for writing to flash for STM32L4
FLASH_Status_t save_to_flash_ST(uint8_t *data);

// Specific helper function for reading from flash for STM32L4
FLASH_Status_t read_flash_ST(uint8_t* data);

// Specific helper function for erasing flash for STM32L4
HAL_StatusTypeDef erase_flash_ST();


// Helper functions

FLASH_Status_t save_to_flash_ST(uint8_t *data)
{	
    volatile uint64_t data_to_FLASH[(strlen((char*)data) / 8) + (int)((strlen((char*)data) % 8) != 0)];
    memset((uint8_t*)data_to_FLASH, 0, strlen((char*)data_to_FLASH));
    strcpy((char*)data_to_FLASH, (char*)data);

    volatile uint32_t data_length = (strlen((char*)data_to_FLASH) / 8) + (int)((strlen((char*)data_to_FLASH) % 8) != 0);
    volatile uint16_t pages = (strlen((char*)data) / page_size) + (int)((strlen((char*)data) % page_size) != 0);
        
    // Unlock flash
    HAL_FLASH_Unlock();
    HAL_FLASH_OB_Unlock();
    
    // Initialize erase struct for number of pages needed 
    FLASH_EraseInitTypeDef EraseInitStruct;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.Banks = FLASH_BANK_2;
    EraseInitStruct.Page = 0; // bank 2 page 0 is 0x80800000
    EraseInitStruct.NbPages = pages;
        
    volatile uint32_t write_cnt = 0;
    volatile uint32_t index = 0;
    volatile HAL_StatusTypeDef status;
    uint32_t PageError;
    
    // Erase flash
    if(HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) == HAL_ERROR)
    {
        printf("Erase PageError: %lu\n", PageError);
        return SAVE_STATUS_ERASE_ERROR;
    }
    
    // Program flash
    while (index < data_length)
    {
        if (status == HAL_OK)
        {
            status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, FLASH_STORAGE + write_cnt, data_to_FLASH[index]); // FAST
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


FLASH_Status_t read_flash_ST(uint8_t* data)
{
    volatile uint32_t read_data;
    volatile uint32_t read_cnt = 0;
    
    do
    {
        read_data = *(uint32_t*)(FLASH_STORAGE + read_cnt);
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

FLASH_Status_t save_to_flash(DevConfig_IoT_Info_t* info)
{
    FLASH_Status_t status = SAVE_STATUS_ERROR;

    const char *format = "hostname=%s device_id=%s primary_key=%s";
    
    char writeData[MAX_READ_BUFF] = { 0 };
    
    // Create credential string using format string
    if (sprintf(writeData, format, info->hostname, info->device_id, info->primary_key) < 0) 
    {
        printf("Error parsing credentials to store. \n");
        return SAVE_STATUS_ERROR;
    }
    
    // Call device specific implementation of FLASH storage
    status = save_to_flash_ST((uint8_t *)(writeData));
    
    return status;
}



FLASH_Status_t read_flash(DevConfig_IoT_Info_t* info)
{
    FLASH_Status_t status = READ_STATUS_FLASH_ERROR;

    char readData[MAX_READ_BUFF] = { 0 };

    const char *format = "hostname=%s device_id=%s primary_key=%s"; 

    // Call MCU specific flash reading function
    status = read_flash_ST((uint8_t*)(readData));
    
    if(sscanf(readData, format, info->hostname, info->device_id, info->primary_key) < 0)
    {
        status = READ_STATUS_FLASH_ERROR;
    }
    
    // info->wifi_info = softAP_wifi;
    
    return status;
}


// Device Configuration Interfact functions

bool verify_mem_status(void)
{
    // how to verify the memory is valid
    return true; // TODO
}

bool has_credentials(void)
{
    bool ret_val = true;
    volatile uint32_t read_data;
    
    read_data = *(uint32_t*)(FLASH_STORAGE);
    
    if (read_data == 0xFFFFFFFF)
    {
        ret_val = false;
    }
    
    return ret_val;
}

FLASH_Status_t erase_flash()
{
    if(erase_flash_ST() != HAL_OK)
    {
        printf("HAL Error in erasing flash. \n");
        return ERASE_STATUS_ERROR;
    }

    return STATUS_OK;
}


HAL_StatusTypeDef erase_flash_ST()
{
	// unlock flash
	HAL_FLASH_Unlock();
	HAL_FLASH_OB_Unlock();
	
	FLASH_EraseInitTypeDef EraseInitStruct;
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.Banks = FLASH_BANK_2;
	EraseInitStruct.Page = 0; // bank 2 page 0 is 0x80800000
	EraseInitStruct.NbPages = 0x1;
        
	uint32_t PageError;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) == HAL_ERROR) {
        printf("Erase PageError: %lu\n", PageError);
        return PageError;
    }
	
	// Lock flash
	HAL_FLASH_OB_Lock();
	HAL_FLASH_Lock();		
	
	return HAL_OK;
}