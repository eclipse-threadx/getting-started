/**
  ******************************************************************************
  * @file    stm32wb_at_ll.h
  * @author  MCD Application Team
  * @brief   Header file of AT client.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32WB_AT_LL_H
#define STM32WB_AT_LL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32wb_at.h"
#include "stm32wb_at_ll.h"

/* Private includes ----------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macros -----------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
uint8_t stm32wb_at_ll_Init(void);
uint8_t stm32wb_at_ll_DeInit(void);
uint8_t stm32wb_at_ll_Transmit(uint8_t *pBuff, uint16_t Size);
void stm32wb_at_ll_Async_receive(uint8_t new_frame);

#ifdef __cplusplus
}
#endif

#endif /* STM32WB_AT_LL_H */
