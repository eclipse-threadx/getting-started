/**
 ******************************************************************************
 * @file    b_u585i_iot02a_bus.h
 * @author  MCD Application Team
 * @brief   This file contains the common defines and functions prototypes for
 *          the b_u585i_iot02a_bus.c driver.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef B_U585I_IOT02A_BUS_H
#define B_U585I_IOT02A_BUS_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "b_u585i_iot02a_conf.h"

#if defined(BSP_USE_CMSIS_OS)
#include "cmsis_os.h"
#endif /* BSP_USE_CMSIS_OS */
/** @addtogroup BSP
 * @{
 */

/** @addtogroup B_U585I_IOT02A
 * @{
 */

/** @addtogroup B_U585I_IOT02A_BUS
 * @{
 */
/** @defgroup B_U585I_IOT02A_BUS_Exported_Types BUS Exported Types
 * @{
 */
#if (USE_HAL_I2C_REGISTER_CALLBACKS > 0)
    typedef struct
    {
        pI2C_CallbackTypeDef pMspI2cInitCb;
        pI2C_CallbackTypeDef pMspI2cDeInitCb;
    } BSP_I2C_Cb_t;
#endif /* (USE_HAL_I2C_REGISTER_CALLBACKS > 0) */

/**
 * @}
 */
/** @defgroup B_U585I_IOT02A_BUS_Exported_Constants BUS Exported Constants
 * @{
 */
/* Definition for I2C1 clock resources */
#define BUS_I2C1 I2C1

#define BUS_I2C1_CLK_ENABLE()  __HAL_RCC_I2C1_CLK_ENABLE()
#define BUS_I2C1_CLK_DISABLE() __HAL_RCC_I2C1_CLK_DISABLE()

#define BUS_I2C1_SCL_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOB_CLK_ENABLE()
#define BUS_I2C1_SCL_GPIO_CLK_DISABLE() __HAL_RCC_GPIOB_CLK_DISABLE()

#define BUS_I2C1_SDA_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOB_CLK_ENABLE()
#define BUS_I2C1_SDA_GPIO_CLK_DISABLE() __HAL_RCC_GPIOB_CLK_DISABLE()

#define BUS_I2C1_FORCE_RESET()   __HAL_RCC_I2C1_FORCE_RESET()
#define BUS_I2C1_RELEASE_RESET() __HAL_RCC_I2C1_RELEASE_RESET()

/* Definition for I2C1 Pins */
#define BUS_I2C1_SCL_PIN       GPIO_PIN_8
#define BUS_I2C1_SCL_GPIO_PORT GPIOB
#define BUS_I2C1_SCL_AF        GPIO_AF4_I2C1

#define BUS_I2C1_SDA_PIN       GPIO_PIN_9
#define BUS_I2C1_SDA_GPIO_PORT GPIOB
#define BUS_I2C1_SDA_AF        GPIO_AF4_I2C1

#ifndef BUS_I2C1_FREQUENCY
#define BUS_I2C1_FREQUENCY 400000U /* Frequency of I2C1 = 400 KHz*/
#endif                             /* BUS_I2C1_FREQUENCY */

/* Definition for I2C2 clock resources */
#define BUS_I2C2 I2C2

#define BUS_I2C2_CLK_ENABLE()  __HAL_RCC_I2C2_CLK_ENABLE()
#define BUS_I2C2_CLK_DISABLE() __HAL_RCC_I2C2_CLK_DISABLE()

#define BUS_I2C2_SCL_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOH_CLK_ENABLE()
#define BUS_I2C2_SCL_GPIO_CLK_DISABLE() __HAL_RCC_GPIOH_CLK_DISABLE()

#define BUS_I2C2_SDA_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOH_CLK_ENABLE()
#define BUS_I2C2_SDA_GPIO_CLK_DISABLE() __HAL_RCC_GPIOH_CLK_DISABLE()

#define BUS_I2C2_FORCE_RESET()   __HAL_RCC_I2C2_FORCE_RESET()
#define BUS_I2C2_RELEASE_RESET() __HAL_RCC_I2C2_RELEASE_RESET()

/* Definition for I2C2 Pins */
#define BUS_I2C2_SCL_PIN       GPIO_PIN_4
#define BUS_I2C2_SCL_GPIO_PORT GPIOH
#define BUS_I2C2_SCL_AF        GPIO_AF4_I2C2

#define BUS_I2C2_SDA_PIN       GPIO_PIN_5
#define BUS_I2C2_SDA_GPIO_PORT GPIOH
#define BUS_I2C2_SDA_AF        GPIO_AF4_I2C2

#ifndef BUS_I2C2_FREQUENCY
#define BUS_I2C2_FREQUENCY 400000U /* Frequency of I2C2 = 400 KHz*/
#endif                             /* BUS_I2C2_FREQUENCY */

    /**
     * @}
     */

    /** @addtogroup B_U585I_IOT02A_BUS_Exported_Variables
     * @{
     */
    extern I2C_HandleTypeDef hbus_i2c1;
    extern I2C_HandleTypeDef hbus_i2c2;
    /**
     * @}
     */

    /** @addtogroup B_U585I_IOT02A_BUS_Exported_Functions
     * @{
     */
    int32_t BSP_I2C1_Init(void);
    int32_t BSP_I2C1_DeInit(void);
    int32_t BSP_I2C1_WriteReg(uint16_t DevAddr, uint16_t Reg, uint8_t* pData, uint16_t Length);
    int32_t BSP_I2C1_ReadReg(uint16_t DevAddr, uint16_t Reg, uint8_t* pData, uint16_t Length);
    int32_t BSP_I2C1_WriteReg16(uint16_t DevAddr, uint16_t Reg, uint8_t* pData, uint16_t Length);
    int32_t BSP_I2C1_ReadReg16(uint16_t DevAddr, uint16_t Reg, uint8_t* pData, uint16_t Length);
    int32_t BSP_I2C1_Recv(uint16_t DevAddr, uint8_t* pData, uint16_t Length);
    int32_t BSP_I2C1_Send(uint16_t DevAddr, uint8_t* pData, uint16_t Length);
    int32_t BSP_I2C1_IsReady(uint16_t DevAddr, uint32_t Trials);

    int32_t BSP_I2C2_Init(void);
    int32_t BSP_I2C2_DeInit(void);
    int32_t BSP_I2C2_WriteReg(uint16_t DevAddr, uint16_t Reg, uint8_t* pData, uint16_t Length);
    int32_t BSP_I2C2_ReadReg(uint16_t DevAddr, uint16_t Reg, uint8_t* pData, uint16_t Length);
    int32_t BSP_I2C2_WriteReg16(uint16_t DevAddr, uint16_t Reg, uint8_t* pData, uint16_t Length);
    int32_t BSP_I2C2_ReadReg16(uint16_t DevAddr, uint16_t Reg, uint8_t* pData, uint16_t Length);
    int32_t BSP_I2C2_Recv(uint16_t DevAddr, uint8_t* pData, uint16_t Length);
    int32_t BSP_I2C2_Send(uint16_t DevAddr, uint8_t* pData, uint16_t Length);
    int32_t BSP_I2C2_IsReady(uint16_t DevAddr, uint32_t Trials);
    int32_t BSP_GetTick(void);

#if (USE_HAL_I2C_REGISTER_CALLBACKS > 0)
    int32_t BSP_I2C1_RegisterDefaultMspCallbacks(void);
    int32_t BSP_I2C1_RegisterMspCallbacks(BSP_I2C_Cb_t* Callback);
    int32_t BSP_I2C2_RegisterDefaultMspCallbacks(void);
    int32_t BSP_I2C2_RegisterMspCallbacks(BSP_I2C_Cb_t* Callback);
#endif /* USE_HAL_I2C_REGISTER_CALLBACKS */
    __weak HAL_StatusTypeDef MX_I2C1_Init(I2C_HandleTypeDef* hI2c, uint32_t timing);
    __weak HAL_StatusTypeDef MX_I2C2_Init(I2C_HandleTypeDef* hI2c, uint32_t timing);

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

#ifdef __cplusplus
}
#endif

#endif /* B_U585I_IOT02A_BUS_H */
