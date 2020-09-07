/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef _BOARD_INIT_H
#define _BOARD_INIT_H

#include "stm32f4xx_hal.h"

#define BUTTON_A_PIN GPIO_PIN_4
#define BUTTON_B_PIN GPIO_PIN_10

#define BUTTON_A_IS_PRESSED ((GPIOA->IDR & GPIO_PIN_4) == 0)
#define BUTTON_B_IS_PRESSED ((GPIOA->IDR & GPIO_PIN_10) == 0)

#define WIFI_LED_ON()  GPIOB->BSRR = GPIO_PIN_2
#define WIFI_LED_OFF() GPIOB->BSRR = (uint32_t)GPIO_PIN_2 << 16

#define AZURE_LED_ON()  GPIOA->BSRR = GPIO_PIN_15
#define AZURE_LED_OFF() GPIOA->BSRR = (uint32_t)GPIO_PIN_15 << 16

#define USER_LED_ON()  GPIOC->BSRR = GPIO_PIN_13
#define USER_LED_OFF() GPIOC->BSRR = (uint32_t)GPIO_PIN_13 << 16

#define RGB_LED_SET_R(value) TIM3->CCR1 = value
#define RGB_LED_SET_G(value) TIM2->CCR2 = value
#define RGB_LED_SET_B(value) TIM3->CCR2 = value

extern UART_HandleTypeDef UartHandle;

/* Define prototypes. */
void board_init(void);

#endif // _BOARD_INIT_H
