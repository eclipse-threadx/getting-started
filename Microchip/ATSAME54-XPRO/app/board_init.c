/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */
   
#include "board_init.h"

#include "weather_click.h"

#define CONF_AT24MAC_ADDRESS 0x5e

extern UCHAR _nx_driver_hardware_address[];

static void read_macaddress()
{
    const uint8_t addr = 0x9A;

    i2c_m_sync_enable(&I2C_AT24MAC);
    i2c_m_sync_set_slaveaddr(&I2C_AT24MAC, CONF_AT24MAC_ADDRESS, I2C_M_SEVEN);
    io_write(&(I2C_AT24MAC.io), &addr, 1);
    io_read(&(I2C_AT24MAC.io), _nx_driver_hardware_address, 6);
}

void board_init()
{
    // Initializes MCU, drivers and middleware
    atmel_start_init();

    // Load the MAC address from EEPROM into NetXDuo
    read_macaddress();

    // Initialize Weather Click
    if (init_weather_click() != BME280_OK)
    {
        printf("FAILED to initialize weather click board\r\n");
    }
}
