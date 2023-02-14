/**
  ******************************************************************************
  * @file    stm32wb_at_ble.h
  * @author  MCD Application Team
  * @brief   Header file of AT BLE commands.
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
#ifndef STM32WB_AT_BLE_H
#define STM32WB_AT_BLE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32wb_at.h"

/* Private includes ----------------------------------------------------------*/
#define FOREACH_AT_BLE_CMD(AT_BLE_CMD)\
        AT_BLE_CMD(BLE_TEST)\
        AT_BLE_CMD(BLE_SVC)\
        AT_BLE_CMD(BLE_NOTIF_VAL)\
        AT_BLE_CMD(BLE_CONN_INT)\
        AT_BLE_CMD(BLE_NONE)\

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,
#define GENERATE_AT_BLE_CB_PROTO(CMD) uint8_t stm32wb_at_##CMD##_cb(stm32wb_at_##CMD##_t *param);
#define GENERATE_AT_BLE_CB_FUNC(CMD) __weak uint8_t stm32wb_at_##CMD##_cb(stm32wb_at_##CMD##_t *param){UNUSED(param);return 1;}
#define GENERATE_AT_BLE_SERVER_CB_PROTO(CMD) uint8_t stm32wb_at_##CMD##_server_cb(char *buff);
#define GENERATE_AT_BLE_SERVER_CB_FUNC(CMD) __weak uint8_t stm32wb_at_##CMD##_server_cb(char *buff){return 1;}

#define FOREACH_AT_BLE_EVT(AT_BLE_EVT)\
        AT_BLE_EVT(BLE_EVT_WRITE)\
        AT_BLE_EVT(BLE_EVT_CONN)\
        AT_BLE_EVT(BLE_EVT_NONE)\

#define GENERATE_AT_BLE_EVT_CB_PROTO(CMD) uint8_t stm32wb_at_##CMD##_cb(stm32wb_at_##CMD##_t *param);
#define GENERATE_AT_BLE_EVT_CB_FUNC(CMD) __weak uint8_t stm32wb_at_##CMD##_cb(stm32wb_at_##CMD##_t *param){UNUSED(param);return 1;}

/* Exported types ------------------------------------------------------------*/
/* BLE_TEST */
typedef struct
{
  uint8_t dummy;
} stm32wb_at_BLE_TEST_t;

/* BLE_SVC */
typedef struct
{
  uint8_t index;
} stm32wb_at_BLE_SVC_t;

/* BLE_NOTIF_VAL */
typedef struct
{
  uint8_t svc_index;
  uint8_t char_index;
  uint16_t value;
} stm32wb_at_BLE_NOTIF_VAL_t;

/* BLE_CONN_INT */
typedef struct
{
  uint16_t min;
  uint16_t max;
} stm32wb_at_BLE_CONN_INT_t;

/* BLE_NONE */
typedef struct
{
  uint8_t dummy;
} stm32wb_at_BLE_NONE_t;

/* BLE_EVT_WRITE */
typedef struct
{
  uint8_t svc_index;
  uint8_t char_index;
  uint32_t value;
} stm32wb_at_BLE_EVT_WRITE_t;

/* BLE_EVT_CONN */
typedef struct
{
  uint8_t status;
} stm32wb_at_BLE_EVT_CONN_t;

/* BLE_EVT_NONE */
typedef struct
{
  uint8_t dummy;
} stm32wb_at_BLE_EVT_NONE_t;

/* Exported constants --------------------------------------------------------*/
typedef enum AT_BLE_CMD_ENUM {
  FOREACH_AT_BLE_CMD(GENERATE_ENUM)
}stm32wb_at_BLE_CMD_t;

typedef enum AT_BLE_EVT_ENUM {
  FOREACH_AT_BLE_EVT(GENERATE_ENUM)
}stm32wb_at_BLE_EVT_t;

extern const char* AT_BLE_CMD_STRING[];
extern const char* AT_BLE_EVT_STRING[];

/* Exported macros -----------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
FOREACH_AT_BLE_CMD(GENERATE_AT_BLE_CB_PROTO)
FOREACH_AT_BLE_EVT(GENERATE_AT_BLE_EVT_CB_PROTO)

#ifdef __cplusplus
}
#endif

#endif /* STM32WB_AT_BLE_H */
