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
 */
#include "wwd_constants.h"
#include "wwd_wifi.h"
#include "internal/wwd_sdpcm.h"
#include <string.h>
#include "internal/wwd_internal.h"
#include "internal/bus_protocols/wwd_bus_protocol_interface.h"

/******************************************************
 *                      Macros
 ******************************************************/
#define VERIFY_RESULT( x )     { wwd_result_t verify_result; verify_result = ( x ); if ( verify_result != WWD_SUCCESS ) return verify_result; }

/******************************************************
 *                    Constants
 ******************************************************/
#define PLATFORM_WLAN_RAM_BASE  (0x0)
#define PLATFORM_WLAN_RAM_SIZE  (CHIP_RAM_SIZE)
#define WLAN_SHARED_ADDR        (PLATFORM_WLAN_RAM_BASE + PLATFORM_WLAN_RAM_SIZE - 4)

#define WLAN_BUS_UP_ATTEMPTS  (1000)

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
static wiced_bool_t bus_is_up               = WICED_FALSE;

/******************************************************
 *               Function Definitions
 ******************************************************/
wiced_bool_t wwd_bus_is_up( void )
{
    return bus_is_up;
}

wwd_result_t wwd_wifi_read_wlan_log( char* buffer, uint32_t buffer_size )
{
    wwd_result_t result;

    result = wwd_wifi_read_wlan_log_unsafe( WLAN_SHARED_ADDR, buffer, buffer_size );

    return result;
}

wwd_result_t wwd_wifi_set_custom_country_code( const wiced_country_info_t* country_code )
{
    wiced_buffer_t buffer;
    wwd_result_t   result;
    wiced_country_info_t* data;

    data = (wiced_country_info_t*) wwd_sdpcm_get_ioctl_buffer( &buffer, (uint16_t) sizeof(wiced_country_info_t) + 10 );
    if( data == NULL )
    {
        return WWD_BUFFER_UNAVAILABLE_TEMPORARY;
    }
    memcpy( data, country_code, sizeof(wiced_country_info_t) );
    result = wwd_sdpcm_send_ioctl( SDPCM_SET, WLC_SET_CUSTOM_COUNTRY, buffer, NULL, WWD_STA_INTERFACE );

    return result;
}

wwd_result_t wwd_chip_specific_init( void )
{
    return WWD_SUCCESS;
}

wwd_result_t wwd_chip_specific_socsram_init( void )
{
    return WWD_SUCCESS;
}

wwd_result_t wwd_ensure_wlan_bus_is_up( void )
{
    uint8_t csr = 0;
    uint32_t attempts = (uint32_t) WLAN_BUS_UP_ATTEMPTS;

    /* Ensure HT clock is up */
    if ( bus_is_up == WICED_TRUE )
    {
        return WWD_SUCCESS;
    }

    /* Bus specific wakeup routine */
    VERIFY_RESULT( wwd_bus_specific_wakeup( ));

    VERIFY_RESULT( wwd_bus_write_register_value( BACKPLANE_FUNCTION, (uint32_t) SDIO_CHIP_CLOCK_CSR, (uint8_t) 1, (uint32_t) SBSDIO_HT_AVAIL_REQ ) );

    do
    {
        VERIFY_RESULT( wwd_bus_read_register_value( BACKPLANE_FUNCTION, (uint32_t) SDIO_CHIP_CLOCK_CSR, (uint8_t) 1, &csr ) );
        --attempts;
    }
    while ( ( ( csr & SBSDIO_HT_AVAIL ) == 0 ) &&
            ( attempts != 0 ) &&
            ( host_rtos_delay_milliseconds( (uint32_t) 1 ), 1==1 ) );

    if (attempts == 0)
    {
        return WWD_SDIO_BUS_UP_FAIL;
    }
    else
    {
        bus_is_up = WICED_TRUE;
        return WWD_SUCCESS;
    }
}

wwd_result_t wwd_allow_wlan_bus_to_sleep( void )
{
    /* Clear HT clock request */
    if (bus_is_up == WICED_TRUE)
    {
        bus_is_up = WICED_FALSE;
        VERIFY_RESULT( wwd_bus_write_register_value( BACKPLANE_FUNCTION, (uint32_t) SDIO_CHIP_CLOCK_CSR, (uint8_t) 1, 0 ));

        /* Bus specific sleep routine */
        return wwd_bus_specific_sleep( );
    }
    else
    {
        return WWD_SUCCESS;
    }
}
