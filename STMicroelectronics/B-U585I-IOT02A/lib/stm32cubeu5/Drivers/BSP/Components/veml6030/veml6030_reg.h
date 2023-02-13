/*
 ******************************************************************************
 * @file    veml6030_reg.h
 * @author  MCD Application Team
 * @brief   This file contains all the functions prototypes for the
 *          veml6030_reg.c driver.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef VEML6030_REGS_H
#define VEML6030_REGS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <math.h>

/* VEML6030 registers */
#define VEML6030_REG_ALS_CONF            0x00
#define VEML6030_REG_ALS_WH              0x01
#define VEML6030_REG_ALS_WL              0x02
#define VEML6030_REG_POWER_SAVING        0x03
#define VEML6030_REG_ALS                 0x04
#define VEML6030_REG_WHITE               0x05
#define VEML6030_REG_ALS_INT             0x06

/* ALS gain constants */
#define VEML6030_CONF_GAIN_1     (0x00 << 11)
#define VEML6030_CONF_GAIN_2     (0x01 << 11)
#define VEML6030_CONF_GAIN_1_8   (0x02 << 11)
#define VEML6030_CONF_GAIN_1_4   (0x03 << 11)
#define VEML6030_GAIN_MASK     (uint16_t)(0x1800)

/* ALS integration times (ms) */
#define VEML6030_CONF_IT25       (0x0C << 6)
#define VEML6030_CONF_IT50       (0x08 << 6)
#define VEML6030_CONF_IT100      (0x00 << 6)
#define VEML6030_CONF_IT200      (0x01 << 6)
#define VEML6030_CONF_IT400      (0x02 << 6)
#define VEML6030_CONF_IT800      (0x03 << 6)
/* ALS integration times - all bits */
#define VEML6030_CONF_IT_MASK   (0x0f << 6)

/* Register 0x0: ALS_CONF */
/* ALS persistent protect number */
#define VEML6030_CONF_PERS_1     (0x00 << 4)
#define VEML6030_CONF_PERS_2     (0x01 << 4)
#define VEML6030_CONF_PERS_4     (0x02 << 4)
#define VEML6030_CONF_PERS_8     (0x03 << 4)
#define VEML6030_PERS_MASK     (uint16_t)(0x0030)
/* ALS interrupt enable*/
#define VEML6030_CONF_IT_ENABLE  (0x01 << 1)
/* ALS shutdown setting */
#define VEML6030_CONF_SHUTDOWN   0x01

/* Register 0x3: POWER SAVING */
/* Power saving modes */
#define VEML6030_POWER_SAVING_PSM_1   (0x00 << 1)
#define VEML6030_POWER_SAVING_PSM_2   (0x01 << 1)
#define VEML6030_POWER_SAVING_PSM_3   (0x02 << 1)
#define VEML6030_POWER_SAVING_PSM_4   (0x03 << 1)
#define VEML6030_POWER_SAVING_ENABLE  (0x01U)
#define VEML6030_POWER_SAVING_DISABLE (0x00U)

/************** Generic Function  *******************/

typedef int32_t (*VEML6030_Write_Func)(void *, uint16_t, uint8_t *, uint16_t);
typedef int32_t (*VEML6030_Read_Func)(void *, uint16_t, uint8_t *, uint16_t);

typedef struct
{
  VEML6030_Write_Func   WriteReg;
  VEML6030_Read_Func    ReadReg;
  void                *handle;
} veml6030_ctx_t;

/*******************************************************************************
  * Register      : Generic - All
  * Address       : Generic - All
  * Bit Group Name: None
  * Permission    : W
  *******************************************************************************/
int32_t veml6030_write_reg(veml6030_ctx_t *ctx, uint16_t reg, uint16_t *pdata, uint16_t length);
int32_t veml6030_read_reg(veml6030_ctx_t *ctx, uint16_t reg, uint16_t *pdata, uint16_t length);



#ifdef __cplusplus
}
#endif

#endif /*VEML6030_REGS_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
