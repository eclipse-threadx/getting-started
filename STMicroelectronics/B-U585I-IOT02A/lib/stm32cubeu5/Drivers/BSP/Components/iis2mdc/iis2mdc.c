/**
 ******************************************************************************
 * @file    iis2mdc.c
 * @author  MEMS Software Solutions Team
 * @brief   IIS2MDC driver file
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
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
#include "iis2mdc.h"

/** @addtogroup BSP BSP
 * @{
 */

/** @addtogroup Component Component
 * @{
 */

/** @defgroup IIS2MDC IIS2MDC
 * @{
 */

/** @defgroup IIS2MDC_Exported_Variables IIS2MDC Exported Variables
 * @{
 */

IIS2MDC_CommonDrv_t IIS2MDC_COMMON_Driver =
{
  IIS2MDC_Init,
  IIS2MDC_DeInit,
  IIS2MDC_ReadID,
  IIS2MDC_GetCapabilities,
};

IIS2MDC_MAG_Drv_t IIS2MDC_MAG_Driver =
{
  IIS2MDC_MAG_Enable,
  IIS2MDC_MAG_Disable,
  IIS2MDC_MAG_GetSensitivity,
  IIS2MDC_MAG_GetOutputDataRate,
  IIS2MDC_MAG_SetOutputDataRate,
  IIS2MDC_MAG_GetFullScale,
  IIS2MDC_MAG_SetFullScale,
  IIS2MDC_MAG_GetAxes,
  IIS2MDC_MAG_GetAxesRaw,
};

/**
 * @}
 */

/** @defgroup IIS2MDC_Private_Function_Prototypes IIS2MDC Private Function Prototypes
 * @{
 */

static int32_t ReadMagRegWrap(void *Handle, uint8_t Reg, uint8_t *pData, uint16_t Length);
static int32_t WriteMagRegWrap(void *Handle, uint8_t Reg, uint8_t *pData, uint16_t Length);

/**
 * @}
 */

/** @defgroup IIS2MDC_Exported_Functions IIS2MDC Exported Functions
 * @{
 */

/**
 * @brief  Register Component Bus IO operations
 * @param  pObj the device pObj
 * @retval 0 in case of success, an error code otherwise
 */
int32_t IIS2MDC_RegisterBusIO(IIS2MDC_Object_t *pObj, IIS2MDC_IO_t *pIO)
{
  int32_t ret = IIS2MDC_OK;

  if (pObj == NULL)
  {
    ret = IIS2MDC_ERROR;
  }
  else
  {
    pObj->IO.Init      = pIO->Init;
    pObj->IO.DeInit    = pIO->DeInit;
    pObj->IO.BusType   = pIO->BusType;
    pObj->IO.Address   = pIO->Address;
    pObj->IO.WriteReg  = pIO->WriteReg;
    pObj->IO.ReadReg   = pIO->ReadReg;
    pObj->IO.GetTick   = pIO->GetTick;

    pObj->Ctx.read_reg  = ReadMagRegWrap;
    pObj->Ctx.write_reg = WriteMagRegWrap;
    pObj->Ctx.handle    = pObj;

    if (pObj->IO.Init == NULL)
    {
      ret = IIS2MDC_ERROR;
    }
    else if (pObj->IO.Init() != IIS2MDC_OK)
    {
      ret = IIS2MDC_ERROR;
    }
    else
    {
      if (pObj->IO.BusType != IIS2MDC_I2C_BUS) /* If the bus type is not I2C */
      {
        /* Disable I2C interface support and enable eventually SPI 4-Wires only the first time */
        if (pObj->is_initialized == 0U)
        {
          if (pObj->IO.BusType == IIS2MDC_SPI_4WIRES_BUS) /* SPI 4-Wires */
          {
            /* Enable SPI 4-Wires and disable I2C support on the component */
            uint8_t data = 0x34;

            if (IIS2MDC_Write_Reg(pObj, IIS2MDC_CFG_REG_C, data) != IIS2MDC_OK)
            {
              ret = IIS2MDC_ERROR;
            }
          }
          else
          {
            /* Disable I2C interface on the component */
            if (iis2mdc_i2c_interface_set(&(pObj->Ctx), IIS2MDC_I2C_DISABLE) != IIS2MDC_OK)
            {
              ret = IIS2MDC_ERROR;
            }
          }
        }
      }
    }
  }

  return ret;
}

/**
 * @brief  Initialize the IIS2MDC sensor
 * @param  pObj the device pObj
 * @retval 0 in case of success, an error code otherwise
 */
int32_t IIS2MDC_Init(IIS2MDC_Object_t *pObj)
{
  /* Enable BDU */
  if (iis2mdc_block_data_update_set(&(pObj->Ctx), PROPERTY_ENABLE) != IIS2MDC_OK)
  {
    return IIS2MDC_ERROR;
  }

  /* Operating mode selection - power down */
  if (iis2mdc_operating_mode_set(&(pObj->Ctx), IIS2MDC_POWER_DOWN) != IIS2MDC_OK)
  {
    return IIS2MDC_ERROR;
  }

  /* Output data rate selection */
  if (iis2mdc_data_rate_set(&(pObj->Ctx), IIS2MDC_ODR_100Hz) != IIS2MDC_OK)
  {
    return IIS2MDC_ERROR;
  }

  /* Self Test disabled. */
  if (iis2mdc_self_test_set(&(pObj->Ctx), PROPERTY_DISABLE) != IIS2MDC_OK)
  {
    return IIS2MDC_ERROR;
  }

  pObj->is_initialized = 1;

  return IIS2MDC_OK;
}

/**
 * @brief  Deinitialize the IIS2MDC magnetometer sensor
 * @param  pObj the device pObj
 * @retval 0 in case of success, an error code otherwise
 */
int32_t IIS2MDC_DeInit(IIS2MDC_Object_t *pObj)
{
  /* Disable the component */
  if (IIS2MDC_MAG_Disable(pObj) != IIS2MDC_OK)
  {
    return IIS2MDC_ERROR;
  }

  pObj->is_initialized = 0;

  return IIS2MDC_OK;
}

/**
 * @brief  Read component ID
 * @param  pObj the device pObj
 * @param  Id the WHO_AM_I value
 * @retval 0 in case of success, an error code otherwise
 */
int32_t IIS2MDC_ReadID(IIS2MDC_Object_t *pObj, uint8_t *Id)
{
  if (iis2mdc_device_id_get(&(pObj->Ctx), Id) != IIS2MDC_OK)
  {
    return IIS2MDC_ERROR;
  }

  return IIS2MDC_OK;
}

/**
 * @brief  Get IIS2MDC magnetometer sensor capabilities
 * @param  pObj Component object pointer
 * @param  Capabilities pointer to IIS2MDC magnetometer sensor capabilities
 * @retval Component status
 */
int32_t IIS2MDC_GetCapabilities(IIS2MDC_Object_t *pObj, IIS2MDC_Capabilities_t *Capabilities)
{
  /* Prevent unused argument(s) compilation warning */
  (void)(pObj);

  Capabilities->Acc          = 0;
  Capabilities->Gyro         = 0;
  Capabilities->Magneto      = 1;
  Capabilities->LowPower     = 0;
  Capabilities->GyroMaxFS    = 0;
  Capabilities->AccMaxFS     = 0;
  Capabilities->MagMaxFS     = 50;
  Capabilities->GyroMaxOdr   = 0.0f;
  Capabilities->AccMaxOdr    = 0.0f;
  Capabilities->MagMaxOdr    = 100.0f;
  return IIS2MDC_OK;
}

/**
 * @brief Enable the IIS2MDC magnetometer sensor
 * @param pObj the device pObj
 * @retval 0 in case of success, an error code otherwise
 */
int32_t IIS2MDC_MAG_Enable(IIS2MDC_Object_t *pObj)
{
  /* Check if the component is already enabled */
  if (pObj->mag_is_enabled == 1U)
  {
    return IIS2MDC_OK;
  }

  /* Output data rate selection. */
  if (iis2mdc_operating_mode_set(&(pObj->Ctx), IIS2MDC_CONTINUOUS_MODE) != IIS2MDC_OK)
  {
    return IIS2MDC_ERROR;
  }

  pObj->mag_is_enabled = 1;

  return IIS2MDC_OK;
}

/**
 * @brief Disable the IIS2MDC magnetometer sensor
 * @param pObj the device pObj
 * @retval 0 in case of success, an error code otherwise
 */
int32_t IIS2MDC_MAG_Disable(IIS2MDC_Object_t *pObj)
{
  /* Check if the component is already disabled */
  if (pObj->mag_is_enabled == 0U)
  {
    return IIS2MDC_OK;
  }

  /* Output data rate selection - power down. */
  if (iis2mdc_operating_mode_set(&(pObj->Ctx), IIS2MDC_POWER_DOWN) != IIS2MDC_OK)
  {
    return IIS2MDC_ERROR;
  }

  pObj->mag_is_enabled = 0;

  return IIS2MDC_OK;
}

/**
 * @brief  Get the IIS2MDC magnetometer sensor sensitivity
 * @param  pObj the device pObj
 * @param  Sensitivity pointer
 * @retval 0 in case of success, an error code otherwise
 */
int32_t IIS2MDC_MAG_GetSensitivity(IIS2MDC_Object_t *pObj, float *Sensitivity)
{
  UNUSED(pObj);
  *Sensitivity = IIS2MDC_MAG_SENSITIVITY_FS_50GAUSS;

  return IIS2MDC_OK;
}

/**
 * @brief  Get the IIS2MDC magnetometer sensor output data rate
 * @param  pObj the device pObj
 * @param  Odr pointer where the output data rate is written
 * @retval 0 in case of success, an error code otherwise
 */
int32_t IIS2MDC_MAG_GetOutputDataRate(IIS2MDC_Object_t *pObj, float *Odr)
{
  int32_t ret = IIS2MDC_OK;
  iis2mdc_odr_t odr_low_level;

  /* Get current output data rate. */
  if (iis2mdc_data_rate_get(&(pObj->Ctx), &odr_low_level) != IIS2MDC_OK)
  {
    return IIS2MDC_ERROR;
  }

  switch (odr_low_level)
  {
    case IIS2MDC_ODR_10Hz:
      *Odr = 10.0f;
      break;

    case IIS2MDC_ODR_20Hz:
      *Odr = 20.0f;
      break;

    case IIS2MDC_ODR_50Hz:
      *Odr = 50.0f;
      break;

    case IIS2MDC_ODR_100Hz:
      *Odr = 100.0f;
      break;

    default:
      ret = IIS2MDC_ERROR;
      break;
  }

  return ret;
}

/**
 * @brief  Set the IIS2MDC magnetometer sensor output data rate
 * @param  pObj the device pObj
 * @param  Odr the output data rate value to be set
 * @retval 0 in case of success, an error code otherwise
 */
int32_t IIS2MDC_MAG_SetOutputDataRate(IIS2MDC_Object_t *pObj, float Odr)
{
  iis2mdc_odr_t new_odr;

  new_odr = (Odr <= 10.000f) ? IIS2MDC_ODR_10Hz
            : (Odr <= 20.000f) ? IIS2MDC_ODR_20Hz
            : (Odr <= 50.000f) ? IIS2MDC_ODR_50Hz
            :                    IIS2MDC_ODR_100Hz;

  if (iis2mdc_data_rate_set(&(pObj->Ctx), new_odr) != IIS2MDC_OK)
  {
    return IIS2MDC_ERROR;
  }

  return IIS2MDC_OK;
}


/**
 * @brief  Get the IIS2MDC magnetometer sensor full scale
 * @param  pObj the device pObj
 * @param  FullScale pointer where the full scale is written
 * @retval 0 in case of success, an error code otherwise
 */
int32_t IIS2MDC_MAG_GetFullScale(IIS2MDC_Object_t *pObj, int32_t *FullScale)
{
  UNUSED(pObj);
  *FullScale = 50;

  return IIS2MDC_OK;
}

/**
 * @brief  Set the IIS2MDC magnetometer sensor full scale
 * @param  pObj the device pObj
 * @param  FullScale the functional full scale to be set
 * @retval 0 in case of success, an error code otherwise
 */
int32_t IIS2MDC_MAG_SetFullScale(IIS2MDC_Object_t *pObj, int32_t FullScale)
{
  UNUSED(pObj);
  UNUSED(FullScale);
  return IIS2MDC_OK;
}

/**
 * @brief  Get the IIS2MDC magnetometer sensor raw axes
 * @param  pObj the device pObj
 * @param  Value pointer where the raw values of the axes are written
 * @retval 0 in case of success, an error code otherwise
 */
int32_t IIS2MDC_MAG_GetAxesRaw(IIS2MDC_Object_t *pObj, IIS2MDC_AxesRaw_t *Value)
{
  iis2mdc_axis3bit16_t data_raw;

  /* Read raw data values. */
  if (iis2mdc_magnetic_raw_get(&(pObj->Ctx), data_raw.i16bit) != IIS2MDC_OK)
  {
    return IIS2MDC_ERROR;
  }

  /* Format the data. */
  Value->x = data_raw.i16bit[0];
  Value->y = data_raw.i16bit[1];
  Value->z = data_raw.i16bit[2];

  return IIS2MDC_OK;
}

/**
 * @brief  Get the IIS2MDC magnetometer sensor axes
 * @param  pObj the device pObj
 * @param  MagneticField pointer where the values of the axes are written
 * @retval 0 in case of success, an error code otherwise
 */
int32_t IIS2MDC_MAG_GetAxes(IIS2MDC_Object_t *pObj, IIS2MDC_Axes_t *MagneticField)
{
  iis2mdc_axis3bit16_t data_raw;
  float sensitivity;

  /* Read raw data values. */
  if (iis2mdc_magnetic_raw_get(&(pObj->Ctx), data_raw.i16bit) != IIS2MDC_OK)
  {
    return IIS2MDC_ERROR;
  }

  /* Get IIS2MDC actual sensitivity. */
  (void)IIS2MDC_MAG_GetSensitivity(pObj, &sensitivity);

  /* Calculate the data. */
  MagneticField->x = (int32_t)((float)((float)data_raw.i16bit[0] * sensitivity));
  MagneticField->y = (int32_t)((float)((float)data_raw.i16bit[1] * sensitivity));
  MagneticField->z = (int32_t)((float)((float)data_raw.i16bit[2] * sensitivity));

  return IIS2MDC_OK;
}

/**
 * @brief  Get the IIS2MDC register value for magnetic sensor
 * @param  pObj the device pObj
 * @param  Reg address to be read
 * @param  Data pointer where the value is written
 * @retval 0 in case of success, an error code otherwise
 */
int32_t IIS2MDC_Read_Reg(IIS2MDC_Object_t *pObj, uint8_t Reg, uint8_t *Data)
{
  if (iis2mdc_read_reg(&(pObj->Ctx), Reg, Data, 1) != IIS2MDC_OK)
  {
    return IIS2MDC_ERROR;
  }

  return IIS2MDC_OK;
}

/**
 * @brief  Set the IIS2MDC register value for magnetic sensor
 * @param  pObj the device pObj
 * @param  Reg address to be written
 * @param  Data value to be written
 * @retval 0 in case of success, an error code otherwise
 */
int32_t IIS2MDC_Write_Reg(IIS2MDC_Object_t *pObj, uint8_t Reg, uint8_t Data)
{
  if (iis2mdc_write_reg(&(pObj->Ctx), Reg, &Data, 1) != IIS2MDC_OK)
  {
    return IIS2MDC_ERROR;
  }

  return IIS2MDC_OK;
}

/**
 * @brief  Get the IIS2MDC MAG data ready bit value
 * @param  pObj the device pObj
 * @param  Status the status of data ready bit
 * @retval 0 in case of success, an error code otherwise
 */
int32_t IIS2MDC_MAG_Get_DRDY_Status(IIS2MDC_Object_t *pObj, uint8_t *Status)
{
  if (iis2mdc_mag_data_ready_get(&(pObj->Ctx), Status) != IIS2MDC_OK)
  {
    return IIS2MDC_ERROR;
  }

  return IIS2MDC_OK;
}

/**
 * @brief  Get the IIS2MDC MAG initialization status
 * @param  pObj the device pObj
 * @param  Status 1 if initialized, 0 otherwise
 * @retval 0 in case of success, an error code otherwise
 */
int32_t IIS2MDC_MAG_Get_Init_Status(IIS2MDC_Object_t *pObj, uint8_t *Status)
{
  if (pObj == NULL)
  {
    return IIS2MDC_ERROR;
  }

  *Status = pObj->is_initialized;

  return IIS2MDC_OK;
}

/**
 * @}
 */

/** @defgroup IIS2MDC_Private_Functions IIS2MDC Private Functions
 * @{
 */

/**
 * @brief  Wrap Read register component function to Bus IO function
 * @param  Handle the device handler
 * @param  Reg the register address
 * @param  pData the stored data pointer
 * @param  Length the length
 * @retval 0 in case of success, an error code otherwise
 */
static int32_t ReadMagRegWrap(void *Handle, uint8_t Reg, uint8_t *pData, uint16_t Length)
{
  IIS2MDC_Object_t *pObj = (IIS2MDC_Object_t *)Handle;

  if (pObj->IO.BusType == IIS2MDC_I2C_BUS) /* I2C */
  {
    /* Enable Multi-byte read */
    return pObj->IO.ReadReg(pObj->IO.Address, (Reg | 0x80U), pData, Length);
  }
  else   /* SPI 3-Wires or SPI 4-Wires */
  {
    /* Enable Multi-byte read */
    return pObj->IO.ReadReg(pObj->IO.Address, (Reg | 0x40U), pData, Length);
  }
}

/**
 * @brief  Wrap Write register component function to Bus IO function
 * @param  Handle the device handler
 * @param  Reg the register address
 * @param  pData the stored data pointer
 * @param  Length the length
 * @retval 0 in case of success, an error code otherwise
 */
static int32_t WriteMagRegWrap(void *Handle, uint8_t Reg, uint8_t *pData, uint16_t Length)
{
  IIS2MDC_Object_t *pObj = (IIS2MDC_Object_t *)Handle;

  if (pObj->IO.BusType == IIS2MDC_I2C_BUS) /* I2C */
  {
    /* Enable Multi-byte write */
    return pObj->IO.WriteReg(pObj->IO.Address, (Reg | 0x80U), pData, Length);
  }
  else   /* SPI 3-Wires or SPI 4-Wires */
  {
    /* Enable Multi-byte write */
    return pObj->IO.WriteReg(pObj->IO.Address, (Reg | 0x40U), pData, Length);
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
