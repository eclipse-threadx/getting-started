/**
  ******************************************************************************
  * @file    sx8651.h
  * @author  MCD Application Team
  * @brief   This file contains all the functions prototypes for the
  *          sx8651.c Touch screen driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef SX8651_H
#define SX8651_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "sx8651_reg.h"
#include <stddef.h>
#include "sx8651_conf.h"

/* Macros --------------------------------------------------------------------*/

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Component
  * @{
  */

/** @addtogroup SX8651
  * @{
  */

/** @defgroup SX8651_Exported_Constants SX8651 Exported Constants
  * @{
  */
#define SX8651_OK                      (0)
#define SX8651_ERROR                   (-1)

/* Max detectable simultaneous touches */
#define SX8651_MAX_NB_TOUCH             2U

/* Software reset value of SX8651_SOFTRESET_REG */
#define SX8651_SOFTRESET_VALUE           0xDEU

/* Psedo Chip IDs, @I2CRegChanMsk */
#define SX8651_ID               0xC0

/* Psedo Identification register */
#define SX8651_REG_CHP_ID       0x08

/* Global interrupt Enable bit */
#define SX8651_GIT_EN           0x01

/**
  * @}
  */

/* Exported types ------------------------------------------------------------*/

/** @defgroup SX8651_Exported_Types SX8651 Exported Types
  * @{
  */
typedef struct
{
  uint32_t  Radian;
  uint32_t  OffsetLeftRight;
  uint32_t  OffsetUpDown;
  uint32_t  DistanceLeftRight;
  uint32_t  DistanceUpDown;
  uint32_t  DistanceZoom;
} SX8651_Gesture_Init_t;

typedef int32_t (*SX8651_Init_Func)(void);
typedef int32_t (*SX8651_DeInit_Func)(void);
typedef int32_t (*SX8651_GetTick_Func)(void);
typedef int32_t (*SX8651_Delay_Func)(uint32_t);
typedef int32_t (*SX8651_WriteReg_Func)(uint16_t, uint16_t, uint8_t *, uint16_t);
typedef int32_t (*SX8651_ReadReg_Func)(uint16_t, uint16_t, uint8_t *, uint16_t);
typedef int32_t (*SX8651_ReadData_Func)(uint16_t, uint8_t *, uint16_t);
typedef int32_t (*SX8651_WriteData_Func)(uint16_t, uint8_t *, uint16_t);

typedef struct
{
  SX8651_Init_Func          Init;
  SX8651_DeInit_Func        DeInit;
  uint16_t                  Address;
  SX8651_WriteReg_Func      WriteReg;
  SX8651_ReadReg_Func       ReadReg;
  SX8651_ReadData_Func      ReadData;
  SX8651_WriteData_Func     WriteCmd;
  SX8651_GetTick_Func       GetTick;
} SX8651_IO_t;

typedef struct
{
  uint32_t  TouchDetected;
  uint32_t  TouchX;
  uint32_t  TouchY;
} SX8651_State_t;

typedef struct
{
  uint32_t  TouchDetected;
  uint32_t  TouchX[SX8651_MAX_NB_TOUCH];
  uint32_t  TouchY[SX8651_MAX_NB_TOUCH];
  uint32_t  TouchWeight[SX8651_MAX_NB_TOUCH];
  uint32_t  TouchEvent[SX8651_MAX_NB_TOUCH];
  uint32_t  TouchArea[SX8651_MAX_NB_TOUCH];
} SX8651_MultiTouch_State_t;

typedef struct
{
  SX8651_IO_t         IO;
  sx8651_ctx_t        Ctx;
  uint8_t             IsInitialized;
} SX8651_Object_t;

typedef struct
{
  uint8_t   MultiTouch;
  uint8_t   Gesture;
  uint8_t   MaxTouch;
  uint32_t  MaxXl;
  uint32_t  MaxYl;
} SX8651_Capabilities_t;

typedef struct
{
  int32_t (*Init)(SX8651_Object_t *);
  int32_t (*DeInit)(SX8651_Object_t *);
  int32_t (*GestureConfig)(SX8651_Object_t *, SX8651_Gesture_Init_t *);
  int32_t (*ReadID)(SX8651_Object_t *, uint32_t *);
  int32_t (*GetState)(SX8651_Object_t *, SX8651_State_t *);
  int32_t (*GetMultiTouchState)(SX8651_Object_t *, SX8651_MultiTouch_State_t *);
  int32_t (*GetGesture)(SX8651_Object_t *, uint8_t *);
  int32_t (*GetCapabilities)(SX8651_Object_t *, SX8651_Capabilities_t *);
  int32_t (*EnableIT)(SX8651_Object_t *);
  int32_t (*DisableIT)(SX8651_Object_t *);
  int32_t (*ClearIT)(SX8651_Object_t *);
  int32_t (*ITStatus)(SX8651_Object_t *);
} SX8651_TS_Drv_t;
/**
  * @}
  */

/** @addtogroup SX8651_Exported_Variables
  * @{
  */
extern SX8651_TS_Drv_t SX8651_TS_Driver;
/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @addtogroup SX8651_Exported_Functions
  * @{
  */

int32_t SX8651_RegisterBusIO(SX8651_Object_t *pObj, SX8651_IO_t *pIO);
int32_t SX8651_Init(SX8651_Object_t *pObj);
int32_t SX8651_DeInit(SX8651_Object_t *pObj);
int32_t SX8651_GestureConfig(SX8651_Object_t *pObj, SX8651_Gesture_Init_t *GestureInit);
int32_t SX8651_ReadID(SX8651_Object_t *pObj, uint32_t *Id);
int32_t SX8651_GetState(SX8651_Object_t *pObj, SX8651_State_t *State);
int32_t SX8651_GetMultiTouchState(SX8651_Object_t *pObj, SX8651_MultiTouch_State_t *State);
int32_t SX8651_GetGesture(SX8651_Object_t *pObj, uint8_t *GestureId);
int32_t SX8651_EnableIT(SX8651_Object_t *pObj);
int32_t SX8651_DisableIT(SX8651_Object_t *pObj);
int32_t SX8651_ITStatus(SX8651_Object_t *pObj);
int32_t SX8651_ClearIT(SX8651_Object_t *pObj);
int32_t SX8651_GetCapabilities(SX8651_Object_t *pObj, SX8651_Capabilities_t *Capabilities);

/**
  * @}
  */

#ifdef __cplusplus
}
#endif
#endif /* SX8651_H */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
