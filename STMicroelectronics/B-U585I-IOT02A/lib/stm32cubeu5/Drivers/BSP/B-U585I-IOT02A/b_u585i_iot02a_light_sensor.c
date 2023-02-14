/**
  ******************************************************************************
  * @file    b_u585i_iot02a_light_sensor.c
  * @author  MCD Application Team
  * @brief   This file provides a set of functions needed to manage the
  *          Ambient light sensor mounted on the B_U585I_IOT02A board.
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

/* Includes ------------------------------------------------------------------*/
#include "b_u585i_iot02a_light_sensor.h"
#include "b_u585i_iot02a_bus.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup B_U585I_IOT02A
  * @{
  */

/** @defgroup B_U585I_IOT02A_LIGHT_SENSORS LIGHT_SENSORS
  * @{
  */

/** @defgroup B_U585I_IOT02A_LIGHT_SENSORS_Private_Defines LIGHT_SENSORS Private Defines
  * @{
  */

/**
  * @}
  */

/** @defgroup B_U585I_IOT02A_LIGHT_SENSORS_Exported_Variables LIGHT_SENSORS Exported Variables
  * @{
  */
/* Ambiant Light sensor context */
LIGHT_SENSOR_Ctx_t LIGHT_SENSOR_Ctx[LIGHT_SENSOR_INSTANCES_NBR]={0};

void                *VEML6030_LIGHT_SENSOR_CompObj[LIGHT_SENSOR_INSTANCES_NBR] = {NULL};
static LIGHT_SENSOR_Drv_t *VEML6030_LIGHT_SENSOR_Drv[LIGHT_SENSOR_INSTANCES_NBR] = {NULL};
/**
  * @}
  */

/** @defgroup B_U585I_IOT02A_LIGHT_SENSORS_Private_Function_Prototypes LIGHT_SENSORS Private Function Prototypes
  * @{
  */
static int32_t VEML6030_Probe(uint32_t Instance);
/**
  * @}
  */

/** @defgroup B_U585I_IOT02A_LIGHT_SENSORS_Exported_Functions LIGHT_SENSORS Exported Functions
  * @{
  */

/**
  * @brief  Initializes and configures the Ambiant light Sensor and
  *         configures all necessary hardware resources (GPIOs, I2C, clocks..).
  * @param  Instance Ambiant Light Sensor instance. Could be only 0.
  * @retval BSP status
  */
int32_t BSP_LIGHT_SENSOR_Init(uint32_t Instance)
{
  int32_t status = BSP_ERROR_NONE;
  if (Instance >= LIGHT_SENSOR_INSTANCES_NBR)
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /*Probe VEML6030 */
    if(VEML6030_Probe(Instance)!= BSP_ERROR_NONE)
    {
      status = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      LIGHT_SENSOR_Ctx[Instance].Gain = VEML6030_CONF_GAIN_1 ;
      LIGHT_SENSOR_Ctx[Instance].ExposureTime = VEML6030_CONF_IT100;

    }
  }
  return status;
}

/**
  * @brief  De-Initializes the Ambiant light Sensor functionalities
  * @param  Instance Ambiant Light Sensor instance. Could be only 0.
  * @retval BSP status
  */
int32_t BSP_LIGHT_SENSOR_DeInit(uint32_t Instance)
{
  int32_t status = BSP_ERROR_NONE;
  if (Instance >= LIGHT_SENSOR_INSTANCES_NBR)
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* DeInitialize the component */
    if (VEML6030_LIGHT_SENSOR_Drv[Instance]->DeInit(VEML6030_LIGHT_SENSOR_CompObj[Instance]) < 0)
    {
      status = BSP_ERROR_COMPONENT_FAILURE;
    }
  }
  return status;
}

/**
  * @brief Read the light sensor device ID.
  * @param Instance    Light sensor instance.
  * @param pId    Pointer to the device ID.
  * @retval BSP status
  */
int32_t BSP_LIGHT_SENSOR_ReadID(uint32_t Instance, uint32_t *pId)
{
  int32_t ret;

  if (Instance >= LIGHT_SENSOR_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if (VEML6030_LIGHT_SENSOR_Drv[Instance]->ReadID(VEML6030_LIGHT_SENSOR_CompObj[Instance], pId) < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }

  return ret;
}

/**
  * @brief Get the light sensor capabilities.
  * @param Instance    Light sensor instance.
  * @param pCapabilities    Pointer to the light sensor capabilities.
  * @retval BSP status
  */
int32_t BSP_LIGHT_SENSOR_GetCapabilities(uint32_t Instance, LIGHT_SENSOR_Capabilities_t *pCapabilities)
{
  int32_t ret;

  if (Instance >= LIGHT_SENSOR_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if (VEML6030_LIGHT_SENSOR_Drv[Instance]->GetCapabilities(VEML6030_LIGHT_SENSOR_CompObj[Instance], pCapabilities) < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }

  return ret;
}
/**
  * @brief  Set the ambiant light integration gain.
  * @param   Instance ambiant light sensor instance. Could be only 0.
  * @param  Gain ambiant light integration gain to be configured
  * @retval BSP status
  */
int32_t BSP_LIGHT_SENSOR_SetGain(uint32_t Instance, uint8_t Channel, uint32_t Gain)
{
  int32_t status = BSP_ERROR_NONE;

  if (Instance >= LIGHT_SENSOR_INSTANCES_NBR)
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Set the Gain Value */
    if (VEML6030_LIGHT_SENSOR_Drv[Instance]->SetGain(VEML6030_LIGHT_SENSOR_CompObj[Instance],Channel,Gain) < 0)
    {
      status = BSP_ERROR_COMPONENT_FAILURE;
    }
    LIGHT_SENSOR_Ctx[Instance].Gain = Gain;
  }
  return status ;
}

/**
  * @brief  Returns the gain of the ambiant light sensor.
  * @param  Instance ambiant light sensor instance. Could be only 0.
  * @param  Gain  Pointer to ingration time
  * @retval BSP status
  */
int32_t BSP_LIGHT_SENSOR_GetGain(uint32_t Instance, uint8_t Channel, uint32_t *pGain)
{
  int32_t status = BSP_ERROR_NONE;

  if (Instance >= LIGHT_SENSOR_INSTANCES_NBR)
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Get the Gain Value */
    if (VEML6030_LIGHT_SENSOR_Drv[Instance]->GetGain(VEML6030_LIGHT_SENSOR_CompObj[Instance],Channel,pGain) < 0)
    {
      status = BSP_ERROR_COMPONENT_FAILURE;
    }
  }
  return status ;
}


/**
  * @brief Set the exposure time.
  * @param Instance       Light sensor instance.
  * @param ExposureTime    New exposure time (the integration time) to be applied.
  * @warning This function must not be called when a capture is ongoing.
  * @retval BSP status
  */
int32_t BSP_LIGHT_SENSOR_SetExposureTime(uint32_t Instance, uint32_t ExposureTime)
{
  int32_t ret;

  if (Instance >= LIGHT_SENSOR_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if (VEML6030_LIGHT_SENSOR_Drv[Instance]->SetExposureTime(VEML6030_LIGHT_SENSOR_CompObj[Instance], ExposureTime) < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    LIGHT_SENSOR_Ctx[Instance].ExposureTime = ExposureTime ;
    ret = BSP_ERROR_NONE;
  }

  return ret;
}

/**
  * @brief Get the exposure time.
  * @param Instance    Light sensor instance.
  * @param pExposureTime    Pointer to the current exposure time value.
  * @retval BSP status
  */
int32_t BSP_LIGHT_SENSOR_GetExposureTime(uint32_t Instance, uint32_t *pExposureTime)
{
  int32_t ret;

  if (Instance >= LIGHT_SENSOR_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if (VEML6030_LIGHT_SENSOR_Drv[Instance]->GetExposureTime(VEML6030_LIGHT_SENSOR_CompObj[Instance], pExposureTime) < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }

  return ret;
}

/**
  * @brief Set the inter-measurement time.
  * @param Instance    Light sensor instance.
  * @param InterMeasurementTime    Inter-measurement to be applied.
  * @note The InterMeasurementTime is the refresh time determined by Power Saving Mode (PSM) and the Integration Time (ALS_IT).
  * @note This should be configured only when using the device in continuous mode.
  * @warning This function must not be called when a capture is ongoing.
  * @retval BSP status
  */
int32_t BSP_LIGHT_SENSOR_SetInterMeasurementTime(uint32_t Instance, uint32_t InterMeasurementTime)
{
  int32_t ret;

  if (Instance >= LIGHT_SENSOR_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if (VEML6030_LIGHT_SENSOR_Drv[Instance]->SetInterMeasurementTime(VEML6030_LIGHT_SENSOR_CompObj[Instance], InterMeasurementTime) < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }

  return ret;
}

/**
  * @brief Get the inter-measurement time.
  * @param Instance    Light sensor instance.
  * @param pInterMeasurementTime    Pointer to the current inter-measurement time.
  * @retval BSP status
  */
int32_t BSP_LIGHT_SENSOR_GetInterMeasurementTime(uint32_t Instance, uint32_t *pInterMeasurementTime)
{
  int32_t ret;

  if (Instance >= LIGHT_SENSOR_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if (VEML6030_LIGHT_SENSOR_Drv[Instance]->GetInterMeasurementTime(VEML6030_LIGHT_SENSOR_CompObj[Instance], pInterMeasurementTime) < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }

  return ret;
}

/**
  * @brief Start the light measurement on all channels.
  * @param Instance    Light sensor instance.
  * @param Mode    Measurement mode (continuous mode)
  * @retval BSP status
  */
int32_t BSP_LIGHT_SENSOR_Start(uint32_t Instance, uint8_t Mode)
{
  int32_t ret;

  if (Instance >= LIGHT_SENSOR_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if (VEML6030_LIGHT_SENSOR_Drv[Instance]->Start(VEML6030_LIGHT_SENSOR_CompObj[Instance], Mode) < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }

  return ret;
}

/**
  * @brief Stop the measurement on all channels.
  * @param Instance    Light sensor instance.
  * @retval BSP status
  */
int32_t BSP_LIGHT_SENSOR_Stop(uint32_t Instance)
{
  int32_t ret;

  if (Instance >= LIGHT_SENSOR_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if (VEML6030_LIGHT_SENSOR_Drv[Instance]->Stop(VEML6030_LIGHT_SENSOR_CompObj[Instance]) < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }

  return ret;
}

/**
  * @brief Start flicker capture.
  * @param Instance    Light sensor instance.
  * @param Channel    The channel that will be used for flicker detection.
  * @param OutputMode    Analog or Digital depending on the hardware configuration.
  * @note The application must call BSP_LIGHT_SENSOR_Start before calling this function.
  * @warning The flicker can be started only on one channel at a time.
  * @note Calling this function will enable ALS capture on all the other channels.
  * @retval BSP status
  */
int32_t BSP_LIGHT_SENSOR_StartFlicker(uint32_t Instance, uint8_t Channel, uint8_t OutputMode)
{
  int32_t ret;

  if (Instance >= LIGHT_SENSOR_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if (VEML6030_LIGHT_SENSOR_Drv[Instance]->StartFlicker(VEML6030_LIGHT_SENSOR_CompObj[Instance], Channel, OutputMode) < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }

  return ret;
}

/**
  * @brief Stop flicker capture.
  * @param Instance     Light sensor instance.
  * @retval BSP status
  */
int32_t BSP_LIGHT_SENSOR_StopFlicker(uint32_t Instance)
{
  int32_t ret;

  if (Instance >= LIGHT_SENSOR_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if (VEML6030_LIGHT_SENSOR_Drv[Instance]->StopFlicker(VEML6030_LIGHT_SENSOR_CompObj[Instance]) < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }

  return ret;
}

/**
  * @brief Returns the measurement values for all the channels.
  * @param Instance    Light sensor instance.
  * @param pResult    Pointer to an array which will be filled with the values of each channel.
  * @note  pResult[0] for ALS channel value, pResult[1] for white channel value.
  * @note The array size must match the number of channels of the device.
  * @retval BSP status
  */
int32_t BSP_LIGHT_SENSOR_GetValues(uint32_t Instance, uint32_t *pResult)
{
  int32_t ret;

  if (Instance >= LIGHT_SENSOR_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if (VEML6030_LIGHT_SENSOR_Drv[Instance]->GetValues(VEML6030_LIGHT_SENSOR_CompObj[Instance], pResult) < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }

  return ret;
}

/**
  * @brief Enable and disable control features.
  * @param Instance       Light sensor instance.
  * @param ControlMode    Feature to be be enabled or disabled.
  * @param Value    Value to be applied.
  * @warning This function must not be called when a capture is ongoing.
  * @retval BSP status
  */
int32_t BSP_LIGHT_SENSOR_SetControlMode(uint32_t Instance, uint32_t ControlMode, uint32_t Value)
{
  int32_t ret;

  if (Instance >= LIGHT_SENSOR_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if (VEML6030_LIGHT_SENSOR_Drv[Instance]->SetControlMode(VEML6030_LIGHT_SENSOR_CompObj[Instance], ControlMode, Value) < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }

  return ret;
}

/**
  * @}
  */

/** @defgroup B_U585I_IOT02A_LIGHT_SENSORS_Private_Functions LIGHT_SENSORS Private Functions
  * @{
  */
/**
  * @brief  Probe the VEML6030 motion sensor driver.
  * @retval BSP status.
  */
static int32_t VEML6030_Probe(uint32_t Instance)
{
  int32_t                 status = BSP_ERROR_NONE;
  VEML6030_IO_t            IOCtx;
  static VEML6030_Object_t VEML6030_Obj;

  /* Configure the motion sensor driver */
  IOCtx.ReadAddress   = VEML6030_I2C_READ_ADD;
  IOCtx.WriteAddress  = VEML6030_I2C_WRITE_ADD;
  IOCtx.Init          = BSP_I2C2_Init;
  IOCtx.DeInit        = BSP_I2C2_DeInit;
  IOCtx.IsReady       = BSP_I2C2_IsReady;
  IOCtx.ReadReg       = BSP_I2C2_ReadReg;
  IOCtx.WriteReg      = BSP_I2C2_WriteReg;
  IOCtx.GetTick       = BSP_GetTick;

  /* Register Component Bus IO operations */
  if (VEML6030_RegisterBusIO(&VEML6030_Obj, &IOCtx) != VEML6030_OK)
  {
    status = BSP_ERROR_BUS_FAILURE;
  }
  else if( HAL_I2C_IsDeviceReady(&hbus_i2c2,IOCtx.ReadAddress , 300, 2000)!=HAL_OK)
  {
    /* Component failure*/
    status = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    VEML6030_LIGHT_SENSOR_CompObj[Instance]=&VEML6030_Obj;
    VEML6030_LIGHT_SENSOR_Drv[Instance]=(LIGHT_SENSOR_Drv_t *)&VEML6030_Driver;

    /* Initialize the component */
    if (VEML6030_LIGHT_SENSOR_Drv[Instance]->Init(VEML6030_LIGHT_SENSOR_CompObj[Instance]) < 0)
    {
      status = BSP_ERROR_COMPONENT_FAILURE;
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/