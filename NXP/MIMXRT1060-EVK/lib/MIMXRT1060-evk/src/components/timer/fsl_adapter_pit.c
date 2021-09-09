/*
 * Copyright 2018-2019 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_device_registers.h"
#include "fsl_adapter_timer.h"
#include "fsl_pit.h"

typedef struct _hal_timer_handle_struct_t
{
    uint32_t timeout;
    uint32_t timerClock_Hz;
    hal_timer_callback_t callback;
    void *callbackParam;
    uint8_t instance;
} hal_timer_handle_struct_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/

static PIT_Type *const s_PitBase[] = PIT_BASE_PTRS;

static hal_timer_handle_t s_timerHandle[sizeof(s_PitBase) / sizeof(PIT_Type *)];
/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
static void HAL_TimerInterruptHandle(uint8_t instance)
{
    hal_timer_handle_struct_t *halTimerState = (hal_timer_handle_struct_t *)s_timerHandle[instance];

    PIT_ClearStatusFlags(s_PitBase[halTimerState->instance], kPIT_Chnl_0, PIT_TFLG_TIF_MASK);
    if (halTimerState->callback != NULL)
    {
        halTimerState->callback(halTimerState->callbackParam);
    }
}

#if (defined(FSL_FEATURE_SOC_PIT_COUNT) && (FSL_FEATURE_SOC_PIT_COUNT > 0U))
void PIT_IRQHandler(void);
void PIT_IRQHandler(void)
{
    HAL_TimerInterruptHandle(0);
    SDK_ISR_EXIT_BARRIER;
}
void PIT0_IRQHandler(void);
void PIT0_IRQHandler(void)
{
    HAL_TimerInterruptHandle(0);
    SDK_ISR_EXIT_BARRIER;
}
#endif /* FSL_FEATURE_SOC_PIT_COUNT */

#if (defined(FSL_FEATURE_SOC_PIT_COUNT) && (FSL_FEATURE_SOC_PIT_COUNT > 1U))
void PIT1_IRQHandler(void);
void PIT1_IRQHandler(void)
{
    HAL_TimerInterruptHandle(1);
    SDK_ISR_EXIT_BARRIER;
}
#endif /* FSL_FEATURE_SOC_PIT_COUNT */

#if (defined(FSL_FEATURE_SOC_PIT_COUNT) && (FSL_FEATURE_SOC_PIT_COUNT > 2U))
void PIT2_IRQHandler(void)
{
    HAL_TimerInterruptHandle(2);
    SDK_ISR_EXIT_BARRIER;
}
#endif /* FSL_FEATURE_SOC_PIT_COUNT */

#if (defined(FSL_FEATURE_SOC_PIT_COUNT) && (FSL_FEATURE_SOC_PIT_COUNT > 3U))
void PIT3_IRQHandler(void)
{
    HAL_TimerInterruptHandle(3);
    SDK_ISR_EXIT_BARRIER;
}
#endif /* FSL_FEATURE_SOC_PIT_COUNT */
/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/
hal_timer_status_t HAL_TimerInit(hal_timer_handle_t halTimerHandle, hal_timer_config_t *halTimerConfig)
{
    IRQn_Type instanceIrq[][FSL_FEATURE_PIT_TIMER_COUNT] = PIT_IRQS;
    IRQn_Type irqId;
    hal_timer_handle_struct_t *halTimerState = halTimerHandle;
    /* Structure of initialize PIT */
    pit_config_t pitConfig;

    assert(sizeof(hal_timer_handle_struct_t) == HAL_TIMER_HANDLE_SIZE);
    assert(halTimerConfig);
    assert(halTimerHandle);
    assert(halTimerConfig->instance < (sizeof(s_PitBase) / sizeof(PIT_Type *)));

    halTimerState->timeout  = halTimerConfig->timeout;
    halTimerState->instance = halTimerConfig->instance;
    irqId                   = instanceIrq[halTimerState->instance][0];
    /*
     * pitConfig.enableRunInDebug = false;
     */
    PIT_GetDefaultConfig(&pitConfig);
    assert(s_PitBase[halTimerState->instance]);
    /* Init pit module */
    PIT_Init(s_PitBase[halTimerState->instance], &pitConfig);
    halTimerState->timerClock_Hz = halTimerConfig->srcClock_Hz;
    /* Set timer period for channel 0 */
    PIT_SetTimerPeriod(s_PitBase[halTimerState->instance], kPIT_Chnl_0,
                       (uint32_t)USEC_TO_COUNT(halTimerState->timeout, halTimerState->timerClock_Hz));
    /* Enable timer interrupts for channel 0 */
    PIT_EnableInterrupts(s_PitBase[halTimerState->instance], kPIT_Chnl_0, (uint32_t)kPIT_TimerInterruptEnable);
    s_timerHandle[halTimerState->instance] = halTimerHandle;

    NVIC_SetPriority((IRQn_Type)irqId, HAL_TIMER_ISR_PRIORITY);
    (void)EnableIRQ(irqId);
    return kStatus_HAL_TimerSuccess;
}

/*************************************************************************************/
void HAL_TimerDeinit(hal_timer_handle_t halTimerHandle)
{
    assert(halTimerHandle);
    hal_timer_handle_struct_t *halTimerState = halTimerHandle;
    s_timerHandle[halTimerState->instance]   = NULL;
    PIT_Deinit(s_PitBase[halTimerState->instance]);
}
/*************************************************************************************/
void HAL_TimerEnable(hal_timer_handle_t halTimerHandle)
{
    assert(halTimerHandle);
    hal_timer_handle_struct_t *halTimerState = halTimerHandle;
    PIT_StartTimer(s_PitBase[halTimerState->instance], kPIT_Chnl_0);
}

/*************************************************************************************/
void HAL_TimerDisable(hal_timer_handle_t halTimerHandle)
{
    assert(halTimerHandle);
    hal_timer_handle_struct_t *halTimerState = halTimerHandle;
    PIT_StopTimer(s_PitBase[halTimerState->instance], kPIT_Chnl_0);
}

/*************************************************************************************/
void HAL_TimerInstallCallback(hal_timer_handle_t halTimerHandle, hal_timer_callback_t callback, void *callbackParam)
{
    assert(halTimerHandle);
    hal_timer_handle_struct_t *halTimerState = halTimerHandle;
    halTimerState->callback                  = callback;
    halTimerState->callbackParam             = callbackParam;
}

uint32_t HAL_TimerGetMaxTimeout(hal_timer_handle_t halTimerHandle)
{
    uint64_t reserveCount;
    assert(halTimerHandle);
    hal_timer_handle_struct_t *halTimerState = halTimerHandle;
    reserveCount                             = MSEC_TO_COUNT((4U), (halTimerState->timerClock_Hz));
    if (reserveCount < MSEC_TO_COUNT(1U, halTimerState->timerClock_Hz))
    {
        return 1000U;
    }
    return (uint32_t)COUNT_TO_USEC(((uint64_t)0xFFFFFFFFUL - reserveCount), halTimerState->timerClock_Hz);
}
/* return micro us */
uint32_t HAL_TimerGetCurrentTimerCount(hal_timer_handle_t halTimerHandle)
{
    assert(halTimerHandle);
    hal_timer_handle_struct_t *halTimerState = halTimerHandle;
    return (uint32_t)COUNT_TO_USEC((uint64_t)PIT_GetCurrentTimerCount(s_PitBase[halTimerState->instance], kPIT_Chnl_0),
                                   halTimerState->timerClock_Hz);
}

hal_timer_status_t HAL_TimerUpdateTimeout(hal_timer_handle_t halTimerHandle, uint32_t timeout)
{
    uint32_t tickCount;
    assert(halTimerHandle);
    hal_timer_handle_struct_t *halTimerState = halTimerHandle;
    halTimerState->timeout                   = timeout;
    tickCount = (uint32_t)USEC_TO_COUNT(halTimerState->timeout, halTimerState->timerClock_Hz);
    if ((tickCount < 1U) || (tickCount > 0xfffffff0U))
    {
        return kStatus_HAL_TimerOutOfRanger;
    }
    PIT_SetTimerPeriod(s_PitBase[halTimerState->instance], kPIT_Chnl_0, tickCount);
    return kStatus_HAL_TimerSuccess;
}

void HAL_TimerExitLowpower(hal_timer_handle_t halTimerHandle)
{
    assert(halTimerHandle);
}

void HAL_TimerEnterLowpower(hal_timer_handle_t halTimerHandle)
{
    assert(halTimerHandle);
}
