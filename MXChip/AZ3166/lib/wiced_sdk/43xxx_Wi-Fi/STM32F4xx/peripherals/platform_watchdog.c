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
 *
 * Debugging Watchdog Reset
 *
 * 1. If a watchdog reset occured previously, a warning message will be printed to the console
 * 2. To obtain useful information for debugging, run WICED with JTAG debug mode and attempt to reproduce the issue
 * 3. While running in JTAG debug mode, WICED starts a shadow watchdog timer.
 *    - It utilises STM32F2 TIM7.
 *    - It's expiry time is set 90% of that of the actual watchdog.
 *    - It also get reloaded together with the actual watchdog when the watchdog kick function is called.
 *    - When the problem is reproduced, a break point is generated from the shadow watchdog ISR - shadow_watchdog_irq()
 * 4. If the code breaks in shadow_watchdog_irq(), it means that the independent watchdog is about to bite.
 *    - Observe the Debug view and examine where the software gets stuck and why the watchdog wasn't kicked.
 *    - Click "Resume" to continue and let the actual watchdog take effect.
 */

#include "platform_cmsis.h"
#include "platform_constants.h"
#include "platform_peripheral.h"
#include "platform_stdio.h"
#include "platform_isr.h"
#include "platform_isr_interface.h"
#include "platform_assert.h"
#include "wwd_assert.h"
#include "wwd_rtos.h"
#include "wiced_defaults.h"
#include "platform_config.h" /* For CPU_CLOCK_HZ */

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define WATCHDOG_PRESCALER              (IWDG_Prescaler_256)
#define WATCHDOG_TIMEOUT_MULTIPLIER     (184)

#if (CPU_CLOCK_HZ == 120000000)
    #define DBG_WATCHDOG_TIMEOUT_MULTIPLIER    (2250)
#elif (CPU_CLOCK_HZ == 100000000)
    #define DBG_WATCHDOG_TIMEOUT_MULTIPLIER    (1875)
#elif (CPU_CLOCK_HZ == 96000000)
    #define DBG_WATCHDOG_TIMEOUT_MULTIPLIER    (3600)

#elif !defined(DBG_WATCHDOG_TIMEOUT_MULTIPLIER)
    #error DBG_WATCHDOG_TIMEOUT_MULTIPLIER must be manually defined for this platform
#endif

#define DBG_WATCHDOG_PRESCALER          (24000)

#ifdef APPLICATION_WATCHDOG_TIMEOUT_SECONDS
#define WATCHDOG_TIMEOUT                (APPLICATION_WATCHDOG_TIMEOUT_SECONDS * WATCHDOG_TIMEOUT_MULTIPLIER)
#define DBG_WATCHDOG_TIMEOUT            (((APPLICATION_WATCHDOG_TIMEOUT_SECONDS * DBG_WATCHDOG_TIMEOUT_MULTIPLIER)>0xFFFF)?0xFFFF:(APPLICATION_WATCHDOG_TIMEOUT_SECONDS * DBG_WATCHDOG_TIMEOUT_MULTIPLIER))
#else
#define WATCHDOG_TIMEOUT                (MAX_WATCHDOG_TIMEOUT_SECONDS * WATCHDOG_TIMEOUT_MULTIPLIER)
#define DBG_WATCHDOG_TIMEOUT            (((MAX_WATCHDOG_TIMEOUT_SECONDS * DBG_WATCHDOG_TIMEOUT_MULTIPLIER)>0xFFFF)?0xFFFF:(MAX_WATCHDOG_TIMEOUT_SECONDS * DBG_WATCHDOG_TIMEOUT_MULTIPLIER))
#endif /* APPLICATION_WATCHDOG_TIMEOUT_SECONDS */

#if (defined(APPLICATION_WATCHDOG_TIMEOUT_SECONDS) && (APPLICATION_WATCHDOG_TIMEOUT_SECONDS > MAX_WATCHDOG_TIMEOUT_SECONDS))
#error APPLICATION_WATCHDOG_TIMEOUT_SECONDS must NOT be larger than 22 seconds
#endif

/* It is possible to define the set of DBG_WATCHDOG_STM32_TIMER constants from outside this file
 * By default we use TIM7 however on STM32F401 and STM32F411  and the STM32F412 platforms we use TIM4 */
#ifndef DBG_WATCHDOG_STM32_TIMER
#if !defined(STM32F401xx) && !defined(STM32F411xE) && !defined(STM32F412xG)
    #define DBG_WATCHDOG_STM32_TIMER               TIM7
    #define DBG_WATCHDOG_STM32_TIMER_PERIPHERAL    RCC_APB1Periph_TIM7
    #define DBG_WATCHDOG_STM32_TIMER_IRQN          TIM7_IRQn
    #define DBG_WATCHDOG_STM32_TIMER_IRQ           TIM7_irq
#else
    #define DBG_WATCHDOG_STM32_TIMER               TIM4
    #define DBG_WATCHDOG_STM32_TIMER_PERIPHERAL    RCC_APB1Periph_TIM4
    #define DBG_WATCHDOG_STM32_TIMER_IRQN          TIM4_IRQn
    #define DBG_WATCHDOG_STM32_TIMER_IRQ           TIM4_irq
#endif
#endif

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
 *               Static Function Declarations
 ******************************************************/

#ifndef WICED_DISABLE_WATCHDOG
static void init_dbg_watchdog  ( void );
static void reload_dbg_watchdog( void );
#endif

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

platform_result_t platform_watchdog_init( void )
{
#ifndef WICED_DISABLE_WATCHDOG
    /* Allow writign to the Watchdog registers */
    IWDG_WriteAccessCmd( IWDG_WriteAccess_Enable );

    /* Watchdog frequency calculations
     *
     * LSI : min 17kHz, typ 32kHz, max 47kHz
     *
     * Set prescaler to divide by 256
     *
     * Watchdog count freq: min 66.4Hz, typ 125Hz, max 183.6Hz
     *
     * Reload value maximum = 4095,
     * so minimum time to reset with this value is 22.3 seconds
     *
     */
    /* Set the Watchdog prescaler to LSI/256 */
    IWDG_SetPrescaler(WATCHDOG_PRESCALER);

    /* Set the reload value to obtain the requested minimum time to reset */
    IWDG_SetReload( (uint16_t)WATCHDOG_TIMEOUT );

    /* Start the watchdog */
    IWDG_ReloadCounter();
    IWDG_Enable();

    /* shadow watchdog for debugging lockup issue. Enabled only during debugging */
    if ( DBGMCU->APB1FZ & DBGMCU_APB1_FZ_DBG_IWDEG_STOP )
    {
        init_dbg_watchdog();
    }

    return PLATFORM_SUCCESS;
#else
    return PLATFORM_FEATURE_DISABLED;
#endif
}

platform_result_t platform_watchdog_kick( void )
{
#ifndef WICED_DISABLE_WATCHDOG
    /* Reload IWDG counter */
    IWDG_ReloadCounter( );

    if ( DBGMCU->APB1FZ & DBGMCU_APB1_FZ_DBG_IWDEG_STOP )
    {
        reload_dbg_watchdog();
    }

    return PLATFORM_SUCCESS;
#else
    return PLATFORM_FEATURE_DISABLED;
#endif
}

wiced_bool_t platform_watchdog_check_last_reset( void )
{
#ifndef WICED_DISABLE_WATCHDOG
    if ( RCC->CSR & RCC_CSR_WDGRSTF )
    {
        /* Clear the flag and return */
        RCC->CSR |= RCC_CSR_RMVF;
        return WICED_TRUE;
    }
#endif

    return WICED_FALSE;
}

#ifndef WICED_DISABLE_WATCHDOG
static void init_dbg_watchdog( void )
{
    TIM_TimeBaseInitTypeDef tim_time_base_init_struct;

    RCC_APB1PeriphClockCmd( DBG_WATCHDOG_STM32_TIMER_PERIPHERAL, ENABLE  );
    RCC_APB1PeriphResetCmd( DBG_WATCHDOG_STM32_TIMER_PERIPHERAL, DISABLE );

    /* Set dbg_watchdog timeout to 90% of the actual watchdog timeout to ensure it break before the actual watchdog bites
     * Timeout calculation
     * - Period per TIM clock cycle : 120MHz (CPU clock) / 2 (APB1 pre-scaler) / DBG_WATCHDOG_PRESCALER = 2.5ms
     *                                DBG_WATCHDOG_PRESCALER = 24000
     * - Timeout                    : ( 0.9 * 2.5ms * 1000 ) * timeout_in_seconds
     *                                DBG_WATCHDOG_TIMEOUT_MULTIPLIER =  ( 0.9 * 2.5ms * 1000 ) = 2250
     */
    tim_time_base_init_struct.TIM_Prescaler         = DBG_WATCHDOG_PRESCALER;
    tim_time_base_init_struct.TIM_CounterMode       = TIM_CounterMode_Up;
    tim_time_base_init_struct.TIM_Period            = DBG_WATCHDOG_TIMEOUT;
    tim_time_base_init_struct.TIM_ClockDivision     = TIM_CKD_DIV1;
    tim_time_base_init_struct.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit( DBG_WATCHDOG_STM32_TIMER, &tim_time_base_init_struct );

    TIM_ClearITPendingBit( DBG_WATCHDOG_STM32_TIMER, TIM_IT_Update );
    TIM_ITConfig( DBG_WATCHDOG_STM32_TIMER, TIM_IT_Update, ENABLE );

    TIM_UpdateRequestConfig( DBG_WATCHDOG_STM32_TIMER, TIM_UpdateSource_Regular );

    NVIC_EnableIRQ( DBG_WATCHDOG_STM32_TIMER_IRQN );

    TIM_Cmd( DBG_WATCHDOG_STM32_TIMER, ENABLE );
}

static void reload_dbg_watchdog( void )
{
    TIM_TimeBaseInitTypeDef tim_time_base_init_struct;

    TIM_Cmd( DBG_WATCHDOG_STM32_TIMER, DISABLE );

    tim_time_base_init_struct.TIM_Prescaler         = DBG_WATCHDOG_PRESCALER;
    tim_time_base_init_struct.TIM_CounterMode       = TIM_CounterMode_Up;
    tim_time_base_init_struct.TIM_Period            = DBG_WATCHDOG_TIMEOUT;
    tim_time_base_init_struct.TIM_ClockDivision     = TIM_CKD_DIV1;
    tim_time_base_init_struct.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit( DBG_WATCHDOG_STM32_TIMER, &tim_time_base_init_struct );

    TIM_Cmd( DBG_WATCHDOG_STM32_TIMER, ENABLE );
}
#endif /* ifndef WICED_DISABLE_WATCHDOG */

/******************************************************
 *             IRQ Handlers Definition
 ******************************************************/

PLATFORM_DEFINE_ISR( dbg_watchdog_irq )
{
    /* If the code breaks here, it means that the independent watchdog is about to bite.
     * Observe the Debug view and examine where the software gets stuck and why
     * the watchdog wasn't kicked.
     * Click "Resume" to continue and let the actual watchdog take effect.
     */
    DBG_WATCHDOG_STM32_TIMER->SR = (uint16_t)~TIM_IT_Update;
    WICED_TRIGGER_BREAKPOINT( );
}

/******************************************************
 *               IRQ Handlers Mapping
 ******************************************************/

PLATFORM_MAP_ISR( dbg_watchdog_irq, DBG_WATCHDOG_STM32_TIMER_IRQ )

