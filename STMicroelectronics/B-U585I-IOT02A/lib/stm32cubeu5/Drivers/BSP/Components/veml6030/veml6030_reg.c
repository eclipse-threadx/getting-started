/*
 ******************************************************************************
 * @file    veml6030_reg.c
 * @author  MCD Application Team
 * @brief   VEML6030 driver file
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

#include "veml6030_reg.h"
/** @addtogroup BSP
  * @{
  */

/** @addtogroup Components
  * @{
  */

/** @addtogroup VEML6030
  * @brief     This file provides a set of functions needed to drive the
  *            VEML6030 Light sensor.
  * @{
  */

/**
  * @brief  Read VEML6030 component registers
  * @param  ctx component contex
  * @param  reg Register to read from
  * @param  pdata Pointer to data buffer
  * @param  length Number of data to read
  * @retval Component status
  */
int32_t veml6030_read_reg(veml6030_ctx_t *ctx, uint16_t reg, uint16_t *pdata, uint16_t length)
{
  int32_t ret;
  uint16_t tmp;

  ret = ctx->ReadReg(ctx->handle, reg, (uint8_t *)pdata, length);

  if(ret >= 0)
  {
    tmp = ((uint16_t)(*pdata >> 8) & 0x00FF);
    tmp |= ((uint16_t)(*pdata << 8) & 0xFF00);
    *pdata = tmp;
  }
  return ret;
}

/**
  * @brief  Write VEML6030 component registers
  * @param  ctx component contex
  * @param  reg Register to write to
  * @param  pdata Pointer to data buffer
  * @param  length Number of data to write
  * @retval Component status
  */
int32_t veml6030_write_reg(veml6030_ctx_t *ctx, uint16_t reg, uint16_t *pdata, uint16_t length)
{
  uint16_t tmp;
  tmp = ((uint16_t)(*pdata >> 8) & 0x00FF);
  tmp |= ((uint16_t)(*pdata << 8) & 0xFF00);

  return ctx->WriteReg(ctx->handle, reg, (uint8_t *)&tmp, length);
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/


