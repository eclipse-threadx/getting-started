/*
 * Copyright 2018-2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __TIMERS_MANAGER_H__
#define __TIMERS_MANAGER_H__

#include "fsl_common.h"
#if (defined(COMMON_TASK_ENABLE) && (COMMON_TASK_ENABLE == 0U))
#include "fsl_component_common_task.h"
#endif /* COMMON_TASK_ENABLE */
/*!
 * @addtogroup Timer_Manager
 * @{
 */

/*!
 * @brief The timer manager component
 *
 * The timer manager is built based on the timer adapter component provided by the NXP
 * MCUXpresso SDK. It could provide bellow features:
 * shall support SingleShot,repeater,one minute timer,one second timer and low power mode
 * shall support timer open ,close, start and stop operation, and support callback function install
 * And provide 1ms accuracy timers
 *
 * The timer manager would be used with different HW timer modules like FTM, PIT, LPTMR.
 * But at the same time, only one HW timer module could be used. On different platforms,different
 * HW timer module would be used. For the platforms which have multiple HW timer modules,
 * one HW timer module would be selected as the default, but it is easy to change the default
 * HW timer module to another. Just two steps to switch the HW timer module:
 * 1.Remove the default HW timer module source file from the project
 * 2.Add the expected HW timer module source file to the project.
 * For example, in platform FRDM-K64F, there are two HW timer modules available, FTM and PIT.
 * FTM is used as the default HW timer, so ftm_adapter.c and timer.h is included in the project by
 * default.If PIT is expected to be used as the HW timer, ftm_adapter.c need to be removed from the
 * project and pit_adapter.c should be included in the project
 */
/*****************************************************************************
******************************************************************************
* Public macros
******************************************************************************
*****************************************************************************/
/*
 * @brief   Configures the common task enable.If set to 1, then timer will use common task and consume less ram/flash
 * size.
 */
#ifndef TM_COMMON_TASK_ENABLE
#define TM_COMMON_TASK_ENABLE (0)
#if (defined(COMMON_TASK_ENABLE) && (COMMON_TASK_ENABLE == 0U))
#undef TM_COMMON_TASK_ENABLE
#define TM_COMMON_TASK_ENABLE (0U)
#endif
#endif
/*
 * @brief   Configures the timer task stack size.
 */
#ifndef TM_TASK_STACK_SIZE
#define TM_TASK_STACK_SIZE (600U)
#endif

/*
 * @brief   Configures the timer task priority.
 */
#ifndef TM_TASK_PRIORITY
#define TM_TASK_PRIORITY (4U)
#endif

/*
 * @brief   Enable/Disable Low Power Timer
 * VALID RANGE: TRUE/FALSE
 */
#ifndef TM_ENABLE_LOW_POWER_TIMER
#define TM_ENABLE_LOW_POWER_TIMER (1)
#endif
/*
 * @brief   Enable/Disable TimeStamp
 * VALID RANGE: TRUE/FALSE
 */
#ifndef TM_ENABLE_TIME_STAMP
#define TM_ENABLE_TIME_STAMP (0)
#endif

/*
 * @brief   Enable/Disable clock select
 * This macro just for lptmr timer clock select, if the lptmr does not to use the default clock source.
 */
#ifndef TM_ENABLE_TIMER_CLOCK_SELECT
#define TM_ENABLE_TIMER_CLOCK_SELECT (0)
#endif
/*
 * @brief   Enable/Disable clock select
 * This macro just for lptmr time stamp clock select, if the lptmr does not to use the default clock source.
 */
#ifndef TM_ENABLE_TIME_STAMP_CLOCK_SELECT
#define TM_ENABLE_TIME_STAMP_CLOCK_SELECT (0)
#endif
/*! @brief Definition of timer manager handle size. */
#define TIMER_HANDLE_SIZE (32U)

/*!
 * @brief Defines the timer manager handle
 *
 * This macro is used to define a 4 byte aligned timer manager handle.
 * Then use "(eeprom_handle_t)name" to get the timer manager handle.
 *
 * The macro should be global and could be optional. You could also define timer manager handle by yourself.
 *
 * This is an example,
 * @code
 * TIMER_MANAGER_HANDLE_DEFINE(timerManagerHandle);
 * @endcode
 *
 * @param name The name string of the timer manager handle.
 */
#define TIMER_MANAGER_HANDLE_DEFINE(name) uint32_t name[(TIMER_HANDLE_SIZE + sizeof(uint32_t) - 1U) / sizeof(uint32_t)]

/*****************************************************************************
******************************************************************************
* Public type definitions
******************************************************************************
*****************************************************************************/
/**@brief Timer status. */
typedef enum _timer_status
{
    kStatus_TimerSuccess    = kStatus_Success,                           /*!< Success */
    kStatus_TimerInvalidId  = MAKE_STATUS(kStatusGroup_TIMERMANAGER, 1), /*!< Invalid Id */
    kStatus_TimerNotSupport = MAKE_STATUS(kStatusGroup_TIMERMANAGER, 2), /*!< Not Support */
    kStatus_TimerOutOfRange = MAKE_STATUS(kStatusGroup_TIMERMANAGER, 3), /*!< Out Of Range */
    kStatus_TimerError      = MAKE_STATUS(kStatusGroup_TIMERMANAGER, 4), /*!< Fail */
} timer_status_t;

/**@brief Timer modes. */
typedef enum _timer_mode
{
    kTimerModeSingleShot     = 0x01U, /**< The timer will expire only once. */
    kTimerModeIntervalTimer  = 0x02U, /**< The timer will restart each time it expires. */
    kTimerModeSetMinuteTimer = 0x04U, /**< The timer will one minute timer. */
    kTimerModeSetSecondTimer = 0x08U, /**< The timer will one second timer. */
    kTimerModeLowPowerTimer  = 0x10U, /**< The timer will low power mode timer. */
} timer_mode_t;

/**@brief Timer config. */
typedef struct _timer_config
{
    uint32_t srcClock_Hz; /**< The timer source clock frequency. */
    uint8_t instance;     /*!< Hardware timer module instance, for example: if you want use FTM0,then the instance
                               is configured to 0, if you want use FTM2 hardware timer, then configure the instance
                               to 2, detail information please refer to the SOC corresponding RM. Invalid instance
                               value will cause initialization failure. */
#if (defined(TM_ENABLE_TIMER_CLOCK_SELECT) && (TM_ENABLE_TIMER_CLOCK_SELECT > 0U))
    uint8_t clockSrcSelect; /*!< Select clock source. It is just for lptmr timer clock select, if the lptmr does not
                                 to use the default clock source*/
#endif
#if (defined(TM_ENABLE_TIME_STAMP) && (TM_ENABLE_TIME_STAMP > 0U))
    uint32_t timeStampSrcClock_Hz; /**< The timer stamp source clock frequency. */
    uint8_t timeStampInstance;     /**< Hardware timer module instance. This instance for time stamp */
#if (defined(TM_ENABLE_TIME_STAMP_CLOCK_SELECT) && (TM_ENABLE_TIME_STAMP_CLOCK_SELECT > 0U))
    uint8_t timeStampClockSrcSelect; /*!< Select clock source. It is just for lptmr timer clock select, if the lptmr
                                        does not to use the default clock source*/
#endif
#endif
} timer_config_t;

/*
 * @brief   Timer handle
 */
typedef void *timer_handle_t;

/*
 * @brief   Timer callback fiction
 */
typedef void (*timer_callback_t)(void *param);

/*
 * \brief   Converts the macro argument from seconds to microseconds
 */
#define TmSecondsToMicroseconds(n) ((uint64_t)((n)*1000000UL))

/*
 * \brief   Converts the macro argument from seconds to milliseconds
 */
#define TmSecondsToMilliseconds(n) ((uint32_t)((n)*1000UL))

/*
 * \brief   Converts the macro argument from microseconds to seconds
 */
#define TmMicrosecondsToSeconds(n) (((n) + 500000U) / 1000000U)
/*****************************************************************************
******************************************************************************
* Public memory declarations
******************************************************************************
*****************************************************************************/

/*****************************************************************************
******************************************************************************
* Public prototypes
******************************************************************************
*****************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif /* _cplusplus */

/*!
 * @brief Initializes timer manager module with the user configuration structure.
 *
 * For Initializes timer manager,
 *  @code
 *  timer_config_t timerConfig;
 *  timerConfig.instance = 0;
 *  timerConfig.srcClock_Hz = BOARD_GetTimerSrcClock();
 *  TM_Init(&timerConfig);
 *  @endcode
 *
 * @param timerConfig              Pointer to user-defined timer configuration structure.
 * @retval kStatus_TimerSuccess    Timer manager initialization succeed.
 * @retval kStatus_TimerError      An error occurred.
 */
timer_status_t TM_Init(timer_config_t *timerConfig);

/*!
 * @brief Deinitialize timer manager module.
 *
 */
void TM_Deinit(void);

/*!
 * @brief Power up timer manager module.
 *
 */
void TM_ExitLowpower(void);

/*!
 * @brief Power down timer manager module.
 *
 */
void TM_EnterLowpower(void);

/*!
 * @brief Open a timer with user handle.
 *
 * @param timerHandle              Pointer to a memory space of size #TIMER_HANDLE_SIZE allocated by the caller.
 * The handle should be 4 byte aligned, because unaligned access doesn't be supported on some devices.
 * You can define the handle in the following two ways:
 * #TIMER_MANAGER_HANDLE_DEFINE(timerHandle);
 * or
 * uint32_t timerHandle[((TIMER_HANDLE_SIZE + sizeof(uint32_t) - 1U) / sizeof(uint32_t))];
 * @retval kStatus_TimerSuccess    Timer open succeed.
 * @retval kStatus_TimerError      An error occurred.
 */
timer_status_t TM_Open(timer_handle_t timerHandle);

/*!
 * @brief Close a timer with user handle.
 *
 * @param timerHandle              the handle of the timer
 *
 * @retval kStatus_TimerSuccess    Timer close succeed.
 * @retval kStatus_TimerError      An error occurred.
 */
timer_status_t TM_Close(timer_handle_t timerHandle);

/*!
 * @brief  Install a specified timer callback
 *
 * @param timerHandle     the handle of the timer
 * @param callback        callback function
 * @param callbackParam   parameter to callback function
 *
 * @retval kStatus_TimerSuccess   Timer install callback succeed.
 *
 */
timer_status_t TM_InstallCallback(timer_handle_t timerHandle, timer_callback_t callback, void *callbackParam);

/*!
 * @brief  Start a specified timer
 *
 * @param timerHandle    the handle of the timer
 * @param timerType       The mode of the timer, for example: kTimerModeSingleShot for the timer will expire
 *                       only once, kTimerModeIntervalTimer, the timer will restart each time it expires.
 *                       If low power mode is used at the same time. It should be set like this: kTimerModeSingleShot |
 * kTimerModeLowPowerTimer
 * @param timerTimeout   The timer timeout in milliseconds unit for kTimerModeSingleShot, kTimerModeIntervalTimer
 *                       and kTimerModeLowPowerTimer,if kTimerModeSetMinuteTimer timeout for minutes unit, if
 *                       kTimerModeSetSecondTimer the timeout for seconds unit.
 *
 * @retval kStatus_TimerSuccess    Timer start succeed.
 * @retval kStatus_TimerError      An error occurred.
 */
timer_status_t TM_Start(timer_handle_t timerHandle, uint8_t timerType, uint32_t timerTimeout);

/*!
 * @brief  Stop a specified timer
 *
 * @param timerHandle         the handle of the timer
 *
 * @retval kStatus_TimerSuccess    Timer stop succeed.
 * @retval kStatus_TimerError      An error occurred.
 */
timer_status_t TM_Stop(timer_handle_t timerHandle);

/*!
 * @brief  Check if a specified timer is active
 *
 * @param timerHandle    the handle of the timer
 *
 * @retval return 1 if timer is active, return 0 if timer is not active.
 */
uint8_t TM_IsTimerActive(timer_handle_t timerHandle);

/*!
 * @brief  Check if a specified timer is ready
 *
 * @param timerHandle     the handle of the timer
 *
 * @retval return 1 if timer is ready, return 0 if timer is not ready.
 */
uint8_t TM_IsTimerReady(timer_handle_t timerHandle);

/*!
 * @brief  Returns the remaining time until timeout
 *
 * @param timerHandle       the handle of the timer
 *
 * @retval remaining time in microseconds until first timer timeouts.
 */
uint32_t TM_GetRemainingTime(timer_handle_t timerHandle);

/*!
 * @brief Get the first expire time of timer
 *
 * @param timerType  The mode of the timer, for example: kTimerModeSingleShot for the timer will expire
 *                   only once, kTimerModeIntervalTimer, the timer will restart each time it expires.
 *
 * @retval return the first expire time of all timer.
 */
uint32_t TM_GetFirstExpireTime(uint8_t timerType);

/*!
 * @brief Returns the handle of the timer of the first allocated timer that has the
 *        specified parameter.
 *
 * @param param       specified parameter of timer
 *
 * @retval return the handle of the timer if success.
 */
timer_handle_t TM_GetFirstTimerWithParam(void *param);

/*!
 * @brief  Check if all timers except the LP timers are OFF
 *
 *
 * @retval return 1 there are no active non-low power timers, 0 otherwise.
 */
uint8_t TM_AreAllTimersOff(void);

/*!
 * @brief Returns not counted time before system entering in sleep, This function is called
 *        by Low Power module.
 *
 * @retval return microseconds that wasn't counted before entering in sleep.
 */
uint32_t TM_NotCountedTimeBeforeSleep(void);

/*!
 * @brief Sync low power timer in sleep mode, This function is called by Low Power module;
 *
 * @param sleepDurationTmrUs    sleep duration in microseconds unit
 *
 */
void TM_SyncLpmTimers(uint32_t sleepDurationTmrUs);

/*!
 * @brief Make timer task ready after wakeup from lowpower mode, This function is called
 *        by Low Power module;
 *
 */
void TM_MakeTimerTaskReady(void);

/*!
 * @brief Get a time-stamp value
 *
 */
uint64_t TM_GetTimestamp(void);

#if defined(__cplusplus)
}
#endif
/*! @}*/
#endif /* #ifndef __TIMERS_MANAGER_H__ */
