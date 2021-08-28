/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */
   
#include "board_init.h"

#include "weather_click.h"

void board_init()
{
    // Initializes MCU, drivers and middleware
    atmel_start_init();

    // Initialize Weather Click
    if (init_weather_click() != BME280_OK)
    {
        printf("FAILED to initialize weather click board\r\n");
    }
}
