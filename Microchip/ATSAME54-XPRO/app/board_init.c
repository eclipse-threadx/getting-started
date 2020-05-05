#include "board_init.h"

void board_init()
{
    /* Initializes MCU, drivers and middleware */
    atmel_start_init();
    printf("Board initialized.\r\n");

    WeatherClick_initialize();
    printf("Temperature sensor initialized.\r\n");
}
