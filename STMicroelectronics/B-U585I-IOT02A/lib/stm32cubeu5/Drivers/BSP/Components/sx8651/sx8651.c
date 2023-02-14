/**
  ******************************************************************************
  * @file    sx8651.c
  * @author  MCD Application Team
  * @brief   This file provides a set of functions needed to manage the SX8651
  *          Touch screen device.
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

/* Includes ------------------------------------------------------------------*/
#include "sx8651.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Component
  * @{
  */

/** @defgroup SX8651 SX8651
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/** @defgroup SX8651_Exported_Variables SX8651 Exported Variables
  * @{
  */

/* Touch screen driver structure initialization */
SX8651_TS_Drv_t SX8651_TS_Driver =
{
  SX8651_Init,
  SX8651_DeInit,
  SX8651_GestureConfig,
  SX8651_ReadID,
  SX8651_GetState,
  SX8651_GetMultiTouchState,
  SX8651_GetGesture,
  SX8651_GetCapabilities,
  SX8651_EnableIT,
  SX8651_DisableIT,
  SX8651_ClearIT,
  SX8651_ITStatus
};
/**
  * @}
  */

/** @defgroup SX8651_Private_Function_Prototypes SX8651 Private Function Prototypes
  * @{
  */
static int32_t SX8651_Delay(SX8651_Object_t *pObj, uint32_t Delay);
static int32_t SX8651_DetectTouch(SX8651_Object_t *pObj);
static int32_t ReadRegWrap(void *handle, uint8_t Reg, uint8_t *Data, uint16_t Length);
static int32_t WriteRegWrap(void *handle, uint8_t Reg, uint8_t *Data, uint16_t Length);
static int32_t ReadDataWrap(void *handle, uint8_t *pData, uint16_t Length);
static int32_t WriteCmdWrap(void *handle, uint8_t *pData, uint16_t Length);
static int32_t SX8651_Reset(SX8651_Object_t *pObj);

/**
  * @}
  */

/** @defgroup SX8651_Exported_Functions SX8651 Exported Functions
  * @{
  */

/**
  * @brief  Register IO bus to component object
  * @param  Component object pointer
  * @retval Component status
  */
int32_t SX8651_RegisterBusIO(SX8651_Object_t *pObj, SX8651_IO_t *pIO)
{
  int32_t ret;

  if (pObj == NULL)
  {
    ret = SX8651_ERROR;
  }
  else
  {
    pObj->IO.Init      = pIO->Init;
    pObj->IO.DeInit    = pIO->DeInit;
    pObj->IO.Address   = pIO->Address;
    pObj->IO.WriteReg  = pIO->WriteReg;
    pObj->IO.ReadReg   = pIO->ReadReg;
    pObj->IO.ReadData  = pIO->ReadData;
    pObj->IO.WriteCmd  = pIO->WriteCmd;
    pObj->IO.GetTick   = pIO->GetTick;

    pObj->Ctx.ReadReg  = ReadRegWrap;
    pObj->Ctx.WriteReg = WriteRegWrap;
    pObj->Ctx.ReadData = ReadDataWrap;
    pObj->Ctx.WriteCmd = WriteCmdWrap;

    pObj->Ctx.handle   = pObj;

    if (pObj->IO.Init != NULL)
    {
      ret = pObj->IO.Init();
    }
    else
    {
      ret = SX8651_ERROR;
    }
  }

  return ret;
}

/**
  * @brief  Get SX8651 sensor capabilities
  * @param  pObj Component object pointer
  * @param  Capabilities pointer to SX8651 sensor capabilities
  * @retval Component status
  */
int32_t SX8651_GetCapabilities(SX8651_Object_t *pObj, SX8651_Capabilities_t *Capabilities)
{
  /* Prevent unused argument(s) compilation warning */
  (void)(pObj);

  /* Store component's capabilities */
  Capabilities->MultiTouch = 1;
  Capabilities->Gesture    = 1;
  Capabilities->MaxTouch   = SX8651_MAX_NB_TOUCH;
  Capabilities->MaxXl      = SX8651_MAX_X_LENGTH;
  Capabilities->MaxYl      = SX8651_MAX_Y_LENGTH;

  return SX8651_OK;
}

/**
  * @brief  Initialize the SX8651 communication bus
  *         from MCU to SX8651 : ie I2C channel initialization (if required).
  * @param  pObj Component object pointer
  * @retval Component status
  */
int32_t SX8651_Init(SX8651_Object_t *pObj)
{
  int32_t ret = SX8651_OK;
  uint8_t mode;

  if (pObj->IsInitialized == 0U)
  {
    /* Generate sx8651 Software reset */
    if (SX8651_Reset(pObj) != SX8651_OK)
    {
      ret = SX8651_ERROR;
    }
    else
    {
      pObj->IsInitialized = 1U;

      /* Get the current register value */
      ret += sx8651_read_reg(&pObj->Ctx, SX8651_READ_ADDR | SX8651_REG_CTRL0, &mode, 1);

      /* Configuration:
      - RATE                     : 300 cps
      - Settling time of POWDLY  : 140 uS
      */
      mode = REG_CTRL0_RATE_300CPS | REG_CTRL0_POWDLY_140US;

      /* Write the new register value */
      ret += sx8651_write_reg(&pObj->Ctx, SX8651_REG_CTRL0, &mode, 1);

      /* Get the current register value */
      ret += sx8651_read_reg(&pObj->Ctx, SX8651_READ_ADDR | SX8651_REG_CTRL1, &mode, 1);

      /* Configuration:
      - Touch average control    : 3 samples
      - Pen Detect Resistor      : 25k ohm
      - conditional interrupts   : interrupt generated when pen detect is successful
      */
      mode = REG_CTRL0_RATE_300CPS | REG_CTRL1_CONDIRQ_DETECTED | \
             REG_CTRL1_RPDNT_25KOHM | REG_CTRL1_FILT_3AVERAGE;

      /* Write the new register value */
      ret += sx8651_write_reg(&pObj->Ctx, SX8651_REG_CTRL1, &mode, 1);

      /* Get the current register value */
      ret += sx8651_read_reg(&pObj->Ctx, SX8651_READ_ADDR | SX8651_REG_CTRL2, &mode, 1);

      /* Configuration:
      - Settling time while filtering   : 140 us
      */
      mode = REG_CTRL2_SETDLY_140US;

      /* Write the new register value */
      ret += sx8651_write_reg(&pObj->Ctx, SX8651_REG_CTRL2, &mode, 1);

      /* Get the current register value */
      ret += sx8651_read_reg(&pObj->Ctx, SX8651_READ_ADDR | SX8651_REG_CTRL3, &mode, 1);

      /* Rm : 542 ohm */
      mode = REG_CTRL3_RM_SEL_Y_542OHM | REG_CTRL3_RM_SEL_X_542OHM;

      /* Write the new register value */
      ret += sx8651_write_reg(&pObj->Ctx, SX8651_REG_CTRL3, &mode, 1);

      /* Get the current register value */
      ret += sx8651_read_reg(&pObj->Ctx, SX8651_READ_ADDR | SX8651_REG_CHAN_MSK, &mode, 1);

      /* Set the Functionalities to be Enabled
      XCONV & YCONV to support single-touch by default
      */
      mode = REG_CHAN_MSK_XCONV | REG_CHAN_MSK_YCONV;

      /* Write the new register value */
      ret += sx8651_write_reg(&pObj->Ctx, SX8651_REG_CHAN_MSK, &mode, 1);

      /* Start Touch Screen function, in AUTO mode (RATE > 0) */
      mode = SX8651_CMD_MANAUTO;

      /* Write the new register value */
      ret += sx8651_write_cmd(&pObj->Ctx, &mode, 1);

      /* Wait for 2 ms */
      SX8651_Delay(pObj, 2);
    }
  }

  if (ret != SX8651_OK)
  {
    ret = SX8651_ERROR;
  }

  return ret;
}


/**
  * @brief  De-Initialize the SX8651 communication bus
  *         from MCU to SX8651 : ie I2C channel initialization (if required).
  * @param  pObj Component object pointer
  * @retval SX8651_OK
  */
int32_t SX8651_DeInit(SX8651_Object_t *pObj)
{
  int32_t ret = SX8651_OK;

  if (pObj->IsInitialized == 1U)
  {
    pObj->IsInitialized = 0;
  }

  return ret;
}

/**
  * @brief  Configure the SX8651 gesture
  * @param  pObj  Component object pointer
  * @param  GestureInit Gesture init structure
  * @retval SX8651_OK
  */
int32_t SX8651_GestureConfig(SX8651_Object_t *pObj, SX8651_Gesture_Init_t *GestureInit)
{
  /* Prevent unused argument(s) compilation warning */
  (void)(pObj);
  (void)(GestureInit);

  /* Always return SX8651_OK as feature not supported */
  return SX8651_OK;
}

/**
  * @brief  Read the SX8651 device ID, pre initialize I2C in case of need to be
  *         able to read the SX8651 device ID, and verify this is an SX8651.
  * @param  pObj Component object pointer
  * @param  Id Component ID pointer
  * @retval Component status
  */
int32_t SX8651_ReadID(SX8651_Object_t *pObj, uint32_t *Id)
{
  int32_t ret = SX8651_OK;
  uint8_t data = 0;

  if (sx8651_read_reg(&pObj->Ctx, SX8651_READ_ADDR | SX8651_REG_CHAN_MSK, &data, 1) != SX8651_OK)
  {
    ret = SX8651_ERROR;
  }
  *Id = (uint32_t)data;

  return ret;
}

/**
  * @brief  Get the touch screen X and Y positions values
  * @param  pObj Component object pointer
  * @param  State Single Touch structure pointer
  * @retval Component status
  */
int32_t SX8651_GetState(SX8651_Object_t *pObj, SX8651_State_t *State)
{
  int32_t ret = SX8651_OK;
  uint8_t data[4];

  State->TouchDetected = 0;
  State->TouchX = 0;
  State->TouchY = 0;

  if (SX8651_DetectTouch(pObj) == 1)
  {
    State->TouchDetected = 1;
    if (sx8651_read_data(&pObj->Ctx, data, (uint16_t)sizeof(data)) != SX8651_OK)
    {
      ret = SX8651_ERROR;
    }
    else
    {
      State->TouchX = ((data[0] & 0xF) << 8) | data[1];
      State->TouchY = ((data[2] & 0xF) << 8) | data[3];
    }
  }

  return ret;
}

/**
  * @brief  Get the touch screen Xn and Yn positions values in multi-touch mode
  * @param  pObj Component object pointer
  * @param  State Multi Touch structure pointer
  * @retval SX8651_OK
  */
int32_t SX8651_GetMultiTouchState(SX8651_Object_t *pObj, SX8651_MultiTouch_State_t *State)
{
  /* Prevent unused argument(s) compilation warning */
  (void)(pObj);
  (void)(State);

  /* Always return SX8651_OK as feature not supported */
  return SX8651_OK;
}

/**
  * @brief  Get Gesture ID
  * @param  pObj Component object pointer
  * @param  GestureId: gesture ID
  * @retval SX8651_OK
  */
int32_t SX8651_GetGesture(SX8651_Object_t *pObj, uint8_t *GestureId)
{
  /* Prevent unused argument(s) compilation warning */
  (void)(pObj);
  (void)(GestureId);

  /* Always return SX8651_OK as feature not supported */
  return SX8651_OK;
}

/**
  * @brief  Configure the SX8651 device to generate IT on given INT pin
  *         connected to MCU as EXTI.
  * @param  pObj Component object pointer
  * @retval SX8651_OK
  */
int32_t SX8651_EnableIT(SX8651_Object_t *pObj)
{
  /* Prevent unused argument(s) compilation warning */
  (void)(pObj);

  /* Always return SX8651_OK as feature not supported */
  return SX8651_OK;
}

/**
  * @brief  Configure the SX8651 device to stop generating IT on given
  *         INT pin connected to MCU as EXTI.
  * @param  pObj Component object pointer
  * @retval SX8651_OK
  */
int32_t SX8651_DisableIT(SX8651_Object_t *pObj)
{
  /* Prevent unused argument(s) compilation warning */
  (void)(pObj);

  /* Always return SX8651_OK as feature not supported */
  return SX8651_OK;
}

/**
  * @brief  Get IT status from SX8651 interrupt status registers
  * @param  pObj Component object pointer
  * @retval SX8651_OK
  */
int32_t SX8651_ITStatus(SX8651_Object_t *pObj)
{
  /* Prevent unused argument(s) compilation warning */
  (void)(pObj);

  /* Always return SX8651_OK as feature not supported */
  return SX8651_OK;
}

/**
  * @brief  Clear IT status in SX8651 interrupt status clear registers
  * @param  pObj Component object pointer
  * @retval SX8651_OK
  */
int32_t SX8651_ClearIT(SX8651_Object_t *pObj)
{
  /* Prevent unused argument(s) compilation warning */
  (void)(pObj);

  /* Always return SX8651_OK as feature not supported */
  return SX8651_OK;
}

/**
  * @brief  Reset the sx8651 by Software.
  * @param  pObj  Pointer to component object.
  * @retval Component status
  */
int32_t SX8651_Reset(SX8651_Object_t *pObj)
{
  int32_t ret = SX8651_OK;
  uint8_t tmp = SX8651_SOFTRESET_VALUE;

  /* Trigger a software reset of sx8651 */
  if (sx8651_write_reg(&pObj->Ctx, SX8651_SOFTRESET_REG, &tmp, 1) != SX8651_OK)
  {
    ret = SX8651_ERROR;
  }
  else
  {
    /* Wait for a delay to ensure registers erasing */
    SX8651_Delay(pObj, 2);
  }

  return ret;
}

static int32_t SX8651_Delay(SX8651_Object_t *pObj, uint32_t Delay)
{
  uint32_t tickstart;
  tickstart = pObj->IO.GetTick();
  while ((pObj->IO.GetTick() - tickstart) < Delay)
  {
  }
  return SX8651_OK;
}

/**
  * @brief  Return number of touches detected if there are any.
  *         Try to detect new touches and forget the old ones (reset internal global
  *         variables).
  * @param  pObj Component object pointer
  * @retval Number of active touches detected (can be 0, 1 or 2) or SX8651_ERROR
  *         in case of error
  */
static int32_t SX8651_DetectTouch(SX8651_Object_t *pObj)
{
  int32_t ret;
  uint8_t nb_touch = 0;

  sx8651_read_reg(&pObj->Ctx, SX8651_READ_ADDR | SX8651_REG_STAT, &nb_touch, 1);
  nb_touch = ((nb_touch & REG_STAT_CONVIRQ) == REG_STAT_CONVIRQ);

  if ((nb_touch & REG_STAT_CONVIRQ) == REG_STAT_CONVIRQ)
  {
    nb_touch = 1;
  }
  ret = (int32_t)nb_touch;

  return ret;
}

/**
  * @brief  Function
  * @param  handle: Component object handle
  * @param  Reg: The target register address to write
  * @param  pData: The target register value to be written
  * @param  Length: buffer size to be written
  * @retval error status
  */
static int32_t ReadRegWrap(void *handle, uint8_t Reg, uint8_t *pData, uint16_t Length)
{
  SX8651_Object_t *pObj = (SX8651_Object_t *)handle;

  return pObj->IO.ReadReg(pObj->IO.Address, Reg, pData, Length);
}

/**
  * @brief  Function
  * @param  handle: Component object handle
  * @param  Reg: The target register address to write
  * @param  pData: The target register value to be written
  * @param  Length: buffer size to be written
  * @retval error status
  */
static int32_t WriteRegWrap(void *handle, uint8_t Reg, uint8_t *pData, uint16_t Length)
{
  SX8651_Object_t *pObj = (SX8651_Object_t *)handle;

  return pObj->IO.WriteReg(pObj->IO.Address, Reg, pData, Length);
}

/**
  * @brief  Function
  * @param  handle: Component object handle
  * @param  pData: The target register value to be written
  * @param  Length: buffer size to be written
  * @retval error status
  */
static int32_t ReadDataWrap(void *handle, uint8_t *pData, uint16_t Length)
{
  SX8651_Object_t *pObj = (SX8651_Object_t *)handle;

  return pObj->IO.ReadData(pObj->IO.Address, pData, Length);
}

/**
  * @brief  Function
  * @param  handle: Component object handle
  * @param  pData: The target register value to be written
  * @param  Length: buffer size to be written
  * @retval error status
  */
static int32_t WriteCmdWrap(void *handle, uint8_t *pData, uint16_t Length)
{
  SX8651_Object_t *pObj = (SX8651_Object_t *)handle;

  return pObj->IO.WriteCmd(pObj->IO.Address, pData, Length);
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
