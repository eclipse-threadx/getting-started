/**
 ******************************************************************************
 * @file    veml6030.h
 * @author  MCD Application Team
 * @brief   VEML6030 header driver file
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
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
#ifndef VEML6030_H
#define VEML6030_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "veml6030_reg.h"
#include <string.h>

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Components
  * @{
  */

/** @addtogroup VEML6030
  * @{
  */

/** @defgroup VEML6030_Exported_Types
  * @{
  */

typedef int32_t (*VEML6030_Init_Func)(void);
typedef int32_t (*VEML6030_DeInit_Func)(void);
typedef int32_t (*VEML6030_GetTick_Func)(void);
typedef int32_t (*VEML6030_Delay_Func)(uint32_t);
typedef int32_t (*VEML6030_WriteReg_Func)(uint16_t, uint16_t, uint8_t*, uint16_t);
typedef int32_t (*VEML6030_ReadReg_Func)(uint16_t, uint16_t, uint8_t*, uint16_t);
typedef int32_t (*VEML6030_IsReady_Func)(uint16_t, uint32_t);

typedef struct
{
  VEML6030_Init_Func          Init;
  VEML6030_DeInit_Func        DeInit;
  uint16_t                    ReadAddress;
  uint16_t                    WriteAddress;
  VEML6030_IsReady_Func       IsReady;
  VEML6030_WriteReg_Func      WriteReg;
  VEML6030_ReadReg_Func       ReadReg;
  VEML6030_GetTick_Func       GetTick;
} VEML6030_IO_t;


typedef struct
{
  VEML6030_IO_t         IO;
  veml6030_ctx_t        Ctx;
  uint8_t IsInitialized;
  uint8_t IsContinuous;
  uint8_t IsStarted;
} VEML6030_Object_t;


typedef struct
{
  uint8_t NumberOfChannels;  /*!< Max: LIGHT_SENSOR_MAX_CHANNELS */
  uint8_t FlickerDetection;  /*!< Not available: 0, Available: 1 */
  uint8_t Autogain;          /*!< Not available: 0, Available: 1 */
} VEML6030_Capabilities_t;

typedef struct
{
  int32_t (*Init)(VEML6030_Object_t *);
  int32_t (*DeInit)(VEML6030_Object_t *);
  int32_t (*ReadID)(VEML6030_Object_t *, uint32_t *);
  int32_t (*GetCapabilities)(VEML6030_Object_t *, VEML6030_Capabilities_t *);
  int32_t (*SetExposureTime)(VEML6030_Object_t *, uint32_t);
  int32_t (*GetExposureTime)(VEML6030_Object_t *, uint32_t *);
  int32_t (*SetGain)(VEML6030_Object_t *, uint8_t, uint32_t);
  int32_t (*GetGain)(VEML6030_Object_t *, uint8_t, uint32_t *);
  int32_t (*SetInterMeasurementTime)(VEML6030_Object_t *, uint32_t);
  int32_t (*GetInterMeasurementTime)(VEML6030_Object_t *, uint32_t *);
  int32_t (*Start)(VEML6030_Object_t *, uint32_t);
  int32_t (*Stop)(VEML6030_Object_t *);
  int32_t (*StartFlicker)(VEML6030_Object_t *, uint8_t, uint8_t);
  int32_t (*StopFlicker)(VEML6030_Object_t *);
  int32_t (*GetValues)(VEML6030_Object_t *, uint32_t *);
  int32_t (*SetControlMode)(VEML6030_Object_t *, uint32_t, uint32_t);
}VEML6030_Drv_t;

/**
  * @}
  */

/** @defgroup VEML6030_Exported_Constants
  * @{
  */

/* VEML6030 error codes */
#define VEML6030_OK                      (0)
#define VEML6030_ERROR                   (-1)
#define VEML6030_INVALID_PARAM           (-2)

/* softowre ID */
#define VEML6030_ID              (0x6030U)

/**
  * @brief  VEML6030 Features Parameters
  */

/* VEML6030 Channel */
#define VEML6030_ALS_CHANNEL     (0U)
#define VEML6030_WHITE_CHANNEL   (1U)
#define VEML6030_MAX_CHANNELS    (2U)  /*!< Number of channels of the device */

/* VEML6030 capture modes */
#define VEML6030_MODE_CONTINUOUS (1U)

/* VEML6030 interrupt */
#define VEML6030_INT_TH_NONE 0U
#define VEML6030_INT_TH_LOW 1U
#define VEML6030_INT_TH_HIGH 2U

 /* VEML6030 I2C ADDRESS */
#define VEML6030_I2C_READ_ADD 0x21
#define VEML6030_I2C_WRITE_ADD 0x20

/* VEML6030 Refresh Time (Intermesurment Time) definitions */
/* VEML6030’s refresh time can be determined by Power Saving Mode (PSM) and the Integration Time (ALS_IT) */
/* The tow first LSB bits of each refresh time first byte definition represent the integration time */
/* The tow first MSB bits of each refresh time first byte definition represent the Power saving Mode */
#define VEML6030_REFRESH_TIME_600   (0x00U)
#define VEML6030_REFRESH_TIME_700   (0x01U)
#define VEML6030_REFRESH_TIME_900   (0x02U)
#define VEML6030_REFRESH_TIME_1100  (0x04U)
#define VEML6030_REFRESH_TIME_1200  (0x05U)
#define VEML6030_REFRESH_TIME_1300  (0x03U)
#define VEML6030_REFRESH_TIME_1400  (0x06U)
#define VEML6030_REFRESH_TIME_1800  (0x07U)
#define VEML6030_REFRESH_TIME_2100  (0x08U)
#define VEML6030_REFRESH_TIME_2200  (0x09U)
#define VEML6030_REFRESH_TIME_2400  (0x0AU)
#define VEML6030_REFRESH_TIME_2800  (0x0BU)
#define VEML6030_REFRESH_TIME_4100  (0x0CU)
#define VEML6030_REFRESH_TIME_4200  (0x0DU)
#define VEML6030_REFRESH_TIME_4400  (0x0EU)
#define VEML6030_REFRESH_TIME_4800  (0x0FU)

/* driver structure */
extern VEML6030_Drv_t VEML6030_Driver;

/**
  * @}
  */

/** @addtogroup VEML6030_Exported_Functions VEML6030 Exported Functions
 * @{
 */

 int32_t VEML6030_RegisterBusIO(VEML6030_Object_t *pObj, VEML6030_IO_t *pIO);
 int32_t VEML6030_Init(VEML6030_Object_t *pObj);
 int32_t VEML6030_DeInit(VEML6030_Object_t *pObj);
 int32_t VEML6030_ReadID(VEML6030_Object_t *pObj, uint32_t *pId);
 int32_t VEML6030_SetExposureTime(VEML6030_Object_t *pObj, uint32_t ExposureTime);
 int32_t VEML6030_GetExposureTime(VEML6030_Object_t *pObj, uint32_t *pExposureTime);
 int32_t VEML6030_GetCapabilities(VEML6030_Object_t *pObj,  VEML6030_Capabilities_t *pCapabilities);
 int32_t VEML6030_SetGain(VEML6030_Object_t *pObj, uint8_t Channel, uint32_t Gain);
 int32_t VEML6030_GetGain(VEML6030_Object_t *pObj, uint8_t Channel, uint32_t *Gain);
 int32_t VEML6030_SetInterMeasurementTime(VEML6030_Object_t *pObj, uint32_t InterMeasurementTime);
 int32_t VEML6030_GetInterMeasurementTime(VEML6030_Object_t *pObj, uint32_t *InterMeasurementTime);
 int32_t VEML6030_Start(VEML6030_Object_t *pObj, uint32_t);
 int32_t VEML6030_Stop(VEML6030_Object_t *pObj);
 int32_t VEML6030_GetValues(VEML6030_Object_t *, uint32_t *Values);
 int32_t VEML6030_StartFlicker(VEML6030_Object_t *pObj, uint8_t Channel, uint8_t OutputMode);
 int32_t VEML6030_StopFlicker(VEML6030_Object_t *pObj);
 int32_t VEML6030_SetControlMode(VEML6030_Object_t *pObj, uint32_t ControlMode, uint32_t Value);

 int32_t VEML6030_SetPowerSavingMode(VEML6030_Object_t *pObj, uint32_t PowerMode);
 int32_t VEML6030_GetPowerSavingMode(VEML6030_Object_t *pObj, uint32_t *pPowerMode);
 int32_t VEML6030_SetPersistence(VEML6030_Object_t *pObj, uint32_t Persistence);
 int32_t VEML6030_GetPersistence(VEML6030_Object_t *pObj, uint32_t *Persistence);
 int32_t VEML6030_GetWhiteValues(VEML6030_Object_t *pObj, uint32_t *Values);
 /* interrupt Mangement Functions */
 int32_t VEML6030_SetHighThreshold(VEML6030_Object_t *pObj , uint16_t Threshold);
 int32_t VEML6030_GetHighThreshold(VEML6030_Object_t *pObj , uint32_t *Threshold);
 int32_t VEML6030_SetLowThreshold(VEML6030_Object_t *pObj , uint16_t Threshold);
 int32_t VEML6030_GetLowThreshold(VEML6030_Object_t *pObj , uint32_t *Threshold);
 int32_t VEML6030_GetIntStatus(VEML6030_Object_t *pObj,uint32_t *status);
 int32_t VEML6030_Disable_IT(VEML6030_Object_t *pObj);
 int32_t VEML6030_Enable_IT(VEML6030_Object_t *pObj);

/**
  * @}
  */
#ifdef __cplusplus
}
#endif

#endif /* VEML6030_H */

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