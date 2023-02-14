/**
  ******************************************************************************
  * @file    stm32wb_at_ble.c
  * @author  MCD Application Team
  * @brief   AT BLE commands
  *
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

/* Includes ------------------------------------------------------------------*/
#include "stm32wb_at_ble.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported variable ---------------------------------------------------------*/
const char *AT_BLE_CMD_STRING[] = {
  FOREACH_AT_BLE_CMD(GENERATE_STRING)
};

const char *AT_BLE_EVT_STRING[] = {
  FOREACH_AT_BLE_EVT(GENERATE_STRING)
};

/* Exported functions --------------------------------------------------------*/
FOREACH_AT_BLE_CMD(GENERATE_AT_BLE_CB_FUNC)
FOREACH_AT_BLE_EVT(GENERATE_AT_BLE_EVT_CB_FUNC)
