/**
  ******************************************************************************
  * @file    sx8651_reg.c
  * @author  MCD Application Team
  * @brief   This file provides unitary register function to control the sx8651 Touch
  *
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
#include "sx8651_reg.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Component
  * @{
  */

/** @defgroup SX8651 SX8651
  * @{
  */
/**
  * @brief  Read SX8651 registers.
  * @param  ctx Component context
  * @param  reg Component reg to read from
  * @param  pdata pointer to data to be read
  * @param  length Length of data to read
  * @retval Component status
  */
int32_t sx8651_read_reg(sx8651_ctx_t *ctx, uint8_t reg, uint8_t *pdata, uint16_t length)
{
  uint8_t reg_r = 0x40;

  reg_r |= reg;
  return ctx->ReadReg(ctx->handle, reg, pdata, length);
}

/**
  * @brief  Write SX8651 registers.
  * @param  ctx Component context
  * @param  reg Component reg to write to
  * @param  pdata pointer to data to be written
  * @param  length Length of data to write
  * @retval Component status
  */
int32_t sx8651_write_reg(sx8651_ctx_t *ctx, uint8_t reg, uint8_t *pdata, uint16_t length)
{
  return ctx->WriteReg(ctx->handle, reg, pdata, length);
}

/**
  * @brief  Read SX8651 data
  * @param  ctx Component context
  * @param  pdata pointer to data to be read
  * @param  length Length of data to read
  * @retval Component status
  */
int32_t sx8651_read_data(sx8651_ctx_t *ctx, uint8_t *pdata, uint16_t length)
{
  return ctx->ReadData(ctx->handle, pdata, length);
}

/**
  * @brief  Write SX8651 registers.
  * @param  ctx Component context
  * @param  pdata pointer to data to be written
  * @param  length Length of data to write
  * @retval Component status
  */
int32_t sx8651_write_cmd(sx8651_ctx_t *ctx, uint8_t *pdata, uint16_t length)
{
  return ctx->WriteCmd(ctx->handle, pdata, length);
}

/**
  * @brief  Read SX8651 registers.
  * @param  ctx Component context
  * @param  reg Component reg to read from
  * @param  pdata pointer to data to be read
  * @param  length Length of data to read
  * @retval Component status
  */
int32_t sx8651_read_chan(sx8651_ctx_t *ctx, uint8_t chan, uint8_t *pdata, uint16_t length)
{
  uint8_t reg_r = 0x80;

  reg_r |= chan;
  return ctx->ReadReg(ctx->handle, reg_r, pdata, length);
}

/**
  * @brief  Set SX8651 rate in coordinates per sec
  * @param  ctx Component context
  * @param  value Value to write to SX8651_CTRL_0_REG register
  * @retval Component status
  */
int32_t  sx8651_ctrl_rate_w(sx8651_ctx_t *ctx, uint8_t value)
{
  int32_t ret;
  uint8_t tmp;

  ret = sx8651_read_reg(ctx, SX8651_CTRL_0_REG, &tmp, 1);

  if (ret == 0)
  {
    tmp &= ~SX8651_CTRL_RATE_BIT_MASK;
    tmp |= value << SX8651_CTRL_RATE_BIT_POSITION;

    ret = sx8651_write_reg(ctx, SX8651_CTRL_0_REG, &tmp, 1);
  }

  return ret;
}

/**
  * @brief  Get SX8651 rate in coordinates per sec
  * @param  ctx Component context
  * @param  value pointer to the value of SX8651_CTRL_0_REG register
  * @retval Component status
  */
int32_t  sx8651_ctrl_rate_r(sx8651_ctx_t *ctx, uint8_t *value)
{
  int32_t ret;

  ret = sx8651_read_reg(ctx, SX8651_CTRL_0_REG, (uint8_t *)value, 1);

  if (ret == 0)
  {
    *value &= SX8651_CTRL_RATE_BIT_MASK;
    *value = *value >> SX8651_CTRL_RATE_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Set SX8651 settling time
  * @param  ctx Component context
  * @param  value pointer to the value of SX8651_CTRL_0_REG register
  * @retval Component status
  */
int32_t  sx8651_ctrl_powdly_w(sx8651_ctx_t *ctx, uint8_t value)
{
  int32_t ret;
  uint8_t tmp;

  ret = sx8651_read_reg(ctx, SX8651_CTRL_0_REG, &tmp, 1);

  if (ret == 0)
  {
    tmp &= ~SX8651_CTRL_AUXAQC_BIT_MASK;
    tmp |= value << SX8651_CTRL_AUXAQC_BIT_POSITION;

    ret = sx8651_write_reg(ctx, SX8651_CTRL_0_REG, &tmp, 1);
  }

  return ret;
}

/**
  * @brief  Get SX8651 settling time
  * @param  ctx Component context
  * @param  value pointer to the value of SX8651_CTRL_0_REG register
  * @retval Component status
  */
int32_t  sx8651_ctrl_powdly_r(sx8651_ctx_t *ctx, uint8_t *value)
{
  int32_t ret;

  ret = sx8651_read_reg(ctx, SX8651_CTRL_0_REG, (uint8_t *)value, 1);

  if (ret == 0)
  {
    *value &= SX8651_CTRL_POWDLY_BIT_MASK;
    *value = *value >> SX8651_CTRL_POWDLY_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Set the SX8651 AUX trigger
  * @param  ctx Component context
  * @param  value pointer to the value of SX8651_CTRL_1_REG register
  * @retval Component status
  */
int32_t  sx8651_ctrl_auxaqc_w(sx8651_ctx_t *ctx, uint8_t value)
{
  int32_t ret;
  uint8_t tmp;

  ret = sx8651_read_reg(ctx, SX8651_CTRL_1_REG, &tmp, 1);

  if (ret == 0)
  {
    tmp &= ~SX8651_CTRL_AUXAQC_BIT_MASK;
    tmp |= value << SX8651_CTRL_AUXAQC_BIT_POSITION;

    ret = sx8651_write_reg(ctx, SX8651_CTRL_1_REG, &tmp, 1);
  }

  return ret;
}

/**
  * @brief  Get the SX8651 AUX trigger
  * @param  ctx Component context
  * @param  value pointer to the value of SX8651_CTRL_1_REG register
  * @retval Component status
  */
int32_t  sx8651_ctrl_auxaqc_r(sx8651_ctx_t *ctx, uint8_t *value)
{
  int32_t ret;

  ret = sx8651_read_reg(ctx, SX8651_CTRL_1_REG, (uint8_t *)value, 1);

  if (ret == 0)
  {
    *value &= SX8651_CTRL_AUXAQC_BIT_MASK;
    *value = *value >> SX8651_CTRL_AUXAQC_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Set the SX8651 conditional interrupts
  * @param  ctx Component context
  * @param  value pointer to the value of SX8651_CTRL_1_REG register
  * @retval Component status
  */
int32_t  sx8651_ctrl_condirq_w(sx8651_ctx_t *ctx, uint8_t value)
{
  int32_t ret;
  uint8_t tmp;

  ret = sx8651_read_reg(ctx, SX8651_CTRL_1_REG, &tmp, 1);

  if (ret == 0)
  {
    tmp &= ~SX8651_CTRL_CONDIRQ_BIT_MASK;
    tmp |= value << SX8651_CTRL_CONDIRQ_BIT_POSITION;

    ret = sx8651_write_reg(ctx, SX8651_CTRL_1_REG, &tmp, 1);
  }

  return ret;
}

/**
  * @brief  Get the SX8651 conditional interrupts
  * @param  ctx Component context
  * @param  value pointer to the value of SX8651_CTRL_1_REG register
  * @retval Component status
  */
int32_t  sx8651_ctrl_condirq_r(sx8651_ctx_t *ctx, uint8_t *value)
{
  int32_t ret;

  ret = sx8651_read_reg(ctx, SX8651_CTRL_1_REG, (uint8_t *)value, 1);

  if (ret == 0)
  {
    *value &= SX8651_CTRL_CONDIRQ_BIT_MASK;
    *value = *value >> SX8651_CTRL_CONDIRQ_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Set the SX8651 Pen Detect Resistor
  * @param  ctx Component context
  * @param  value pointer to the value of SX8651_CTRL_1_REG register
  * @retval Component status
  */
int32_t  sx8651_ctrl_rpdnt_w(sx8651_ctx_t *ctx, uint8_t value)
{
  int32_t ret;
  uint8_t tmp;

  ret = sx8651_read_reg(ctx, SX8651_CTRL_1_REG, &tmp, 1);

  if (ret == 0)
  {
    tmp &= ~SX8651_CTRL_RPDNT_BIT_MASK;
    tmp |= value << SX8651_CTRL_RPDNT_BIT_POSITION;

    ret = sx8651_write_reg(ctx, SX8651_CTRL_1_REG, &tmp, 1);
  }

  return ret;
}

/**
  * @brief  Get the SX8651 Pen Detect Resistor
  * @param  ctx Component context
  * @param  value pointer to the value of SX8651_CTRL_1_REG register
  * @retval Component status
  */
int32_t  sx8651_ctrl_rpdnt_r(sx8651_ctx_t *ctx, uint8_t *value)
{
  int32_t ret;

  ret = sx8651_read_reg(ctx, SX8651_CTRL_1_REG, (uint8_t *)value, 1);

  if (ret == 0)
  {
    *value &= SX8651_CTRL_RPDNT_BIT_MASK;
    *value = *value >> SX8651_CTRL_RPDNT_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Set the SX8651 Digital filter control
  * @param  ctx Component context
  * @param  value pointer to the value of SX8651_CTRL_1_REG register
  * @retval Component status
  */
int32_t  sx8651_ctrl_filt_w(sx8651_ctx_t *ctx, uint8_t value)
{
  int32_t ret;
  uint8_t tmp;

  ret = sx8651_read_reg(ctx, SX8651_CTRL_1_REG, &tmp, 1);

  if (ret == 0)
  {
    tmp &= ~SX8651_CTRL_FILT_BIT_MASK;
    tmp |= value << SX8651_CTRL_FILT_BIT_POSITION;

    ret = sx8651_write_reg(ctx, SX8651_CTRL_1_REG, &tmp, 1);
  }

  return ret;
}

/**
  * @brief  Get the SX8651 Digital filter control
  * @param  ctx Component context
  * @param  value pointer to the value of SX8651_CTRL_1_REG register
  * @retval Component status
  */
int32_t  sx8651_ctrl_filt_r(sx8651_ctx_t *ctx, uint8_t *value)
{
  int32_t ret;

  ret = sx8651_read_reg(ctx, SX8651_CTRL_1_REG, (uint8_t *)value, 1);

  if (ret == 0)
  {
    *value &= SX8651_CTRL_FILT_BIT_MASK;
    *value = *value >> SX8651_CTRL_FILT_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Set the SX8651 Settling time while filtering
  * @param  ctx Component context
  * @param  value pointer to the value of SX8651_CTRL_2_REG register
  * @retval Component status
  */
int32_t  sx8651_ctrl_setdly_w(sx8651_ctx_t *ctx, uint8_t value)
{
  int32_t ret;
  uint8_t tmp;

  ret = sx8651_read_reg(ctx, SX8651_CTRL_2_REG, &tmp, 1);

  if (ret == 0)
  {
    tmp &= ~SX8651_CTRL_SETDLY_BIT_MASK;
    tmp |= value << SX8651_CTRL_SETDLY_BIT_POSITION;

    ret = sx8651_write_reg(ctx, SX8651_CTRL_2_REG, &tmp, 1);
  }

  return ret;
}

/**
  * @brief  Get the SX8651 Settling time while filtering
  * @param  ctx Component context
  * @param  value pointer to the value of SX8651_CTRL_2_REG register
  * @retval Component status
  */
int32_t  sx8651_ctrl_setdly_r(sx8651_ctx_t *ctx, uint8_t *value)
{
  int32_t ret;

  ret = sx8651_read_reg(ctx, SX8651_CTRL_2_REG, (uint8_t *)value, 1);

  if (ret == 0)
  {
    *value &= SX8651_CTRL_SETDLY_BIT_MASK;
    *value = *value >> SX8651_CTRL_SETDLY_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Set the SX8651 Rm select Y
  * @param  ctx Component context
  * @param  value pointer to the value of SX8651_CTRL_3_REG register
  * @retval Component status
  */
int32_t  sx8651_ctrl_rmsel_y_w(sx8651_ctx_t *ctx, uint8_t value)
{
  int32_t ret;
  uint8_t tmp;

  ret = sx8651_read_reg(ctx, SX8651_CTRL_3_REG, &tmp, 1);

  if (ret == 0)
  {
    tmp &= ~SX8651_CTRL_RMSEL_Y_BIT_MASK;
    tmp |= value << SX8651_CTRL_RMSEL_Y_BIT_POSITION;

    ret = sx8651_write_reg(ctx, SX8651_CTRL_3_REG, &tmp, 1);
  }

  return ret;
}

/**
  * @brief  Get the SX8651 Rm select Y
  * @param  ctx Component context
  * @param  value pointer to the value of SX8651_CTRL_3_REG register
  * @retval Component status
  */
int32_t  sx8651_ctrl_rmsel_y_r(sx8651_ctx_t *ctx, uint8_t *value)
{
  int32_t ret;

  ret = sx8651_read_reg(ctx, SX8651_CTRL_3_REG, (uint8_t *)value, 1);

  if (ret == 0)
  {
    *value &= SX8651_CTRL_RMSEL_Y_BIT_MASK;
    *value = *value >> SX8651_CTRL_RMSEL_Y_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Set the SX8651 Rm select X
  * @param  ctx Component context
  * @param  value pointer to the value of SX8651_CTRL_3_REG register
  * @retval Component status
  */
int32_t  sx8651_ctrl_rmsel_x_w(sx8651_ctx_t *ctx, uint8_t value)
{
  int32_t ret;
  uint8_t tmp;

  ret = sx8651_read_reg(ctx, SX8651_CTRL_3_REG, &tmp, 1);

  if (ret == 0)
  {
    tmp &= ~SX8651_CTRL_RMSEL_X_BIT_MASK;
    tmp |= value << SX8651_CTRL_RMSEL_X_BIT_POSITION;

    ret = sx8651_write_reg(ctx, SX8651_CTRL_3_REG, &tmp, 1);
  }

  return ret;
}

/**
  * @brief  Get the SX8651 Rm select X
  * @param  ctx Component context
  * @param  value pointer to the value of SX8651_CTRL_3_REG register
  * @retval Component status
  */
int32_t  sx8651_ctrl_rmsel_x_r(sx8651_ctx_t *ctx, uint8_t *value)
{
  int32_t ret;

  ret = sx8651_read_reg(ctx, SX8651_CTRL_3_REG, (uint8_t *)value, 1);

  if (ret == 0)
  {
    *value &= SX8651_CTRL_RMSEL_X_BIT_MASK;
    *value = *value >> SX8651_CTRL_RMSEL_X_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Set the SX8651 XCONV
  * @param  ctx Component context
  * @param  value pointer to the value of SX8651_CHANMSK_REG_REG register
  * @retval Component status
  */
int32_t  sx8651_ctrl_xconv_w(sx8651_ctx_t *ctx, uint8_t value)
{
  int32_t ret;
  uint8_t tmp;

  ret = sx8651_read_reg(ctx, SX8651_CHANMSK_REG, &tmp, 1);

  if (ret == 0)
  {
    tmp &= ~SX8651_CTRL_XCONV_BIT_MASK;
    tmp |= value << SX8651_CTRL_XCONV_BIT_POSITION;

    ret = sx8651_write_reg(ctx, SX8651_CHANMSK_REG, &tmp, 1);
  }

  return ret;
}

/**
  * @brief  Get the SX8651 XCONV
  * @param  ctx Component context
  * @param  value pointer to the value of SX8651_CHANMSK_REG register
  * @retval Component status
  */
int32_t  sx8651_ctrl_xconv_r(sx8651_ctx_t *ctx, uint8_t *value)
{
  int32_t ret;

  ret = sx8651_read_reg(ctx, SX8651_CHANMSK_REG, (uint8_t *)value, 1);

  if (ret == 0)
  {
    *value &= SX8651_CTRL_XCONV_BIT_MASK;
    *value = *value >> SX8651_CTRL_XCONV_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Set the SX8651 YCONV
  * @param  ctx Component context
  * @param  value pointer to the value of SX8651_CHANMSK_REG_REG register
  * @retval Component status
  */
int32_t  sx8651_ctrl_yconv_w(sx8651_ctx_t *ctx, uint8_t value)
{
  int32_t ret;
  uint8_t tmp;

  ret = sx8651_read_reg(ctx, SX8651_CHANMSK_REG, &tmp, 1);

  if (ret == 0)
  {
    tmp &= ~SX8651_CTRL_YCONV_BIT_MASK;
    tmp |= value << SX8651_CTRL_YCONV_BIT_POSITION;

    ret = sx8651_write_reg(ctx, SX8651_CHANMSK_REG, &tmp, 1);
  }

  return ret;
}

/**
  * @brief  Get the SX8651 YCONV
  * @param  ctx Component context
  * @param  value pointer to the value of SX8651_CHANMSK_REG register
  * @retval Component status
  */
int32_t  sx8651_ctrl_yconv_r(sx8651_ctx_t *ctx, uint8_t *value)
{
  int32_t ret;

  ret = sx8651_read_reg(ctx, SX8651_CHANMSK_REG, (uint8_t *)value, 1);

  if (ret == 0)
  {
    *value &= SX8651_CTRL_YCONV_BIT_MASK;
    *value = *value >> SX8651_CTRL_YCONV_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Set the SX8651 Z1CONV
  * @param  ctx Component context
  * @param  value pointer to the value of SX8651_CHANMSK_REG_REG register
  * @retval Component status
  */
int32_t  sx8651_ctrl_z1conv_w(sx8651_ctx_t *ctx, uint8_t value)
{
  int32_t ret;
  uint8_t tmp;

  ret = sx8651_read_reg(ctx, SX8651_CHANMSK_REG, &tmp, 1);

  if (ret == 0)
  {
    tmp &= ~SX8651_CTRL_Z1CONV_BIT_MASK;
    tmp |= value << SX8651_CTRL_Z1CONV_BIT_POSITION;

    ret = sx8651_write_reg(ctx, SX8651_CHANMSK_REG, &tmp, 1);
  }

  return ret;
}

/**
  * @brief  Get the SX8651 Z1CONV
  * @param  ctx Component context
  * @param  value pointer to the value of SX8651_CHANMSK_REG register
  * @retval Component status
  */
int32_t  sx8651_ctrl_z1conv_r(sx8651_ctx_t *ctx, uint8_t *value)
{
  int32_t ret;

  ret = sx8651_read_reg(ctx, SX8651_CHANMSK_REG, (uint8_t *)value, 1);

  if (ret == 0)
  {
    *value &= SX8651_CTRL_Z1CONV_BIT_MASK;
    *value = *value >> SX8651_CTRL_Z1CONV_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Set the SX8651 Z2CONV
  * @param  ctx Component context
  * @param  value pointer to the value of SX8651_CHANMSK_REG_REG register
  * @retval Component status
  */
int32_t  sx8651_ctrl_z2conv_w(sx8651_ctx_t *ctx, uint8_t value)
{
  int32_t ret;
  uint8_t tmp;

  ret = sx8651_read_reg(ctx, SX8651_CHANMSK_REG, &tmp, 1);

  if (ret == 0)
  {
    tmp &= ~SX8651_CTRL_Z2CONV_BIT_MASK;
    tmp |= value << SX8651_CTRL_Z2CONV_BIT_POSITION;

    ret = sx8651_write_reg(ctx, SX8651_CHANMSK_REG, &tmp, 1);
  }

  return ret;
}

/**
  * @brief  Get the SX8651 Z2CONV
  * @param  ctx Component context
  * @param  value pointer to the value of SX8651_CHANMSK_REG register
  * @retval Component status
  */
int32_t  sx8651_ctrl_z2conv_r(sx8651_ctx_t *ctx, uint8_t *value)
{
  int32_t ret;

  ret = sx8651_read_reg(ctx, SX8651_CHANMSK_REG, (uint8_t *)value, 1);

  if (ret == 0)
  {
    *value &= SX8651_CTRL_Z2CONV_BIT_MASK;
    *value = *value >> SX8651_CTRL_Z2CONV_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Set the SX8651 AUXCONV
  * @param  ctx Component context
  * @param  value pointer to the value of SX8651_CHANMSK_REG_REG register
  * @retval Component status
  */
int32_t  sx8651_ctrl_auxconv_w(sx8651_ctx_t *ctx, uint8_t value)
{
  int32_t ret;
  uint8_t tmp;

  ret = sx8651_read_reg(ctx, SX8651_CHANMSK_REG, &tmp, 1);

  if (ret == 0)
  {
    tmp &= ~SX8651_CTRL_AUXCONV_BIT_MASK;
    tmp |= value << SX8651_CTRL_AUXCONV_BIT_POSITION;

    ret = sx8651_write_reg(ctx, SX8651_CHANMSK_REG, &tmp, 1);
  }

  return ret;
}

/**
  * @brief  Get the SX8651 AUXCONV
  * @param  ctx Component context
  * @param  value pointer to the value of SX8651_CHANMSK_REG register
  * @retval Component status
  */
int32_t  sx8651_ctrl_auxconv_r(sx8651_ctx_t *ctx, uint8_t *value)
{
  int32_t ret;

  ret = sx8651_read_reg(ctx, SX8651_CHANMSK_REG, (uint8_t *)value, 1);

  if (ret == 0)
  {
    *value &= SX8651_CTRL_AUXCONV_BIT_MASK;
    *value = *value >> SX8651_CTRL_AUXCONV_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Set the SX8651 RXCONV
  * @param  ctx Component context
  * @param  value pointer to the value of SX8651_CHANMSK_REG_REG register
  * @retval Component status
  */
int32_t  sx8651_ctrl_rxconv_w(sx8651_ctx_t *ctx, uint8_t value)
{
  int32_t ret;
  uint8_t tmp;

  ret = sx8651_read_reg(ctx, SX8651_CHANMSK_REG, &tmp, 1);

  if (ret == 0)
  {
    tmp &= ~SX8651_CTRL_RXCONV_BIT_MASK;
    tmp |= value << SX8651_CTRL_RXCONV_BIT_POSITION;

    ret = sx8651_write_reg(ctx, SX8651_CHANMSK_REG, &tmp, 1);
  }

  return ret;
}

/**
  * @brief  Get the SX8651 RXCONV
  * @param  ctx Component context
  * @param  value pointer to the value of SX8651_CHANMSK_REG register
  * @retval Component status
  */
int32_t  sx8651_ctrl_rxconv_r(sx8651_ctx_t *ctx, uint8_t *value)
{
  int32_t ret;

  ret = sx8651_read_reg(ctx, SX8651_CHANMSK_REG, (uint8_t *)value, 1);

  if (ret == 0)
  {
    *value &= SX8651_CTRL_RXCONV_BIT_MASK;
    *value = *value >> SX8651_CTRL_RXCONV_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Set the SX8651 RYCONV
  * @param  ctx Component context
  * @param  value pointer to the value of SX8651_CHANMSK_REG_REG register
  * @retval Component status
  */
int32_t  sx8651_ctrl_ryconv_w(sx8651_ctx_t *ctx, uint8_t value)
{
  int32_t ret;
  uint8_t tmp;

  ret = sx8651_read_reg(ctx, SX8651_CHANMSK_REG, &tmp, 1);

  if (ret == 0)
  {
    tmp &= ~SX8651_CTRL_RYCONV_BIT_MASK;
    tmp |= value << SX8651_CTRL_RYCONV_BIT_POSITION;

    ret = sx8651_write_reg(ctx, SX8651_CHANMSK_REG, &tmp, 1);
  }

  return ret;
}

/**
  * @brief  Get the SX8651 RYCONV
  * @param  ctx Component context
  * @param  value pointer to the value of SX8651_CHANMSK_REG register
  * @retval Component status
  */
int32_t  sx8651_ctrl_ryconv_r(sx8651_ctx_t *ctx, uint8_t *value)
{
  int32_t ret;

  ret = sx8651_read_reg(ctx, SX8651_CHANMSK_REG, (uint8_t *)value, 1);

  if (ret == 0)
  {
    *value &= SX8651_CTRL_RYCONV_BIT_MASK;
    *value = *value >> SX8651_CTRL_RYCONV_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Get SX8651 end of conversion interrupt CONIRQ
  * @param  ctx Component context
  * @param  value pointer to the value of SX8651_STAT_REG register
  * @retval Component status
  */
int32_t  sx8651_ctrl_convirq(sx8651_ctx_t *ctx, uint8_t *value)
{
  int32_t ret;

  ret = sx8651_read_reg(ctx, SX8651_STAT_REG, (uint8_t *)value, 1);

  if (ret == 0)
  {
    *value &= SX8651_CTRL_CONVIRQ_BIT_MASK;
    *value = *value >> SX8651_CTRL_CONVIRQ_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Get SX8651 pen detect interrupt PENIRQ
  * @param  ctx Component context
  * @param  value pointer to the value of SX8651_STAT_REG register
  * @retval Component status
  */
int32_t  sx8651_ctrl_penirq(sx8651_ctx_t *ctx, uint8_t *value)
{
  int32_t ret;

  ret = sx8651_read_reg(ctx, SX8651_STAT_REG, (uint8_t *)value, 1);

  if (ret == 0)
  {
    *value &= SX8651_CTRL_PENIRQ_BIT_MASK;
    *value = *value >> SX8651_CTRL_PENIRQ_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Set SX8651 Reset register
  * @param  ctx Component context
  * @param  value pointer to the value of SX8651_SOFTRESET_REG register
  * @retval Component status
  */
int32_t  sx8651_ctrl_softreset(sx8651_ctx_t *ctx, uint8_t value)
{
  int32_t ret;
  uint8_t tmp;

  ret = sx8651_read_reg(ctx, SX8651_SOFTRESET_REG, &tmp, 1);

  if (ret == 0)
  {
    tmp &= ~SX8651_CTRL_SOFTRESET_BIT_MASK;
    tmp |= value << SX8651_CTRL_SOFTRESET_BIT_POSITION;

    ret = sx8651_write_reg(ctx, SX8651_SOFTRESET_REG, &tmp, 1);
  }

  return ret;
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
