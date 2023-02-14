/**
  ******************************************************************************
  * @file    sx8651_reg.h
  * @author  MCD Application Team
  * @brief   Header of sx8651_reg.c
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef SX8651_REG_H
#define SX8651_REG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Macros --------------------------------------------------------------------*/

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Component
  * @{
  */

/** @addtogroup SX8651
  * @{
  */

/* Exported types ------------------------------------------------------------*/

/** @defgroup SX8651_Exported_Types SX8651 Exported Types
  * @{
  */
/************** Generic Function  *******************/

typedef int32_t (*SX8651_Write_Func)(void *, uint8_t, uint8_t *, uint16_t);
typedef int32_t (*SX8651_Read_Func)(void *, uint8_t, uint8_t *, uint16_t);
typedef int32_t (*SX8651_ReadBuff_Func)(void *, uint8_t *, uint16_t);
typedef int32_t (*SX8651_WriteCmd_Func)(void *, uint8_t *, uint16_t);

/**
  * @}
  */

/** @defgroup SX8651_Imported_Globals SX8651 Imported Globals
  * @{
  */
typedef struct
{
  SX8651_Write_Func       WriteReg;
  SX8651_Read_Func        ReadReg;
  SX8651_ReadBuff_Func    ReadData;
  SX8651_WriteCmd_Func    WriteCmd;
  void                    *handle;
} sx8651_ctx_t;
/**
  * @}
  */

/** @defgroup SX8651_Exported_Constants SX8651 Exported Constants
  * @{
  */

/* Touch controller registers (W/R) */
#define SX8651_CTRL_0_REG        0x00
#define SX8651_CTRL_1_REG        0x01
#define SX8651_CTRL_2_REG        0x02
#define SX8651_CTRL_3_REG        0x03

/* (W/R) */
#define SX8651_CHANMSK_REG       0x04

/* SX8651 status register (R)*/
#define SX8651_STAT_REG          0x45

/* Software Reset register (W) */
#define SX8651_SOFTRESET_REG     0x1F

/* MANAUTO mode */
#define SX8651_MODE_MANAUTO      0xB0

/* PENTRG mode */
#define SX8651_MODE_PENTRG      0xE0

#define SX8651_MODE_PENDET      0xC0


#define SX8651_P1_XH_TP_BIT_MASK        0xFF
#define SX8651_P1_XL_TP_BIT_MASK        0x0F
#define SX8651_P1_YH_TP_BIT_MASK        0xFF
#define SX8651_P1_YL_TP_BIT_MASK        0x0F


/* Read Address OR value */
#define SX8651_READ_ADDR        0x40

/* command SELECT(CHAN) Address OR value */
#define SX8651_CMD_SELECT       0x80

/* command CONVERT(CHAN) Address OR value */
#define SX8651_CMD_CONVERT      0x90

/* command MANAUTO Address OR value */
#define SX8651_CMD_MANAUTO      0xB0

/* command PENDET Address OR value */
#define SX8651_CMD_PENDET       0xC0

/* command PENTRG Address OR value */
#define SX8651_CMD_PENTRG       0xE0

/* IO expander functionalities */

/* Global Interrupts definitions */
#define SX8651_CONVIRQ          0x80  /* End of conversion sequence interrupt */
#define SX8651_PENIRQ           0x20  /* Pen detected interrupt */
#define SX8651_ALL_GIT          0xA0  /* All global interrupts          */
#define SX8651_TS_IT            (SX8651_CONVIRQ) /* Touch screen interrupts */

/* General Control Registers */
#define SX8651_REG_CTRL0                0x00
#define SX8651_REG_CTRL1                0x01
#define SX8651_REG_CTRL2                0x02
#define SX8651_REG_CTRL3                0x03
#define SX8651_REG_CHAN_MSK             0x04
#define SX8651_REG_SOFT_RESET           0x1F

/* Interrupt system Registers */
#define SX8651_REG_STAT                 0x05

/* IO Pins definition */
#define SX8651_PIN_0                    0x01
#define SX8651_PIN_1                    0x02
#define SX8651_PIN_2                    0x04
#define SX8651_PIN_3                    0x08
#define SX8651_PIN_4                    0x10
#define SX8651_PIN_5                    0x20
#define SX8651_PIN_6                    0x40
#define SX8651_PIN_7                    0x80
#define SX8651_PIN_ALL                  0xFF

/* TS registers masks */
#define SX8651_TS_CTRL_ENABLE         0x01
#define SX8651_TS_CTRL_STATUS         0x80

/* REG_CTRL0 bits */
#define REG_CTRL0_RATE_100CPS           0x60
#define REG_CTRL0_RATE_300CPS           0x80
#define REG_CTRL0_RATE_1KCPS            0xB0
#define REG_CTRL0_POWDLY_36US           0x06
#define REG_CTRL0_POWDLY_140US          0x08
#define REG_CTRL0_POWDLY_1MS            0x0B

/* REG_CTRL1 bits */
#define REG_CTRL1_AUXAQC_ANA            0x00
#define REG_CTRL1_AUXAQC_RISING         0x40
#define REG_CTRL1_AUXAQC_FALLING        0x80
#define REG_CTRL1_AUXAQC_BOTH           0xC0
#define REG_CTRL1_CONDIRQ_ALWAYS        0x00
#define REG_CTRL1_CONDIRQ_DETECTED      0x20
#define REG_CTRL1_RPDNT_100KOHM         0x00
#define REG_CTRL1_RPDNT_200KOHM         0x04
#define REG_CTRL1_RPDNT_50KOHM          0x08
#define REG_CTRL1_RPDNT_25KOHM          0x0C
#define REG_CTRL1_FILT_DISABLE          0x00
#define REG_CTRL1_FILT_3AVERAGE         0x01
#define REG_CTRL1_FILT_5AVERAGE         0x02
#define REG_CTRL1_FILT_7AVERAGE         0x03

/* REG_CTRL2 bits */
#define REG_CTRL2_SETDLY_36US           0x06
#define REG_CTRL2_SETDLY_140US          0x08
#define REG_CTRL2_SETDLY_1MS            0x0B

/* REG_CTRL3 bits */
#define REG_CTRL3_RM_SEL_Y_137OHM         0x00
#define REG_CTRL3_RM_SEL_Y_242OHM         0x08
#define REG_CTRL3_RM_SEL_Y_542OHM         0x10
#define REG_CTRL3_RM_SEL_Y_1327OHM        0x18
#define REG_CTRL3_RM_SEL_Y_2865OHM        0x20
#define REG_CTRL3_RM_SEL_Y_6405OHM        0x28
#define REG_CTRL3_RM_SEL_Y_13KOHM         0x30
#define REG_CTRL3_RM_SEL_Y_40KOHM         0x38
#define REG_CTRL3_RM_SEL_X_137OHM         0x00
#define REG_CTRL3_RM_SEL_X_242OHM         0x01
#define REG_CTRL3_RM_SEL_X_542OHM         0x02
#define REG_CTRL3_RM_SEL_X_1327OHM        0x03
#define REG_CTRL3_RM_SEL_X_2865OHM        0x04
#define REG_CTRL3_RM_SEL_X_6405OHM        0x05
#define REG_CTRL3_RM_SEL_X_13KOHM         0x06
#define REG_CTRL3_RM_SEL_X_40KOHM         0x07

/* REG_CHAN_MSK bits */
#define REG_CHAN_MSK_XCONV              0x80
#define REG_CHAN_MSK_YCONV              0x40
#define REG_CHAN_MSK_Z1CONV             0x20
#define REG_CHAN_MSK_Z2CONV             0x10
#define REG_CHAN_MSK_AUXCONV            0x08
#define REG_CHAN_MSK_RXCONV             0x04
#define REG_CHAN_MSK_RYCONV             0x02

/* REG_STAT bits */
#define REG_STAT_CONVIRQ                0x80
#define REG_STAT_PENIRQ                 0x40

/* Data Channel bits */
#define DATA_CHANNEL_INVALID            0x8000
#define DATA_CHANNEL_MASK               0x7000
#define DATA_CHANNEL_MASK_X             0x0000
#define DATA_CHANNEL_MASK_Y             0x1000
#define DATA_CHANNEL_MASK_SEQ           0x7000
#define DATA_CHANNEL_DATA               0x0FFF
/**
  * @}
  */

/*******************************************************************************
  * Register      : Generic - All
  * Address       : Generic - All
  * Bit Group Name: None
  * Permission    : W
  *******************************************************************************/
int32_t sx8651_write_reg(sx8651_ctx_t *ctx, uint8_t reg, uint8_t *pbuf, uint16_t length);
int32_t sx8651_read_reg(sx8651_ctx_t *ctx, uint8_t reg, uint8_t *pbuf, uint16_t length);
int32_t sx8651_read_data(sx8651_ctx_t *ctx, uint8_t *pdata, uint16_t length);
int32_t sx8651_write_cmd(sx8651_ctx_t *ctx, uint8_t *pdata, uint16_t length);
int32_t sx8651_read_chan(sx8651_ctx_t *ctx, uint8_t chan, uint8_t *pdata, uint16_t length);

/**************** Base Function  *******************/

/*******************************************************************************
  * Register      : CTRL_0
  * Address       : 0X00
  * Bit Group Name: Touch RATE
  * Permission    : RW
  * Default value : 0x00U
  *******************************************************************************/
#define   SX8651_CTRL_RATE_BIT_MASK        0xF0U
#define   SX8651_CTRL_RATE_BIT_POSITION    4
int32_t  sx8651_ctrl_rate_w(sx8651_ctx_t *ctx, uint8_t value);
int32_t  sx8651_ctrl_rate_r(sx8651_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
  * Register      : CTRL_0
  * Address       : 0X00
  * Bit Group Name: Touch POWDLY
  * Permission    : RW
  * Default value : 0x00U
  *******************************************************************************/
#define   SX8651_CTRL_POWDLY_BIT_MASK        0x0FU
#define   SX8651_CTRL_POWDLY_BIT_POSITION    0
int32_t  sx8651_ctrl_powdly_w(sx8651_ctx_t *ctx, uint8_t value);
int32_t  sx8651_ctrl_powdly_r(sx8651_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
  * Register      : CTRL_1
  * Address       : 0X01
  * Bit Group Name: Touch AUXAQC
  * Permission    : RW
  * Default value : 0x0U
  *******************************************************************************/
#define   SX8651_CTRL_AUXAQC_BIT_MASK        0xC0U
#define   SX8651_CTRL_AUXAQC_BIT_POSITION    6
int32_t  sx8651_ctrl_auxaqc_w(sx8651_ctx_t *ctx, uint8_t value);
int32_t  sx8651_ctrl_auxaqc_r(sx8651_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
  * Register      : CTRL_1
  * Address       : 0X01
  * Bit Group Name: Touch CONDIRQ
  * Permission    : RW
  * Default value : 0x1U
  *******************************************************************************/
#define   SX8651_CTRL_CONDIRQ_BIT_MASK        0x20U
#define   SX8651_CTRL_CONDIRQ_BIT_POSITION    5
int32_t  sx8651_ctrl_condirq_w(sx8651_ctx_t *ctx, uint8_t value);
int32_t  sx8651_ctrl_condirq_r(sx8651_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
  * Register      : CTRL_1
  * Address       : 0X01
  * Bit Group Name: Touch RPDNT
  * Permission    : RW
  * Default value : 0x0U
  *******************************************************************************/
#define   SX8651_CTRL_RPDNT_BIT_MASK        0x0CU
#define   SX8651_CTRL_RPDNT_BIT_POSITION    2
int32_t  sx8651_ctrl_rpdnt_w(sx8651_ctx_t *ctx, uint8_t value);
int32_t  sx8651_ctrl_rpdnt_r(sx8651_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
  * Register      : CTRL_1
  * Address       : 0X01
  * Bit Group Name: Touch FILT
  * Permission    : RW
  * Default value : 0x0U
  *******************************************************************************/
#define   SX8651_CTRL_FILT_BIT_MASK        0x03U
#define   SX8651_CTRL_FILT_BIT_POSITION    0
int32_t  sx8651_ctrl_filt_w(sx8651_ctx_t *ctx, uint8_t value);
int32_t  sx8651_ctrl_filt_r(sx8651_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
  * Register      : CTRL_2
  * Address       : 0X02
  * Bit Group Name: Touch SETDLY
  * Permission    : RW
  * Default value : 0x0U
  *******************************************************************************/
#define   SX8651_CTRL_SETDLY_BIT_MASK        0x0FU
#define   SX8651_CTRL_SETDLY_BIT_POSITION    0
int32_t  sx8651_ctrl_setdly_w(sx8651_ctx_t *ctx, uint8_t value);
int32_t  sx8651_ctrl_setdly_r(sx8651_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
  * Register      : CTRL_3
  * Address       : 0X03
  * Bit Group Name: Touch RM_SEL_Y
  * Permission    : RW
  *******************************************************************************/
#define   SX8651_CTRL_RMSEL_Y_BIT_MASK        0x38U
#define   SX8651_CTRL_RMSEL_Y_BIT_POSITION    3
int32_t  sx8651_ctrl_rmsel_y_w(sx8651_ctx_t *ctx, uint8_t value);
int32_t  sx8651_ctrl_rmsel_y_r(sx8651_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
  * Register      : CTRL_3
  * Address       : 0X03
  * Bit Group Name: Touch RM_SEL_X
  * Permission    : RW
  *******************************************************************************/
#define   SX8651_CTRL_RMSEL_X_BIT_MASK        0x07U
#define   SX8651_CTRL_RMSEL_X_BIT_POSITION    0
int32_t  sx8651_ctrl_rmsel_x_w(sx8651_ctx_t *ctx, uint8_t value);
int32_t  sx8651_ctrl_rmsel_x_r(sx8651_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
  * Register      : CHANMSK
  * Address       : 0X04
  * Bit Group Name: Touch XCONV
  * Permission    : RW
  * Default value : 0x1U
  *******************************************************************************/
#define   SX8651_CTRL_XCONV_BIT_MASK        0x80U
#define   SX8651_CTRL_XCONV_BIT_POSITION    7
int32_t  sx8651_ctrl_xconv_w(sx8651_ctx_t *ctx, uint8_t value);
int32_t  sx8651_ctrl_xconv_r(sx8651_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
  * Register      : CHANMSK
  * Address       : 0X04
  * Bit Group Name: Touch YCONV
  * Permission    : RW
  * Default value : 0x1U
  *******************************************************************************/
#define   SX8651_CTRL_YCONV_BIT_MASK        0x40U
#define   SX8651_CTRL_YCONV_BIT_POSITION    6
int32_t  sx8651_ctrl_yconv_w(sx8651_ctx_t *ctx, uint8_t value);
int32_t  sx8651_ctrl_yconv_r(sx8651_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
  * Register      : CHANMSK
  * Address       : 0X04
  * Bit Group Name: Touch Z1CONV
  * Permission    : RW
  * Default value : 0x0U
  *******************************************************************************/
#define   SX8651_CTRL_Z1CONV_BIT_MASK        0x20U
#define   SX8651_CTRL_Z1CONV_BIT_POSITION    5
int32_t  sx8651_ctrl_z1conv_w(sx8651_ctx_t *ctx, uint8_t value);
int32_t  sx8651_ctrl_z1conv_r(sx8651_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
  * Register      : CHANMSK
  * Address       : 0X04
  * Bit Group Name: Touch Z2CONV
  * Permission    : RW
  * Default value : 0x0U
  *******************************************************************************/
#define   SX8651_CTRL_Z2CONV_BIT_MASK        0x10U
#define   SX8651_CTRL_Z2CONV_BIT_POSITION    4
int32_t  sx8651_ctrl_z2conv_w(sx8651_ctx_t *ctx, uint8_t value);
int32_t  sx8651_ctrl_z2conv_r(sx8651_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
  * Register      : CHANMSK
  * Address       : 0X04
  * Bit Group Name: Touch AUXCONV
  * Permission    : RW
  * Default value : 0x0U
  *******************************************************************************/
#define   SX8651_CTRL_AUXCONV_BIT_MASK        0x08U
#define   SX8651_CTRL_AUXCONV_BIT_POSITION    3
int32_t  sx8651_ctrl_auxconv_w(sx8651_ctx_t *ctx, uint8_t value);
int32_t  sx8651_ctrl_auxconv_r(sx8651_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
  * Register      : CHANMSK
  * Address       : 0X04
  * Bit Group Name: Touch RXCONV
  * Permission    : RW
  * Default value : 0x0U
  *******************************************************************************/
#define   SX8651_CTRL_RXCONV_BIT_MASK        0x04U
#define   SX8651_CTRL_RXCONV_BIT_POSITION    2
int32_t  sx8651_ctrl_rxconv_w(sx8651_ctx_t *ctx, uint8_t value);
int32_t  sx8651_ctrl_rxconv_r(sx8651_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
  * Register      : CHANMSK
  * Address       : 0X04
  * Bit Group Name: Touch RYCONV
  * Permission    : RW
  * Default value : 0x0U
  *******************************************************************************/
#define   SX8651_CTRL_RYCONV_BIT_MASK        0x02U
#define   SX8651_CTRL_RYCONV_BIT_POSITION    1
int32_t  sx8651_ctrl_ryconv_w(sx8651_ctx_t *ctx, uint8_t value);
int32_t  sx8651_ctrl_ryconv_r(sx8651_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
  * Register      : STAT
  * Address       : 0X05
  * Bit Group Name: Touch CONVIRQ
  * Permission    : R
  * Default value : 0x0U
  *******************************************************************************/
#define   SX8651_CTRL_CONVIRQ_BIT_MASK        0x80U
#define   SX8651_CTRL_CONVIRQ_BIT_POSITION    7
int32_t  sx8651_ctrl_convirq(sx8651_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
  * Register      : STAT
  * Address       : 0X05
  * Bit Group Name: Touch PENIRQ
  * Permission    : R
  * Default value : 0x0U
  *******************************************************************************/
#define   SX8651_CTRL_PENIRQ_BIT_MASK        0x40U
#define   SX8651_CTRL_PENIRQ_BIT_POSITION    6
int32_t  sx8651_ctrl_penirq(sx8651_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
  * Register      : SOFTRESET
  * Address       : 0X1F
  * Bit Group Name: Touch SOFTRESET
  * Permission    : W
  * Default value : 0x00U
  *******************************************************************************/
#define   SX8651_CTRL_SOFTRESET_BIT_MASK        0xFFU
#define   SX8651_CTRL_SOFTRESET_BIT_POSITION    0
int32_t  sx8651_ctrl_softreset(sx8651_ctx_t *ctx, uint8_t value);

/**
  * @}
  */

#ifdef __cplusplus
}
#endif
#endif /* SX8651_REG_H */


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
