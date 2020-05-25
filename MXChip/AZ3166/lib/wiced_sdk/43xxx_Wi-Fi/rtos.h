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
#pragma once

#include "tx_api.h"
#include "wiced_result.h"
#include "wiced_utilities.h"
#include "wwd_rtos.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/

#define WICED_HARDWARE_IO_WORKER_THREAD             ((wiced_worker_thread_t*)&wiced_hardware_io_worker_thread)
#define WICED_NETWORKING_WORKER_THREAD              ((wiced_worker_thread_t*)&wiced_networking_worker_thread )
#define LOWER_THAN_PRIORITY_OF( thread )            ((thread).handle.tx_thread_priority + 1)
#define HIGHER_THAN_PRIORITY_OF( thread )           ((thread).handle.tx_thread_priority - 1)

#define WICED_PRIORITY_TO_NATIVE_PRIORITY(priority) ( priority )
#define WICED_END_OF_THREAD(thread)                 malloc_leak_check( &(thread).handle, LEAK_CHECK_THREAD); (void)(thread)
#define WICED_END_OF_CURRENT_THREAD( )              malloc_leak_check( NULL, LEAK_CHECK_THREAD)
#define WICED_END_OF_CURRENT_THREAD_NO_LEAK_CHECK( )

#define WICED_TO_MALLOC_THREAD( x )                 ((malloc_thread_handle) &((x)->handle ))

#define WICED_GET_THREAD_HANDLE( thread )           (&(( thread )->handle ))

#define WICED_GET_QUEUE_HANDLE( queue )             (&(( queue )->handle ))

/* This is the priority offset between ACs. */
#define PRIO_BOOST_OF_RX_THREAD_WRT_TX 8

/* All TX thread priority will be atleast this much lower than RX thread */
#define PRIO_OFFSET_BETWEEN_AC 2

/******************************************************
 *                    Constants
 ******************************************************/

/* Configuration of Built-in Worker Threads
 *
 * 1. wiced_hardware_io_worker_thread is designed to handle deferred execution of quick, non-blocking hardware I/O operations.
 *    - priority         : higher than that of wiced_networking_worker_thread
 *    - stack size       : small. Consequently, no printf is allowed here.
 *    - event queue size : the events are quick; therefore, large queue isn't required.
 *
 * 2. wiced_networking_worker_thread is designed to handle deferred execution of networking operations
 *    - priority         : lower to allow wiced_hardware_io_worker_thread to preempt and run
 *    - stack size       : considerably larger than that of wiced_hardware_io_worker_thread because of the networking functions.
 *    - event queue size : larger than that of wiced_hardware_io_worker_thread because networking operation may block
 */
#ifndef HARDWARE_IO_WORKER_THREAD_STACK_SIZE
#ifdef DEBUG
#define HARDWARE_IO_WORKER_THREAD_STACK_SIZE     (768) /* debug builds can use larger stack for example because of compiled-in asserts, switched off optimisation, etc */
#else
#define HARDWARE_IO_WORKER_THREAD_STACK_SIZE     (512)
#endif
#endif
#define HARDWARE_IO_WORKER_THREAD_QUEUE_SIZE      (10)

#ifndef NETWORKING_WORKER_THREAD_STACK_SIZE
#define NETWORKING_WORKER_THREAD_STACK_SIZE   (6*1024)
#endif
#define NETWORKING_WORKER_THREAD_QUEUE_SIZE       (15)

#define RTOS_NAME                     "ThreadX"
#define RTOS_VERSION                  ThreadX_VERSION

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

typedef TX_EVENT_FLAGS_GROUP wiced_event_flags_t;

typedef host_semaphore_type_t wiced_semaphore_t;

typedef TX_MUTEX wiced_mutex_t;

typedef void (*timer_handler_t)( void* arg );

typedef TX_TIMER wiced_timer_t;

/******************************************************
 *                    Structures
 ******************************************************/

typedef struct
{
    uint32_t last_update;                      /**< Last updated time     */
    uint32_t longest_delay;                    /**< Longest delay         */
} thread_monitor_info_t;

typedef struct
{
    TX_THREAD handle;                           /**< TX thread Handle     */
    void*     stack;                            /**< Thread Stack Pointer */
} wiced_thread_t;

typedef struct
{
    TX_QUEUE handle;                            /**< TX Queue Handle      */
    void*    buffer;                            /**< Queue data           */
} wiced_queue_t;

typedef struct
{
    wiced_thread_t        thread;                /**< Wiced Thread Structure        */
    wiced_queue_t         event_queue;           /**< Wiced Event Queue Structure   */
    thread_monitor_info_t monitor_info;          /**< Thread Monitor info structure */
} wiced_worker_thread_t;

typedef wiced_result_t (*event_handler_t)( void* arg );

typedef struct
{
    event_handler_t        function;              /**< Event handler callback function */
    void*                  arg;                   /**< Argument to function            */
    wiced_timer_t          timer;                 /**< Wiced Timer structure           */
    wiced_worker_thread_t* thread;                /**< Wiced Worker Thread pointer     */
} wiced_timed_event_t;

/******************************************************
 *                 Global Variables
 ******************************************************/

extern wiced_worker_thread_t wiced_hardware_io_worker_thread;
extern wiced_worker_thread_t wiced_networking_worker_thread;

/******************************************************
 *               Function Declarations
 ******************************************************/

#ifdef __cplusplus
} /* extern "C" */
#endif
