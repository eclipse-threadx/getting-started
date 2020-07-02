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
 *  Implementation of wiced_rtos.c for ThreadX
 *
 *  This is the ThreadX implementation of the Wiced RTOS
 *  abstraction layer.
 *  It provides Wiced with standard ways of using threads,
 *  semaphores and time functions
 *
 */

#include "wwd_rtos.h"
#include <stdint.h>
#include "wwd_constants.h"
#include "RTOS/wwd_rtos_interface.h"
#include "wwd_assert.h"
#ifdef __GNUC__
#include <reent.h>
#endif /* #ifdef __GNUC__ */
#include <tx_api.h>
#include <tx_thread.h>
#include "platform/wwd_platform_interface.h"
#include "platform_config.h"

#ifdef __GNUC__
void __malloc_lock(struct _reent *ptr);
void __malloc_unlock(struct _reent *ptr);
#endif /* ifdef __GNUC__ */

/******************************************************
 *             Constants
 ******************************************************/

/******************************************************
 *             Variables
 ******************************************************/
#ifdef __GNUC__
static UINT     malloc_mutex_inited = TX_FALSE;
static TX_MUTEX malloc_mutex;
#endif /* ifdef __GNUC__ */

/******************************************************
 *             Function definitions
 ******************************************************/

#ifdef DEBUG
static VOID wiced_threadx_stack_error_handler( TX_THREAD * thread );
#endif /* ifdef DEBUG */

static wwd_result_t host_rtos_create_thread_common( /*@out@*/ host_thread_type_t* thread, void(*entry_function)( uint32_t ), const char* name, /*@null@*/ void* stack, uint32_t stack_size, uint32_t priority, host_rtos_thread_config_type_t *config );

/**
 * Creates a new thread
 *
 * @param thread         : pointer to variable which will receive handle of created thread
 * @param entry_function : main thread function
 * @param name           : a string thread name used for a debugger
 *
 * @returns WWD_SUCCESS on success, WICED_ERROR otherwise
 */
wwd_result_t host_rtos_create_thread( /*@out@*/ host_thread_type_t* thread, void(*entry_function)( uint32_t ), const char* name, /*@null@*/ void* stack, uint32_t stack_size, uint32_t priority )
{
    host_rtos_thread_config_type_t config;
    config.arg        = 0;
    config.time_slice = 0;

    return host_rtos_create_thread_common( thread, entry_function, (char*) name, stack, (ULONG) stack_size, (UINT) priority, &config );
}

wwd_result_t host_rtos_create_thread_with_arg( /*@out@*/ host_thread_type_t* thread, void(*entry_function)( uint32_t ), const char* name, /*@null@*/ void* stack, uint32_t stack_size, uint32_t priority, uint32_t arg )
{
    host_rtos_thread_config_type_t config;
    config.arg        = arg;
    config.time_slice = 0;

    return host_rtos_create_thread_common(  thread, entry_function, (char*) name, stack, (ULONG) stack_size, (UINT) priority, &config );
}

/**
 * Creates a new thread
 *
 * @param thread         : pointer to variable which will receive handle of created thread
 * @param entry_function : main thread function
 * @param name           : a string thread name used for a debugger
 * @param config     : os specific thread creation params
 * @returns WWD_SUCCESS on success, WICED_ERROR otherwise
 */
wwd_result_t host_rtos_create_configed_thread(  /*@out@*/ host_thread_type_t* thread, void(*entry_function)( uint32_t ), const char* name, /*@null@*/ void* stack, uint32_t stack_size, uint32_t priority, host_rtos_thread_config_type_t *config )
{
    return host_rtos_create_thread_common( thread, entry_function, name, stack, stack_size, priority, config );
}

static wwd_result_t host_rtos_create_thread_common( /*@out@*/ host_thread_type_t* thread, void(*entry_function)( uint32_t ), const char* name, /*@null@*/ void* stack, uint32_t stack_size, uint32_t priority, host_rtos_thread_config_type_t *config )
{
    UINT status;
    ULONG time_slice = TX_NO_TIME_SLICE;
    uint32_t arg = 0;
    if ( stack == NULL )
    {
        wiced_assert("host_rtos_create_thread: stack is null\n", 0 != 0 );
        return WWD_THREAD_STACK_NULL;
    }

    if ( config != NULL )
    {
        time_slice = config->time_slice;
        arg = config->arg;
    }

#ifdef DEBUG
    tx_thread_stack_error_notify( wiced_threadx_stack_error_handler );
#endif /* ifdef DEBUG */

    status = tx_thread_create( thread, (char*) name, (void(*)( ULONG )) entry_function, arg, stack, (ULONG) stack_size, (UINT) priority, (UINT) priority, time_slice, (UINT) TX_AUTO_START );

    return ( status == TX_SUCCESS ) ? WWD_SUCCESS : WWD_THREAD_CREATE_FAILED;
}

/**
 * Terminates the current thread
 *
 * This does nothing since ThreadX threads can exit by just returning
 *
 * @param thread         : handle of the thread to terminate
 *
 * @returns WWD_SUCCESS on success, WICED_ERROR otherwise
 */
wwd_result_t host_rtos_finish_thread( host_thread_type_t* thread )
{
    UINT status;
    status = tx_thread_terminate( thread );
    return ( status == TX_SUCCESS ) ? WWD_SUCCESS : WWD_THREAD_FINISH_FAIL;
}

/**
 * Deletes a terminated thread
 *
 * ThreadX requires that another thread deletes any terminated threads
 *
 * @param thread         : handle of the terminated thread to delete
 *
 * @returns WWD_SUCCESS on success, WICED_ERROR otherwise
 */
wwd_result_t host_rtos_delete_terminated_thread( host_thread_type_t* thread )
{
    UINT status;
    status = tx_thread_delete( thread );
    return ( status == TX_SUCCESS )? WWD_SUCCESS : WWD_THREAD_DELETE_FAIL;
}

/**
 * Blocks the current thread until the indicated thread is complete
 *
 * @param thread         : handle of the thread to terminate
 *
 * @returns WWD_SUCCESS on success, WICED_ERROR otherwise
 */
wwd_result_t host_rtos_join_thread( host_thread_type_t* thread )
{
    if (thread == NULL || thread->tx_thread_id != TX_THREAD_ID)
    {
        /*
         * Invalid thread pointer.
         */

        return WWD_BADARG;
    }

    while ( ( thread->tx_thread_state != TX_COMPLETED ) && ( thread->tx_thread_state != TX_TERMINATED ) )
    {
        host_rtos_delay_milliseconds( 10 );
    }
    return WWD_SUCCESS;
}

wwd_result_t host_rtos_init_mutex( host_mutex_type_t* mutex )
{
    /* Mutex uses priority inheritance */
    if ( tx_mutex_create( mutex, (CHAR*) "", TX_INHERIT ) != TX_SUCCESS )
    {
        return WWD_SEMAPHORE_ERROR;
    }

    return WWD_SUCCESS;
}

wwd_result_t host_rtos_lock_mutex( host_mutex_type_t* mutex )
{
    if ( tx_mutex_get( mutex, TX_WAIT_FOREVER ) != TX_SUCCESS )
    {
        return WWD_SEMAPHORE_ERROR;
    }

    return WWD_SUCCESS;
}

wwd_result_t host_rtos_unlock_mutex( host_mutex_type_t* mutex )
{
    if ( tx_mutex_put( mutex ) != TX_SUCCESS )
    {
        return WWD_SEMAPHORE_ERROR;
    }

    return WWD_SUCCESS;
}

wwd_result_t host_rtos_deinit_mutex( host_mutex_type_t* mutex )
{
    if ( tx_mutex_delete( mutex ) != TX_SUCCESS )
    {
        return WWD_SEMAPHORE_ERROR;
    }

    return WWD_SUCCESS;
}

/**
 * Creates a semaphore
 *
 * @param semaphore         : pointer to variable which will receive handle of created semaphore
 *
 * @returns WWD_SUCCESS on success, WICED_ERROR otherwise
 */
wwd_result_t host_rtos_init_semaphore( /*@out@*/ host_semaphore_type_t* semaphore ) /*@modifies *semaphore@*/
{
    return ( tx_semaphore_create( semaphore, (char*) "", 0 ) == TX_SUCCESS ) ? WWD_SUCCESS : WWD_SEMAPHORE_ERROR;
}

/**
 * Gets a semaphore
 *
 * If value of semaphore is larger than zero, then the semaphore is decremented and function returns
 * Else If value of semaphore is zero, then current thread is suspended until semaphore is set.
 * Value of semaphore should never be below zero
 *
 * Must not be called from interrupt context, since it could block, and since an interrupt is not a
 * normal thread, so could cause RTOS problems if it tries to suspend it.
 *
 * @param semaphore   : Pointer to variable which will receive handle of created semaphore
 * @param timeout_ms  : Maximum period to block for. Can be passed NEVER_TIMEOUT to request no timeout
 * @param will_set_in_isr : True if the semaphore will be set in an ISR. Currently only used for NoOS/NoNS
 *
 */
wwd_result_t host_rtos_get_semaphore( host_semaphore_type_t* semaphore, uint32_t timeout_ms, wiced_bool_t will_set_in_isr )
{
    UINT result;

    UNUSED_PARAMETER( will_set_in_isr );

    result = tx_semaphore_get( semaphore, ( timeout_ms == NEVER_TIMEOUT ) ? TX_WAIT_FOREVER : (ULONG) ( timeout_ms * SYSTICK_FREQUENCY / 1000 ) );
    if ( result == TX_SUCCESS )
    {
        return WWD_SUCCESS;
    }
    else if ( result == TX_NO_INSTANCE )
    {
        return WWD_TIMEOUT;
    }
    else if ( result == TX_WAIT_ABORTED )
    {
        return WWD_WAIT_ABORTED;
    }
    else
    {
        wiced_assert( "semaphore error ", 0 );
        return WWD_SEMAPHORE_ERROR;
    }
}

/**
 * Sets a semaphore
 *
 * If any threads are waiting on the semaphore, the first thread is resumed
 * Else increment semaphore.
 *
 * Can be called from interrupt context, so must be able to handle resuming other
 * threads from interrupt context.
 *
 * @param semaphore       : Pointer to variable which will receive handle of created semaphore
 * @param called_from_ISR : Value of WICED_TRUE indicates calling from interrupt context
 *                          Value of WICED_FALSE indicates calling from normal thread context
 *
 * @return wwd_result_t : WWD_SUCCESS if semaphore was successfully set
 *                        : WICED_ERROR if an error occurred
 *
 */
wwd_result_t host_rtos_set_semaphore( host_semaphore_type_t* semaphore, wiced_bool_t called_from_ISR )
{
    UNUSED_PARAMETER( called_from_ISR );

    return ( tx_semaphore_put( semaphore ) == TX_SUCCESS )? WWD_SUCCESS : WWD_SEMAPHORE_ERROR;
}

/**
 * Deletes a semaphore
 *
 * WICED uses this function to delete a semaphore.
 *
 * @param semaphore         : Pointer to the semaphore handle
 *
 * @return wwd_result_t : WWD_SUCCESS if semaphore was successfully deleted
 *                        : WICED_ERROR if an error occurred
 *
 */
wwd_result_t host_rtos_deinit_semaphore( host_semaphore_type_t* semaphore )
{
    return ( tx_semaphore_delete( semaphore ) == TX_SUCCESS )? WWD_SUCCESS : WWD_SEMAPHORE_ERROR;
}

/**
 * Gets time in milliseconds since RTOS start
 *
 * @Note: since this is only 32 bits, it will roll over every 49 days, 17 hours.
 *
 * @returns Time in milliseconds since RTOS started.
 */
wwd_time_t host_rtos_get_time( void )  /*@modifies internalState@*/
{
    return (wwd_time_t) ( tx_time_get( ) * ( 1000 / SYSTICK_FREQUENCY ) );
}

/**
 * Delay for a number of milliseconds
 *
 * Processing of this function depends on the minimum sleep
 * time resolution of the RTOS.
 * The current thread sleeps for the longest period possible which
 * is less than the delay required, then makes up the difference
 * with a tight loop
 *
 * @return wwd_result_t : WWD_SUCCESS if delay was successful
 *                        : WICED_ERROR if an error occurred
 *
 */
wwd_result_t host_rtos_delay_milliseconds( uint32_t num_ms )
{
    if ( ( num_ms * SYSTICK_FREQUENCY / 1000 ) != 0 )
    {
        if ( ( tx_thread_sleep( (ULONG) ( num_ms * SYSTICK_FREQUENCY / 1000 ) ) ) != TX_SUCCESS )
        {
            return WWD_SLEEP_ERROR;
        }
    }
    else
    {
        uint32_t time_reference = host_platform_get_cycle_count( );
        int32_t wait_time       = (int32_t)num_ms * (int32_t)CPU_CLOCK_HZ / 1000;
        while ( wait_time > 0 )
        {
            uint32_t current_time = host_platform_get_cycle_count( );
            wait_time            -= (int32_t)( current_time - time_reference );
            time_reference        = current_time;
        }
    }

    return WWD_SUCCESS;
}

unsigned long host_rtos_get_tickrate( void )
{
    return SYSTICK_FREQUENCY;
}

wwd_result_t host_rtos_init_queue( host_queue_type_t* queue, void* buffer, uint32_t buffer_size, uint32_t message_size )
{
    if ( ( message_size % 4 ) > 0 )
    {
        wiced_assert("Unaligned message size\n", 0);
        return WWD_QUEUE_ERROR;
    }


    if ( tx_queue_create( queue, (CHAR*) "queue", (UINT) ( message_size / 4 ), (VOID *) buffer, (ULONG) buffer_size ) != TX_SUCCESS )
    {
        return WWD_QUEUE_ERROR;
    }

    return WWD_SUCCESS;
}

wwd_result_t host_rtos_push_to_queue( host_queue_type_t* queue, void* message, uint32_t timeout_ms )
{
    if ( tx_queue_send( queue, (VOID*) message, ( timeout_ms == NEVER_TIMEOUT ) ? TX_WAIT_FOREVER : (ULONG) ( timeout_ms * SYSTICK_FREQUENCY / 1000 ) ) != TX_SUCCESS )
    {
        return WWD_QUEUE_ERROR;
    }

    return WWD_SUCCESS;
}

wwd_result_t host_rtos_pop_from_queue( host_queue_type_t* queue, void* message, uint32_t timeout_ms )
{
    UINT result = tx_queue_receive( queue, (VOID*)message, ( timeout_ms == NEVER_TIMEOUT ) ? TX_WAIT_FOREVER : (ULONG)( timeout_ms * SYSTICK_FREQUENCY / 1000 ) );
    if ( result == TX_SUCCESS )
    {
        return WWD_SUCCESS;
    }
    else if ( result == TX_QUEUE_EMPTY )
    {
        return WWD_TIMEOUT;
    }
    else if ( result == TX_WAIT_ABORTED )
    {
        return WWD_WAIT_ABORTED;
    }
    else
    {
        wiced_assert( "queue error ", 0 );
        return WWD_QUEUE_ERROR;
    }
}

wwd_result_t host_rtos_deinit_queue( host_queue_type_t* queue )
{
    if ( tx_queue_delete( queue ) != TX_SUCCESS )
    {
        return WWD_QUEUE_ERROR;
    }

    return WWD_SUCCESS;
}

#ifdef DEBUG
static VOID wiced_threadx_stack_error_handler( TX_THREAD * thread )
{
    UNUSED_PARAMETER( thread );

    wiced_assert( "ThreadX stack overflow", 0 != 0 );
}
#endif /* ifdef DEBUG */

#ifdef __GNUC__
void __malloc_lock(struct _reent *ptr)
{
    UNUSED_PARAMETER( ptr );
    if ( malloc_mutex_inited == TX_FALSE )
    {
        UINT status;
        status = tx_mutex_create( &malloc_mutex, (CHAR*) "malloc_mutex", TX_FALSE );

        wiced_assert( "Error creating mutex", status == TX_SUCCESS );
        REFERENCE_DEBUG_ONLY_VARIABLE( status );

        malloc_mutex_inited = TX_TRUE;
    }

    tx_mutex_get( &malloc_mutex, TX_WAIT_FOREVER );
}

void __malloc_unlock(struct _reent *ptr)
{
    UNUSED_PARAMETER( ptr );
    tx_mutex_put( &malloc_mutex );
}
#endif
