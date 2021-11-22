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
 * File Name    : r_wifi_sx_ulpgn_os_wrap.c
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
#include "r_wifi_sx_ulpgn_os_wrap.h"

/**********************************************************************************************************************
 Macro definitions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Local Typedef definitions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Exported global variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Private (static) variables and functions
 *********************************************************************************************************************/
/* for Tick count functions */
static OS_TICK s_exec_tick_count_flg = 0;
static OS_TICK s_tick_begin = 0;
static OS_TICK s_tick_timeout = 0;
static void(*sp_callback)(void);

/*
 * Mutex
 */
/**********************************************************************************************************************
 * Function Name: os_wrap_mutex_create
 * Description  : This function creates the specified mutex.
 * Arguments    : p_mutex      - Pointer to mutex control block
 *                p_mutex_name - Pointer to mutex name
 * Return Value : OS_WRAP_SUCCESS
 *                OS_WRAP_OTHER_ERR
 *********************************************************************************************************************/
e_os_return_code_t os_wrap_mutex_create(OS_MUTEX *p_mutex, char *p_mutex_name)
{
#if BSP_CFG_RTOS_USED == 1        /* FreeRTOS is used.   */
    *p_mutex = xSemaphoreCreateMutex();
    if (NULL == *p_mutex)
    {
        return OS_WRAP_OTHER_ERR;
    }
#elif BSP_CFG_RTOS_USED == 5      /* Azure RTOS is used. */
    if (TX_SUCCESS != tx_mutex_create(p_mutex, p_mutex_name, TX_INHERIT))
    {
        return OS_WRAP_OTHER_ERR;
    }
#endif
    return OS_WRAP_SUCCESS;
}
/**********************************************************************************************************************
 * End of function os_wrap_mutex_create
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: os_wrap_mutex_delete
 * Description  : This function deletes the specified mutex.
 * Arguments    : p_mutex - Pointer to mutex control block
 * Return Value : None
 *********************************************************************************************************************/
void os_wrap_mutex_delete(OS_MUTEX *p_mutex)
{
#if BSP_CFG_RTOS_USED == 1        /* FreeRTOS is used.   */
    if (NULL != *p_mutex)
    {
        vSemaphoreDelete(*p_mutex);
        *p_mutex = NULL;
    }
#elif BSP_CFG_RTOS_USED == 5      /* Azure RTOS is used. */
    if (TX_MUTEX_ID == p_mutex->tx_mutex_id)
    {
        tx_mutex_delete(p_mutex);
    }
#endif /* BSP_CFG_RTOS_USED */
}
/**********************************************************************************************************************
 * End of function os_wrap_mutex_delete
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: os_wrap_mutex_take
 * Description  : This function gets the specified mutex.
 * Arguments    : p_mutex    - Pointer to mutex control block
 *                block_time - Suspension option
 *                isr   - call from ISR (0:No, 1:Yes)
 * Return Value : OS_WRAP_SUCCESS
 *                OS_WRAP_OTHER_ERR
 *********************************************************************************************************************/
e_os_return_code_t os_wrap_mutex_take(OS_MUTEX *p_mutex, uint32_t block_time, e_call_from_isr_t isr)
{
#if BSP_CFG_RTOS_USED == 1        /* FreeRTOS is used.   */
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (CALL_ISR == isr)
    {
        if (pdTRUE != xSemaphoreTakeFromISR(*p_mutex, &xHigherPriorityTaskWoken))
        {
            return OS_WRAP_OTHER_ERR;
        }
    }
    else
    {
        if (pdTRUE != xSemaphoreTake(*p_mutex, block_time))
        {
            return OS_WRAP_OTHER_ERR;
        }
    }
#elif BSP_CFG_RTOS_USED == 5      /* Azure RTOS is used. */
    if (TX_SUCCESS != tx_mutex_get(p_mutex, block_time))
    {
        return OS_WRAP_OTHER_ERR;
    }
#endif /* BSP_CFG_RTOS_USED */
    return OS_WRAP_SUCCESS;
}
/**********************************************************************************************************************
 * End of function os_wrap_mutex_take
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: os_wrap_mutex_give
 * Description  : This function releases the specified mutex.
 * Arguments    : p_mutex - Pointer to mutex control block
 *                isr   - call from ISR (0:No, 1:Yes)
 * Return Value : OS_WRAP_SUCCESS
 *                OS_WRAP_OTHER_ERR
 *********************************************************************************************************************/
e_os_return_code_t os_wrap_mutex_give(OS_MUTEX *p_mutex, e_call_from_isr_t isr)
{
#if BSP_CFG_RTOS_USED == 1        /* FreeRTOS is used.   */
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (CALL_ISR == isr)
    {
        if (pdTRUE != xSemaphoreGiveFromISR(*p_mutex, &xHigherPriorityTaskWoken))
        {
            return OS_WRAP_OTHER_ERR;
        }
    }
    else
    {
        if (pdTRUE != xSemaphoreGive(*p_mutex))
        {
            return OS_WRAP_OTHER_ERR;
        }
    }
#elif BSP_CFG_RTOS_USED == 5      /* Azure RTOS is used. */
    if (TX_SUCCESS != tx_mutex_put(p_mutex))
    {
        return OS_WRAP_OTHER_ERR;
    }
#endif /* BSP_CFG_RTOS_USED */
    return OS_WRAP_SUCCESS;
}
/**********************************************************************************************************************
 * End of function os_wrap_mutex_give
 *********************************************************************************************************************/

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
void os_wrap_sleep(uint32_t val, e_timer_unit_t unit)
{
    uint32_t tick;

    if (UNIT_MSEC == unit)
    {
        tick =  OS_WRAP_MS_TO_TICKS(val);
    }
    else
    {
        tick =  val;
    }
#if BSP_CFG_RTOS_USED == 1        /* FreeRTOS is used.   */
    vTaskDelay(tick);
#elif BSP_CFG_RTOS_USED == 5      /* Azure RTOS is used. */
    tx_thread_sleep(tick);
#endif /* BSP_CFG_RTOS_USED */
}
/**********************************************************************************************************************
 * End of function os_wrap_sleep
 *********************************************************************************************************************/

/*
 * Tick count
 */
/**********************************************************************************************************************
 * Function Name: os_wrap_tickcount_get
 * Description  : This function returns system clock value.
 * Arguments    : None
 * Return Value : Returns the system clock value
 *********************************************************************************************************************/
OS_TICK os_wrap_tickcount_get(void)
{
#if BSP_CFG_RTOS_USED == 1        /* FreeRTOS is used.   */
    return xTaskGetTickCount();
#elif BSP_CFG_RTOS_USED == 5      /* Azure RTOS is used. */
    return tx_time_get();
#endif /* BSP_CFG_RTOS_USED */
}
/**********************************************************************************************************************
 * End of function os_wrap_tickcount_get
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: tick_count_start
 * Description  : Start tick counting.
 * Arguments    : timeout_ms  - expired threshold
                  p_func      - callback function
 * Return Value : None
 *********************************************************************************************************************/
void tick_count_start(uint32_t timeout_ms, void(*p_func)(void))
{
    if (0 == timeout_ms)
    {
        return;
    }

    s_exec_tick_count_flg = 1;
    sp_callback = p_func;
    s_tick_timeout = OS_WRAP_MS_TO_TICKS(timeout_ms);
    s_tick_begin = os_wrap_tickcount_get();
}
/**********************************************************************************************************************
 * End of function tick_count_start
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: tick_count_stop
 * Description  : Stop tick counting.
 * Arguments    : None
 * Return Value : None
 *********************************************************************************************************************/
void tick_count_stop(void)
{
    s_exec_tick_count_flg = 0;
    sp_callback = NULL;
}
/**********************************************************************************************************************
 * End of function tick_count_stop
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: tick_count_check
 * Description  : check a expired tick count.
 * Arguments    : None
 * Return Value : TICK_NOT_EXPIERD
 *                TICK_EXPIERD
 *********************************************************************************************************************/
uint32_t tick_count_check(void)
{
    uint32_t ret = TICK_NOT_EXPIERD;
    OS_TICK tick_tmp;

    if (1 != s_exec_tick_count_flg)
    {
        return ret;
    }

    tick_tmp = os_wrap_tickcount_get() - s_tick_begin;
    if (tick_tmp >= s_tick_timeout)
    {
        if (NULL != sp_callback)
        {
            sp_callback();
        }
        s_exec_tick_count_flg = 0;
        ret = TICK_EXPIERD;
    }
    return ret;
}
/**********************************************************************************************************************
 * End of function tick_count_check
 *********************************************************************************************************************/
