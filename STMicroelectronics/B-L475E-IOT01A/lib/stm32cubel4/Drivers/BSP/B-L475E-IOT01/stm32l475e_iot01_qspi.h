/**
  ******************************************************************************
  * @file    stm32l475e_iot01_qspi.h
  * @author  MCD Application Team
  * @brief   This file contains the common defines and functions prototypes for
  *          the stm32l475e_iot01_qspi.c driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
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
#ifndef __STM32L475E_IOT01_QSPI_H
#define __STM32L475E_IOT01_QSPI_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"
#include "../Components/mx25r6435f/mx25r6435f.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32L475E_IOT01
  * @{
  */

/** @addtogroup STM32L475E_IOT01_QSPI
  * @{
  */

/* Exported constants --------------------------------------------------------*/ 
/** @defgroup STM32L475E_IOT01_QSPI_Exported_Constants QSPI Exported Constants
  * @{
  */
/* QSPI Error codes */
#define QSPI_OK            ((uint8_t)0x00)
#define QSPI_ERROR         ((uint8_t)0x01)
#define QSPI_BUSY          ((uint8_t)0x02)
#define QSPI_NOT_SUPPORTED ((uint8_t)0x04)
#define QSPI_SUSPENDED     ((uint8_t)0x08)

/**
  * @}
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup STM32L475E_IOT01_QSPI_Exported_Types QSPI Exported Types
  * @{
  */
/* QSPI Info */
typedef struct {
  uint32_t FlashSize;          /*!< Size of the flash */
  uint32_t EraseSectorSize;    /*!< Size of sectors for the erase operation */
  uint32_t EraseSectorsNumber; /*!< Number of sectors for the erase operation */
  uint32_t ProgPageSize;       /*!< Size of pages for the program operation */
  uint32_t ProgPagesNumber;    /*!< Number of pages for the program operation */
} QSPI_Info;

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @defgroup STM32L475E_IOT01_QSPI_Exported_Functions QSPI Exported Functions
  * @{
  */
uint8_t BSP_QSPI_Init                  (void);
uint8_t BSP_QSPI_DeInit                (void);
uint8_t BSP_QSPI_Read                  (uint8_t* pData, uint32_t ReadAddr, uint32_t Size);
uint8_t BSP_QSPI_Write                 (uint8_t* pData, uint32_t WriteAddr, uint32_t Size);
uint8_t BSP_QSPI_Erase_Block           (uint32_t BlockAddress);
uint8_t BSP_QSPI_Erase_Sector          (uint32_t Sector);
uint8_t BSP_QSPI_Erase_Chip            (void);
uint8_t BSP_QSPI_GetStatus             (void);
uint8_t BSP_QSPI_GetInfo               (QSPI_Info* pInfo);
uint8_t BSP_QSPI_EnableMemoryMappedMode(void);
uint8_t BSP_QSPI_SuspendErase          (void);
uint8_t BSP_QSPI_ResumeErase           (void);
uint8_t BSP_QSPI_EnterDeepPowerDown    (void);
uint8_t BSP_QSPI_LeaveDeepPowerDown    (void);

void BSP_QSPI_MspInit(void);
void BSP_QSPI_MspDeInit(void);
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

#endif /* __STM32L475E_IOT01_QSPI_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
