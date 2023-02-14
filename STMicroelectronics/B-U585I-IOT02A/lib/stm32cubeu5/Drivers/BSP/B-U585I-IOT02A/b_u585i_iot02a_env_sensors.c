/**
  ******************************************************************************
  * @file    b_u585i_iot02a_env_sensors.c
  * @author  MCD Application Team
  * @brief   This file provides a set of functions needed to manage the
  *          environmental sensors mounted on the B_U585I_IOT02A board.
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
#include "b_u585i_iot02a_env_sensors.h"
#include "b_u585i_iot02a_bus.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup B_U585I_IOT02A
  * @{
  */

/** @defgroup B_U585I_IOT02A_ENV_SENSORS ENVIRONMENTAL_SENSORS
  * @{
  */

/** @defgroup B_U585I_IOT02A_ENV_SENSORS_Private_Defines ENVIRONMENTAL_SENSORS Private Defines
  * @{
  */
/* Functions IDx */
#define TEMPERATURE_ID  0U
#define PRESSURE_ID     1U
#define HUMIDITY_ID     2U
/**
  * @}
  */

/** @defgroup B_U585I_IOT02A_ENV_SENSORS_Exported_Variables ENVIRONMENTAL_SENSORS Exported Variables
  * @{
  */
/* Environmental sensor context */
ENV_SENSOR_Ctx_t Env_Sensor_Ctx[ENV_SENSOR_INSTANCES_NBR] = {{0}, {0}};

/* Environmental sensor component object */
void *Env_Sensor_CompObj[ENV_SENSOR_INSTANCES_NBR] = {0, 0};

/* Environmental sensor common driver */
ENV_SENSOR_CommonDrv_t *Env_Sensor_Drv[ENV_SENSOR_INSTANCES_NBR] = {0, 0};

/* Environmental sensor function driver */
ENV_SENSOR_FuncDrv_t *Env_Sensor_FuncDrv[ENV_SENSOR_INSTANCES_NBR][ENV_SENSOR_FUNCTIONS_NBR] = {{0, 0, 0},
  {0, 0, 0}
};
/**
  * @}
  */

/** @defgroup B_U585I_IOT02A_ENV_SENSORS_Private_Function_Prototypes ENVIRONMENTAL_SENSORS Private Function Prototypes
  * @{
  */
static int32_t HTS221_Probe(uint32_t Functions);
static int32_t LPS22HH_Probe(uint32_t Functions);
/**
  * @}
  */

/** @defgroup B_U585I_IOT02A_ENV_SENSORS_Exported_Functions ENVIRONMENTAL_SENSORS Exported Functions
  * @{
  */

/**
  * @brief  Initialize the environmental sensor.
  * @param  Instance Environmental sensor instance.
  * @param  Functions Environmental sensor functions. Could be :
  *         - ENV_TEMPERATURE and/or ENV_HUMIDITY for instance 0
  *         - ENV_TEMPERATURE and/or ENV_PRESSURE for instance 1
  * @retval BSP status.
  */
int32_t BSP_ENV_SENSOR_Init(uint32_t Instance, uint32_t Functions)
{
  int32_t status = BSP_ERROR_NONE;

  if (Instance >= ENV_SENSOR_INSTANCES_NBR)
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  else if ((Instance == 0U) && ((Functions & ENV_PRESSURE) != 0U))
  {
    status = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else if ((Instance == 1U) && ((Functions & ENV_HUMIDITY) != 0U))
  {
    status = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else
  {
    /* Probe the motion sensor */
    if (Instance == 0U)
    {
      if (HTS221_Probe(Functions) != BSP_ERROR_NONE)
      {
        status = BSP_ERROR_COMPONENT_FAILURE;
      }
    }
    else /* Instance = 1 */
    {
      if (LPS22HH_Probe(Functions) != BSP_ERROR_NONE)
      {
        status = BSP_ERROR_COMPONENT_FAILURE;
      }
    }

    if (status == BSP_ERROR_NONE)
    {
      /* Store current initialized functions */
      Env_Sensor_Ctx[Instance].Functions |= Functions;
    }
  }
  return status;
}

/**
  * @brief  De-initialize environmental sensor.
  * @param  Instance Environmental sensor instance.
  * @retval BSP status.
  */
int32_t BSP_ENV_SENSOR_DeInit(uint32_t Instance)
{
  int32_t status = BSP_ERROR_NONE;

  if (Instance >= ENV_SENSOR_INSTANCES_NBR)
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  else if (Env_Sensor_Ctx[Instance].Functions != 0U)
  {
    /* De-initialize the sensor */
    if (Env_Sensor_Drv[Instance]->DeInit(Env_Sensor_CompObj[Instance]) < 0)
    {
      status = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      /* Reset current initialized functions */
      Env_Sensor_Ctx[Instance].Functions = 0U;
    }
  }
  else
  {
    /* Nothing to do (not initialized) */
  }
  return status;
}

/**
  * @brief  Get environmental sensor capabilities.
  * @param  Instance Environmental sensor instance.
  * @param  Capabilities Pointer to environmental sensor capabilities.
  * @retval BSP status.
  */
int32_t BSP_ENV_SENSOR_GetCapabilities(uint32_t Instance, ENV_SENSOR_Capabilities_t *Capabilities)
{
  int32_t status = BSP_ERROR_NONE;

  if ((Instance >= ENV_SENSOR_INSTANCES_NBR) || (Capabilities == NULL))
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  else if (Env_Sensor_Ctx[Instance].Functions == 0U)
  {
    status = BSP_ERROR_NO_INIT;
  }
  else
  {
    /* Get the sensor capabilities */
    if (Env_Sensor_Drv[Instance]->GetCapabilities(Env_Sensor_CompObj[Instance], Capabilities) < 0)
    {
      status = BSP_ERROR_COMPONENT_FAILURE;
    }
  }

  return status;
}

/**
  * @brief  Read environmental sensor ID.
  * @param  Instance Environmental sensor instance.
  * @param  Id Pointer to environmental sensor ID.
  * @retval BSP status.
  */
int32_t BSP_ENV_SENSOR_ReadID(uint32_t Instance, uint8_t *Id)
{
  int32_t status = BSP_ERROR_NONE;

  if ((Instance >= ENV_SENSOR_INSTANCES_NBR) || (Id == NULL))
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  else if (Env_Sensor_Ctx[Instance].Functions == 0U)
  {
    status = BSP_ERROR_NO_INIT;
  }
  else
  {
    /* Read the environmental sensor ID */
    if (Env_Sensor_Drv[Instance]->ReadID(Env_Sensor_CompObj[Instance], Id) < 0)
    {
      status = BSP_ERROR_COMPONENT_FAILURE;
    }
  }

  return status;
}

/**
  * @brief  Enable one environmental sensor function.
  * @param  Instance Environmental sensor instance.
  * @param  Function Environmental sensor function. Could be :
  *         - ENV_TEMPERATURE or ENV_HUMIDITY for instance 0
  *         - ENV_TEMPERATURE or ENV_PRESSURE for instance 1
  * @retval BSP status.
  */
int32_t BSP_ENV_SENSOR_Enable(uint32_t Instance, uint32_t Function)
{
  int32_t status = BSP_ERROR_NONE;
  uint8_t index;

  if (Instance >= ENV_SENSOR_INSTANCES_NBR)
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  else if ((Instance == 0U) && (Function == ENV_PRESSURE))
  {
    status = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else if ((Instance == 1U) && (Function == ENV_HUMIDITY))
  {
    status = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else if ((Env_Sensor_Ctx[Instance].Functions & Function) == 0U)
  {
    status = BSP_ERROR_NO_INIT;
  }
  else
  {
    /* Enable the environmental sensor function */
    index = (Function == ENV_TEMPERATURE) ? TEMPERATURE_ID : ((Function == ENV_PRESSURE) ? PRESSURE_ID : HUMIDITY_ID);
    if (Env_Sensor_FuncDrv[Instance][index]->Enable(Env_Sensor_CompObj[Instance]) < 0)
    {
      status = BSP_ERROR_COMPONENT_FAILURE;
    }
  }

  return status;
}

/**
  * @brief  Disable one environmental sensor function.
  * @param  Instance Environmental sensor instance.
  * @param  Function Environmental sensor function. Could be :
  *         - ENV_TEMPERATURE or ENV_HUMIDITY for instance 0
  *         - ENV_TEMPERATURE or ENV_PRESSURE for instance 1
  * @retval BSP status.
  */
int32_t BSP_ENV_SENSOR_Disable(uint32_t Instance, uint32_t Function)
{
  int32_t status = BSP_ERROR_NONE;
  uint8_t index;

  if (Instance >= ENV_SENSOR_INSTANCES_NBR)
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  else if ((Instance == 0U) && (Function == ENV_PRESSURE))
  {
    status = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else if ((Instance == 1U) && (Function == ENV_HUMIDITY))
  {
    status = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else if ((Env_Sensor_Ctx[Instance].Functions & Function) == 0U)
  {
    status = BSP_ERROR_NO_INIT;
  }
  else
  {
    /* Disable the environmental sensor function */
    index = (Function == ENV_TEMPERATURE) ? TEMPERATURE_ID : ((Function == ENV_PRESSURE) ? PRESSURE_ID : HUMIDITY_ID);
    if (Env_Sensor_FuncDrv[Instance][index]->Disable(Env_Sensor_CompObj[Instance]) < 0)
    {
      status = BSP_ERROR_COMPONENT_FAILURE;
    }
  }

  return status;
}

/**
  * @brief  Get output data rate from one environmental sensor function.
  * @param  Instance Environmental sensor instance.
  * @param  Function Environmental sensor function. Could be :
  *         - ENV_TEMPERATURE or ENV_HUMIDITY for instance 0
  *         - ENV_TEMPERATURE or ENV_PRESSURE for instance 1
  * @param  Odr Pointer to output data rate.
  * @retval BSP status.
  */
int32_t BSP_ENV_SENSOR_GetOutputDataRate(uint32_t Instance, uint32_t Function, float_t *Odr)
{
  int32_t status = BSP_ERROR_NONE;
  uint8_t index;

  if ((Instance >= ENV_SENSOR_INSTANCES_NBR) || (Odr == NULL))
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  else if ((Instance == 0U) && (Function == ENV_PRESSURE))
  {
    status = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else if ((Instance == 1U) && (Function == ENV_HUMIDITY))
  {
    status = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else if ((Env_Sensor_Ctx[Instance].Functions & Function) == 0U)
  {
    status = BSP_ERROR_NO_INIT;
  }
  else
  {
    /* Get the environmental sensor output data rate */
    index = (Function == ENV_TEMPERATURE) ? TEMPERATURE_ID : ((Function == ENV_PRESSURE) ? PRESSURE_ID : HUMIDITY_ID);
    if (Env_Sensor_FuncDrv[Instance][index]->GetOutputDataRate(Env_Sensor_CompObj[Instance], Odr) < 0)
    {
      status = BSP_ERROR_COMPONENT_FAILURE;
    }
  }

  return status;
}

/**
  * @brief  Set output data rate of one environmental sensor function.
  * @param  Instance Environmental sensor instance.
  * @param  Function Environmental sensor function. Could be :
  *         - ENV_TEMPERATURE or ENV_HUMIDITY for instance 0
  *         - ENV_TEMPERATURE or ENV_PRESSURE for instance 1
  * @param  Odr Output data rate.
  * @retval BSP status.
  */
int32_t BSP_ENV_SENSOR_SetOutputDataRate(uint32_t Instance, uint32_t Function, float_t Odr)
{
  int32_t status = BSP_ERROR_NONE;
  uint8_t index;

  if (Instance >= ENV_SENSOR_INSTANCES_NBR)
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  else if ((Instance == 0U) && (Function == ENV_PRESSURE))
  {
    status = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else if ((Instance == 1U) && (Function == ENV_HUMIDITY))
  {
    status = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else if ((Env_Sensor_Ctx[Instance].Functions & Function) == 0U)
  {
    status = BSP_ERROR_NO_INIT;
  }
  else
  {
    /* Set the environmental sensor output data rate */
    index = (Function == ENV_TEMPERATURE) ? TEMPERATURE_ID : ((Function == ENV_PRESSURE) ? PRESSURE_ID : HUMIDITY_ID);
    if (Env_Sensor_FuncDrv[Instance][index]->SetOutputDataRate(Env_Sensor_CompObj[Instance], Odr) < 0)
    {
      status = BSP_ERROR_COMPONENT_FAILURE;
    }
  }

  return status;
}

/**
  * @brief  Get environmental sensor value.
  * @param  Instance Environmental sensor instance.
  * @param  Function Environmental sensor function. Could be :
  *         - ENV_TEMPERATURE or ENV_HUMIDITY for instance 0
  *         - ENV_TEMPERATURE or ENV_PRESSURE for instance 1
  * @param  Value Pointer to environmental sensor value.
  * @retval BSP status.
  */
int32_t BSP_ENV_SENSOR_GetValue(uint32_t Instance, uint32_t Function, float_t *Value)
{
  int32_t status = BSP_ERROR_NONE;
  uint8_t index;

  if ((Instance >= ENV_SENSOR_INSTANCES_NBR) || (Value == NULL))
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  else if ((Instance == 0U) && (Function == ENV_PRESSURE))
  {
    status = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else if ((Instance == 1U) && (Function == ENV_HUMIDITY))
  {
    status = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else if ((Env_Sensor_Ctx[Instance].Functions & Function) == 0U)
  {
    status = BSP_ERROR_NO_INIT;
  }
  else
  {
    /* Get environmental sensor value */
    index = (Function == ENV_TEMPERATURE) ? TEMPERATURE_ID : ((Function == ENV_PRESSURE) ? PRESSURE_ID : HUMIDITY_ID);
    if (Env_Sensor_FuncDrv[Instance][index]->GetValue(Env_Sensor_CompObj[Instance], Value) < 0)
    {
      status = BSP_ERROR_COMPONENT_FAILURE;
    }
  }

  return status;
}
/**
  * @}
  */

/** @defgroup B_U585I_IOT02A_ENV_SENSORS_Private_Functions ENVIRONMENTAL_SENSORS Private Functions
  * @{
  */
/**
  * @brief  Probe the HTS221 environmental sensor driver.
  * @param  Functions Environmental sensor functions. Could be :
  *         - ENV_TEMPERATURE and/or ENV_HUMIDITY
  * @retval BSP status.
  */
static int32_t HTS221_Probe(uint32_t Functions)
{
  int32_t                status = BSP_ERROR_NONE;
  HTS221_IO_t            IOCtx;
  uint8_t                hts221_id;
  static HTS221_Object_t HTS221_Obj;

  /* Configure the environmental sensor driver */
  IOCtx.BusType     = HTS221_I2C_BUS;
  IOCtx.Address     = HTS221_I2C_ADDRESS;
  IOCtx.Init        = BSP_I2C2_Init;
  IOCtx.DeInit      = BSP_I2C2_DeInit;
  IOCtx.ReadReg     = BSP_I2C2_ReadReg;
  IOCtx.WriteReg    = BSP_I2C2_WriteReg;
  IOCtx.GetTick     = BSP_GetTick;

  /* Register Component Bus IO operations */
  if (HTS221_RegisterBusIO(&HTS221_Obj, &IOCtx) != HTS221_OK)
  {
    status = BSP_ERROR_BUS_FAILURE;
  }
  /* Read the sensor ID */
  else if (HTS221_ReadID(&HTS221_Obj, &hts221_id) != HTS221_OK)
  {
    status = BSP_ERROR_COMPONENT_FAILURE;
  }
  /* Check if the returned sensor ID is correct */
  else if (hts221_id != HTS221_ID)
  {
    status = BSP_ERROR_UNKNOWN_COMPONENT;
  }
  else
  {
    Env_Sensor_CompObj[0] = &HTS221_Obj;
    Env_Sensor_Drv[0]     = (ENV_SENSOR_CommonDrv_t *) &HTS221_COMMON_Driver;
    if (Env_Sensor_Drv[0]->Init(Env_Sensor_CompObj[0]) < 0)
    {
      status = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      if ((Functions & ENV_TEMPERATURE) != 0U)
      {
        Env_Sensor_FuncDrv[0][TEMPERATURE_ID] = (ENV_SENSOR_FuncDrv_t *) &HTS221_TEMP_Driver;
      }
      if ((Functions & ENV_HUMIDITY) != 0U)
      {
        Env_Sensor_FuncDrv[0][HUMIDITY_ID] = (ENV_SENSOR_FuncDrv_t *) &HTS221_HUM_Driver;
      }
    }
  }

  return status;
}

/**
  * @brief  Probe the LPS22HH environmental sensor driver.
  * @param  Functions Environmental sensor functions. Could be :
  *         - ENV_TEMPERATURE and/or ENV_PRESSURE
  * @retval BSP status.
  */
static int32_t LPS22HH_Probe(uint32_t Functions)
{
  int32_t                 status = BSP_ERROR_NONE;
  LPS22HH_IO_t            IOCtx;
  uint8_t                 lps22hh_id;
  static LPS22HH_Object_t LPS22HH_Obj;

  /* Configure the environmental sensor driver */
  IOCtx.BusType     = LPS22HH_I2C_BUS;
  IOCtx.Address     = LPS22HH_I2C_ADD_H;
  IOCtx.Init        = BSP_I2C2_Init;
  IOCtx.DeInit      = BSP_I2C2_DeInit;
  IOCtx.ReadReg     = BSP_I2C2_ReadReg;
  IOCtx.WriteReg    = BSP_I2C2_WriteReg;
  IOCtx.GetTick     = BSP_GetTick;

  /* Register Component Bus IO operations */
  if (LPS22HH_RegisterBusIO(&LPS22HH_Obj, &IOCtx) != LPS22HH_OK)
  {
    status = BSP_ERROR_BUS_FAILURE;
  }
  /* Read the sensor ID */
  else if (LPS22HH_ReadID(&LPS22HH_Obj, &lps22hh_id) != LPS22HH_OK)
  {
    status = BSP_ERROR_COMPONENT_FAILURE;
  }
  /* Check if the returned sensor ID is correct */
  else if (lps22hh_id != LPS22HH_ID)
  {
    status = BSP_ERROR_UNKNOWN_COMPONENT;
  }
  else
  {
    Env_Sensor_CompObj[1] = &LPS22HH_Obj;
    Env_Sensor_Drv[1]     = (ENV_SENSOR_CommonDrv_t *) &LPS22HH_COMMON_Driver;
    if (Env_Sensor_Drv[1]->Init(Env_Sensor_CompObj[1]) < 0)
    {
      status = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      if ((Functions & ENV_TEMPERATURE) != 0U)
      {
        Env_Sensor_FuncDrv[1][TEMPERATURE_ID] = (ENV_SENSOR_FuncDrv_t *) &LPS22HH_TEMP_Driver;
      }
      if ((Functions & ENV_PRESSURE) != 0U)
      {
        Env_Sensor_FuncDrv[1][PRESSURE_ID] = (ENV_SENSOR_FuncDrv_t *) &LPS22HH_PRESS_Driver;
      }
    }
  }

  return status;
}
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
