/**
 ******************************************************************************
 * @file    iis2mdc.h
 * @author  MEMS Software Solutions Team
 * @brief   IIS2MDC header driver file
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef IIS2MDC_H
#define IIS2MDC_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "iis2mdc_reg.h"
#include <string.h>

/** @addtogroup BSP BSP
 * @{
 */

/** @addtogroup Component Component
 * @{
 */

/** @addtogroup IIS2MDC IIS2MDC
 * @{
 */

/** @defgroup IIS2MDC_Exported_Types IIS2MDC Exported Types
 * @{
 */

typedef int32_t (*IIS2MDC_Init_Func)(void);
typedef int32_t (*IIS2MDC_DeInit_Func)(void);
typedef int32_t (*IIS2MDC_GetTick_Func)(void);
typedef int32_t (*IIS2MDC_WriteReg_Func)(uint16_t, uint16_t, uint8_t *, uint16_t);
typedef int32_t (*IIS2MDC_ReadReg_Func)(uint16_t, uint16_t, uint8_t *, uint16_t);

typedef struct
{
  IIS2MDC_Init_Func          Init;
  IIS2MDC_DeInit_Func        DeInit;
  uint32_t                   BusType; /* 0 means I2C, 1 means SPI-3-Wires */
  uint8_t                    Address;
  IIS2MDC_WriteReg_Func      WriteReg;
  IIS2MDC_ReadReg_Func       ReadReg;
  IIS2MDC_GetTick_Func       GetTick;
} IIS2MDC_IO_t;


typedef struct
{
  int16_t x;
  int16_t y;
  int16_t z;
} IIS2MDC_AxesRaw_t;

typedef struct
{
  int32_t x;
  int32_t y;
  int32_t z;
} IIS2MDC_Axes_t;

typedef struct
{
  IIS2MDC_IO_t        IO;
  stmdev_ctx_t        Ctx;
  uint8_t             is_initialized;
  uint8_t             mag_is_enabled;
} IIS2MDC_Object_t;

typedef struct
{
  uint8_t   Acc;
  uint8_t   Gyro;
  uint8_t   Magneto;
  uint8_t   LowPower;
  uint32_t  GyroMaxFS;
  uint32_t  AccMaxFS;
  uint32_t  MagMaxFS;
  float     GyroMaxOdr;
  float     AccMaxOdr;
  float     MagMaxOdr;
} IIS2MDC_Capabilities_t;

typedef struct
{
  int32_t (*Init)(IIS2MDC_Object_t *);
  int32_t (*DeInit)(IIS2MDC_Object_t *);
  int32_t (*ReadID)(IIS2MDC_Object_t *, uint8_t *);
  int32_t (*GetCapabilities)(IIS2MDC_Object_t *, IIS2MDC_Capabilities_t *);
} IIS2MDC_CommonDrv_t;

typedef struct
{
  int32_t (*Enable)(IIS2MDC_Object_t *);
  int32_t (*Disable)(IIS2MDC_Object_t *);
  int32_t (*GetSensitivity)(IIS2MDC_Object_t *, float *);
  int32_t (*GetOutputDataRate)(IIS2MDC_Object_t *, float *);
  int32_t (*SetOutputDataRate)(IIS2MDC_Object_t *, float);
  int32_t (*GetFullScale)(IIS2MDC_Object_t *, int32_t *);
  int32_t (*SetFullScale)(IIS2MDC_Object_t *, int32_t);
  int32_t (*GetAxes)(IIS2MDC_Object_t *, IIS2MDC_Axes_t *);
  int32_t (*GetAxesRaw)(IIS2MDC_Object_t *, IIS2MDC_AxesRaw_t *);
} IIS2MDC_MAG_Drv_t;

typedef union{
  int16_t i16bit[3];
  uint8_t u8bit[6];
} iis2mdc_axis3bit16_t;

typedef union{
  int16_t i16bit;
  uint8_t u8bit[2];
} iis2mdc_axis1bit16_t;

typedef union{
  int32_t i32bit[3];
  uint8_t u8bit[12];
} iis2mdc_axis3bit32_t;

typedef union{
  int32_t i32bit;
  uint8_t u8bit[4];
} iis2mdc_axis1bit32_t;

/**
 * @}
 */

/** @defgroup IIS2MDC_Exported_Constants IIS2MDC Exported Constants
 * @{
 */

#define IIS2MDC_OK                     0
#define IIS2MDC_ERROR                 -1

#define IIS2MDC_I2C_BUS               0U
#define IIS2MDC_SPI_4WIRES_BUS        1U /* NOTE: if you use the component in SPI 4-Wires mode, you lose the DRDY pin */
#define IIS2MDC_SPI_3WIRES_BUS        2U

#define IIS2MDC_MAG_SENSITIVITY_FS_50GAUSS  1.500f  /**< Sensitivity value for 50 gauss full scale [mgauss/LSB] */

#ifndef UNUSED
#define UNUSED(X) (void)X
#endif

/**
 * @}
 */

/** @defgroup IIS2MDC_Exported_Functions IIS2MDC Exported Functions
 * @{
 */

int32_t IIS2MDC_RegisterBusIO(IIS2MDC_Object_t *pObj, IIS2MDC_IO_t *pIO);
int32_t IIS2MDC_Init(IIS2MDC_Object_t *pObj);
int32_t IIS2MDC_DeInit(IIS2MDC_Object_t *pObj);
int32_t IIS2MDC_ReadID(IIS2MDC_Object_t *pObj, uint8_t *id);
int32_t IIS2MDC_GetCapabilities(IIS2MDC_Object_t *pObj, IIS2MDC_Capabilities_t *Capabilities);

int32_t IIS2MDC_MAG_Enable(IIS2MDC_Object_t *pObj);
int32_t IIS2MDC_MAG_Disable(IIS2MDC_Object_t *pObj);
int32_t IIS2MDC_MAG_GetSensitivity(IIS2MDC_Object_t *pObj, float *sensitivity);
int32_t IIS2MDC_MAG_GetOutputDataRate(IIS2MDC_Object_t *pObj, float *odr);
int32_t IIS2MDC_MAG_SetOutputDataRate(IIS2MDC_Object_t *pObj, float odr);
int32_t IIS2MDC_MAG_GetFullScale(IIS2MDC_Object_t *pObj, int32_t *fullscale);
int32_t IIS2MDC_MAG_SetFullScale(IIS2MDC_Object_t *pObj, int32_t fullscale);
int32_t IIS2MDC_MAG_GetAxes(IIS2MDC_Object_t *pObj, IIS2MDC_Axes_t *magnetic_field);
int32_t IIS2MDC_MAG_GetAxesRaw(IIS2MDC_Object_t *pObj, IIS2MDC_AxesRaw_t *value);

int32_t IIS2MDC_Read_Reg(IIS2MDC_Object_t *pObj, uint8_t reg, uint8_t *data);
int32_t IIS2MDC_Write_Reg(IIS2MDC_Object_t *pObj, uint8_t reg, uint8_t data);

int32_t IIS2MDC_MAG_Get_DRDY_Status(IIS2MDC_Object_t *pObj, uint8_t *status);
int32_t IIS2MDC_MAG_Get_Init_Status(IIS2MDC_Object_t *pObj, uint8_t *status);

/**
 * @}
 */

/** @addtogroup IIS2MDC_Exported_Variables IIS2MDC Exported Variables
 * @{
 */

extern IIS2MDC_CommonDrv_t IIS2MDC_COMMON_Driver;
extern IIS2MDC_MAG_Drv_t IIS2MDC_MAG_Driver;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif

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
