/**
  ******************************************************************************
  * @file    b_u585i_iot02a_env_sensors.h
  * @author  MCD Application Team
  * @brief   This file contains the common defines and functions prototypes for
  *          the b_u585i_iot02a_env_sensors driver.
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
#ifndef B_U585I_IOT02A_ENV_SENSORS_H
#define B_U585I_IOT02A_ENV_SENSORS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "b_u585i_iot02a_conf.h"
#include "b_u585i_iot02a_errno.h"
#include "../Components/Common/env_sensor.h"
#include "../Components/lps22hh/lps22hh.h"
#include "../Components/hts221/hts221.h"
#include <math.h>

/** @addtogroup BSP
  * @{
  */

/** @addtogroup B_U585I_IOT02A
  * @{
  */

/** @addtogroup B_U585I_IOT02A_ENV_SENSORS
  * @{
  */

/** @defgroup B_U585I_IOT02A_ENV_SENSORS_Exported_Types ENVIRONMENTAL_SENSORS Exported Types
  * @{
  */
typedef struct
{
  uint8_t Temperature;
  uint8_t Pressure;
  uint8_t Humidity;
  uint8_t LowPower;
  float   HumMaxOdr;
  float   TempMaxOdr;
  float   PressMaxOdr;
} ENV_SENSOR_Capabilities_t;

typedef struct
{
  uint32_t Functions;
} ENV_SENSOR_Ctx_t;
/**
  * @}
  */

/** @defgroup B_U585I_IOT02A_ENV_SENSORS_Exported_Constants ENVIRONMENTAL_SENSORS Exported Constants
  * @{
  */
/* Two environmental sensors are mounted on the B_U585I_IOT02A board,
   so the number of environmental sensors instances is set to 2.
   Instance 0 corresponds to HTS221 (temperature and humidity capabilities).
   Instance 1 corresponds to LPS22HH (temperature and pressure capabilities). */
#define ENV_SENSOR_INSTANCES_NBR    2U

#define ENV_SENSOR_FUNCTIONS_NBR    3U

#define ENV_TEMPERATURE      1U
#define ENV_PRESSURE         2U
#define ENV_HUMIDITY         4U
/**
  * @}
  */

/** @addtogroup B_U585I_IOT02A_ENV_SENSORS_Exported_Variables
  * @{
  */
/* Environmental sensor context */
extern ENV_SENSOR_Ctx_t Env_Sensor_Ctx[ENV_SENSOR_INSTANCES_NBR];

/* Environmental sensor component object */
extern void *Env_Sensor_CompObj[ENV_SENSOR_INSTANCES_NBR];

/* Environmental sensor common driver */
extern ENV_SENSOR_CommonDrv_t *Env_Sensor_Drv[ENV_SENSOR_INSTANCES_NBR];

/* Environmental sensor function driver */
extern ENV_SENSOR_FuncDrv_t *Env_Sensor_FuncDrv[ENV_SENSOR_INSTANCES_NBR][ENV_SENSOR_FUNCTIONS_NBR];
/**
  * @}
  */

/** @addtogroup B_U585I_IOT02A_ENV_SENSORS_Exported_Functions
  * @{
  */
int32_t BSP_ENV_SENSOR_Init(uint32_t Instance, uint32_t Functions);
int32_t BSP_ENV_SENSOR_DeInit(uint32_t Instance);
int32_t BSP_ENV_SENSOR_GetCapabilities(uint32_t Instance, ENV_SENSOR_Capabilities_t *Capabilities);
int32_t BSP_ENV_SENSOR_ReadID(uint32_t Instance, uint8_t *Id);
int32_t BSP_ENV_SENSOR_Enable(uint32_t Instance, uint32_t Function);
int32_t BSP_ENV_SENSOR_Disable(uint32_t Instance, uint32_t Function);
int32_t BSP_ENV_SENSOR_GetOutputDataRate(uint32_t Instance, uint32_t Function, float_t *Odr);
int32_t BSP_ENV_SENSOR_SetOutputDataRate(uint32_t Instance, uint32_t Function, float_t Odr);
int32_t BSP_ENV_SENSOR_GetValue(uint32_t Instance, uint32_t Function, float_t *Value);
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* B_U585I_IOT02A_ENV_SENSORS_H */
