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
 * STM32F2xx MCU powersave implementation
 */
#include <stdint.h>
#include <string.h>
#include "platform_cmsis.h"
#include "core_cmFunc.h"
#include "platform_init.h"
#include "platform_constants.h"
#include "platform_assert.h"
#include "platform_peripheral.h"
#include "platform_isr_interface.h"
#include "platform_sleep.h"
#include "wwd_rtos_isr.h"
#include "wiced_defaults.h"
#include "wiced_low_power.h"

/******************************************************
 *                      Macros
 ******************************************************/

#define NUMBER_OF_LSE_TICKS_PER_MS( scale_factor ) ( 32768 / 1000 / scale_factor )
#define CONVERT_FROM_TICKS_TO_MS( n, s )           ( n / NUMBER_OF_LSE_TICKS_PER_MS(s) )

/******************************************************
 *                    Constants
 ******************************************************/

#define RTC_INTERRUPT_EXTI_LINE       EXTI_Line22
#define WUT_COUNTER_MAX               0xffff
#define CK_SPRE_CLOCK_SOURCE_SELECTED 0xFFFF

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

#ifndef WICED_DISABLE_MCU_POWERSAVE
static unsigned long     stop_mode_power_down_hook( unsigned long sleep_ms );
static platform_result_t select_wut_prescaler_calculate_wakeup_time( unsigned long* wakeup_time, unsigned long sleep_ms, unsigned long* scale_factor );
#else
static unsigned long  idle_power_down_hook( unsigned long sleep_ms );
#endif

/******************************************************
 *               Variable Definitions
 ******************************************************/

#ifndef WICED_DISABLE_MCU_POWERSAVE
/* Default RTC time. Set to 12:20:30 08/04/2013 Monday */
static const platform_rtc_time_t default_rtc_time =
{
   .sec     = 30,
   .min     = 20,
   .hr      = 12,
   .weekday = 1,
   .date    = 8,
   .month   = 4,
   .year    = 13,
};

static unsigned long rtc_timeout_start_time       = 0;
static int32_t       stm32f2_clock_needed_counter = 0;
#endif /* #ifndef WICED_DISABLE_MCU_POWERSAVE */

/******************************************************
 *               Function Definitions
 ******************************************************/

platform_result_t platform_mcu_powersave_init( void )
{
#ifndef WICED_DISABLE_MCU_POWERSAVE
    EXTI_InitTypeDef EXTI_InitStructure;
    RTC_InitTypeDef  RTC_InitStruct;

    RTC_DeInit( );

    RTC_InitStruct.RTC_HourFormat = RTC_HourFormat_24;

    /* RTC ticks every second */
    RTC_InitStruct.RTC_AsynchPrediv = 0x7F;
    RTC_InitStruct.RTC_SynchPrediv = 0xFF;

    RTC_Init( &RTC_InitStruct );

    /* Enable the PWR clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

    /* RTC clock source configuration ------------------------------------------*/
    /* Allow access to BKP Domain */
    PWR_BackupAccessCmd(ENABLE);

    /* USE LSE if there is an external OSC otherwise use LSI */
#if !defined(STM_LSE_OFF)
    /* Enable the LSE OSC using XTAL */
    RCC_LSEConfig(RCC_LSE_ON);
    /* Wait till LSE is ready */
    while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
    {
    }

    /* Select the RTC Clock Source */
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
#else
    /* Enable the bypass mode to enable using internal clock */
    RCC_LSEConfig(RCC_LSE_OFF);

    /* Select the RTC Clock Source */
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
#endif


    /* Enable the RTC Clock */
    RCC_RTCCLKCmd(ENABLE);

    /* RTC configuration -------------------------------------------------------*/
    /* Wait for RTC APB registers synchronisation */
    RTC_WaitForSynchro();

    RTC_WakeUpCmd( DISABLE );
    EXTI_ClearITPendingBit( RTC_INTERRUPT_EXTI_LINE );
    PWR_ClearFlag(PWR_FLAG_WU);
    RTC_ClearFlag(RTC_FLAG_WUTF);

    RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div2);

    EXTI_ClearITPendingBit( RTC_INTERRUPT_EXTI_LINE );
    EXTI_InitStructure.EXTI_Line    = RTC_INTERRUPT_EXTI_LINE;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_EnableIRQ( RTC_WKUP_IRQn );

    RTC_ITConfig(RTC_IT_WUT, DISABLE);

    /* Prepare Stop-Mode but leave disabled */
//    PWR_ClearFlag(PWR_FLAG_WU);
    PWR->CR  |= PWR_CR_LPDS;
    PWR->CR  &= (unsigned long) ( ~( PWR_CR_PDDS ) );
    SCB->SCR |= ( (unsigned long) SCB_SCR_SLEEPDEEP_Msk );

//#ifdef RTC_ENABLED
    /* application must have wiced_application_default_time structure declared somewhere, otherwise it wont compile */
    /* write default application time inside rtc */
    platform_rtc_set_time( &default_rtc_time );
//#endif /* RTC_ENABLED */

    return PLATFORM_SUCCESS;
#else
    return PLATFORM_FEATURE_DISABLED;
#endif
}

platform_result_t platform_mcu_powersave_disable( void )
{
#ifndef WICED_DISABLE_MCU_POWERSAVE
    WICED_DISABLE_INTERRUPTS();
    if ( stm32f2_clock_needed_counter <= 0 )
    {
        SCB->SCR &= (~((unsigned long)SCB_SCR_SLEEPDEEP_Msk));
        stm32f2_clock_needed_counter = 0;
    }
    stm32f2_clock_needed_counter++;
    WICED_ENABLE_INTERRUPTS();

    return PLATFORM_SUCCESS;
#else
    return PLATFORM_FEATURE_DISABLED;
#endif
}

platform_result_t platform_mcu_powersave_enable( void )
{
#ifndef WICED_DISABLE_MCU_POWERSAVE
    WICED_DISABLE_INTERRUPTS();
    stm32f2_clock_needed_counter--;
    if ( stm32f2_clock_needed_counter <= 0 )
    {
        SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
        stm32f2_clock_needed_counter = 0;
    }
    WICED_ENABLE_INTERRUPTS();

    return PLATFORM_SUCCESS;
#else
    return PLATFORM_FEATURE_DISABLED;
#endif
}

void platform_mcu_powersave_exit_notify( void )
{
}

#ifndef WICED_DISABLE_MCU_POWERSAVE
static platform_result_t select_wut_prescaler_calculate_wakeup_time( unsigned long* wakeup_time, unsigned long sleep_ms, unsigned long* scale_factor )
{
    unsigned long temp;
    wiced_bool_t scale_factor_is_found = WICED_FALSE;
    int i                              = 0;

    static unsigned long int available_wut_prescalers[] =
    {
        RTC_WakeUpClock_RTCCLK_Div2,
        RTC_WakeUpClock_RTCCLK_Div4,
        RTC_WakeUpClock_RTCCLK_Div8,
        RTC_WakeUpClock_RTCCLK_Div16
    };
    static unsigned long scale_factor_values[] = { 2, 4, 8, 16 };

    if ( sleep_ms == 0xFFFFFFFF )
    {
        /* wake up in a 100ms, since currently there may be no tasks to run, but after a few milliseconds */
        /* some of them can get unblocked( for example a task is blocked on mutex with unspecified ) */
        *scale_factor = 2;
        RTC_WakeUpClockConfig( RTC_WakeUpClock_RTCCLK_Div2 );
        *wakeup_time = NUMBER_OF_LSE_TICKS_PER_MS( scale_factor_values[0] ) * 100;
    }
    else
    {
        for ( i = 0; i < 4; i++ )
        {
            temp = NUMBER_OF_LSE_TICKS_PER_MS( scale_factor_values[i] ) * sleep_ms;
            if ( temp < WUT_COUNTER_MAX )
            {
                scale_factor_is_found = WICED_TRUE;
                *wakeup_time = temp;
                *scale_factor = scale_factor_values[i];
                break;
            }
        }
        if ( scale_factor_is_found )
        {
            /* set new prescaler for wakeup timer */
            RTC_WakeUpClockConfig( available_wut_prescalers[i] );
        }
        else
        {
            /* scale factor can not be picked up for delays more that 32 seconds when RTCLK is selected as a clock source for the wakeup timer
             * for delays more than 32 seconds change from RTCCLK to 1Hz ck_spre clock source( used to update calendar registers ) */
            RTC_WakeUpClockConfig( RTC_WakeUpClock_CK_SPRE_16bits );

            /* with 1Hz ck_spre clock source the resolution changes to seconds  */
            *wakeup_time = ( sleep_ms / 1000 ) + 1;
            *scale_factor = CK_SPRE_CLOCK_SOURCE_SELECTED;

            return PLATFORM_ERROR;
        }
    }

    return PLATFORM_SUCCESS;
}
#endif

/******************************************************
 *         IRQ Handlers Definition & Mapping
 ******************************************************/

#ifndef WICED_DISABLE_MCU_POWERSAVE
WWD_RTOS_DEFINE_ISR( rtc_wkup_irq )
{
    EXTI_ClearITPendingBit( RTC_INTERRUPT_EXTI_LINE );
}

WWD_RTOS_MAP_ISR( rtc_wkup_irq, RTC_WKUP_irq )
#endif

/******************************************************
 *               RTOS Powersave Hooks
 ******************************************************/

void platform_idle_hook( void )
{
    __asm("wfi");
}

uint32_t platform_power_down_hook( uint32_t sleep_ms )
{
#ifdef WICED_DISABLE_MCU_POWERSAVE
    /* If MCU powersave feature is disabled, enter idle mode when powerdown hook is called by the RTOS */
    return idle_power_down_hook( sleep_ms );

#else
    /* If MCU powersave feature is enabled, enter STOP mode when powerdown hook is called by the RTOS */
    return stop_mode_power_down_hook( sleep_ms );

#endif
}

#ifdef WICED_DISABLE_MCU_POWERSAVE
/* MCU Powersave is disabled */
static unsigned long idle_power_down_hook( unsigned long sleep_ms  )
{
    UNUSED_PARAMETER( sleep_ms );
    WICED_ENABLE_INTERRUPTS( );
    __asm("wfi");
    return 0;
}

#else
/* MCU Powersave is enabled */
static unsigned long stop_mode_power_down_hook( unsigned long sleep_ms )
{
    uint32_t retval = 0;
    unsigned long wut_ticks_passed;
    unsigned long scale_factor = 0;
    wiced_result_t result;

   /* pick up the appropriate prescaler for a requested delay */
    select_wut_prescaler_calculate_wakeup_time(&rtc_timeout_start_time, sleep_ms, &scale_factor );

    if ( ( ( ( SCB->SCR & (unsigned long)SCB_SCR_SLEEPDEEP_Msk) ) != 0 ) )
    {
        WICED_SLEEP_CALL_EVENT_HANDLERS( WICED_TRUE, WICED_LOW_POWER_SLEEP, WICED_SLEEP_EVENT_ENTER, &result );
        WICED_DISABLE_INTERRUPTS();

        SysTick->CTRL &= (~(SysTick_CTRL_TICKINT_Msk|SysTick_CTRL_ENABLE_Msk)); /* systick IRQ off */
        RTC_ITConfig(RTC_IT_WUT, ENABLE);

        EXTI_ClearITPendingBit( RTC_INTERRUPT_EXTI_LINE );
        PWR_ClearFlag(PWR_FLAG_WU);
        RTC_ClearFlag(RTC_FLAG_WUTF);

        RTC_SetWakeUpCounter( ( uint32_t )rtc_timeout_start_time );
        RTC_WakeUpCmd( ENABLE );
        platform_rtc_enter_powersave();

        DBGMCU->CR |= 0x03; /* Enable debug in stop mode */

        /* This code will be running with BASEPRI register value set to 0, the main intention behind that is that */
        /* all interrupts must be allowed to wake the CPU from the power-down mode */
        /* the PRIMASK is set to 1( see DISABLE_INTERRUPTS), thus we disable all interrupts before entering the power-down mode */
        /* This may sound contradictory, however according to the ARM CM3 documentation power-management unit */
        /* takes into account only the contents of the BASEPRI register and it is an external from the CPU core unit */
        /* PRIMASK register value doesn't affect its operation. */
        /* So, if the interrupt has been triggered just before the wfi instruction */
        /* it remains pending and wfi instruction will be treated as a nop  */
        __asm("wfi");

        /* After CPU exits powerdown mode, the processer will not execute the interrupt handler(PRIMASK is set to 1) */
        /* Disable rtc for now */
        RTC_WakeUpCmd( DISABLE );
        RTC_ITConfig(RTC_IT_WUT, DISABLE);

        /* Initialise the clocks again */
        platform_init_system_clocks( );

        /* Enable CPU ticks */
        SysTick->CTRL |= (SysTick_CTRL_TICKINT_Msk|SysTick_CTRL_ENABLE_Msk);

        /* Get the time of how long the sleep lasted */
        wut_ticks_passed = rtc_timeout_start_time - RTC_GetWakeUpCounter();
        UNUSED_VARIABLE(wut_ticks_passed);
        platform_rtc_exit_powersave( sleep_ms, &retval );
        /* as soon as interrupts are enabled, we will go and execute the interrupt handler */
        /* which triggered a wake up event */
        WICED_ENABLE_INTERRUPTS();
        UNUSED_PARAMETER( result );
        WICED_SLEEP_CALL_EVENT_HANDLERS( WICED_TRUE, WICED_LOW_POWER_SLEEP, WICED_SLEEP_EVENT_LEAVE, &result );

        return ( unsigned long ) retval;
    }
    else
    {
        UNUSED_PARAMETER(wut_ticks_passed);
        WICED_ENABLE_INTERRUPTS();
        __asm("wfi");

        /* Note: We return 0 ticks passed because system tick is still going when wfi instruction gets executed */
        return 0;
    }
}

#endif /* #ifdef WICED_DISABLE_MCU_POWERSAVE */

