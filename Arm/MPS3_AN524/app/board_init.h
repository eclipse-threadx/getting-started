/* Copyright (c) 2021 Linaro Limited. */

#ifndef _BOARD_INIT_H
#define _BOARD_INIT_H

#include "Driver_USART.h"
#include "device_cfg.h"

/* USART0 driver */
extern ARM_DRIVER_USART Driver_USART0;

void board_init();

#endif // _BOARD_INIT_H
