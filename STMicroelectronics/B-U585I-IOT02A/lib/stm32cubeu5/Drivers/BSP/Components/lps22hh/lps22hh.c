/**
 ******************************************************************************
 * @file    lps22hh.c
 * @author  MEMS Software Solutions Team
 * @brief   LPS22HH driver file
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
#include "lps22hh.h"

/** @addtogroup BSP BSP
 * @{
 */

/** @addtogroup Component Component
 * @{
 */

/** @defgroup LPS22HH LPS22HH
 * @{
 */

/** @defgroup LPS22HH_Exported_Variables LPS22HH Exported Variables
 * @{
 */

LPS22HH_CommonDrv_t LPS22HH_COMMON_Driver =
{
  LPS22HH_Init,
  LPS22HH_DeInit,
  LPS22HH_ReadID,
  LPS22HH_GetCapabilities,
};

LPS22HH_PRESS_Drv_t LPS22HH_PRESS_Driver =
{
  LPS22HH_PRESS_Enable,
  LPS22HH_PRESS_Disable,
  LPS22HH_PRESS_GetOutputDataRate,
  LPS22HH_PRESS_SetOutputDataRate,
  LPS22HH_PRESS_GetPressure,
};

LPS22HH_TEMP_Drv_t LPS22HH_TEMP_Driver =
{
  LPS22HH_TEMP_Enable,
  LPS22HH_TEMP_Disable,
  LPS22HH_TEMP_GetOutputDataRate,
  LPS22HH_TEMP_SetOutputDataRate,
  LPS22HH_TEMP_GetTemperature,
};

/**
 * @}
 */

/** @defgroup LPS22HH_Private_Function_Prototypes LPS22HH Private Function Prototypes
 * @{
 */

static int32_t ReadRegWrap(void *Handle, uint8_t Reg, uint8_t *pData, uint16_t Length);
static int32_t WriteRegWrap(void *Handle, uint8_t Reg, uint8_t *pData, uint16_t Length);
static int32_t LPS22HH_GetOutputDataRate(LPS22HH_Object_t *pObj, float *Odr);
static int32_t LPS22HH_SetOutputDataRate_When_Enabled(LPS22HH_Object_t *pObj, float Odr);
static int32_t LPS22HH_SetOutputDataRate_When_Disabled(LPS22HH_Object_t *pObj, float Odr);
static int32_t LPS22HH_Initialize(LPS22HH_Object_t *pObj);

/**
 * @}
 */

/** @defgroup LPS22HH_Exported_Functions LPS22HH Exported Functions
 * @{
 */

/**
 * @brief  Register Component Bus IO operations
 * @param  pObj the device pObj
 * @retval 0 in case of success, an error code otherwise
 */
int32_t LPS22HH_RegisterBusIO(LPS22HH_Object_t *pObj, LPS22HH_IO_t *pIO)
{
  int32_t ret = LPS22HH_OK;

  if (pObj == NULL)
  {
    ret = LPS22HH_ERROR;
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

    pObj->Ctx.read_reg  = ReadRegWrap;
    pObj->Ctx.write_reg = WriteRegWrap;
    pObj->Ctx.handle   = pObj;

    if (pObj->IO.Init == NULL)
    {
      ret = LPS22HH_ERROR;
    }
    else if (pObj->IO.Init() != LPS22HH_OK)
    {
      ret = LPS22HH_ERROR;
    }
    else
    {
      if (pObj->IO.BusType == LPS22HH_SPI_3WIRES_BUS) /* SPI 3-Wires */
      {
        /* Enable the SPI 3-Wires support only the first time */
        if (pObj->is_initialized == 0U)
        {
          /* Enable SPI 3-Wires on the component */
          uint8_t data = 0x01;

          if (LPS22HH_Write_Reg(pObj, LPS22HH_CTRL_REG1, data) != LPS22HH_OK)
          {
            ret = LPS22HH_ERROR;
          }
        }
      }
    }
  }

  return ret;
}

/**
 * @brief  Initialize the LPS22HH sensor
 * @param  pObj the device pObj
 * @retval 0 in case of success, an error code otherwise
 */
int32_t LPS22HH_Init(LPS22HH_Object_t *pObj)
{
  if (pObj->is_initialized == 0U)
  {
    if (LPS22HH_Initialize(pObj) != LPS22HH_OK)
    {
      return LPS22HH_ERROR;
    }
  }

  pObj->is_initialized = 1U;

  return LPS22HH_OK;
}

/**
 * @brief  Deinitialize the LPS22HH sensor
 * @param  pObj the device pObj
 * @retval 0 in case of success, an error code otherwise
 */
int32_t LPS22HH_DeInit(LPS22HH_Object_t *pObj)
{
  if (pObj->is_initialized == 1U)
  {
    if (LPS22HH_PRESS_Disable(pObj) != LPS22HH_OK)
    {
      return LPS22HH_ERROR;
    }

    if (LPS22HH_TEMP_Disable(pObj) != LPS22HH_OK)
    {
      return LPS22HH_ERROR;
    }
  }

  pObj->is_initialized = 0;

  return LPS22HH_OK;
}

/**
 * @brief  Get WHO_AM_I value
 * @param  pObj the device pObj
 * @param  Id the WHO_AM_I value
 * @retval 0 in case of success, an error code otherwise
 */
int32_t LPS22HH_ReadID(LPS22HH_Object_t *pObj, uint8_t *Id)
{
  if (lps22hh_device_id_get(&(pObj->Ctx), Id) != LPS22HH_OK)
  {
    return LPS22HH_ERROR;
  }

  return LPS22HH_OK;
}

/**
 * @brief  Get LPS22HH sensor capabilities
 * @param  pObj Component object pointer
 * @param  Capabilities pointer to LPS22HH sensor capabilities
 * @retval 0 in case of success, an error code otherwise
 */
int32_t LPS22HH_GetCapabilities(LPS22HH_Object_t *pObj, LPS22HH_Capabilities_t *Capabilities)
{
  /* Prevent unused argument(s) compilation warning */
  (void)(pObj);

  Capabilities->Humidity    = 0;
  Capabilities->Pressure    = 1;
  Capabilities->Temperature = 1;
  Capabilities->LowPower    = 0;
  Capabilities->HumMaxOdr   = 0.0f;
  Capabilities->TempMaxOdr  = 200.0f;
  Capabilities->PressMaxOdr = 200.0f;
  return LPS22HH_OK;
}

/**
 * @brief  Get the LPS22HH initialization status
 * @param  pObj the device pObj
 * @param  Status 1 if initialized, 0 otherwise
 * @retval 0 in case of success, an error code otherwise
 */
int32_t LPS22HH_Get_Init_Status(LPS22HH_Object_t *pObj, uint8_t *Status)
{
  if (pObj == NULL)
  {
    return LPS22HH_ERROR;
  }

  *Status = pObj->is_initialized;

  return LPS22HH_OK;
}

/**
 * @brief  Enable the LPS22HH pressure sensor
 * @param  pObj the device pObj
 * @retval 0 in case of success, an error code otherwise
 */
int32_t LPS22HH_PRESS_Enable(LPS22HH_Object_t *pObj)
{
  /* Check if the component is already enabled */
  if (pObj->press_is_enabled == 1U)
  {
    return LPS22HH_OK;
  }

  /* Output data rate selection. */
  if (lps22hh_data_rate_set(&(pObj->Ctx), pObj->last_odr) != LPS22HH_OK)
  {
    return LPS22HH_ERROR;
  }

  pObj->press_is_enabled = 1;

  return LPS22HH_OK;
}

/**
 * @brief  Disable the LPS22HH pressure sensor
 * @param  pObj the device pObj
 * @retval 0 in case of success, an error code otherwise
 */
int32_t LPS22HH_PRESS_Disable(LPS22HH_Object_t *pObj)
{
  /* Check if the component is already disabled */
  if (pObj->press_is_enabled == 0U)
  {
    return LPS22HH_OK;
  }

  /* Check if the LPS22HH temperature sensor is still enable. */
  /* If yes, skip the disable function, if not call disable function */
  if (pObj->temp_is_enabled == 0U)
  {
    /* Get current output data rate. */
    if (lps22hh_data_rate_get(&(pObj->Ctx), &pObj->last_odr) != LPS22HH_OK)
    {
      return LPS22HH_ERROR;
    }

    /* Output data rate selection - power down. */
    if (lps22hh_data_rate_set(&(pObj->Ctx), LPS22HH_POWER_DOWN) != LPS22HH_OK)
    {
      return LPS22HH_ERROR;
    }
  }

  pObj->press_is_enabled = 0;

  return LPS22HH_OK;
}

/**
 * @brief  Get the LPS22HH pressure sensor output data rate
 * @param  pObj the device pObj
 * @param  Odr pointer where the output data rate is written
 * @retval 0 in case of success, an error code otherwise
 */
int32_t LPS22HH_PRESS_GetOutputDataRate(LPS22HH_Object_t *pObj, float *Odr)
{
  return LPS22HH_GetOutputDataRate(pObj, Odr);
}

/**
 * @brief  Set the LPS22HH pressure sensor output data rate
 * @param  pObj the device pObj
 * @param  Odr the output data rate value to be set
 * @retval 0 in case of success, an error code otherwise
 */
int32_t LPS22HH_PRESS_SetOutputDataRate(LPS22HH_Object_t *pObj, float Odr)
{
  /* Check if the component is enabled */
  if (pObj->press_is_enabled == 1U)
  {
    return LPS22HH_SetOutputDataRate_When_Enabled(pObj, Odr);
  }
  else
  {
    return LPS22HH_SetOutputDataRate_When_Disabled(pObj, Odr);
  }
}

/**
 * @brief  Get the LPS22HH pressure value
 * @param  pObj the device pObj
 * @param  Value pointer where the pressure value is written
 * @retval 0 in case of success, an error code otherwise
 */
int32_t LPS22HH_PRESS_GetPressure(LPS22HH_Object_t *pObj, float *Value)
{
  lps22hh_axis1bit32_t data_raw_pressure;

  (void)memset(data_raw_pressure.u8bit, 0x00, sizeof(int32_t));
  if (lps22hh_pressure_raw_get(&(pObj->Ctx), (uint32_t *)&data_raw_pressure.i32bit) != LPS22HH_OK)
  {
    return LPS22HH_ERROR;
  }

  *Value = lps22hh_from_lsb_to_hpa((uint32_t)data_raw_pressure.i32bit);

  return LPS22HH_OK;
}

/**
 * @brief  Get the LPS22HH pressure data ready bit value
 * @param  pObj the device pObj
 * @param  Status the status of data ready bit
 * @retval 0 in case of success, an error code otherwise
 */
int32_t LPS22HH_PRESS_Get_DRDY_Status(LPS22HH_Object_t *pObj, uint8_t *Status)
{
  if (lps22hh_press_flag_data_ready_get(&(pObj->Ctx), Status) != LPS22HH_OK)
  {
    return LPS22HH_ERROR;
  }

  return LPS22HH_OK;
}

/**
 * @brief  Enable the LPS22HH temperature sensor
 * @param  pObj the device pObj
 * @retval 0 in case of success, an error code otherwise
 */
int32_t LPS22HH_TEMP_Enable(LPS22HH_Object_t *pObj)
{
  /* Check if the component is already enabled */
  if (pObj->temp_is_enabled == 1U)
  {
    return LPS22HH_OK;
  }

  /* Output data rate selection. */
  if (lps22hh_data_rate_set(&(pObj->Ctx), pObj->last_odr) != LPS22HH_OK)
  {
    return LPS22HH_ERROR;
  }

  pObj->temp_is_enabled = 1;

  return LPS22HH_OK;
}

/**
 * @brief  Disable the LPS22HH temperature sensor
 * @param  pObj the device pObj
 * @retval 0 in case of success, an error code otherwise
 */
int32_t LPS22HH_TEMP_Disable(LPS22HH_Object_t *pObj)
{
  /* Check if the component is already disabled */
  if (pObj->temp_is_enabled == 0U)
  {
    return LPS22HH_OK;
  }

  /* Check if the LPS22HH pressure sensor is still enable. */
  /* If yes, skip the disable function, if not call disable function */
  if (pObj->press_is_enabled == 0U)
  {
    /* Get current output data rate. */
    if (lps22hh_data_rate_get(&(pObj->Ctx), &pObj->last_odr) != LPS22HH_OK)
    {
      return LPS22HH_ERROR;
    }

    /* Output data rate selection - power down. */
    if (lps22hh_data_rate_set(&(pObj->Ctx), LPS22HH_POWER_DOWN) != LPS22HH_OK)
    {
      return LPS22HH_ERROR;
    }
  }

  pObj->temp_is_enabled = 0;

  return LPS22HH_OK;
}

/**
 * @brief  Get the LPS22HH temperature sensor output data rate
 * @param  pObj the device pObj
 * @param  Odr pointer where the output data rate is written
 * @retval 0 in case of success, an error code otherwise
 */
int32_t LPS22HH_TEMP_GetOutputDataRate(LPS22HH_Object_t *pObj, float *Odr)
{
  return LPS22HH_GetOutputDataRate(pObj, Odr);
}

/**
 * @brief  Set the LPS22HH temperature sensor output data rate
 * @param  pObj the device pObj
 * @param  Odr the output data rate value to be set
 * @retval 0 in case of success, an error code otherwise
 */
int32_t LPS22HH_TEMP_SetOutputDataRate(LPS22HH_Object_t *pObj, float Odr)
{
  /* Check if the component is enabled */
  if (pObj->temp_is_enabled == 1U)
  {
    return LPS22HH_SetOutputDataRate_When_Enabled(pObj, Odr);
  }
  else
  {
    return LPS22HH_SetOutputDataRate_When_Disabled(pObj, Odr);
  }
}

/**
 * @brief  Get the LPS22HH temperature value
 * @param  pObj the device pObj
 * @param  Value pointer where the temperature value is written
 * @retval 0 in case of success, an error code otherwise
 */
int32_t LPS22HH_TEMP_GetTemperature(LPS22HH_Object_t *pObj, float *Value)
{
  lps22hh_axis1bit16_t data_raw_temperature;

  (void)memset(data_raw_temperature.u8bit, 0x00, sizeof(int16_t));
  if (lps22hh_temperature_raw_get(&(pObj->Ctx), &data_raw_temperature.i16bit) != LPS22HH_OK)
  {
    return LPS22HH_ERROR;
  }

  *Value = lps22hh_from_lsb_to_celsius(data_raw_temperature.i16bit);

  return LPS22HH_OK;
}

/**
 * @brief  Get the LPS22HH temperature data ready bit value
 * @param  pObj the device pObj
 * @param  Status the status of data ready bit
 * @retval 0 in case of success, an error code otherwise
 */
int32_t LPS22HH_TEMP_Get_DRDY_Status(LPS22HH_Object_t *pObj, uint8_t *Status)
{
  if (lps22hh_temp_flag_data_ready_get(&(pObj->Ctx), Status) != LPS22HH_OK)
  {
    return LPS22HH_ERROR;
  }

  return LPS22HH_OK;
}

/**
 * @brief  Get the LPS22HH register value
 * @param  pObj the device pObj
 * @param  Reg address to be written
 * @param  Data value to be written
 * @retval 0 in case of success, an error code otherwise
 */
int32_t LPS22HH_Read_Reg(LPS22HH_Object_t *pObj, uint8_t Reg, uint8_t *Data)
{
  if (lps22hh_read_reg(&(pObj->Ctx), Reg, Data, 1) != LPS22HH_OK)
  {
    return LPS22HH_ERROR;
  }

  return LPS22HH_OK;
}

/**
 * @brief  Set the LPS22HH register value
 * @param  pObj the device pObj
 * @param  Reg address to be written
 * @param  Data value to be written
 * @retval 0 in case of success, an error code otherwise
 */
int32_t LPS22HH_Write_Reg(LPS22HH_Object_t *pObj, uint8_t Reg, uint8_t Data)
{
  if (lps22hh_write_reg(&(pObj->Ctx), Reg, &Data, 1) != LPS22HH_OK)
  {
    return LPS22HH_ERROR;
  }

  return LPS22HH_OK;
}

/**
 * @}
 */

/** @defgroup LPS22HH_Private_Functions LPS22HH Private Functions
 * @{
 */

/**
 * @brief  Get the LPS22HH FIFO get temp data
 * @param  pObj the device pObj
 * @param  Status the status of data ready bit
 * @retval 0 in case of success, an error code otherwise
 */
int32_t LPS22HH_Get_Temp(LPS22HH_Object_t *pObj, float *Data)
{
  lps22hh_axis1bit16_t data_raw_temperature;

  (void)memset(data_raw_temperature.u8bit, 0x00, sizeof(int16_t));
  if (lps22hh_temperature_raw_get(&(pObj->Ctx), &data_raw_temperature.i16bit) != LPS22HH_OK)
  {
    return LPS22HH_ERROR;
  }

  *Data = ((float)data_raw_temperature.i16bit) / 100.0f;

  return LPS22HH_OK;
}

/**
 * @brief  Get output data rate
 * @param  pObj the device pObj
 * @param  Odr the output data rate value
 * @retval 0 in case of success, an error code otherwise
 */
static int32_t LPS22HH_GetOutputDataRate(LPS22HH_Object_t *pObj, float *Odr)
{
  int32_t ret = LPS22HH_OK;
  lps22hh_odr_t odr_low_level;

  if (lps22hh_data_rate_get(&(pObj->Ctx), &odr_low_level) != LPS22HH_OK)
  {
    return LPS22HH_ERROR;
  }

  switch (odr_low_level)
  {
    case LPS22HH_POWER_DOWN:
      *Odr = 0.0f;
      break;

    case LPS22HH_1_Hz:
      *Odr = 1.0f;
      break;

    case LPS22HH_10_Hz:
      *Odr = 10.0f;
      break;

    case LPS22HH_25_Hz:
      *Odr = 25.0f;
      break;

    case LPS22HH_50_Hz:
      *Odr = 50.0f;
      break;

    case LPS22HH_75_Hz:
      *Odr = 75.0f;
      break;

    case LPS22HH_100_Hz:
      *Odr = 100.0f;
      break;

    case LPS22HH_200_Hz:
      *Odr = 200.0f;
      break;

    default:
      ret = LPS22HH_ERROR;
      break;
  }

  return ret;
}

/**
 * @brief  Set output data rate
 * @param  pObj the device pObj
 * @param  Odr the output data rate value to be set
 * @retval 0 in case of success, an error code otherwise
 */
static int32_t LPS22HH_SetOutputDataRate_When_Enabled(LPS22HH_Object_t *pObj, float Odr)
{
  lps22hh_odr_t new_odr;

  new_odr = (Odr <=   1.0f) ? LPS22HH_1_Hz
          : (Odr <=  10.0f) ? LPS22HH_10_Hz
          : (Odr <=  25.0f) ? LPS22HH_25_Hz
          : (Odr <=  50.0f) ? LPS22HH_50_Hz
          : (Odr <=  75.0f) ? LPS22HH_75_Hz
          : (Odr <= 100.0f) ? LPS22HH_100_Hz
          :                   LPS22HH_200_Hz;

  if (lps22hh_data_rate_set(&(pObj->Ctx), new_odr) != LPS22HH_OK)
  {
    return LPS22HH_ERROR;
  }

  if (lps22hh_data_rate_get(&(pObj->Ctx), &pObj->last_odr) != LPS22HH_OK)
  {
    return LPS22HH_ERROR;
  }

  return LPS22HH_OK;
}

/**
 * @brief  Set output data rate when disabled
 * @param  pObj the device pObj
 * @param  Odr the output data rate value to be set
 * @retval 0 in case of success, an error code otherwise
 */
static int32_t LPS22HH_SetOutputDataRate_When_Disabled(LPS22HH_Object_t *pObj, float Odr)
{
  pObj->last_odr = (Odr <=   1.0f) ? LPS22HH_1_Hz
                 : (Odr <=  10.0f) ? LPS22HH_10_Hz
                 : (Odr <=  25.0f) ? LPS22HH_25_Hz
                 : (Odr <=  50.0f) ? LPS22HH_50_Hz
                 : (Odr <=  75.0f) ? LPS22HH_75_Hz
                 : (Odr <= 100.0f) ? LPS22HH_100_Hz
                 :                   LPS22HH_200_Hz;

  return LPS22HH_OK;
}

/**
 * @brief  Initialize the LPS22HH sensor
 * @param  pObj the device pObj
 * @retval 0 in case of success, an error code otherwise
 */
static int32_t LPS22HH_Initialize(LPS22HH_Object_t *pObj)
{
  /* Disable MIPI I3C(SM) interface */
  if (lps22hh_i3c_interface_set(&(pObj->Ctx), LPS22HH_I3C_DISABLE) != LPS22HH_OK)
  {
    return LPS22HH_ERROR;
  }

  /* Power down the device, set Low Noise Enable (bit 5), clear One Shot (bit 4) */
  if (lps22hh_data_rate_set(&(pObj->Ctx), (lps22hh_odr_t)(LPS22HH_POWER_DOWN | 0x10)) != LPS22HH_OK)
  {
    return LPS22HH_ERROR;
  }

  /* Disable low-pass filter on LPS22HH pressure data */
  if (lps22hh_lp_bandwidth_set(&(pObj->Ctx), LPS22HH_LPF_ODR_DIV_2) != LPS22HH_OK)
  {
    return LPS22HH_ERROR;
  }

  /* Set block data update mode */
  if (lps22hh_block_data_update_set(&(pObj->Ctx), PROPERTY_ENABLE) != LPS22HH_OK)
  {
    return LPS22HH_ERROR;
  }

  /* Set autoincrement for multi-byte read/write */
  if (lps22hh_auto_increment_set(&(pObj->Ctx), PROPERTY_ENABLE) != LPS22HH_OK)
  {
    return LPS22HH_ERROR;
  }

  pObj->last_odr = LPS22HH_25_Hz;

  return LPS22HH_OK;
}

/**
 * @brief  Get the LPS22HH FIFO data level
 * @param  pObj the device pObj
 * @param  Status the status of data ready bit
 * @retval 0 in case of success, an error code otherwise
 */
int32_t LPS22HH_FIFO_Get_Data(LPS22HH_Object_t *pObj, float *Press, float *Temp)
{
  lps22hh_axis1bit32_t data_raw_pressure;
  lps22hh_axis1bit16_t data_raw_temperature;

  (void)memset(data_raw_pressure.u8bit, 0x00, sizeof(int32_t));
  if (lps22hh_fifo_pressure_raw_get(&(pObj->Ctx), (uint32_t *)&data_raw_pressure.i32bit) != LPS22HH_OK)
  {
	return LPS22HH_ERROR;
  }

  *Press = lps22hh_from_lsb_to_hpa((uint32_t)data_raw_pressure.i32bit);

  (void)memset(data_raw_temperature.u8bit, 0x00, sizeof(int16_t));
  if (lps22hh_fifo_temperature_raw_get(&(pObj->Ctx), &data_raw_temperature.i16bit) != LPS22HH_OK)
  {
    return LPS22HH_ERROR;
  }

  *Temp = lps22hh_from_lsb_to_celsius(data_raw_temperature.i16bit);

  return LPS22HH_OK;
}

/**
 * @brief  Get the LPS22HH FIFO threshold
 * @param  pObj the device pObj
 * @param  Status the status of data ready bit
 * @retval 0 in case of success, an error code otherwise
 */
int32_t LPS22HH_FIFO_Get_FTh_Status(LPS22HH_Object_t *pObj, uint8_t *Status)
{
  if (lps22hh_fifo_wtm_flag_get(&(pObj->Ctx), Status) != LPS22HH_OK)
  {
    return LPS22HH_ERROR;
  }

  return LPS22HH_OK;
}

/**
 * @brief  Get the LPS22HH FIFO full status
 * @param  pObj the device pObj
 * @param  Status the status of data ready bit
 * @retval 0 in case of success, an error code otherwise
 */
int32_t LPS22HH_FIFO_Get_Full_Status(LPS22HH_Object_t *pObj, uint8_t *Status)
{
  if (lps22hh_fifo_full_flag_get(&(pObj->Ctx), Status) != LPS22HH_OK)
  {
    return LPS22HH_ERROR;
  }

  return LPS22HH_OK;
}

/**
 * @brief  Get the LPS22HH FIFO OVR status
 * @param  pObj the device pObj
 * @param  Status the status of data ready bit
 * @retval 0 in case of success, an error code otherwise
 */
int32_t LPS22HH_FIFO_Get_Ovr_Status(LPS22HH_Object_t *pObj, uint8_t *Status)
{
  if (lps22hh_fifo_ovr_flag_get(&(pObj->Ctx), Status) != LPS22HH_OK)
  {
    return LPS22HH_ERROR;
  }

  return LPS22HH_OK;
}

/**
 * @brief  Get the LPS22HH FIFO data level
 * @param  pObj the device pObj
 * @param  Status the status of data ready bit
 * @retval 0 in case of success, an error code otherwise
 */
int32_t LPS22HH_FIFO_Get_Level(LPS22HH_Object_t *pObj, uint8_t *Status)
{
  if (lps22hh_fifo_data_level_get(&(pObj->Ctx), Status) != LPS22HH_OK)
  {
    return LPS22HH_ERROR;
  }

  return LPS22HH_OK;
}

/**
 * @brief  Reset the FIFO interrupt
 * @param  pObj the device pObj
 * @param  interrupt The FIFO interrupt to be reset; values: 0 = FTH; 1 = FULL; 2 = OVR
 * @retval 0 in case of success, an error code otherwise
 */
int32_t LPS22HH_FIFO_Reset_Interrupt(LPS22HH_Object_t *pObj, uint8_t interrupt)
{
  switch (interrupt)
  {
    case 0:
      if (lps22hh_fifo_threshold_on_int_set(&(pObj->Ctx), PROPERTY_DISABLE) != LPS22HH_OK)
      {
        return LPS22HH_ERROR;
      }
      break;
    case 1:
      if (lps22hh_fifo_full_on_int_set(&(pObj->Ctx), PROPERTY_DISABLE) != LPS22HH_OK)
      {
        return LPS22HH_ERROR;
      }
      break;
    case 2:
      if (lps22hh_fifo_ovr_on_int_set(&(pObj->Ctx), PROPERTY_DISABLE) != LPS22HH_OK)
      {
        return LPS22HH_ERROR;
      }
      break;
    default:
      return LPS22HH_ERROR;
  }

  return LPS22HH_OK;
}

/**
 * @brief  Set the FIFO interrupt
 * @param  pObj the device pObj
 * @param  interrupt The FIFO interrupt to be reset; values: 0 = FTH; 1 = FULL; 2 = OVR
 * @retval 0 in case of success, an error code otherwise
 */
int32_t LPS22HH_FIFO_Set_Interrupt(LPS22HH_Object_t *pObj, uint8_t interrupt)
{
  switch (interrupt)
  {
    case 0:
      if (lps22hh_fifo_threshold_on_int_set(&(pObj->Ctx), PROPERTY_ENABLE) != LPS22HH_OK)
      {
        return LPS22HH_ERROR;
      }
      break;
    case 1:
      if (lps22hh_fifo_full_on_int_set(&(pObj->Ctx), PROPERTY_ENABLE) != LPS22HH_OK)
      {
        return LPS22HH_ERROR;
      }
      break;
    case 2:
      if (lps22hh_fifo_ovr_on_int_set(&(pObj->Ctx), PROPERTY_ENABLE) != LPS22HH_OK)
      {
        return LPS22HH_ERROR;
      }
      break;
    default:
      return LPS22HH_ERROR;
  }

  return LPS22HH_OK;
}

/**
 * @brief  Set the FIFO mode
 * @param  pObj the device pObj
 * @param  Mode the FIFO mode to be set
 * @retval 0 in case of success, an error code otherwise
 */
int32_t LPS22HH_FIFO_Set_Mode(LPS22HH_Object_t *pObj, uint8_t Mode)
{
  /* Verify that the passed parameter contains one of the valid values */
  switch ((lps22hh_f_mode_t)Mode)
  {
    case LPS22HH_BYPASS_MODE:
    case LPS22HH_FIFO_MODE:
    case LPS22HH_STREAM_MODE:
    case LPS22HH_STREAM_TO_FIFO_MODE:
    case LPS22HH_BYPASS_TO_STREAM_MODE:
    case LPS22HH_BYPASS_TO_FIFO_MODE:
      break;
    default:
      return LPS22HH_ERROR;
  }

  if (lps22hh_fifo_mode_set(&(pObj->Ctx), (lps22hh_f_mode_t)Mode) != LPS22HH_OK)
  {
    return LPS22HH_ERROR;
  }

  return LPS22HH_OK;
}

/**
 * @brief  Set the LPS22HH FIFO data level
 * @param  pObj the device pObj
 * @param  Status the status of data ready bit
 * @retval 0 in case of success, an error code otherwise
 */
int32_t LPS22HH_FIFO_Set_Watermark_Level(LPS22HH_Object_t *pObj, uint8_t Watermark)
{
  if (lps22hh_fifo_watermark_set(&(pObj->Ctx), Watermark) != LPS22HH_OK)
  {
    return LPS22HH_ERROR;
  }

  return LPS22HH_OK;
}

/**
 * @brief  Set the LPS22HH stop on watermark function
 * @param  pObj the device pObj
 * @param  Stop the state of stop on watermark function
 * @retval 0 in case of success, an error code otherwise
 */
int32_t LPS22HH_FIFO_Stop_On_Watermark(LPS22HH_Object_t *pObj, uint8_t Stop)
{
  if (lps22hh_fifo_stop_on_wtm_set(&(pObj->Ctx), Stop) != LPS22HH_OK)
  {
    return LPS22HH_ERROR;
  }

  return LPS22HH_OK;
}

/**
 * @brief  Set the LPS22HH One Shot Mode
 * @param  pObj the device pObj
 * @retval 0 in case of success, an error code otherwise
 */
int32_t LPS22HH_Set_One_Shot(LPS22HH_Object_t *pObj)
{
  /* Start One Shot Measurement */
  if(lps22hh_data_rate_set(&(pObj->Ctx), LPS22HH_ONE_SHOOT) != LPS22HH_OK)
  {
    return LPS22HH_ERROR;
  }

  return LPS22HH_OK;
}

/**
 * @brief  Get the LPS22HH One Shot Status
 * @param  pObj the device pObj
 * @param  Status pointer to the one shot status (1 means measurements available, 0 means measurements not available yet)
 * @retval 0 in case of success, an error code otherwise
 */
int32_t LPS22HH_Get_One_Shot_Status(LPS22HH_Object_t *pObj, uint8_t *Status)
{
  uint8_t p_da;
  uint8_t t_da;

  /* Get DataReady for pressure */
  if(lps22hh_press_flag_data_ready_get(&(pObj->Ctx), &p_da) != LPS22HH_OK)
  {
    return LPS22HH_ERROR;
  }

  /* Get DataReady for temperature */
  if(lps22hh_temp_flag_data_ready_get(&(pObj->Ctx), &t_da) != LPS22HH_OK)
  {
    return LPS22HH_ERROR;
  }

  if(p_da && t_da)
  {
    *Status = 1;
  }
  else
  {
    *Status = 0;
  }

  return LPS22HH_OK;
}

/**
 * @brief  Enable LPS22HH DRDY interrupt mode
 * @param  pObj the device pObj
 * @retval 0 in case of success, an error code otherwise
 */
int32_t LPS22HH_Enable_DRDY_Interrupt(LPS22HH_Object_t *pObj)
{
  lps22hh_ctrl_reg3_t ctrl1_reg3;

  /* Enable Interrupt DRDY on INT1 */
  if(lps22hh_pin_int_route_get(&(pObj->Ctx), &ctrl1_reg3) != LPS22HH_OK)
  {
    return LPS22HH_ERROR;
  }
  ctrl1_reg3.drdy = 1;
  if(lps22hh_pin_int_route_set(&(pObj->Ctx), &ctrl1_reg3) != LPS22HH_OK)
  {
    return LPS22HH_ERROR;
  }

  return LPS22HH_OK;
}

/**
 * @brief  Set the LPS22HH power mode
 * @param  pObj the device pObj
 * @param  powerMode mode to be set
 * @retval 0 in case of success, an error code otherwise
 */
int32_t LPS22HH_Set_Power_Mode(LPS22HH_Object_t *pObj, uint8_t powerMode)
{
  lps22hh_ctrl_reg2_t ctrl_reg2;

  if(lps22hh_read_reg(&(pObj->Ctx), LPS22HH_CTRL_REG2, (uint8_t*)&ctrl_reg2, 1) != LPS22HH_OK)
  {
    return LPS22HH_ERROR;
  }

  if(powerMode == 1)
  {
    ctrl_reg2.low_noise_en = LPS22HH_LOW_NOISE_EN;
  }
  else
  {
    ctrl_reg2.low_noise_en = LPS22HH_LOW_NOISE_DIS;
  }

  if(lps22hh_write_reg(&(pObj->Ctx), LPS22HH_CTRL_REG2, (uint8_t*)&ctrl_reg2, 1) != LPS22HH_OK)
  {
    return LPS22HH_ERROR;
  }

  return LPS22HH_OK;
}

/**
 * @brief  Set the LPS22HH filter mode
 * @param  pObj the device pObj
 * @param  filterMode to be set
 * @retval 0 in case of success, an error code otherwise
 */
int32_t LPS22HH_Set_Filter_Mode(LPS22HH_Object_t *pObj, uint8_t filterMode)
{
  if(lps22hh_lp_bandwidth_set(&(pObj->Ctx), (lps22hh_lpfp_cfg_t)filterMode) != LPS22HH_OK)
  {
    return LPS22HH_ERROR;
  }
  return LPS22HH_OK;
}

/**
 * @brief  Wrap Read register component function to Bus IO function
 * @param  Handle the device handler
 * @param  Reg the register address
 * @param  pData the stored data pointer
 * @param  Length the length
 * @retval 0 in case of success, an error code otherwise
 */
static int32_t ReadRegWrap(void *Handle, uint8_t Reg, uint8_t *pData, uint16_t Length)
{
  LPS22HH_Object_t *pObj = (LPS22HH_Object_t *)Handle;

  return pObj->IO.ReadReg(pObj->IO.Address, Reg, pData, Length);
}

/**
 * @brief  Wrap Write register component function to Bus IO function
 * @param  Handle the device handler
 * @param  Reg the register address
 * @param  pData the stored data pointer
 * @param  Length the length
 * @retval 0 in case of success, an error code otherwise
 */
static int32_t WriteRegWrap(void *Handle, uint8_t Reg, uint8_t *pData, uint16_t Length)
{
  LPS22HH_Object_t *pObj = (LPS22HH_Object_t *)Handle;

  return pObj->IO.WriteReg(pObj->IO.Address, Reg, pData, Length);
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
