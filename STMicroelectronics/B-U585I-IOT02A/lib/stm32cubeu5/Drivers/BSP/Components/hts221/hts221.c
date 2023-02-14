/**
 ******************************************************************************
 * @file    hts221.c
 * @author  MEMS Software Solutions Team
 * @brief   HTS221 driver file
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
#include "hts221.h"

/** @addtogroup BSP BSP
 * @{
 */

/** @addtogroup Component Component
 * @{
 */

/** @defgroup HTS221 HTS221
 * @{
 */

/** @defgroup HTS221_Exported_Variables HTS221 Exported Variables
 * @{
 */

HTS221_CommonDrv_t HTS221_COMMON_Driver =
{
  HTS221_Init,
  HTS221_DeInit,
  HTS221_ReadID,
  HTS221_GetCapabilities,
};

HTS221_HUM_Drv_t HTS221_HUM_Driver =
{
  HTS221_HUM_Enable,
  HTS221_HUM_Disable,
  HTS221_HUM_GetOutputDataRate,
  HTS221_HUM_SetOutputDataRate,
  HTS221_HUM_GetHumidity,
};

HTS221_TEMP_Drv_t HTS221_TEMP_Driver =
{
  HTS221_TEMP_Enable,
  HTS221_TEMP_Disable,
  HTS221_TEMP_GetOutputDataRate,
  HTS221_TEMP_SetOutputDataRate,
  HTS221_TEMP_GetTemperature,
};

/**
 * @}
 */

/** @defgroup HTS221_Private_Function_Prototypes HTS221 Private Function Prototypes
 * @{
 */

static int32_t ReadRegWrap(void *Handle, uint8_t Reg, uint8_t *pData, uint16_t Length);
static int32_t WriteRegWrap(void *Handle, uint8_t Reg, uint8_t *pData, uint16_t Length);
static int32_t HTS221_GetOutputDataRate(HTS221_Object_t *pObj, float *Odr);
static int32_t HTS221_SetOutputDataRate(HTS221_Object_t *pObj, float Odr);
static int32_t HTS221_Initialize(HTS221_Object_t *pObj);
static float Linear_Interpolation(lin_t *Lin, float Coeff);

/**
 * @}
 */

/** @defgroup HTS221_Exported_Functions HTS221 Exported Functions
 * @{
 */

/**
 * @brief  Register Component Bus IO operations
 * @param  pObj the device pObj
 * @retval 0 in case of success, an error code otherwise
 */
int32_t HTS221_RegisterBusIO(HTS221_Object_t *pObj, HTS221_IO_t *pIO)
{
  int32_t ret;

  if (pObj == NULL)
  {
    ret = HTS221_ERROR;
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

    if (pObj->IO.Init != NULL)
    {
      ret = pObj->IO.Init();
    }
    else
    {
      ret = HTS221_ERROR;
    }
  }

  return ret;
}

/**
 * @brief  Initialize the HTS221 sensor
 * @param  pObj the device pObj
 * @retval 0 in case of success, an error code otherwise
 */
int32_t HTS221_Init(HTS221_Object_t *pObj)
{
  if (pObj->is_initialized == 0U)
  {
    if (HTS221_Initialize(pObj) != HTS221_OK)
    {
      return HTS221_ERROR;
    }
  }

  pObj->is_initialized = 1;

  return HTS221_OK;
}

/**
 * @brief  Deinitialize the HTS221 sensor
 * @param  pObj the device pObj
 * @retval 0 in case of success, an error code otherwise
 */
int32_t HTS221_DeInit(HTS221_Object_t *pObj)
{
  if (pObj->is_initialized == 1U)
  {
    if (HTS221_HUM_Disable(pObj) != HTS221_OK)
    {
      return HTS221_ERROR;
    }

    if (HTS221_TEMP_Disable(pObj) != HTS221_OK)
    {
      return HTS221_ERROR;
    }
  }

  pObj->is_initialized = 0;

  return HTS221_OK;
}

/**
 * @brief  Get WHO_AM_I value
 * @param  pObj the device pObj
 * @param  Id the WHO_AM_I value
 * @retval 0 in case of success, an error code otherwise
 */
int32_t HTS221_ReadID(HTS221_Object_t *pObj, uint8_t *Id)
{
  if (hts221_device_id_get(&(pObj->Ctx), Id) != HTS221_OK)
  {
    return HTS221_ERROR;
  }

  return HTS221_OK;
}

/**
 * @brief  Get HTS221 sensor capabilities
 * @param  pObj Component object pointer
 * @param  Capabilities pointer to HTS221 sensor capabilities
 * @retval 0 in case of success, an error code otherwise
 */
int32_t HTS221_GetCapabilities(HTS221_Object_t *pObj, HTS221_Capabilities_t *Capabilities)
{
  /* Prevent unused argument(s) compilation warning */
  (void)(pObj);

  Capabilities->Humidity    = 1;
  Capabilities->Pressure    = 0;
  Capabilities->Temperature = 1;
  Capabilities->LowPower    = 0;
  Capabilities->HumMaxOdr   = 12.5f;
  Capabilities->TempMaxOdr  = 12.5f;
  Capabilities->PressMaxOdr = 0.0f;
  return HTS221_OK;
}

/**
 * @brief  Get the HTS221 initialization status
 * @param  pObj the device pObj
 * @param  Status 1 if initialized, 0 otherwise
 * @retval 0 in case of success, an error code otherwise
 */
int32_t HTS221_Get_Init_Status(HTS221_Object_t *pObj, uint8_t *Status)
{
  if (pObj == NULL)
  {
    return HTS221_ERROR;
  }

  *Status = pObj->is_initialized;

  return HTS221_OK;
}

/**
 * @brief  Enable the HTS221 humidity sensor
 * @param  pObj the device pObj
 * @retval 0 in case of success, an error code otherwise
 */
int32_t HTS221_HUM_Enable(HTS221_Object_t *pObj)
{
  /* Check if the component is already enabled */
  if (pObj->hum_is_enabled == 1U)
  {
    return HTS221_OK;
  }

  /* Check if the HTS221 temperature sensor is already enabled. */
  /* If yes, skip the enable function, if not call enable function */
  if (pObj->temp_is_enabled == 0U)
  {
    /* Power on the component. */
    if (hts221_power_on_set(&(pObj->Ctx), PROPERTY_ENABLE) != HTS221_OK)
    {
      return HTS221_ERROR;
    }
  }

  pObj->hum_is_enabled = 1;

  return HTS221_OK;
}

/**
 * @brief  Disable the HTS221 humidity sensor
 * @param  pObj the device pObj
 * @retval 0 in case of success, an error code otherwise
 */
int32_t HTS221_HUM_Disable(HTS221_Object_t *pObj)
{
  /* Check if the component is already disabled */
  if (pObj->hum_is_enabled == 0U)
  {
    return HTS221_OK;
  }

  /* Check if the HTS221 temperature sensor is still enable. */
  /* If yes, skip the disable function, if not call disable function */
  if (pObj->temp_is_enabled == 0U)
  {
    /* Power off the component. */
    if (hts221_power_on_set(&(pObj->Ctx), PROPERTY_DISABLE) != HTS221_OK)
    {
      return HTS221_ERROR;
    }
  }

  pObj->hum_is_enabled = 0;

  return HTS221_OK;
}

/**
 * @brief  Get the HTS221 humidity sensor output data rate
 * @param  pObj the device pObj
 * @param  Odr pointer where the output data rate is written
 * @retval 0 in case of success, an error code otherwise
 */
int32_t HTS221_HUM_GetOutputDataRate(HTS221_Object_t *pObj, float *Odr)
{
  return HTS221_GetOutputDataRate(pObj, Odr);
}

/**
 * @brief  Set the HTS221 humidity sensor output data rate
 * @param  pObj the device pObj
 * @param  Odr the output data rate value to be set
 * @retval 0 in case of success, an error code otherwise
 */
int32_t HTS221_HUM_SetOutputDataRate(HTS221_Object_t *pObj, float Odr)
{
  return HTS221_SetOutputDataRate(pObj, Odr);
}

/**
 * @brief  Get the HTS221 humidity value
 * @param  pObj the device pObj
 * @param  Value pointer where the humidity value is written
 * @retval 0 in case of success, an error code otherwise
 */
int32_t HTS221_HUM_GetHumidity(HTS221_Object_t *pObj, float *Value)
{
  hts221_axis1bit16_t data_raw_humidity;
  lin_t lin_hum;

  if (hts221_hum_adc_point_0_get(&(pObj->Ctx), &lin_hum.x0) != HTS221_OK)
  {
    return HTS221_ERROR;
  }

  if (hts221_hum_rh_point_0_get(&(pObj->Ctx), &lin_hum.y0) != HTS221_OK)
  {
    return HTS221_ERROR;
  }

  if (hts221_hum_adc_point_1_get(&(pObj->Ctx), &lin_hum.x1) != HTS221_OK)
  {
    return HTS221_ERROR;
  }

  if (hts221_hum_rh_point_1_get(&(pObj->Ctx), &lin_hum.y1) != HTS221_OK)
  {
    return HTS221_ERROR;
  }

  (void)memset(&data_raw_humidity.i16bit, 0x00, sizeof(int16_t));
  if (hts221_humidity_raw_get(&(pObj->Ctx), &data_raw_humidity.i16bit) != HTS221_OK)
  {
    return HTS221_ERROR;
  }

  *Value = Linear_Interpolation(&lin_hum, (float)data_raw_humidity.i16bit);

  if (*Value < 0.0f)
  {
    *Value = 0.0f;
  }

  if (*Value > 100.0f)
  {
    *Value = 100.0f;
  }

  return HTS221_OK;
}

/**
 * @brief  Get the HTS221 humidity data ready bit value
 * @param  pObj the device pObj
 * @param  Status the status of data ready bit
 * @retval 0 in case of success, an error code otherwise
 */
int32_t HTS221_HUM_Get_DRDY_Status(HTS221_Object_t *pObj, uint8_t *Status)
{
  if (hts221_hum_data_ready_get(&(pObj->Ctx), Status) != HTS221_OK)
  {
    return HTS221_ERROR;
  }

  return HTS221_OK;
}

/**
 * @brief  Enable the HTS221 temperature sensor
 * @param  pObj the device pObj
 * @retval 0 in case of success, an error code otherwise
 */
int32_t HTS221_TEMP_Enable(HTS221_Object_t *pObj)
{
  /* Check if the component is already enabled */
  if (pObj->temp_is_enabled == 1U)
  {
    return HTS221_OK;
  }

  /* Check if the HTS221 humidity sensor is already enabled. */
  /* If yes, skip the enable function, if not call enable function */
  if (pObj->hum_is_enabled == 0U)
  {
    /* Power on the component. */
    if (hts221_power_on_set(&(pObj->Ctx), PROPERTY_ENABLE) != HTS221_OK)
    {
      return HTS221_ERROR;
    }
  }

  pObj->temp_is_enabled = 1;

  return HTS221_OK;
}

/**
 * @brief  Disable the HTS221 temperature sensor
 * @param  pObj the device pObj
 * @retval 0 in case of success, an error code otherwise
 */
int32_t HTS221_TEMP_Disable(HTS221_Object_t *pObj)
{
  /* Check if the component is already disabled */
  if (pObj->temp_is_enabled == 0U)
  {
    return HTS221_OK;
  }

  /* Check if the HTS221 humidity sensor is still enable. */
  /* If yes, skip the disable function, if not call disable function */
  if (pObj->hum_is_enabled == 0U)
  {
    /* Power off the component. */
    if (hts221_power_on_set(&(pObj->Ctx), PROPERTY_DISABLE) != HTS221_OK)
    {
      return HTS221_ERROR;
    }
  }

  pObj->temp_is_enabled = 0;

  return HTS221_OK;
}

/**
 * @brief  Get the HTS221 temperature sensor output data rate
 * @param  pObj the device pObj
 * @param  Odr pointer where the output data rate is written
 * @retval 0 in case of success, an error code otherwise
 */
int32_t HTS221_TEMP_GetOutputDataRate(HTS221_Object_t *pObj, float *Odr)
{
  return HTS221_GetOutputDataRate(pObj, Odr);
}

/**
 * @brief  Set the HTS221 temperature sensor output data rate
 * @param  pObj the device pObj
 * @param  Odr the output data rate value to be set
 * @retval 0 in case of success, an error code otherwise
 */
int32_t HTS221_TEMP_SetOutputDataRate(HTS221_Object_t *pObj, float Odr)
{
  return HTS221_SetOutputDataRate(pObj, Odr);
}

/**
 * @brief  Get the HTS221 temperature value
 * @param  pObj the device pObj
 * @param  Value pointer where the temperature value is written
 * @retval 0 in case of success, an error code otherwise
 */
int32_t HTS221_TEMP_GetTemperature(HTS221_Object_t *pObj, float *Value)
{
  hts221_axis1bit16_t data_raw_temperature;
  lin_t lin_temp;

  if (hts221_temp_adc_point_0_get(&(pObj->Ctx), &lin_temp.x0) != HTS221_OK)
  {
    return HTS221_ERROR;
  }

  if (hts221_temp_deg_point_0_get(&(pObj->Ctx), &lin_temp.y0) != HTS221_OK)
  {
    return HTS221_ERROR;
  }

  if (hts221_temp_adc_point_1_get(&(pObj->Ctx), &lin_temp.x1) != HTS221_OK)
  {
    return HTS221_ERROR;
  }

  if (hts221_temp_deg_point_1_get(&(pObj->Ctx), &lin_temp.y1) != HTS221_OK)
  {
    return HTS221_ERROR;
  }

  (void)memset(&data_raw_temperature.i16bit, 0x00, sizeof(int16_t));
  if (hts221_temperature_raw_get(&(pObj->Ctx), &data_raw_temperature.i16bit) != HTS221_OK)
  {
    return HTS221_ERROR;
  }

  *Value = Linear_Interpolation(&lin_temp, (float)data_raw_temperature.i16bit);

  return HTS221_OK;
}

/**
 * @brief  Get the HTS221 temperature data ready bit value
 * @param  pObj the device pObj
 * @param  Status the status of data ready bit
 * @retval 0 in case of success, an error code otherwise
 */
int32_t HTS221_TEMP_Get_DRDY_Status(HTS221_Object_t *pObj, uint8_t *Status)
{
  if (hts221_temp_data_ready_get(&(pObj->Ctx), Status) != HTS221_OK)
  {
    return HTS221_ERROR;
  }

  return HTS221_OK;
}

/**
 * @brief  Set the HTS221 One Shot Mode
 * @param  pObj the device pObj
 * @retval 0 in case of success, an error code otherwise
 */
int32_t HTS221_Set_One_Shot(HTS221_Object_t *pObj)
{

  /* Set ODR */
  if(hts221_data_rate_set(&(pObj->Ctx), HTS221_ONE_SHOT)!= HTS221_OK)
  {
    return HTS221_ERROR;
  }

  /* Start One Shot Measurement */
  if(hts221_one_shoot_trigger_set(&(pObj->Ctx), 1) != HTS221_OK)
  {
    return HTS221_ERROR;
  }

  return HTS221_OK;
}

/**
 * @brief  Get the HTS221 One Shot Status
 * @param  pObj the device pObj
 * @param  Status pointer to the one shot status (1 means measurements available, 0 means measurements not available yet)
 * @retval 0 in case of success, an error code otherwise
 */
int32_t HTS221_Get_One_Shot_Status(HTS221_Object_t *pObj, uint8_t *Status)
{
  uint8_t h_da;
  uint8_t t_da;

  /* Get DataReady for humidity */
  if(hts221_hum_data_ready_get(&(pObj->Ctx), &h_da) != HTS221_OK)
  {
    return HTS221_ERROR;
  }

  /* Get DataReady for temperature */
  if(hts221_temp_data_ready_get(&(pObj->Ctx), &t_da) != HTS221_OK)
  {
    return HTS221_ERROR;
  }

  if(h_da && t_da)
  {
    *Status = 1;
  }
  else
  {
    *Status = 0;
  }

  return HTS221_OK;
}

/**
 * @brief  Get the HTS221 register value
 * @param  pObj the device pObj
 * @param  Reg address to be read
 * @param  Data pointer where the value is written
 * @retval 0 in case of success, an error code otherwise
 */
int32_t HTS221_Read_Reg(HTS221_Object_t *pObj, uint8_t Reg, uint8_t *Data)
{
  if (hts221_read_reg(&(pObj->Ctx), Reg, Data, 1) != HTS221_OK)
  {
    return HTS221_ERROR;
  }

  return HTS221_OK;
}

/**
 * @brief  Set the HTS221 register value
 * @param  pObj the device pObj
 * @param  Reg address to be written
 * @param  Data value to be written
 * @retval 0 in case of success, an error code otherwise
 */
int32_t HTS221_Write_Reg(HTS221_Object_t *pObj, uint8_t Reg, uint8_t Data)
{
  if (hts221_write_reg(&(pObj->Ctx), Reg, &Data, 1) != HTS221_OK)
  {
    return HTS221_ERROR;
  }

  return HTS221_OK;
}

/**
 * @}
 */

/** @defgroup HTS221_Private_Functions HTS221 Private Functions
 * @{
 */

/**
 * @brief  Get output data rate
 * @param  pObj the device pObj
 * @param  Odr the output data rate value
 * @retval 0 in case of success, an error code otherwise
 */
static int32_t HTS221_GetOutputDataRate(HTS221_Object_t *pObj, float *Odr)
{
  int32_t ret = HTS221_OK;
  hts221_odr_t odr_low_level;

  if (hts221_data_rate_get(&(pObj->Ctx), &odr_low_level) != HTS221_OK)
  {
    return HTS221_ERROR;
  }

  switch (odr_low_level)
  {
    case HTS221_ONE_SHOT:
      *Odr = 0.0f;
      break;

    case HTS221_ODR_1Hz:
      *Odr = 1.0f;
      break;

    case HTS221_ODR_7Hz:
      *Odr = 7.0f;
      break;

    case HTS221_ODR_12Hz5:
      *Odr = 12.5f;
      break;

    default:
      ret = HTS221_ERROR;
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
static int32_t HTS221_SetOutputDataRate(HTS221_Object_t *pObj, float Odr)
{
  hts221_odr_t new_odr;

  new_odr = (Odr <= 1.0f) ? HTS221_ODR_1Hz
            : (Odr <= 7.0f) ? HTS221_ODR_7Hz
            :                 HTS221_ODR_12Hz5;

  if (hts221_data_rate_set(&(pObj->Ctx), new_odr) != HTS221_OK)
  {
    return HTS221_ERROR;
  }

  return HTS221_OK;
}

/**
 * @brief  Initialize the HTS221 sensor
 * @param  pObj the device pObj
 * @retval 0 in case of success, an error code otherwise
 */
static int32_t HTS221_Initialize(HTS221_Object_t *pObj)
{
  /* Power off the component. */
  if (hts221_power_on_set(&(pObj->Ctx), PROPERTY_DISABLE) != HTS221_OK)
  {
    return HTS221_ERROR;
  }

  /* Enable BDU */
  if (hts221_block_data_update_set(&(pObj->Ctx), PROPERTY_ENABLE) != HTS221_OK)
  {
    return HTS221_ERROR;
  }

  /* Set default ODR */
  if (HTS221_SetOutputDataRate(pObj, 1.0f) != HTS221_OK)
  {
    return HTS221_ERROR;
  }

  return HTS221_OK;
}

/**
 * @brief  Enable HTS221 DRDY interrupt mode
 * @param  pObj the device pObj
 * @retval 0 in case of success, an error code otherwise
 */
int32_t HTS221_Enable_DRDY_Interrupt(HTS221_Object_t *pObj)
{
  if (hts221_drdy_on_int_set(&(pObj->Ctx), PROPERTY_ENABLE) != HTS221_OK)
  {
    return HTS221_ERROR;
  }

  return HTS221_OK;
}

/**
 * @brief  Function used to apply coefficient
 * @param  Lin the line
 * @param  Coeff the coefficient
 * @retval Calculation result
 */
static float Linear_Interpolation(lin_t *Lin, float Coeff)
{
  return (((Lin->y1 - Lin->y0) * Coeff) + ((Lin->x1 * Lin->y0) - (Lin->x0 * Lin->y1))) / (Lin->x1 - Lin->x0);
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
  HTS221_Object_t *pObj = (HTS221_Object_t *)Handle;

  if (pObj->IO.BusType == (uint32_t)HTS221_I2C_BUS) /* I2C */
  {
    /* Enable Multi-byte read */
    return pObj->IO.ReadReg(pObj->IO.Address, (Reg | 0x80U), pData, Length);
  }
  else /* SPI 3-Wires */
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
static int32_t WriteRegWrap(void *Handle, uint8_t Reg, uint8_t *pData, uint16_t Length)
{
  HTS221_Object_t *pObj = (HTS221_Object_t *)Handle;

  if (pObj->IO.BusType == (uint32_t)HTS221_I2C_BUS) /* I2C */
  {
    /* Enable Multi-byte write */
    return pObj->IO.WriteReg(pObj->IO.Address, (Reg | 0x80U), pData, Length);
  }
  else /* SPI 3-Wires */
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
