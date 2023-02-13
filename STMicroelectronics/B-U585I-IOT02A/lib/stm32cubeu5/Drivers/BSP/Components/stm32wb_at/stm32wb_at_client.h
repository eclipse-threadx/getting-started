/**
  ******************************************************************************
  * @file    stm32wb_at_client.h
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
#ifndef STM32WB_AT_CLIENT_H
#define STM32WB_AT_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32wb_at.h"
#include "stm32wb_at_ll.h"
#include "stm32wb_at_ble.h"

/* Private includes ----------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macros -----------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
uint8_t stm32wb_at_client_Init(void);
uint8_t stm32wb_at_client_Query(stm32wb_at_BLE_CMD_t cmd);
uint8_t stm32wb_at_client_Process_rx_frame(char * str);
uint8_t stm32wb_at_client_Set(stm32wb_at_BLE_CMD_t cmd, void *param);

#ifdef __cplusplus
}
#endif

#endif /* STM32WB_AT_CLIENT_H */
