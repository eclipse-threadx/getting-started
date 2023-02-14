/**
  ******************************************************************************
  * @file    mfxstm32l152_reg.c
  * @author  MCD Application Team
  * @brief   This file provides a set of functions needed to manage the MFXSTM32L152
  *          IO Expander devices.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2018-2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "mfxstm32l152_reg.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Component
  * @{
  */

/** @addtogroup MFXSTM32L152
  * @{
  */

/** @addtogroup MFXSTM32L152_Exported_Functions
  * @{
  */
/*******************************************************************************
  * Function Name : mfxstm32l152_read_reg
  * Description   : Generic Reading function. It must be fulfilled with either
  *                 I2C or SPI reading functions
  * Input         : Register Address, length of buffer
  * Output        : Data Read
  *******************************************************************************/
int32_t mfxstm32l152_read_reg(mfxstm32l152_ctx_t *ctx, uint16_t reg, uint8_t *data, uint16_t length)
{
  return ctx->ReadReg(ctx->handle, reg, data, length);
}

/*******************************************************************************
  * Function Name : mfxstm32l152_write_reg
  * Description   : Generic Writing function. It must be fulfilled with either
  *                 I2C or SPI writing function
  * Input         : Register Address, Data to be written, length of buffer
  * Output        : None
  *******************************************************************************/
int32_t mfxstm32l152_write_reg(mfxstm32l152_ctx_t *ctx, uint16_t reg, uint8_t *data, uint16_t length)
{
  return ctx->WriteReg(ctx->handle, reg, data, length);
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
