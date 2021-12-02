/**
  ******************************************************************************
  * @file    stm32l475e_iot01_gyro.c
  * @author  MCD Application Team
  * @brief   This file provides a set of functions needed to manage the gyroscope sensor
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
#include "stm32l475e_iot01_gyro.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32L475E_IOT01
  * @{
  */
      
/** @defgroup STM32L475E_IOT01_GYROSCOPE GYROSCOPE
  * @{
  */ 

/** @defgroup STM32L475E_IOT01_GYROSCOPE_Private_Variables GYROSCOPE Private Variables
  * @{
  */
static GYRO_DrvTypeDef *GyroscopeDrv;

/**
  * @}
  */


/** @defgroup STM32L475E_IOT01_GYROSCOPE_Private_Functions GYROSCOPE Private Functions
  * @{
  */ 
/**
  * @brief  Initialize Gyroscope.
  * @retval GYRO_OK or GYRO_ERROR
  */
uint8_t BSP_GYRO_Init(void)
{  
  uint8_t ret = GYRO_ERROR;
  uint16_t ctrl = 0x0000;
  GYRO_InitTypeDef LSM6DSL_InitStructure;

  if(Lsm6dslGyroDrv.ReadID() != LSM6DSL_ACC_GYRO_WHO_AM_I)
  {
    ret = GYRO_ERROR;
  }
  else
  {
    /* Initialize the gyroscope driver structure */
    GyroscopeDrv = &Lsm6dslGyroDrv;

    /* Configure Mems : data rate, power mode, full scale and axes */
    LSM6DSL_InitStructure.Power_Mode = 0;
    LSM6DSL_InitStructure.Output_DataRate = LSM6DSL_ODR_52Hz;
    LSM6DSL_InitStructure.Axes_Enable = 0;
    LSM6DSL_InitStructure.Band_Width = 0;
    LSM6DSL_InitStructure.BlockData_Update = LSM6DSL_BDU_BLOCK_UPDATE;
    LSM6DSL_InitStructure.Endianness = 0;
    LSM6DSL_InitStructure.Full_Scale = LSM6DSL_GYRO_FS_2000; 

    /* Configure MEMS: data rate, full scale  */
    ctrl = (LSM6DSL_InitStructure.Full_Scale | LSM6DSL_InitStructure.Output_DataRate);

    /* Configure MEMS: BDU and Auto-increment for multi read/write */
    ctrl |= ((LSM6DSL_InitStructure.BlockData_Update | LSM6DSL_ACC_GYRO_IF_INC_ENABLED) << 8);

    /* Initialize component */
    GyroscopeDrv->Init(ctrl);
    
    ret = GYRO_OK;
  }
  
  return ret;
}


/**
  * @brief  DeInitialize Gyroscope.
  */
void BSP_GYRO_DeInit(void)
{
  /* DeInitialize the Gyroscope IO interfaces */
  if(GyroscopeDrv != NULL)
  {
    if(GyroscopeDrv->DeInit!= NULL)
    {
      GyroscopeDrv->DeInit();
    }
  }
}


/**
  * @brief  Set/Unset Gyroscope in low power mode.
  * @param  status 0 means disable Low Power Mode, otherwise Low Power Mode is enabled
  */
void BSP_GYRO_LowPower(uint16_t status)
{
  /* Set/Unset component in low-power mode */
  if(GyroscopeDrv != NULL)
  {
    if(GyroscopeDrv->LowPower!= NULL)
    {
      GyroscopeDrv->LowPower(status);
    }
  }
}

/**
  * @brief  Get XYZ angular acceleration from the Gyroscope.
  * @param  pfData: pointer on floating array         
  */
void BSP_GYRO_GetXYZ(float* pfData)
{
  if(GyroscopeDrv != NULL)
  {
    if(GyroscopeDrv->GetXYZ!= NULL)
    {
      GyroscopeDrv->GetXYZ(pfData);
    }
  }
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
