/*
 * Copyright 2019, Cypress Semiconductor Corporation or a subsidiary of 
 * Cypress Semiconductor Corporation. All Rights Reserved.
 * 
 * This software, associated documentation and materials ("Software"),
 * is owned by Cypress Semiconductor Corporation
 * or one of its subsidiaries ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products. Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 */

/** @file
 * Declares ISR prototypes for STM32F4xx MCU family
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

extern void NMIException           ( void );  // Non Maskable Interrupt
extern void HardFaultException     ( void );  // Hard Fault interrupt
extern void MemManageException     ( void );  // Memory Management Fault interrupt
extern void BusFaultException      ( void );  // Bus Fault interrupt
extern void UsageFaultException    ( void );  // Usage Fault interrupt
extern void SVC_irq                ( void );  // SVC interrupt
extern void DebugMonitor           ( void );  // Debug Monitor interrupt
extern void PENDSV_irq             ( void );  // PendSV interrupt
extern void SYSTICK_irq            ( void );  // Sys Tick Interrupt
extern void WWDG_irq               ( void );  // Window WatchDog
extern void PVD_irq                ( void );  // PVD through EXTI Line detection
extern void TAMP_STAMP_irq         ( void );  // Tamper and TimeStamps through the EXTI line
extern void RTC_WKUP_irq           ( void );  // RTC Wakeup through the EXTI line
extern void FLASH_irq              ( void );  // FLASH
extern void RCC_irq                ( void );  // RCC
extern void EXTI0_irq              ( void );  // EXTI Line0
extern void EXTI1_irq              ( void );  // EXTI Line1
extern void EXTI2_irq              ( void );  // EXTI Line2
extern void EXTI3_irq              ( void );  // EXTI Line3
extern void EXTI4_irq              ( void );  // EXTI Line4
extern void DMA1_Stream0_irq       ( void );  // DMA1 Stream 0
extern void DMA1_Stream1_irq       ( void );  // DMA1 Stream 1
extern void DMA1_Stream2_irq       ( void );  // DMA1 Stream 2
extern void DMA1_Stream3_irq       ( void );  // DMA1 Stream 3
extern void DMA1_Stream4_irq       ( void );  // DMA1 Stream 4
extern void DMA1_Stream5_irq       ( void );  // DMA1 Stream 5
extern void DMA1_Stream6_irq       ( void );  // DMA1 Stream 6
extern void ADC_irq                ( void );  // ADC1, ADC2 and ADC3s
extern void CAN1_TX_irq            ( void );  // CAN1 TX
extern void CAN1_RX0_irq           ( void );  // CAN1 RX0
extern void CAN1_RX1_irq           ( void );  // CAN1 RX1
extern void CAN1_SCE_irq           ( void );  // CAN1 SCE
extern void EXTI9_5_irq            ( void );  // External Line[9:5]s
extern void TIM1_BRK_TIM9_irq      ( void );  // TIM1 Break and TIM9
extern void TIM1_UP_TIM10_irq      ( void );  // TIM1 Update and TIM10
extern void TIM1_TRG_COM_TIM11_irq ( void );  // TIM1 Trigger and Commutation and TIM11
extern void TIM1_CC_irq            ( void );  // TIM1 Capture Compare
extern void TIM2_irq               ( void );  // TIM2
extern void TIM3_irq               ( void );  // TIM3
extern void TIM4_irq               ( void );  // TIM4
extern void I2C1_EV_irq            ( void );  // I2C1 Event
extern void I2C1_ER_irq            ( void );  // I2C1 Error
extern void I2C2_EV_irq            ( void );  // I2C2 Event
extern void I2C2_ER_irq            ( void );  // I2C2 Error
extern void SPI1_irq               ( void );  // SPI1
extern void SPI2_irq               ( void );  // SPI2
extern void USART1_irq             ( void );  // USART1
extern void USART2_irq             ( void );  // USART2
extern void USART3_irq             ( void );  // USART3
extern void EXTI15_10_irq          ( void );  // External Line[15:10]s
extern void RTC_Alarm_irq          ( void );  // RTC Alarm (A and B) through EXTI Line
extern void OTG_FS_WKUP_irq        ( void );  // USB OTG FS Wakeup through EXTI line
extern void TIM8_BRK_TIM12_irq     ( void );  // TIM8 Break and TIM12
extern void TIM8_UP_TIM13_irq      ( void );  // TIM8 Update and TIM13
extern void TIM8_TRG_COM_TIM14_irq ( void );  // TIM8 Trigger and Commutation and TIM14
extern void TIM8_CC_irq            ( void );  // TIM8 Capture Compare
extern void DMA1_Stream7_irq       ( void );  // DMA1 Stream7
extern void FSMC_irq               ( void );  // FSMC
extern void SDIO_irq               ( void );  // SDIO
extern void TIM5_irq               ( void );  // TIM5
extern void SPI3_irq               ( void );  // SPI3
extern void UART4_irq              ( void );  // UART4
extern void UART5_irq              ( void );  // UART5
extern void TIM6_DAC_irq           ( void );  // TIM6 and DAC1&2 underrun errors
extern void TIM7_irq               ( void );  // TIM7
extern void DMA2_Stream0_irq       ( void );  // DMA2 Stream 0
extern void DMA2_Stream1_irq       ( void );  // DMA2 Stream 1
extern void DMA2_Stream2_irq       ( void );  // DMA2 Stream 2
extern void DMA2_Stream3_irq       ( void );  // DMA2 Stream 3
extern void DMA2_Stream4_irq       ( void );  // DMA2 Stream 4
extern void ETH_irq                ( void );  // Ethernet
extern void ETH_WKUP_irq           ( void );  // Ethernet Wakeup through EXTI line
extern void CAN2_TX_irq            ( void );  // CAN2 TX
extern void CAN2_RX0_irq           ( void );  // CAN2 RX0
extern void CAN2_RX1_irq           ( void );  // CAN2 RX1
extern void CAN2_SCE_irq           ( void );  // CAN2 SCE
extern void OTG_FS_irq             ( void );  // USB OTG FS
extern void DMA2_Stream5_irq       ( void );  // DMA2 Stream 5
extern void DMA2_Stream6_irq       ( void );  // DMA2 Stream 6
extern void DMA2_Stream7_irq       ( void );  // DMA2 Stream 7
extern void USART6_irq             ( void );  // USART6
extern void I2C3_EV_irq            ( void );  // I2C3 event
extern void I2C3_ER_irq            ( void );  // I2C3 error
extern void OTG_HS_EP1_OUT_irq     ( void );  // USB OTG HS End Point 1 Out
extern void OTG_HS_EP1_IN_irq      ( void );  // USB OTG HS End Point 1 In
extern void OTG_HS_WKUP_irq        ( void );  // USB OTG HS Wakeup through EXTI
extern void OTG_HS_irq             ( void );  // USB OTG HS
extern void DCMI_irq               ( void );  // DCMI
extern void CRYP_irq               ( void );  // CRYP crypto
extern void HASH_RNG_irq           ( void );  // Hash and Rng
extern void FPU_irq                ( void );  // FPU
extern void UART7_irq              ( void );  // UART7

#ifdef __cplusplus
} /* extern "C" */
#endif

