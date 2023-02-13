/**
  ******************************************************************************
  * @file    stm32wb_at.h
  * @author  MCD Application Team
  * @brief   Header file of AT transport layer.
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
#ifndef STM32WB_AT_H
#define STM32WB_AT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
#define AT_PRE_CMD             "AT+"
#define AT_CMD_TEST            "AT"

#define AT_GET                 "?"
#define AT_SET                 "="
#define AT_READ                ":"
#define AT_REPLY               "+"
#define AT_EVENT               "<"
#define AT_SEPARATOR           ","
#define AT_EOL                 "\r\n"

#define AT_CMD_STATUS_OK       "OK"
#define AT_CMD_STATUS_ERROR    "ERROR"

/* Exported macros -----------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
uint8_t stm32wb_at_Init(uint8_t *buff_rx, uint8_t buff_rx_size);
uint8_t stm32wb_at_Received(uint8_t byte);
uint8_t stm32wb_at_Process_rx_frame(char * str);

#ifdef __cplusplus
}
#endif

#endif /* STM32WB_AT_H */
