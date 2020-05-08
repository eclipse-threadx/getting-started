#ifndef _BOARD_INIT_H
#define _BOARD_INIT_H

#include <stdbool.h>

#include "stm32l4xx_hal.h"

extern UART_HandleTypeDef UartHandle;

bool board_init(void);

#endif // _BOARD_INIT_H
