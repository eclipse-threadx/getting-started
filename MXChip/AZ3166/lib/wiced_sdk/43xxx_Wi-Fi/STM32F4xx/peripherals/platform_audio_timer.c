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
 * Audio timer clock functionality
 */

#include "wiced_result.h"
#include "platform_audio.h"


/******************************************************
 *                      Macros
 ******************************************************/

#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER( x ) ( (void)(x) )
#endif

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
 *               Function Declarations
 ******************************************************/

/******************************************************
 *               Variables Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t platform_audio_timer_enable( uint32_t audio_frame_count )
{
    UNUSED_PARAMETER( audio_frame_count );
    return WICED_UNSUPPORTED;
}


wiced_result_t platform_audio_timer_disable( void )
{
    return WICED_UNSUPPORTED;
}


wiced_result_t platform_audio_timer_get_frame_sync( uint32_t timeout_msecs )
{
    UNUSED_PARAMETER( timeout_msecs );
    return WICED_UNSUPPORTED;
}


wiced_result_t platform_audio_timer_set_frame_sync( void )
{
    return WICED_UNSUPPORTED;
}


wiced_result_t platform_audio_timer_get_time( uint32_t *time_hi, uint32_t *time_lo )
{
    UNUSED_PARAMETER( time_hi );
    UNUSED_PARAMETER( time_lo );
    return WICED_UNSUPPORTED;
}


wiced_result_t platform_audio_timer_get_nanoseconds( uint32_t audio_sample_rate, uint32_t* audio_time_secs, uint32_t* audio_time_nanosecs )
{
    UNUSED_PARAMETER( audio_sample_rate );
    UNUSED_PARAMETER( audio_time_secs );
    UNUSED_PARAMETER( audio_time_nanosecs );
    return WICED_UNSUPPORTED;
}


wiced_result_t platform_audio_timer_get_resolution( uint32_t audio_sample_rate, uint32_t *ticks_per_sec )
{
    UNUSED_PARAMETER( audio_sample_rate );
    UNUSED_PARAMETER( ticks_per_sec );
    return WICED_UNSUPPORTED;
}
