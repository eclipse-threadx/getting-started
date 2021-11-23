/**********************************************************************************************************************
 * DISCLAIMER
 * This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
 * other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
 * applicable laws, including copyright laws.
 * THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
 * THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
 * EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
 * SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO
 * THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
 * this software. By using this software, you agree to the additional terms and conditions found by accessing the
 * following link:
 * http://www.renesas.com/disclaimer
 *
 * Copyright (C) 2021 Renesas Electronics Corporation. All rights reserved.
 *********************************************************************************************************************/
/**********************************************************************************************************************
 * File Name    : r_wifi_sx_ulpgn_os_wrap.h
 * Version      : 1.0
 * Description  : RTOS wrap functions definition.
 *********************************************************************************************************************/
/**********************************************************************************************************************
 * History : DD.MM.YYYY Version  Description
 *         : DD.MM.2021 1.00     First Release
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Includes   <System Includes> , "Project Includes"
 *********************************************************************************************************************/
#include <stdio.h>
#include <string.h>

#if defined(__CCRX__) || defined(__ICCRX__) || defined(__RX__)
#include "platform.h"
#endif

#if BSP_CFG_RTOS_USED == 1        /* FreeRTOS is used.   */
#include "FreeRTOS.h"
#elif BSP_CFG_RTOS_USED == 5      /* Azure RTOS is used. */
#include "tx_api.h"
#include "tx_mutex.h"
#include "tx_thread.h"
#include "tx_semaphore.h"
#else
#error  "Error - BSP_CFG_RTOS_USED is not 1(FreeRTOS) or 5(AzureRTOS) !"
#endif /* BSP_CFG_RTOS_USED */

/**********************************************************************************************************************
 Macro definitions
 *********************************************************************************************************************/
#ifndef R_WIFI_SX_ULPGN_OS_WRAP_H
#define R_WIFI_SX_ULPGN_OS_WRAP_H

#if BSP_CFG_RTOS_USED == 1        /* FreeRTOS is used.   */
#define OS_WRAP_MS_TO_TICKS(ms)    pdMS_TO_TICKS(ms)       /* ms -> tick count            */
#elif BSP_CFG_RTOS_USED == 5      /* Azure RTOS is used. */
#define TICK_VALUE                 (1000 / TX_TIMER_TICKS_PER_SECOND)
#define OS_WRAP_MS_TO_TICKS(ms)    (ms < TICK_VALUE ? 1 : ms / TICK_VALUE) /* ms -> tick count */
#endif /* BSP_CFG_RTOS_USED */

#if BSP_CFG_RTOS_USED == 1        /* FreeRTOS is used.   */
typedef SemaphoreHandle_t  OS_MUTEX;            /* mutex              */
typedef TimerHandle_t      OS_TIMER_HANDLE;     /* handle for timer   */
typedef TickType_t         OS_TICK;             /* ticks */
#elif BSP_CFG_RTOS_USED == 5      /* Azure RTOS is used. */
typedef TX_MUTEX           OS_MUTEX;            /* mutex              */
typedef TX_TIMER           OS_TIMER_HANDLE;     /* handle for timer   */
typedef ULONG              OS_TICK;             /* ticks */
#endif /* BSP_CFG_RTOS_USED */

/**********************************************************************************************************************
 Global Typedef definitions
 *********************************************************************************************************************/
typedef enum {
    OS_WRAP_SUCCESS = 0,
    OS_WRAP_OTHER_ERR
} e_os_return_code_t;

typedef enum
{
    UNIT_TICK = 0,
    UNIT_MSEC
} e_timer_unit_t;

typedef enum
{
    CALL_ISR = 0,
    CALL_NOT_ISR
} e_call_from_isr_t;

typedef enum
{
    TICK_NOT_EXPIERD = 0,
    TICK_EXPIERD
} e_tick_expired_t;

/**********************************************************************************************************************
 External global variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Exported global functions
 *********************************************************************************************************************/
/*
 * Mutex functions
 */
/**********************************************************************************************************************
 * Function Name: os_wrap_mutex_create
 * Description  : This function creates the specified mutex.
 * Arguments    : p_mutex      - Pointer to mutex control block
 *                p_mutex_name - Pointer to mutex name
 * Return Value : OS_WRAP_SUCCESS
 *                OS_WRAP_OTHER_ERR
 *********************************************************************************************************************/
e_os_return_code_t os_wrap_mutex_create(OS_MUTEX *p_mutex, char *p_mutex_name);

/**********************************************************************************************************************
 * Function Name: os_wrap_mutex_delete
 * Description  : This function deletes the specified mutex.
 * Arguments    : p_mutex - Pointer to mutex control block
 * Return Value : None
 *********************************************************************************************************************/
void os_wrap_mutex_delete (OS_MUTEX *p_mutex);

/**********************************************************************************************************************
 * Function Name: os_wrap_mutex_take
 * Description  : This function gets the specified mutex.
 * Arguments    : p_mutex    - Pointer to mutex control block
 *                block_time - Suspension option
 *                isr   - call from ISR (0:No, 1:Yes)
 * Return Value : OS_WRAP_SUCCESS
 *                OS_WRAP_OTHER_ERR
 *********************************************************************************************************************/
e_os_return_code_t os_wrap_mutex_take (OS_MUTEX *p_mutex, uint32_t block_time, e_call_from_isr_t isr);

/**********************************************************************************************************************
 * Function Name: os_wrap_mutex_give
 * Description  : This function releases the specified mutex.
 * Arguments    : p_mutex - Pointer to mutex control block
 *                isr   - call from ISR (0:No, 1:Yes)
 * Return Value : OS_WRAP_SUCCESS
 *                OS_WRAP_OTHER_ERR
 *********************************************************************************************************************/
e_os_return_code_t os_wrap_mutex_give (OS_MUTEX *p_mutex, e_call_from_isr_t isr);

/*
 * Common function
 */
/**********************************************************************************************************************
 * Function Name: os_wrap_sleep
 * Description  : This function handles application thread sleep requests.
 * Arguments    : timer_tick - timer value
 *                unit       - tick or milliseconds
 * Return Value : None
 *********************************************************************************************************************/
void os_wrap_sleep (uint32_t val, e_timer_unit_t unit);

/*
 * Tick count
 */
/**********************************************************************************************************************
 * Function Name: os_wrap_tickcount_get
 * Description  : This function returns system clock value.
 * Arguments    : None
 * Return Value : Returns the system clock value
 *********************************************************************************************************************/
OS_TICK os_wrap_tickcount_get (void);

/**********************************************************************************************************************
 * Function Name: tick_count_start
 * Description  : Start tick counting.
 * Arguments    : timeout_ms  - expired threshold
                  p_func      - callback function
 * Return Value : None
 *********************************************************************************************************************/
void tick_count_start (uint32_t timeout_ms, void(*p_func)(void));

/**********************************************************************************************************************
 * Function Name: tick_count_stop
 * Description  : Stop tick counting.
 * Arguments    : None
 * Return Value : None
 *********************************************************************************************************************/
void tick_count_stop (void);

/**********************************************************************************************************************
 * Function Name: tick_count_check
 * Description  : check a expired tick count.
 * Arguments    : None
 * Return Value : TICK_NOT_EXPIERD
 *                TICK_EXPIERD
 *********************************************************************************************************************/
uint32_t tick_count_check (void);

#endif /* R_WIFI_SX_ULPGN_OS_WRAP_H */
