/**
  ******************************************************************************
  * @file    stm32l475e_iot01_hsensor.c
  * @author  MCD Application Team
  * @brief   This file provides a set of functions needed to manage the humidity sensor
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
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
#include "stm32l475e_iot01_hsensor.h"

/** @addtogroup BSP
  * @{
  */ 

/** @addtogroup STM32L475E_IOT01
  * @{
  */

/** @defgroup STM32L475E_IOT01_HUMIDITY HUMIDITY
  * @{
  */

/** @defgroup STM32L475E_IOT01_HUMIDITY_Private_Variables HUMIDITY Private Variables 
  * @{
  */ 
static HSENSOR_DrvTypeDef *Hsensor_drv;  
/**
  * @}
  */

/** @defgroup STM32L475E_IOT01_HUMIDITY_Private_Functions HUMIDITY Private Functions
  * @{
  */ 

/**
  * @brief  Initializes peripherals used by the I2C Humidity Sensor driver.
  * @retval HSENSOR status
  */
uint32_t BSP_HSENSOR_Init(void)
{
  uint32_t ret;
  
  if(HTS221_H_Drv.ReadID(HTS221_I2C_ADDRESS) != HTS221_WHO_AM_I_VAL)
  {
    ret = HSENSOR_ERROR;
  }
  else
  {
    Hsensor_drv = &HTS221_H_Drv;
    /* HSENSOR Init */   
    Hsensor_drv->Init(HTS221_I2C_ADDRESS);
    ret = HSENSOR_OK;
  }
  
  return ret;
}

/**
  * @brief  Read ID of HTS221.
  * @retval HTS221 ID value.
  */
uint8_t BSP_HSENSOR_ReadID(void)
{ 
  return Hsensor_drv->ReadID(HTS221_I2C_ADDRESS);
}

/**
  * @brief  Read Humidity register of HTS221.
  * @retval HTS221 measured humidity value.
  */
float BSP_HSENSOR_ReadHumidity(void)
{ 
  return Hsensor_drv->ReadHumidity(HTS221_I2C_ADDRESS);
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
