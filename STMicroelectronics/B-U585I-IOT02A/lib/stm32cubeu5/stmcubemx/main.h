/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "b_u585i_iot02a.h"
#include "stm32u5xx_hal.h"
#include "stm32u5xx_hal_def.h"

#include "mx_wifi_conf.h"

/* Define the SPI handler available for the WiFi device. */
#define MXCHIP_SPI hspi2
void mxchip_WIFI_ISR(uint16_t pin);

// Private defines
#define MXCHIP_FLOW_Pin         GPIO_PIN_15
#define MXCHIP_FLOW_GPIO_Port   GPIOG
#define MXCHIP_FLOW_EXTI_IRQn   EXTI15_IRQn
#define MXCHIP_NOTIFY_Pin       GPIO_PIN_14
#define MXCHIP_NOTIFY_GPIO_Port GPIOD
#define MXCHIP_NOTIFY_EXTI_IRQn EXTI14_IRQn
#define MXCHIP_NSS_Pin          GPIO_PIN_12
#define MXCHIP_NSS_GPIO_Port    GPIOB
#define MXCHIP_RESET_Pin        GPIO_PIN_15
#define MXCHIP_RESET_GPIO_Port  GPIOF

/* External declarations for Mxchip with SPI.*/
void HAL_SPI_TransferCallback(SPI_HandleTypeDef* hspi);
extern void mxchip_WIFI_ISR(uint16_t pin);
extern void nx_driver_emw3080_interrupt();

void Error_Handler(void);
void Success_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
