/**
  ******************************************************************************
  * @file    stm32wb_at_client.c
  * @author  MCD Application Team
  * @brief   AT client.
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
#include "stm32wb_at_client.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static char client_buff_tx[64];
static stm32wb_at_BLE_CMD_t client_current_cmd;

/* Private function prototypes -----------------------------------------------*/
static uint8_t stm32wb_at_client_Wait_ready(void);

/* Exported functions --------------------------------------------------------*/

/**
 * @brief Init the at client
 * @param None
 * @retval 0 in case of success, an error code otherwise
 */
uint8_t stm32wb_at_client_Init(void)
{
  (void)memset(&client_buff_tx[0], 0, sizeof(client_buff_tx));
  client_current_cmd =  BLE_NONE;

  return 0;
}

/**
 * @brief Query the server on a specified command
 * @param cmd code to query
 * @retval 0 in case of success, an error code otherwise
 */
uint8_t stm32wb_at_client_Query(stm32wb_at_BLE_CMD_t cmd)
{
  uint8_t status;

  (void)stm32wb_at_client_Wait_ready();

  if(cmd == BLE_TEST)
  {
    (void)strcpy(client_buff_tx, AT_CMD_TEST);
    (void)strcat(client_buff_tx, AT_EOL);

    client_current_cmd = cmd;
    status = stm32wb_at_ll_Transmit((uint8_t *)&client_buff_tx[0], (uint16_t)strlen(client_buff_tx));
    (void)memset(&client_buff_tx[0], 0, sizeof(client_buff_tx));
  }
  else if(cmd < BLE_NONE)
  {
    (void)strcpy(client_buff_tx, AT_PRE_CMD);
    (void)strcat(client_buff_tx, AT_BLE_CMD_STRING[cmd]);
    (void)strcat(client_buff_tx, AT_GET);
    (void)strcat(client_buff_tx, AT_EOL);

    client_current_cmd = cmd;
    status = stm32wb_at_ll_Transmit((uint8_t *)&client_buff_tx[0], (uint16_t)strlen(client_buff_tx));
    (void)memset(&client_buff_tx[0], 0, sizeof(client_buff_tx));
  }
  else
  {
    status = 1;
  }

  return status;
}

/**
 * @brief Process an At string and perform required actions
 * @param str pointer to the string to process
 * @retval 0 in case of success, an error code otherwise
 */
uint8_t stm32wb_at_client_Process_rx_frame(char * str)
{
  uint8_t status;
  stm32wb_at_BLE_EVT_t current_evt;

  status = 1;

  if( strcmp(str, AT_CMD_STATUS_OK) == 0)
  {
    client_current_cmd = BLE_NONE;
    status = 0;
  }
  else if( strcmp(str, AT_CMD_STATUS_ERROR) == 0)
  {
    client_current_cmd = BLE_NONE;
    status = 0;
  }
  else if( strncmp(str, AT_REPLY, strlen(AT_REPLY)) == 0)
  {
    str++;
    /* check if it's a response of current command */
    if( strncmp(str, AT_BLE_CMD_STRING[client_current_cmd], strlen(AT_BLE_CMD_STRING[client_current_cmd])) == 0 )
    {
      str += strlen(AT_BLE_CMD_STRING[client_current_cmd]);

      if( strncmp(str, AT_READ, strlen(AT_READ)) == 0)
      {
        str++;
        /* extract params regarding current command */
        switch (client_current_cmd) {
          case BLE_SVC:
          {
            stm32wb_at_BLE_SVC_t param;
            param.index = (uint8_t)strtol(str, NULL, 10);
            status = stm32wb_at_BLE_SVC_cb(&param);
            break;
          }
          case BLE_CONN_INT:
          {
            stm32wb_at_BLE_CONN_INT_t param;
            char * token;

            token = strtok(str, AT_SEPARATOR);
            param.min = (uint8_t)strtol(token, NULL, 10);

            token = strtok(NULL, AT_SEPARATOR);
            param.max = (uint8_t)strtol(token, NULL, 10);

            status = stm32wb_at_BLE_CONN_INT_cb(&param);
            break;
          }
          default:
            break;
        }
      }
    }
  }
  else if( strncmp(str, AT_EVENT, strlen(AT_EVENT)) == 0)
  {
    str++;

    for( current_evt = (stm32wb_at_BLE_EVT_t)0 ; current_evt < BLE_EVT_NONE ; current_evt++ )
    {
      if( strncmp(str, AT_BLE_EVT_STRING[current_evt], strlen(AT_BLE_EVT_STRING[current_evt])) == 0 )
      {
        str += strlen(AT_BLE_EVT_STRING[current_evt]);
        break;
      }
    }
    if( current_evt == BLE_EVT_NONE)
    {
      status = 1;
    }
    else
    {
      if (strncmp(str, AT_SET, strlen(AT_SET)) == 0)
      {
        str++;
        /* extract params regarding current command */
        switch (current_evt) {
          case BLE_EVT_WRITE:
          {
            stm32wb_at_BLE_EVT_WRITE_t param;
            char * token;

            token = strtok(str, AT_SEPARATOR);
            param.svc_index = (uint8_t)strtol(token, NULL, 10);

            token = strtok(NULL, AT_SEPARATOR);
              param.char_index = (uint8_t)strtol(token, NULL, 10);

            token = strtok(NULL, AT_SEPARATOR);
            param.value = (uint8_t)strtol(token, NULL, 10);

            status = stm32wb_at_BLE_EVT_WRITE_cb(&param);
            break;
          }
          case BLE_EVT_CONN:
          {
            stm32wb_at_BLE_EVT_CONN_t param;
            char * token;

            token = strtok(str, AT_SEPARATOR);
            param.status = (uint8_t)strtol(token, NULL, 10);

            status = stm32wb_at_BLE_EVT_CONN_cb(&param);
            break;
          }
          default:
            break;
        }
      }
      else
      {
        status = 1;
      }
    }
  }

  return status;
}

/**
 * @brief Send an AT set with specified command and parameters
 * @param cmd command code
 * @param param pointer to parameters of command
 * @retval 0 in case of success, an error code otherwise
 */
uint8_t stm32wb_at_client_Set(stm32wb_at_BLE_CMD_t cmd, void *param)
{
  uint8_t status;

  (void)stm32wb_at_client_Wait_ready();

  if(cmd < BLE_NONE)
  {
    client_current_cmd = cmd;

    (void)strcpy(client_buff_tx, AT_PRE_CMD);
    (void)strcat(client_buff_tx, AT_BLE_CMD_STRING[cmd]);
    (void)strcat(client_buff_tx, AT_SET);

    switch (cmd) {
      case BLE_SVC:
      {
        stm32wb_at_BLE_SVC_t *p = param;
        char str_tmp[8];

        (void)sprintf(str_tmp, "%d", p->index);
        (void)strcat(client_buff_tx, str_tmp);

        break;
      }
      case BLE_NOTIF_VAL:
      {
        stm32wb_at_BLE_NOTIF_VAL_t *p = param;
        char str_tmp[8];

        (void)sprintf(str_tmp, "%d", p->svc_index);
        (void)strcat(client_buff_tx, str_tmp);
        (void)strcat(client_buff_tx, AT_SEPARATOR);

        (void)sprintf(str_tmp, "%d", p->char_index);
        (void)strcat(client_buff_tx, str_tmp);
        (void)strcat(client_buff_tx, AT_SEPARATOR);

        (void)sprintf(str_tmp, "%d", p->value);
        (void)strcat(client_buff_tx, str_tmp);

        break;
      }
      case BLE_CONN_INT:
      {
        stm32wb_at_BLE_CONN_INT_t *p = param;
        char str_tmp[8];

        (void)sprintf(str_tmp, "%d", p->min);
        (void)strcat(client_buff_tx, str_tmp);
        (void)strcat(client_buff_tx, AT_SEPARATOR);

        (void)sprintf(str_tmp, "%d", p->max);
        (void)strcat(client_buff_tx, str_tmp);

        break;
      }
      default:
        break;
    }

    (void)strcat(client_buff_tx, AT_EOL);

    client_current_cmd = cmd;
    status = stm32wb_at_ll_Transmit((uint8_t *)&client_buff_tx[0], (uint16_t)strlen(client_buff_tx));
    (void)memset(&client_buff_tx[0], 0, sizeof(client_buff_tx));
  }
  else
  {
    status = 1;
  }

  return status;
}

/**
 * @brief Wait for the last AT command completed
 * @param None
 * @retval 0 in case of success, an error code otherwise
 */
static uint8_t stm32wb_at_client_Wait_ready(void)
{
  while (client_current_cmd !=  BLE_NONE)
  {
  }

  return 0;
}
