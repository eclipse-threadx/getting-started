/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef _BOARD_INIT_H
#define _BOARD_INIT_H

#include "stm32l4xx_hal.h"

extern UART_HandleTypeDef UartHandle;

void board_init(void);

#endif // _BOARD_INIT_H
