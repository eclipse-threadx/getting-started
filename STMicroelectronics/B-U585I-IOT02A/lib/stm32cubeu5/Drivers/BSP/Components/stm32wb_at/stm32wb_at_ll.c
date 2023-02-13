/**
  ******************************************************************************
  * @file    stm32wb_at_ll.c
  * @author  MCD Application Team
  * @brief   Transport layer interface file, user have to implement these
  *          functions in his application.
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
#include "stm32wb_at.h"
#include "stm32wb_at_ll.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/**
 * @brief  Init the transport module
 * @param  None
 * @retval 0 in case of success, an error code otherwise
 */
__weak uint8_t stm32wb_at_ll_Init(void)
{
  return 1;
}

/**
 * @brief  De init the transport module
 * @param  None
 * @retval 0 in case of success, an error code otherwise
 */
__weak uint8_t stm32wb_at_ll_DeInit(void)
{
  return 1;
}

/**
 * @brief  Triggers the reception of a byte
 * @param  pBuff pointer to the first byte to transmit
 * @param  Size byte count to transfer
 * @retval 0 in case of success, an error code otherwise
 */
__weak uint8_t stm32wb_at_ll_Transmit(uint8_t *pBuff, uint16_t Size)
{
  UNUSED(pBuff);
  UNUSED(Size);

  return 1;
}

/**
 * @brief  Triggers the reception of a byte
 * @param  new_frame containts the length of a new frame received in rx buffer, else 0
 * @retval None
 */
__weak void stm32wb_at_ll_Async_receive(uint8_t new_frame)
{
  UNUSED(new_frame);
  return;
}
