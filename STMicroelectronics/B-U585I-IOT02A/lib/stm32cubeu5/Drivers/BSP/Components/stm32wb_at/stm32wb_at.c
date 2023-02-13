/**
  ******************************************************************************
  * @file    stm32wb_at.c
  * @author  MCD Application Team
  * @brief   Transport layer.
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
#include "stm32wb_at_client.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t *buffer_rx;
uint8_t buffer_rx_size;
uint8_t buffer_rx_cursor;
char str_received[64];
/* Private function prototypes -----------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

/**
 * @brief Init the at module
 * @param None
 * @retval 0 in case of success, an error code otherwise
 */
uint8_t stm32wb_at_Init(uint8_t *buff_rx, uint8_t buff_rx_size)
{
  uint8_t status;

  if(buff_rx_size >= sizeof(str_received))
  {
    buffer_rx_size = buff_rx_size;
    buffer_rx = buff_rx;
    (void)memset(&buffer_rx[0], 0, buffer_rx_size);
    buffer_rx_cursor = 0;
    (void)memset(&str_received[0], 0, sizeof(str_received));

    stm32wb_at_ll_Async_receive(0);
    status = 0;
  }
  else
  {
    status = 1;
  }
  return status;
}

/**
 * @brief Accumulate bytes received and detects a new frame received
 * @param byte the last byte received
 * @retval 0 in case of success, an error code otherwise
 */
uint8_t stm32wb_at_Received(uint8_t byte)
{
  uint8_t status, i;

  if(buffer_rx_cursor < buffer_rx_size)
  {
    buffer_rx[buffer_rx_cursor] = byte;
    buffer_rx_cursor++;

    /* check if we have received the command delimiter LF */
    if(byte == (uint8_t)'\n')
    {
      if(buffer_rx_cursor > 2U){
        /* remove CR if any */
        if( buffer_rx[buffer_rx_cursor - 2U] == (uint8_t)'\r')
        {
          buffer_rx[buffer_rx_cursor - 1U] = 0U;
          buffer_rx[buffer_rx_cursor - 2U] = (uint8_t)'\n';
          buffer_rx_cursor--;
        }
        /* remove the last \n */
        buffer_rx[buffer_rx_cursor - 1U] = 0U;
        buffer_rx_cursor--;

        if( buffer_rx[0] == 0U)
        {
          i = 1U;
          while(buffer_rx[i] != 0U)
          {
            buffer_rx[i - 1U] = buffer_rx[i];
            i++;
          }
          buffer_rx[i - 1U] = 0U;
        }
      }

      /* check if command is long enough */
      if(buffer_rx_cursor > 1U)
      {
        (void)strcpy(&str_received[0], (char*)&buffer_rx[0]);

        stm32wb_at_ll_Async_receive((uint8_t)sizeof(str_received));

        (void)memset(&buffer_rx[0], 0, buffer_rx_size);
        buffer_rx_cursor = 0U;

        status = stm32wb_at_Process_rx_frame(str_received);
        (void)memset(&str_received[0], 0, sizeof(str_received));
      }
      else
      {
        (void)memset(&buffer_rx[0], 0, buffer_rx_size);
        buffer_rx_cursor = 0U;
      }
    }
    else
    {
      stm32wb_at_ll_Async_receive(0);
    }
    status = 0U;
  }
  else
  {
    status = 1U;
  }

  return status;
}

/**
 * @brief Process a frame received
 * @param str pointer string to process
 * @retval 0 in case of success, an error code otherwise
 */
__weak uint8_t stm32wb_at_Process_rx_frame(char * str)
{
  (void)stm32wb_at_client_Process_rx_frame(str);

  return 0;
}
