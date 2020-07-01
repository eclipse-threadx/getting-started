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
 *  Broadcom WLAN SDIO Protocol interface
 *
 *  Implements the WWD Bus Protocol Interface for SDIO
 *  Provides functions for initialising, de-intitialising 802.11 device,
 *  sending/receiving raw packets etc
 */


#include <string.h> /* For memcpy */
#include "wwd_assert.h"
#include "network/wwd_buffer_interface.h"
#include "RTOS/wwd_rtos_interface.h"
#include "platform/wwd_platform_interface.h"
#include "platform/wwd_resource_interface.h"
#include "chip_constants.h"
#include "internal/wwd_internal.h"
#include "internal/wwd_sdpcm.h"
#include "internal/bus_protocols/wwd_bus_protocol_interface.h"

/******************************************************
 *             Constants
 ******************************************************/
/* function 1 OCP space */
#define SBSDIO_SB_OFT_ADDR_MASK     0x07FFF     /* sb offset addr is <= 15 bits, 32k */
#define SBSDIO_SB_OFT_ADDR_LIMIT    0x08000
#define SBSDIO_SB_ACCESS_2_4B_FLAG  0x08000     /* with b15, maps to 32-bit SB access */

#define F0_WORKING_TIMEOUT_MS (500)
#define F1_AVAIL_TIMEOUT_MS   (500)
#define F2_AVAIL_TIMEOUT_MS   (500)
#define F2_READY_TIMEOUT_MS   (1000)
#define ALP_AVAIL_TIMEOUT_MS  (100)
#define HT_AVAIL_TIMEOUT_MS   (500)
#define ABORT_TIMEOUT_MS      (100)
/* Taken from FALCON_5_90_195_26 dhd/sys/dhd_sdio.c. */
#define SDIO_F2_WATERMARK     (8)

#define INITIAL_READ   4

#define VERIFY_RESULT( x )  { wwd_result_t verify_result; verify_result = (x); if ( verify_result != WWD_SUCCESS ) { WPRINT_WWD_DEBUG(("fail: func=%s line=%d\n", __FUNCTION__, __LINE__)); return verify_result; } }

#ifndef WWD_BUS_SDIO_RESET_DELAY
#define WWD_BUS_SDIO_RESET_DELAY    (1)
#endif

#ifndef WWD_BUS_SDIO_AFTER_RESET_DELAY
#define WWD_BUS_SDIO_AFTER_RESET_DELAY    (1)
#endif

#ifndef WWD_THREAD_POLL_TIMEOUT
#define WWD_THREAD_POLL_TIMEOUT      (NEVER_TIMEOUT)
#endif /* WWD_THREAD_POLL_TIMEOUT */

#ifndef WWD_THREAD_POKE_TIMEOUT
#define WWD_THREAD_POKE_TIMEOUT      (100)
#endif /* WWD_THREAD_POKE_TIMEOUT */

#define HOSTINTMASK                 ( I_HMB_SW_MASK )


/******************************************************
 *             Structures
 ******************************************************/

/******************************************************
 *             Variables
 ******************************************************/

static wiced_bool_t wwd_bus_flow_controlled = WICED_FALSE;

#ifdef WWD_ENABLE_STATS
wwd_bus_stats_t wwd_bus_stats;
#endif /* WWD_ENABLE_STATS */

/******************************************************
 *             Static Function Declarations
 ******************************************************/

static wwd_result_t wwd_bus_sdio_transfer               ( wwd_bus_transfer_direction_t direction, wwd_bus_function_t function, uint32_t address, uint16_t data_size, /*@in@*/ /*@out@*/  uint8_t* data, sdio_response_needed_t response_expected );
static wwd_result_t wwd_bus_sdio_cmd52                  ( wwd_bus_transfer_direction_t direction, wwd_bus_function_t function, uint32_t address, uint8_t value, sdio_response_needed_t response_expected, /*@out@*/ uint8_t* response );
static wwd_result_t wwd_bus_sdio_cmd53                  ( wwd_bus_transfer_direction_t direction, wwd_bus_function_t function, sdio_transfer_mode_t mode, uint32_t address, uint16_t data_size, /*@in@*/ /*@out@*/  uint8_t* data, sdio_response_needed_t response_expected, /*@null@*/ /*@out@*/ uint32_t* response );
static wwd_result_t wwd_bus_sdio_abort_read             ( wiced_bool_t retry );
static wwd_result_t wwd_bus_sdio_download_firmware      ( void );

#ifndef WICED_DISABLE_MCU_POWERSAVE
static wwd_result_t wwd_bus_sdio_set_oob_interrupt      ( uint8_t gpio_pin_number );
#endif

/******************************************************
 *             SDIO Logging
 * Enable this section for logging of SDIO transfers
 * by changing "if 0" to "if 1"
 ******************************************************/
#if 0

#define SDIO_LOG_SIZE (110)
#define SDIO_LOG_HEADER_SIZE (0)   /*(0x30) */

typedef struct sdio_log_entry_struct
{
    wwd_bus_transfer_direction_t  direction;
    wwd_bus_function_t            function;
    uint32_t                  address;
    unsigned long             time;
    unsigned long             length;
#if ( SDIO_LOG_HEADER_SIZE != 0 )
    unsigned char             header[SDIO_LOG_HEADER_SIZE];
#endif /* if ( SDIO_LOG_HEADER_SIZE != 0 ) */
} sdio_log_entry_t;

static int next_sdio_log_pos = 0;
static sdio_log_entry_t sdio_log_data[SDIO_LOG_SIZE];

static void add_log_entry( wwd_bus_transfer_direction_t dir, wwd_bus_function_t function, uint32_t address, unsigned long length, uint8_t* data )
{
    sdio_log_data[next_sdio_log_pos].direction = dir;
    sdio_log_data[next_sdio_log_pos].function  = function;
    sdio_log_data[next_sdio_log_pos].address   = address;
    sdio_log_data[next_sdio_log_pos].time      = host_rtos_get_time();
    sdio_log_data[next_sdio_log_pos].length    = length;
#if ( SDIO_LOG_HEADER_SIZE != 0 )
    memcpy( sdio_log_data[next_sdio_log_pos].header, data, (length>=SDIO_LOG_HEADER_SIZE)?SDIO_LOG_HEADER_SIZE:length );
#else
    UNUSED_PARAMETER(data);
#endif /* if ( SDIO_LOG_HEADER_SIZE != 0 ) */
    next_sdio_log_pos++;
    if (next_sdio_log_pos >= SDIO_LOG_SIZE)
    {
        next_sdio_log_pos = 0;
    }
}
#else /* #if 0 */
#define add_log_entry( dir, function, address, length, data)
#endif /* #if 0 */


/******************************************************
 *             Global Function definitions
 ******************************************************/

void wwd_wait_for_wlan_event( host_semaphore_type_t* transceive_semaphore )
{
    wwd_result_t result = WWD_SUCCESS;
    uint32_t timeout_ms = 1;
    uint32_t delayed_release_timeout_ms;

    REFERENCE_DEBUG_ONLY_VARIABLE( result );

    delayed_release_timeout_ms = wwd_bus_handle_delayed_release( );
    if ( delayed_release_timeout_ms != 0 )
    {
        timeout_ms = delayed_release_timeout_ms;
    }
    else
    {
        result = wwd_allow_wlan_bus_to_sleep( );
        wiced_assert( "Error setting wlan sleep", ( result == WWD_SUCCESS ) || ( result == WWD_PENDING ) );

        if ( result == WWD_SUCCESS )
        {
            timeout_ms = NEVER_TIMEOUT;
        }
    }

    /* Check if we have run out of bus credits */
    if ( wwd_sdpcm_has_tx_packet() == WICED_TRUE && wwd_sdpcm_get_available_credits( ) == 0 )
    {
        /* Keep poking the WLAN until it gives us more credits */
        result = wwd_bus_poke_wlan( );
        wiced_assert( "Poking failed!", result == WWD_SUCCESS );

        result = host_rtos_get_semaphore( transceive_semaphore, (uint32_t) MIN( timeout_ms, WWD_THREAD_POKE_TIMEOUT ), WICED_FALSE );
    }
    else
    {
        result = host_rtos_get_semaphore( transceive_semaphore, (uint32_t) MIN( timeout_ms, WWD_THREAD_POLL_TIMEOUT ), WICED_FALSE );
    }
    wiced_assert("Could not get wwd sleep semaphore\n", ( result == WWD_SUCCESS)||(result == WWD_TIMEOUT ) );

}

/* Device data transfer functions */
wwd_result_t wwd_bus_send_buffer( wiced_buffer_t buffer )
{
    wwd_result_t retval;
    retval = wwd_bus_transfer_bytes( BUS_WRITE, WLAN_FUNCTION, 0, (uint16_t) ( host_buffer_get_current_piece_size( buffer ) - sizeof(wiced_buffer_t) ), (wwd_transfer_bytes_packet_t*)( host_buffer_get_current_piece_data_pointer( buffer ) + sizeof(wiced_buffer_t) ) );
    host_buffer_release( buffer, WWD_NETWORK_TX );
    if ( retval == WWD_SUCCESS )
    {
        DELAYED_BUS_RELEASE_SCHEDULE( WICED_TRUE );
    }
    return retval;
}

wwd_result_t wwd_bus_init( void )
{
    uint8_t        byte_data;
    wwd_result_t   result;
    uint32_t       loop_count;
    wwd_time_t     elapsed_time;
    uint32_t       wifi_firmware_image_size = 0;

    wwd_bus_flow_controlled = WICED_FALSE;

    wwd_bus_init_backplane_window( );

    host_platform_reset_wifi( WICED_TRUE );
    host_platform_power_wifi( WICED_TRUE );
    (void) host_rtos_delay_milliseconds( (uint32_t) WWD_BUS_SDIO_RESET_DELAY );  /* Ignore return - nothing can be done if it fails */
    host_platform_reset_wifi( WICED_FALSE );
    (void) host_rtos_delay_milliseconds( (uint32_t) WWD_BUS_SDIO_AFTER_RESET_DELAY );  /* Ignore return - nothing can be done if it fails */

    VERIFY_RESULT( host_platform_sdio_enumerate() );

    /* Setup the backplane*/
    loop_count = 0;
    do
    {
        /* Enable function 1 (backplane) */
        VERIFY_RESULT( wwd_bus_write_register_value( BUS_FUNCTION, SDIOD_CCCR_IOEN, (uint8_t) 1, SDIO_FUNC_ENABLE_1 ) );
        if (loop_count != 0)
        {
            (void) host_rtos_delay_milliseconds( (uint32_t) 1 );  /* Ignore return - nothing can be done if it fails */
        }
        VERIFY_RESULT( wwd_bus_read_register_value ( BUS_FUNCTION, SDIOD_CCCR_IOEN, (uint8_t) 1, &byte_data ) );
        loop_count++;
        if ( loop_count >= (uint32_t) F0_WORKING_TIMEOUT_MS )
        {
            return WWD_TIMEOUT;
        }
    } while (byte_data != (uint8_t) SDIO_FUNC_ENABLE_1);

#ifndef SDIO_1_BIT
    /* Read the bus width and set to 4 bits */
    VERIFY_RESULT( wwd_bus_read_register_value (BUS_FUNCTION, SDIOD_CCCR_BICTRL, (uint8_t) 1, &byte_data) );
    VERIFY_RESULT( wwd_bus_write_register_value(BUS_FUNCTION, SDIOD_CCCR_BICTRL, (uint8_t) 1, (byte_data & (~BUS_SD_DATA_WIDTH_MASK)) | BUS_SD_DATA_WIDTH_4BIT ) );
    /* NOTE: We don't need to change our local bus settings since we're not sending any data (only using CMD52) until after we change the bus speed further down */
#endif
    /* Set the block size */

    /* Wait till the backplane is ready */
    loop_count = 0;
    while ( ( ( result = wwd_bus_write_register_value( BUS_FUNCTION, SDIOD_CCCR_BLKSIZE_0, (uint8_t) 1, (uint32_t) SDIO_64B_BLOCK ) ) == WWD_SUCCESS ) &&
            ( ( result = wwd_bus_read_register_value ( BUS_FUNCTION, SDIOD_CCCR_BLKSIZE_0, (uint8_t) 1, &byte_data                ) ) == WWD_SUCCESS ) &&
            ( byte_data != (uint8_t)  SDIO_64B_BLOCK ) &&
            ( loop_count < (uint32_t) F0_WORKING_TIMEOUT_MS ) )
    {
        (void) host_rtos_delay_milliseconds( (uint32_t) 1 );  /* Ignore return - nothing can be done if it fails */
        loop_count++;
        if ( loop_count >= (uint32_t) F0_WORKING_TIMEOUT_MS )
        {
            /* If the system fails here, check the high frequency crystal is working */
            WPRINT_WWD_ERROR(("Timeout while setting block size\n"));
            return WWD_TIMEOUT;
        }
    }

    VERIFY_RESULT( result );

    VERIFY_RESULT( wwd_bus_write_register_value( BUS_FUNCTION, SDIOD_CCCR_BLKSIZE_0,   (uint8_t) 1, (uint32_t) SDIO_64B_BLOCK ) );
    VERIFY_RESULT( wwd_bus_write_register_value( BUS_FUNCTION, SDIOD_CCCR_F1BLKSIZE_0, (uint8_t) 1, (uint32_t) SDIO_64B_BLOCK ) );
    VERIFY_RESULT( wwd_bus_write_register_value( BUS_FUNCTION, SDIOD_CCCR_F2BLKSIZE_0, (uint8_t) 1, (uint32_t) SDIO_64B_BLOCK ) );
    VERIFY_RESULT( wwd_bus_write_register_value( BUS_FUNCTION, SDIOD_CCCR_F2BLKSIZE_1, (uint8_t) 1, (uint32_t) 0              ) ); /* Function 2 = 64 */

    /* Enable/Disable Client interrupts */
    VERIFY_RESULT( wwd_bus_write_register_value( BUS_FUNCTION, SDIOD_CCCR_INTEN,       (uint8_t) 1, INTR_CTL_MASTER_EN | INTR_CTL_FUNC1_EN | INTR_CTL_FUNC2_EN ) );


#ifdef HIGH_SPEED_SDIO_CLOCK
    /* This code is required if we want more than 25 MHz clock */
    VERIFY_RESULT( wwd_bus_read_register_value( BUS_FUNCTION, SDIOD_CCCR_SPEED_CONTROL, 1, &byte_data ) );
    if ( ( byte_data & 0x1 ) != 0 )
    {
        VERIFY_RESULT( wwd_bus_write_register_value( BUS_FUNCTION, SDIOD_CCCR_SPEED_CONTROL, 1, byte_data | SDIO_SPEED_EHS ) );
    }
    else
    {
        return WWD_BUS_READ_REGISTER_ERROR;
    }
#endif /* HIGH_SPEED_SDIO_CLOCK */

    /* Switch to high speed mode and change to 4 bit mode */
    VERIFY_RESULT( host_platform_enable_high_speed_sdio( ));

    /* Wait till the backplane is ready */
    loop_count = 0;
    while ( ( ( result = wwd_bus_read_register_value( BUS_FUNCTION, SDIOD_CCCR_IORDY, (uint8_t) 1, &byte_data ) ) == WWD_SUCCESS ) &&
            ( ( byte_data & SDIO_FUNC_READY_1 ) == 0 ) &&
            ( loop_count < (uint32_t) F1_AVAIL_TIMEOUT_MS ) )
    {
        (void) host_rtos_delay_milliseconds( (uint32_t) 1 ); /* Ignore return - nothing can be done if it fails */
        loop_count++;
    }
    if ( loop_count >= (uint32_t) F1_AVAIL_TIMEOUT_MS )
    {
        WPRINT_WWD_ERROR(("Timeout while waiting for backplane to be ready\n"));
        return WWD_TIMEOUT;
    }
    VERIFY_RESULT( result );

    /* Set the ALP */
    VERIFY_RESULT( wwd_bus_write_register_value( BACKPLANE_FUNCTION, SDIO_CHIP_CLOCK_CSR, (uint8_t) 1, (uint32_t)( SBSDIO_FORCE_HW_CLKREQ_OFF | SBSDIO_ALP_AVAIL_REQ | SBSDIO_FORCE_ALP ) ) );

    loop_count = 0;
    while ( ( ( result = wwd_bus_read_register_value( BACKPLANE_FUNCTION, SDIO_CHIP_CLOCK_CSR, (uint8_t) 1, &byte_data ) ) == WWD_SUCCESS ) &&
            ( ( byte_data & SBSDIO_ALP_AVAIL ) == 0 ) &&
            ( loop_count < (uint32_t) ALP_AVAIL_TIMEOUT_MS ) )
    {
        (void) host_rtos_delay_milliseconds( (uint32_t) 1 ); /* Ignore return - nothing can be done if it fails */
        loop_count++;
    }
    if ( loop_count >= (uint32_t) ALP_AVAIL_TIMEOUT_MS )
    {
        WPRINT_WWD_ERROR(("Timeout while waiting for alp clock\n"));
        return WWD_TIMEOUT;
    }
    VERIFY_RESULT( result );

    /* Clear request for ALP */
    VERIFY_RESULT( wwd_bus_write_register_value( BACKPLANE_FUNCTION, SDIO_CHIP_CLOCK_CSR, (uint8_t) 1, 0 ) );

    /* Disable the extra SDIO pull-ups */
#ifndef WWD_BUS_SDIO_USE_WLAN_SDIO_PULLUPS
    VERIFY_RESULT( wwd_bus_write_register_value( BACKPLANE_FUNCTION, SDIO_PULL_UP,  (uint8_t) 1, 0 ) );
#endif

    /* Enable F1 and F2 */
    VERIFY_RESULT( wwd_bus_write_register_value( BUS_FUNCTION, SDIOD_CCCR_IOEN, (uint8_t) 1, SDIO_FUNC_ENABLE_1 | SDIO_FUNC_ENABLE_2 ) );

#ifndef WICED_DISABLE_MCU_POWERSAVE
    /* Enable out-of-band interrupt */
    VERIFY_RESULT( wwd_bus_write_register_value( BUS_FUNCTION, SDIOD_SEP_INT_CTL, (uint8_t) 1, SEP_INTR_CTL_MASK | SEP_INTR_CTL_EN | SEP_INTR_CTL_POL ) );

    /* Set OOB interrupt to the correct WLAN GPIO pin */
    VERIFY_RESULT( wwd_bus_sdio_set_oob_interrupt( host_platform_get_oob_interrupt_pin( ) ) );

    VERIFY_RESULT( host_enable_oob_interrupt( ) );
#endif /* ifndef WICED_DISABLE_MCU_POWERSAVE */

    /* Enable F2 interrupt only */
    VERIFY_RESULT( wwd_bus_write_register_value( BUS_FUNCTION, SDIOD_CCCR_INTEN, (uint8_t) 1, INTR_CTL_MASTER_EN | INTR_CTL_FUNC2_EN ) );

    VERIFY_RESULT( wwd_bus_read_register_value( BUS_FUNCTION, SDIOD_CCCR_IORDY, (uint8_t) 1, &byte_data ) );

    elapsed_time = host_rtos_get_time( );
    result = wwd_bus_sdio_download_firmware( );
    elapsed_time = host_rtos_get_time( ) - elapsed_time;
    host_platform_resource_size( WWD_RESOURCE_WLAN_FIRMWARE, &wifi_firmware_image_size );
    WPRINT_WICED_TEST( ("WLAN FW download size: %lu bytes\n", wifi_firmware_image_size) );
    WPRINT_WICED_TEST( ("WLAN FW download time: %lu ms\n", elapsed_time) );

    if ( result != WWD_SUCCESS )
    {
        /*  either an error or user abort */
        return result;
    }

    /* Wait for F2 to be ready */
    loop_count = 0;
    while ( ( ( result = wwd_bus_read_register_value( BUS_FUNCTION, SDIOD_CCCR_IORDY, (uint8_t) 1, &byte_data ) ) == WWD_SUCCESS ) &&
            ( ( byte_data & SDIO_FUNC_READY_2 ) == 0 ) &&
            ( loop_count < (uint32_t) F2_READY_TIMEOUT_MS ) )
    {
        (void) host_rtos_delay_milliseconds( (uint32_t) 1 ); /* Ignore return - nothing can be done if it fails */
        loop_count++;
    }
    if ( loop_count >= (uint32_t) F2_READY_TIMEOUT_MS )
    {
        /* If your system fails here, it could be due to incorrect NVRAM variables.
         * Check which 'wifi_nvram_image.h' file your platform is using, and
         * check that it matches the WLAN device on your platform, including the
         * crystal frequency.
         */
        WPRINT_WWD_ERROR(("Timeout while waiting for function 2 to be ready\n"));
        /*@-unreachable@*/ /* Reachable after hitting assert */
        return WWD_TIMEOUT;
        /*@+unreachable@*/
    }

    wwd_chip_specific_init();
    VERIFY_RESULT( wwd_ensure_wlan_bus_is_up( ) );

    UNUSED_PARAMETER( elapsed_time );
    return result;
}

wwd_result_t wwd_bus_deinit( void )
{
    wwd_allow_wlan_bus_to_sleep();

    /* put device in reset. */
    host_platform_reset_wifi( WICED_TRUE );

    wwd_bus_set_resource_download_halt( WICED_FALSE );

    DELAYED_BUS_RELEASE_SCHEDULE( WICED_FALSE );

    return WWD_SUCCESS;
}

wwd_result_t wwd_bus_ack_interrupt(uint32_t intstatus)
{
    return wwd_bus_write_backplane_value( (uint32_t) SDIO_INT_STATUS, (uint8_t) 4, intstatus);
}

wiced_bool_t wwd_bus_wake_interrupt_present( void )
{
    uint32_t int_status = 0;

    /* Ensure the wlan backplane bus is up */
    if( WWD_SUCCESS != wwd_ensure_wlan_bus_is_up() )
        return WICED_FALSE;

    if ( wwd_bus_read_backplane_value( (uint32_t) SDIO_INT_STATUS, (uint8_t) 4, (uint8_t*)&int_status ) != WWD_SUCCESS )
    {
        WPRINT_WWD_ERROR(("%s: Error reading interrupt status\n", __FUNCTION__));
        goto exit;
    }
    if ( ( I_HMB_HOST_INT & int_status ) != 0 )
    {
    /* Clear any interrupts */
        if ( wwd_bus_write_backplane_value( (uint32_t) SDIO_INT_STATUS, (uint8_t) 4, I_HMB_HOST_INT ) != WWD_SUCCESS )
        {
           WPRINT_WWD_ERROR(("%s: Error clearing interrupts\n", __FUNCTION__));
           goto exit;
        }
        if ( wwd_bus_read_backplane_value( (uint32_t) SDIO_INT_STATUS, (uint8_t) 4, (uint8_t*)&int_status) != WWD_SUCCESS )
        {
           WPRINT_WWD_ERROR(("%s: Error reading interrupt status\n", __FUNCTION__));
           goto exit;
        }
        WPRINT_WWD_DEBUG(("wwd_bus_wake_interrupt_present after clearing int_status  = [%x]\n", (uint8_t)int_status));
        return WICED_TRUE;
    }
exit:
    return WICED_FALSE;
}

uint32_t wwd_bus_packet_available_to_read(void)
{
    uint32_t int_status = 0;

    /* Ensure the wlan backplane bus is up */
    VERIFY_RESULT( wwd_ensure_wlan_bus_is_up() );

    /* Read the IntStatus */
    if ( wwd_bus_read_backplane_value( (uint32_t) SDIO_INT_STATUS, (uint8_t) 4, (uint8_t*)&int_status ) != WWD_SUCCESS )
    {
        WPRINT_WWD_ERROR(("%s: Error reading interrupt status\n", __FUNCTION__));
        int_status = 0;
        goto exit;
    }

    if ( ( HOSTINTMASK & int_status ) != 0 )
    {
        /* Clear any interrupts */
        if ( wwd_bus_write_backplane_value( (uint32_t) SDIO_INT_STATUS, (uint8_t) 4, int_status & HOSTINTMASK ) != WWD_SUCCESS )
        {
            WPRINT_WWD_ERROR(("%s: Error clearing interrupts\n", __FUNCTION__));
            int_status = 0;
            goto exit;
        }
    }

#if defined(WICED_PLATFORM_MASKS_BUS_IRQ)
    host_platform_unmask_sdio_interrupt();
#endif
exit:
    return ((int_status) & (FRAME_AVAILABLE_MASK));
}

/*
 * From internal documentation: hwnbu-twiki/SdioMessageEncapsulation
 * When data is available on the device, the device will issue an interrupt:
 * - the device should signal the interrupt as a hint that one or more data frames may be available on the device for reading
 * - the host may issue reads of the 4 byte length tag at any time -- that is, whether an interupt has been issued or not
 * - if a frame is available, the tag read should return a nonzero length (>= 4) and the host can then read the remainder of the frame by issuing one or more CMD53 reads
 * - if a frame is not available, the 4byte tag read should return zero
 */
wwd_result_t wwd_bus_read_frame( /*@out@*/ wiced_buffer_t* buffer )
{
    uint16_t hwtag[8];
    uint16_t extra_space_required;
    wwd_result_t result;

    *buffer = NULL;

    /* Ensure the wlan backplane bus is up */
    VERIFY_RESULT( wwd_ensure_wlan_bus_is_up() );

    /* Read the frame header and verify validity */
    memset( hwtag, 0, sizeof(hwtag) );

    result = wwd_bus_sdio_transfer(BUS_READ, WLAN_FUNCTION, 0, (uint16_t) INITIAL_READ, (uint8_t*)hwtag, RESPONSE_NEEDED);
    if ( result != WWD_SUCCESS )
    {
        (void) wwd_bus_sdio_abort_read( WICED_FALSE ); /* ignore return - not much can be done if this fails */
        return WWD_SDIO_RX_FAIL;
    }

    if ( ( ( hwtag[0] | hwtag[1] ) == 0                 ) ||
         ( ( hwtag[0] ^ hwtag[1] ) != (uint16_t) 0xFFFF ) )
    {
        return WWD_HWTAG_MISMATCH;
    }

    if ( ( hwtag[0] == (uint16_t) 12 ) &&
         ( wwd_wlan_status.state == WLAN_UP) )
    {
        result = wwd_bus_sdio_transfer(BUS_READ, WLAN_FUNCTION, 0, (uint16_t) 8, (uint8_t*) &hwtag[2], RESPONSE_NEEDED);
        if ( result != WWD_SUCCESS )
        {
            (void) wwd_bus_sdio_abort_read( WICED_FALSE ); /* ignore return - not much can be done if this fails */
            return WWD_SDIO_RX_FAIL;
        }
        wwd_sdpcm_update_credit((uint8_t*)hwtag);
        return WWD_SUCCESS;
    }

    /* Calculate the space we need to store entire packet */
    if ( ( hwtag[0] > (uint16_t) INITIAL_READ ) )
    {
        extra_space_required = (uint16_t) ( hwtag[0] - (uint16_t) INITIAL_READ );
    }
    else
    {
        extra_space_required = 0;
    }

    /* Allocate a suitable buffer */
    result = host_buffer_get( buffer, WWD_NETWORK_RX, (unsigned short) ( (uint16_t) INITIAL_READ + extra_space_required + (uint16_t) sizeof(wwd_buffer_header_t) ), WICED_FALSE );
    if ( result != WWD_SUCCESS )
    {
        /* Read out the first 12 bytes to get the bus credit information, 4 bytes are already read in hwtag */
        wiced_assert( "Get buffer error", ( ( result == WWD_BUFFER_UNAVAILABLE_TEMPORARY ) || ( result == WWD_BUFFER_UNAVAILABLE_PERMANENT ) ) );
        result = wwd_bus_sdio_transfer(BUS_READ, WLAN_FUNCTION, 0, (uint16_t) 8, (uint8_t*) &hwtag[2], RESPONSE_NEEDED);
        if ( result != WWD_SUCCESS )
        {
            (void) wwd_bus_sdio_abort_read( WICED_FALSE ); /* ignore return - not much can be done if this fails */
            return WWD_SDIO_RX_FAIL;
        }
        result = wwd_bus_sdio_abort_read( WICED_FALSE );
        wiced_assert( "Read-abort failed", result==WWD_SUCCESS );
        REFERENCE_DEBUG_ONLY_VARIABLE( result );

        wwd_sdpcm_update_credit( (uint8_t *)hwtag );
        return WWD_RX_BUFFER_ALLOC_FAIL;
    }

    /* Copy the data already read */
    memcpy( host_buffer_get_current_piece_data_pointer( *buffer ) + sizeof(wwd_buffer_header_t), hwtag, (size_t) INITIAL_READ );

    /* Read the rest of the data */
    if ( extra_space_required > 0 )
    {
        result = wwd_bus_sdio_transfer(BUS_READ, WLAN_FUNCTION, 0, extra_space_required, host_buffer_get_current_piece_data_pointer( *buffer ) + sizeof(wwd_buffer_header_t) + INITIAL_READ, RESPONSE_NEEDED);
        if ( result != WWD_SUCCESS )
        {
            (void) wwd_bus_sdio_abort_read( WICED_FALSE ); /* ignore return - not much can be done if this fails */
            host_buffer_release(*buffer, WWD_NETWORK_RX);
            return WWD_SDIO_RX_FAIL;
        }
    }
    DELAYED_BUS_RELEASE_SCHEDULE( WICED_TRUE );
    return WWD_SUCCESS;
}


/******************************************************
 *     Function definitions for Protocol Common
 ******************************************************/

/* Device register access functions */
wwd_result_t wwd_bus_write_backplane_value( uint32_t address, uint8_t register_length, uint32_t value )
{
    VERIFY_RESULT( wwd_bus_set_backplane_window( address ) );

    address &= SBSDIO_SB_OFT_ADDR_MASK;

    if (register_length == 4)
        address |= SBSDIO_SB_ACCESS_2_4B_FLAG;

    VERIFY_RESULT( wwd_bus_sdio_transfer( BUS_WRITE, BACKPLANE_FUNCTION, address, register_length, (uint8_t*) &value, RESPONSE_NEEDED ));

    return wwd_bus_set_backplane_window( CHIPCOMMON_BASE_ADDRESS );
}

wwd_result_t wwd_bus_read_backplane_value( uint32_t address, uint8_t register_length, /*@out@*/ uint8_t* value )
{
    *value = 0;
    VERIFY_RESULT( wwd_bus_set_backplane_window( address ) );

    address &= SBSDIO_SB_OFT_ADDR_MASK;

    if (register_length == 4)
        address |= SBSDIO_SB_ACCESS_2_4B_FLAG;

    VERIFY_RESULT( wwd_bus_sdio_transfer( BUS_READ, BACKPLANE_FUNCTION, address, register_length, value, RESPONSE_NEEDED ));

    return wwd_bus_set_backplane_window( CHIPCOMMON_BASE_ADDRESS );
}

wwd_result_t wwd_bus_write_register_value( wwd_bus_function_t function, uint32_t address, uint8_t value_length, uint32_t value )
{
    return wwd_bus_sdio_transfer( BUS_WRITE, function, address, value_length, (uint8_t*) &value, RESPONSE_NEEDED );
}

wwd_result_t wwd_bus_transfer_bytes( wwd_bus_transfer_direction_t direction, wwd_bus_function_t function, uint32_t address, uint16_t size, /*@in@*/ /*@out@*/ wwd_transfer_bytes_packet_t* data )
{
    return wwd_bus_sdio_transfer( direction, function, address, size, (uint8_t*)data, RESPONSE_NEEDED );
}

/******************************************************
 *             Static  Function definitions
 ******************************************************/

static wwd_result_t wwd_bus_sdio_transfer( wwd_bus_transfer_direction_t direction, wwd_bus_function_t function, uint32_t address, uint16_t data_size, /*@in@*/ /*@out@*/ uint8_t* data, sdio_response_needed_t response_expected )
{
    /* Note: this function had broken retry logic (never retried), which has been removed.
       *   Failing fast helps problems on the bus get brought to light more quickly
       *   and preserves the original behavior.
       */
    if ( data_size == (uint16_t) 1 )
    {
        return wwd_bus_sdio_cmd52( direction, function, address, *data, response_expected, data );
    }
    else
    {
        return wwd_bus_sdio_cmd53( direction, function, ( data_size >= (uint16_t) 64 ) ? SDIO_BLOCK_MODE : SDIO_BYTE_MODE, address, data_size, data, response_expected, NULL );
    }
}

static wwd_result_t wwd_bus_sdio_cmd52( wwd_bus_transfer_direction_t direction, wwd_bus_function_t function, uint32_t address, uint8_t value, sdio_response_needed_t response_expected, uint8_t* response )
{
    uint32_t sdio_response;
    wwd_result_t result;
    sdio_cmd_argument_t arg;
    arg.value = 0;
    arg.cmd52.function_number  = (unsigned int) ( function & BUS_FUNCTION_MASK );
    arg.cmd52.register_address = (unsigned int) ( address & 0x00001ffff );
    arg.cmd52.rw_flag = (unsigned int) ( ( direction == BUS_WRITE ) ? 1 : 0 );
    arg.cmd52.write_data = value;
    WWD_BUS_STATS_INCREMENT_VARIABLE( cmd52 );
    result = host_platform_sdio_transfer( direction, SDIO_CMD_52, SDIO_BYTE_MODE, SDIO_1B_BLOCK, arg.value, 0, 0, response_expected, &sdio_response );
    WWD_BUS_STATS_CONDITIONAL_INCREMENT_VARIABLE(( result != WWD_SUCCESS ), cmd52_fail );
    if ( response != NULL )
    {
        *response = (uint8_t) ( sdio_response & 0x00000000ff );
    }
    return result;
}

static wwd_result_t wwd_bus_sdio_cmd53( wwd_bus_transfer_direction_t direction, wwd_bus_function_t function, sdio_transfer_mode_t mode, uint32_t address, uint16_t data_size, /*@in@*/ /*@out@*/ uint8_t* data, sdio_response_needed_t response_expected, /*@null@*/ uint32_t* response )
{
    sdio_cmd_argument_t arg;
    wwd_result_t result;

    if ( direction == BUS_WRITE )
    {
        WWD_BUS_STATS_INCREMENT_VARIABLE( cmd53_write );
        add_log_entry(direction, function, address, data_size, data);
    }

    arg.value = 0;
    arg.cmd53.function_number  = (unsigned int) ( function & BUS_FUNCTION_MASK );
    arg.cmd53.register_address = (unsigned int) ( address & BIT_MASK( 17 ) );
    arg.cmd53.op_code = (unsigned int) 1;
    arg.cmd53.rw_flag = (unsigned int) ( ( direction == BUS_WRITE ) ? 1 : 0 );
    if ( mode == SDIO_BYTE_MODE )
    {
        wiced_assert( "wwd_bus_sdio_cmd53: data_size > 512 for byte mode", ( data_size <= (uint16_t) 512 ) );
        arg.cmd53.count = (unsigned int) ( data_size & 0x1FF );

        result = host_platform_sdio_transfer( direction, SDIO_CMD_53, mode, SDIO_64B_BLOCK, arg.value, (uint32_t*) data, data_size, response_expected, response );
        if( result != WWD_SUCCESS )
        {
            goto done;
        }
    }
    else
    {
#ifndef WICED_PLATFORM_DOESNT_USE_TEMP_DMA_BUFFER
        arg.cmd53.count = (unsigned int) ( ( data_size / (uint16_t)SDIO_64B_BLOCK ) & BIT_MASK( 9 ) );
        if ( (uint32_t) ( arg.cmd53.count * (uint16_t)SDIO_64B_BLOCK ) < data_size )
        {
            ++arg.cmd53.count;
        }
        arg.cmd53.block_mode = (unsigned int) 1;
        result = host_platform_sdio_transfer( direction, SDIO_CMD_53, mode, SDIO_64B_BLOCK, arg.value, (uint32_t*) data, data_size, response_expected, response );
        if( result != WWD_SUCCESS )
        {
            goto done;
        }
#else /* WICED_PLATFORM_DOESNT_USE_TEMP_DMA_BUFFER */
        uint16_t sent_data_size = (uint16_t)((( data_size / (uint16_t)SDIO_64B_BLOCK ) & BIT_MASK( 9 ) ) * SDIO_64B_BLOCK);


        result = host_platform_sdio_transfer( direction, SDIO_CMD_53, SDIO_BLOCK_MODE, SDIO_64B_BLOCK, arg.value, (uint32_t*) data, sent_data_size, response_expected, response );
        if( result != WWD_SUCCESS )
        {
            goto done;
        }

        if ( data_size > sent_data_size ) /* Send the remaining bytes using CMD53 Byte mode */
        {
            arg.cmd53.register_address = ( unsigned int ) ( arg.cmd53.register_address + sent_data_size ) & BIT_MASK( 17 );
            arg.cmd53.count = (unsigned int) ( ( data_size - sent_data_size ) & BIT_MASK( 9 ) );
            arg.cmd53.block_mode = (unsigned int) 0;
            result = host_platform_sdio_transfer( direction, SDIO_CMD_53, SDIO_BYTE_MODE, SDIO_1B_BLOCK, arg.value, (uint32_t*) ( data + sent_data_size ), (uint16_t)( data_size - sent_data_size ), response_expected, response );
            if( result != WWD_SUCCESS )
            {
                goto done;
            }
        }
#endif /* WICED_PLATFORM_DOESNT_USE_TEMP_DMA_BUFFER */
    }
    if ( direction == BUS_READ )
    {
        WWD_BUS_STATS_INCREMENT_VARIABLE( cmd53_read );
        add_log_entry(direction, function, address, data_size, data);
    }

done:
    WWD_BUS_STATS_CONDITIONAL_INCREMENT_VARIABLE((( result != WWD_SUCCESS ) && ( direction == BUS_READ )), cmd53_read_fail );
    WWD_BUS_STATS_CONDITIONAL_INCREMENT_VARIABLE((( result != WWD_SUCCESS ) && ( direction == BUS_WRITE )), cmd53_write_fail );
    return result;
}

static wwd_result_t wwd_bus_sdio_download_firmware( void )
{
    uint8_t csr_val = 0;
    wwd_result_t result;
    uint32_t loop_count;

#ifdef WLAN_ARM_CR4
    VERIFY_RESULT( wwd_reset_core( WLAN_ARM_CORE,SICF_CPUHALT,SICF_CPUHALT) );
#else
    VERIFY_RESULT( wwd_disable_device_core( WLAN_ARM_CORE, WLAN_CORE_FLAG_NONE ) );
    VERIFY_RESULT( wwd_disable_device_core( SOCRAM_CORE, WLAN_CORE_FLAG_NONE ) );
    VERIFY_RESULT( wwd_reset_device_core( SOCRAM_CORE, WLAN_CORE_FLAG_NONE ) );

    VERIFY_RESULT( wwd_chip_specific_socsram_init( ));
#endif
 #if 0
    /* 43362 specific: Remap JTAG pins to UART output */
    uint32_t data = 0;
    VERIFY_RESULT( wwd_bus_write_backplane_value(0x18000650, 1, 1) );
    VERIFY_RESULT( wwd_bus_read_backplane_value(0x18000654, 4, (uint8_t*)&data) );
    data |= (1 << 24);
    VERIFY_RESULT( wwd_bus_write_backplane_value(0x18000654, 4, data) );
#endif


#ifdef MFG_TEST_ALTERNATE_WLAN_DOWNLOAD
    VERIFY_RESULT( external_write_wifi_firmware_and_nvram_image( ) );
#else
    result = wwd_bus_write_wifi_firmware_image( );

    if ( result == WWD_UNFINISHED )
    {
        WPRINT_WWD_INFO(("User aborted fw download\n"));
        /* user aborted */
        return result;
    }
    else if ( result != WWD_SUCCESS )
    {
        wiced_assert( "Failed to load wifi firmware\n", result == WWD_SUCCESS);
        return result;
    }

    VERIFY_RESULT( wwd_bus_write_wifi_nvram_image( ) );
#endif /* ifdef MFG_TEST_ALTERNATE_WLAN_DOWNLOAD */

    /* Take the ARM core out of reset */
#ifdef WLAN_ARM_CR4
    VERIFY_RESULT( wwd_reset_core( WLAN_ARM_CORE,0, 0 ) );
#else
    VERIFY_RESULT( wwd_reset_device_core( WLAN_ARM_CORE, WLAN_CORE_FLAG_NONE ) );

    result = wwd_device_core_is_up( WLAN_ARM_CORE );
    if ( result != WWD_SUCCESS )
    {
         WPRINT_WWD_ERROR(("Could not bring ARM core up\n"));
         /*@-unreachable@*/ /* Reachable after hitting assert */
         return result;
         /*@+unreachable@*/
    }
#endif


    /* Wait until the High Throughput clock is available */
    loop_count = 0;
    while ( ( ( result = wwd_bus_read_register_value( BACKPLANE_FUNCTION, SDIO_CHIP_CLOCK_CSR, (uint8_t) 1, &csr_val ) ) == WWD_SUCCESS ) &&
            ( ( csr_val & SBSDIO_HT_AVAIL ) == 0 ) &&
            ( loop_count < (uint32_t) HT_AVAIL_TIMEOUT_MS ) )
    {
        (void) host_rtos_delay_milliseconds( (uint32_t) 1 ); /* Ignore return - nothing can be done if it fails */
        loop_count++;
    }
    if ( loop_count >= (uint32_t) HT_AVAIL_TIMEOUT_MS )
    {
        /* If your system times out here, it means that the WLAN firmware is not booting.
         * Check that your WLAN chip matches the 'wifi_image.c' being built - in GNU toolchain, $(CHIP)
         * makefile variable must be correct.
         */
         WPRINT_WWD_ERROR(("Timeout while waiting for high throughput clock\n"));
         /*@-unreachable@*/ /* Reachable after hitting assert */
         return WWD_TIMEOUT;
         /*@+unreachable@*/
    }
    if ( result != WWD_SUCCESS )
    {
        WPRINT_WWD_ERROR(("Error while waiting for high throughput clock\n"));
        /*@-unreachable@*/ /* Reachable after hitting assert */
        return result;
        /*@+unreachable@*/
    }

    /* Set up the interrupt mask and enable interrupts */
    VERIFY_RESULT( wwd_bus_write_backplane_value( SDIO_INT_HOST_MASK, (uint8_t) 4, HOSTINTMASK ) );

    /* Enable F2 interrupts. This wasn't required for 4319 but is for the 43362 */
    VERIFY_RESULT( wwd_bus_write_backplane_value( SDIO_FUNCTION_INT_MASK, (uint8_t) 1, SDIO_FUNC_MASK_F1 | SDIO_FUNC_MASK_F2) );

    /* Lower F2 Watermark to avoid DMA Hang in F2 when SD Clock is stopped. */
    VERIFY_RESULT( wwd_bus_write_register_value( BACKPLANE_FUNCTION, SDIO_FUNCTION2_WATERMARK, (uint8_t) 1, (uint32_t) SDIO_F2_WATERMARK ) );

    return WWD_SUCCESS;
}

/** Aborts a SDIO read of a packet from the 802.11 device
 *
 * This function is necessary because the only way to obtain the size of the next
 * available received packet is to read the first four bytes of the packet.
 * If the system reads these four bytes, and then fails to allocate the required
 * memory, then this function allows the system to abort the packet read cleanly,
 * and to optionally tell the 802.11 device to keep it allowing reception once
 * memory is available.
 *
 * In order to do this abort, the following actions are performed:
 * - Sets abort bit for Function 2 (WLAN Data) to request stopping transfer
 * - Sets Read Frame Termination bit to flush and reset fifos
 * - If packet is to be kept and resent by 802.11 device, a NAK  is sent
 * - Wait whilst the Fifo is emptied of the packet ( reading during this period would cause all zeros to be read )
 *
 * @param retry : WICED_TRUE if 802.11 device is to keep and resend packet
 *                WICED_FALSE if 802.11 device is to drop packet
 *
 * @return WWD_SUCCESS if successful, otherwise error code
 */
static wwd_result_t wwd_bus_sdio_abort_read( wiced_bool_t retry )
{
    WWD_BUS_STATS_INCREMENT_VARIABLE( read_aborts );

    /* Abort transfer on WLAN_FUNCTION */
    VERIFY_RESULT( wwd_bus_write_register_value( BUS_FUNCTION, SDIOD_CCCR_IOABORT, (uint8_t) 1, (uint32_t) WLAN_FUNCTION ) );

    /* Send frame terminate */
    VERIFY_RESULT( wwd_bus_write_register_value( BACKPLANE_FUNCTION, SDIO_FRAME_CONTROL, (uint8_t) 1, SFC_RF_TERM ) );

    /* If we want to retry message, send NAK */
    if ( retry == WICED_TRUE )
    {
        VERIFY_RESULT( wwd_bus_write_backplane_value( (uint32_t) SDIO_TO_SB_MAIL_BOX, (uint8_t) 1, SMB_NAK ) );
    }

    return WWD_SUCCESS;
}

wwd_result_t wwd_bus_read_register_value( wwd_bus_function_t function, uint32_t address, uint8_t value_length, /*@out@*/ uint8_t* value )
{
    memset( value, 0, (size_t) value_length );
    return wwd_bus_sdio_transfer( BUS_READ, function, address, value_length, value, RESPONSE_NEEDED );
}

wwd_result_t wwd_bus_poke_wlan( void )
{
    /*TODO: change 1<<3 to a register hash define */
    return wwd_bus_write_backplane_value( SDIO_TO_SB_MAILBOX, (uint8_t) 4, (uint32_t)( 1 << 3 ) );
}

wwd_result_t wwd_bus_set_flow_control( uint8_t value )
{
    if ( value != 0 )
    {
        wwd_bus_flow_controlled = WICED_TRUE;
    }
    else
    {
        wwd_bus_flow_controlled = WICED_FALSE;
    }
    return WWD_SUCCESS;
}

wiced_bool_t wwd_bus_is_flow_controlled( void )
{
    return wwd_bus_flow_controlled;
}

wwd_result_t wwd_bus_specific_wakeup( void )
{
    return WWD_SUCCESS;
}

wwd_result_t wwd_bus_specific_sleep( void )
{
    return WWD_SUCCESS;
}

#ifndef WICED_DISABLE_MCU_POWERSAVE
static wwd_result_t wwd_bus_sdio_set_oob_interrupt( uint8_t gpio_pin_number )
{
    if ( gpio_pin_number == 1 )
    {
        /* Redirect to OOB interrupt to GPIO1 */
        VERIFY_RESULT( wwd_bus_write_register_value( BACKPLANE_FUNCTION, SDIO_GPIO_SELECT, (uint8_t)1, (uint32_t) 0xF ) );
        VERIFY_RESULT( wwd_bus_write_register_value( BACKPLANE_FUNCTION, SDIO_GPIO_OUTPUT, (uint8_t)1, (uint32_t) 0x0 ) );

        /* Enable GPIOx (bit x) */
        VERIFY_RESULT( wwd_bus_write_register_value( BACKPLANE_FUNCTION, SDIO_GPIO_ENABLE, (uint8_t)1, (uint32_t)0x2 ) );

        /* Set GPIOx (bit x) on Chipcommon GPIO Control register */
        VERIFY_RESULT( wwd_bus_write_register_value( BACKPLANE_FUNCTION, CHIPCOMMON_GPIO_CONTROL, (uint8_t)4, (uint32_t)0x2 ) );
    }

    return WWD_SUCCESS;
}
#endif

void wwd_bus_init_stats( void )
{
#ifdef WWD_ENABLE_STATS
    memset( &wwd_bus_stats, 0, sizeof(wwd_bus_stats) );
#endif /* WWD_ENABLE_STATS */
}

wwd_result_t wwd_bus_print_stats( wiced_bool_t reset_after_print )
{
#ifdef WWD_ENABLE_STATS
    WPRINT_MACRO(( "Bus Stats.. \n"
                   "cmd52:%ld, cmd53_read:%ld, cmd53_write:%ld\n"
                   "cmd52_fail:%ld, cmd53_read_fail:%ld, cmd53_write_fail:%ld\n"
                   "oob_intrs:%ld, sdio_intrs:%ld, error_intrs:%ld, read_aborts:%ld\n",
                   wwd_bus_stats.cmd52, wwd_bus_stats.cmd53_read, wwd_bus_stats.cmd53_write,
                   wwd_bus_stats.cmd52_fail, wwd_bus_stats.cmd53_read_fail, wwd_bus_stats.cmd53_write_fail,
                   wwd_bus_stats.oob_intrs, wwd_bus_stats.sdio_intrs, wwd_bus_stats.error_intrs, wwd_bus_stats.read_aborts ));

    if ( reset_after_print == WICED_TRUE )
    {
        memset( &wwd_bus_stats, 0, sizeof(wwd_bus_stats) );
    }
    return WWD_SUCCESS;
#else /* WWD_ENABLE_STATS */
    UNUSED_VARIABLE(reset_after_print);
    return WWD_DOES_NOT_EXIST;
#endif /* WWD_ENABLE_STATS */
}

/* Waking the firmware up from Deep Sleep */
wwd_result_t wwd_bus_reinit( wiced_bool_t wake_from_firmware )
{
    wwd_result_t result = WWD_SUCCESS;
    uint8_t        byte_data;
    uint32_t       loop_count;
    loop_count = 0;

    /* Setup the backplane*/
    loop_count = 0;

    do
    {
       /* Enable function 1 (backplane) */
       VERIFY_RESULT( wwd_bus_write_register_value( BUS_FUNCTION, SDIOD_CCCR_IOEN, (uint8_t) 1, SDIO_FUNC_ENABLE_1 ) );
       if (loop_count != 0)
       {
          (void) host_rtos_delay_milliseconds( (uint32_t) 1 );  /* Ignore return - nothing can be done if it fails */
       }

       VERIFY_RESULT( wwd_bus_read_register_value ( BUS_FUNCTION, SDIOD_CCCR_IOEN, (uint8_t) 1, &byte_data ) );
       loop_count++;
       if ( loop_count >= (uint32_t) F0_WORKING_TIMEOUT_MS )
       {
            WPRINT_WWD_ERROR(("Timeout on CCCR update\n"));
            return WWD_TIMEOUT;
       }
    }
    while (byte_data != (uint8_t) SDIO_FUNC_ENABLE_1);

#ifndef SDIO_1_BIT
    /* Read the bus width and set to 4 bits */
    VERIFY_RESULT( wwd_bus_read_register_value (BUS_FUNCTION, SDIOD_CCCR_BICTRL, (uint8_t) 1, &byte_data) );
    VERIFY_RESULT( wwd_bus_write_register_value(BUS_FUNCTION, SDIOD_CCCR_BICTRL, (uint8_t) 1, (byte_data & (~BUS_SD_DATA_WIDTH_MASK)) | BUS_SD_DATA_WIDTH_4BIT ) );
    /* NOTE: We don't need to change our local bus settings since we're not sending any data (only using CMD52) until after we change the bus speed further down */
#endif

    /* Set the block size */
    /* Wait till the backplane is ready */
    loop_count = 0;
    while ( ( ( result = wwd_bus_write_register_value( BUS_FUNCTION, SDIOD_CCCR_BLKSIZE_0, (uint8_t) 1, (uint32_t) SDIO_64B_BLOCK ) ) == WWD_SUCCESS ) &&
            ( ( result = wwd_bus_read_register_value ( BUS_FUNCTION, SDIOD_CCCR_BLKSIZE_0, (uint8_t) 1, &byte_data                ) ) == WWD_SUCCESS ) &&
            ( byte_data != (uint8_t)  SDIO_64B_BLOCK ) &&
            ( loop_count < (uint32_t) F0_WORKING_TIMEOUT_MS ) )
    {
        (void) host_rtos_delay_milliseconds( (uint32_t) 1 );  /* Ignore return - nothing can be done if it fails */
        loop_count++;
        if ( loop_count >= (uint32_t) F0_WORKING_TIMEOUT_MS )
        {
            /* If the system fails here, check the high frequency crystal is working */
            WPRINT_WWD_ERROR(("Timeout while setting block size\n"));
            return WWD_TIMEOUT;
        }
    }

    VERIFY_RESULT( result );

    WPRINT_WWD_DEBUG(("Modding registers for blocks\n"));

    VERIFY_RESULT( wwd_bus_write_register_value( BUS_FUNCTION, SDIOD_CCCR_BLKSIZE_0,   (uint8_t) 1, (uint32_t) SDIO_64B_BLOCK ) );
    VERIFY_RESULT( wwd_bus_write_register_value( BUS_FUNCTION, SDIOD_CCCR_F1BLKSIZE_0, (uint8_t) 1, (uint32_t) SDIO_64B_BLOCK ) );
    VERIFY_RESULT( wwd_bus_write_register_value( BUS_FUNCTION, SDIOD_CCCR_F2BLKSIZE_0, (uint8_t) 1, (uint32_t) SDIO_64B_BLOCK ) );
    VERIFY_RESULT( wwd_bus_write_register_value( BUS_FUNCTION, SDIOD_CCCR_F2BLKSIZE_1, (uint8_t) 1, (uint32_t) 0              ) ); /* Function 2 = 64 */

    /* Enable/Disable Client interrupts */
    VERIFY_RESULT( wwd_bus_write_register_value( BUS_FUNCTION, SDIOD_CCCR_INTEN,       (uint8_t) 1, INTR_CTL_MASTER_EN | INTR_CTL_FUNC1_EN | INTR_CTL_FUNC2_EN ) );

#ifdef HIGH_SPEED_SDIO_CLOCK
    WPRINT_WWD_DEBUG(("SDIO HS clock enable\n"));

    /* This code is required if we want more than 25 MHz clock */
    VERIFY_RESULT( wwd_bus_read_register_value( BUS_FUNCTION, SDIOD_CCCR_SPEED_CONTROL, 1, &byte_data ) );
    if ( ( byte_data & 0x1 ) != 0 )
    {
        VERIFY_RESULT( wwd_bus_write_register_value( BUS_FUNCTION, SDIOD_CCCR_SPEED_CONTROL, 1, byte_data | SDIO_SPEED_EHS ) );
    }
    else
    {
        return WWD_BUS_READ_REGISTER_ERROR;
    }
#endif /* HIGH_SPEED_SDIO_CLOCK */

    /* Switch to high speed mode and change to 4 bit mode */
    VERIFY_RESULT( host_platform_enable_high_speed_sdio( ));
    /* Wait till the backplane is ready */
    loop_count = 0;
    while ( ( ( result = wwd_bus_read_register_value( BUS_FUNCTION, SDIOD_CCCR_IORDY, (uint8_t) 1, &byte_data ) ) == WWD_SUCCESS ) &&
            ( ( byte_data & SDIO_FUNC_READY_1 ) == 0 ) &&
            ( loop_count < (uint32_t) F1_AVAIL_TIMEOUT_MS ) )
    {
        (void) host_rtos_delay_milliseconds( (uint32_t) 1 ); /* Ignore return - nothing can be done if it fails */
        loop_count++;
    }

    if ( loop_count >= (uint32_t) F1_AVAIL_TIMEOUT_MS )
    {
        WPRINT_WWD_ERROR(("Timeout while waiting for backplane to be ready\n"));
        return WWD_TIMEOUT;
    }
    VERIFY_RESULT( result );

    /* Set the ALP */
    VERIFY_RESULT( wwd_bus_write_register_value( BACKPLANE_FUNCTION, SDIO_CHIP_CLOCK_CSR, (uint8_t) 1, (uint32_t)( SBSDIO_FORCE_HW_CLKREQ_OFF | SBSDIO_ALP_AVAIL_REQ | SBSDIO_FORCE_ALP ) ) );
    loop_count = 0;
    while ( ( ( result = wwd_bus_read_register_value( BACKPLANE_FUNCTION, SDIO_CHIP_CLOCK_CSR, (uint8_t) 1, &byte_data ) ) != WWD_SUCCESS ) ||
            ( ( ( byte_data & SBSDIO_ALP_AVAIL ) == 0 ) &&
            ( loop_count < (uint32_t) ALP_AVAIL_TIMEOUT_MS ) ) )
    {
        (void) host_rtos_delay_milliseconds( (uint32_t) 1 ); /* Ignore return - nothing can be done if it fails */
        loop_count++;
    }
    if ( loop_count >= (uint32_t) ALP_AVAIL_TIMEOUT_MS )
    {
        WPRINT_WWD_ERROR(("Timeout while waiting for alp clock\n"));
        return WWD_TIMEOUT;
    }
    VERIFY_RESULT( result );

    /* Clear request for ALP */
    VERIFY_RESULT( wwd_bus_write_register_value( BACKPLANE_FUNCTION, SDIO_CHIP_CLOCK_CSR, (uint8_t) 1, 0 ) );

    /* Disable the extra SDIO pull-ups */
#ifndef WWD_BUS_SDIO_USE_WLAN_SDIO_PULLUPS
    VERIFY_RESULT( wwd_bus_write_register_value( BACKPLANE_FUNCTION, SDIO_PULL_UP,  (uint8_t) 1, 0 ) );
#endif

    /* Enable F1 and F2 */
    VERIFY_RESULT( wwd_bus_write_register_value( BUS_FUNCTION, SDIOD_CCCR_IOEN, (uint8_t) 1, SDIO_FUNC_ENABLE_1 | SDIO_FUNC_ENABLE_2 ) );

#ifndef WICED_DISABLE_MCU_POWERSAVE
    /* Enable out-of-band interrupt */
    VERIFY_RESULT( wwd_bus_write_register_value( BUS_FUNCTION, SDIOD_SEP_INT_CTL, (uint8_t) 1, SEP_INTR_CTL_MASK | SEP_INTR_CTL_EN | SEP_INTR_CTL_POL ) );

    /* Set OOB interrupt to the correct WLAN GPIO pin */
    VERIFY_RESULT( wwd_bus_sdio_set_oob_interrupt( host_platform_get_oob_interrupt_pin( ) ) );

    VERIFY_RESULT( host_enable_oob_interrupt( ) );
#endif /* ifndef WICED_DISABLE_MCU_POWERSAVE */

    /* Enable F2 interrupt only */
    VERIFY_RESULT( wwd_bus_write_register_value( BUS_FUNCTION, SDIOD_CCCR_INTEN, (uint8_t) 1, INTR_CTL_MASTER_EN | INTR_CTL_FUNC2_EN ) );

    VERIFY_RESULT( wwd_bus_read_register_value( BUS_FUNCTION, SDIOD_CCCR_IORDY, (uint8_t) 1, &byte_data ) );

    result = wwd_bus_sdio_download_firmware( );

    if ( result != WWD_SUCCESS )
    {
       /*  either an error or user abort */
       WPRINT_WWD_DEBUG(("FW download failed\n"));
       return result;
    }

    /* Wait for F2 to be ready */
    loop_count = 0;
    while ( ( ( result = wwd_bus_read_register_value( BUS_FUNCTION, SDIOD_CCCR_IORDY, (uint8_t) 1, &byte_data ) ) != WWD_SUCCESS ) ||
            ( ( ( byte_data & SDIO_FUNC_READY_2 ) == 0 ) &&
            ( loop_count < (uint32_t) F2_READY_TIMEOUT_MS ) ) )
    {
        (void) host_rtos_delay_milliseconds( (uint32_t) 1 ); /* Ignore return - nothing can be done if it fails */
        loop_count++;
    }

    if ( loop_count >= (uint32_t) F2_READY_TIMEOUT_MS )
    {
        WPRINT_WWD_DEBUG(("Timeout while waiting for function 2 to be ready\n"));

        if ( WICED_TRUE == wake_from_firmware )
        {
            /* If your system fails here, it could be due to incorrect NVRAM variables.
                      * Check which 'wifi_nvram_image.h' file your platform is using, and
                      * check that it matches the WLAN device on your platform, including the
                      * crystal frequency.
                      */
            WPRINT_WWD_ERROR(("F2 failed on wake fr FW\n"));
            /*@-unreachable@*/ /* Reachable after hitting assert */
            return WWD_TIMEOUT;
            /*@+unreachable@*/
        }
        /* Else: Ignore this failure if we're doing a reinit due to host wake: Linux DHD also ignores */

    }

    /* Do chip specific init */
    VERIFY_RESULT(wwd_chip_specific_init());

    /* Ensure Bus is up */
    VERIFY_RESULT(wwd_ensure_wlan_bus_is_up());

    /* Allow bus to go to  sleep */
    VERIFY_RESULT(wwd_allow_wlan_bus_to_sleep());

    WPRINT_WWD_INFO(("wwd_bus_reinit Completed \n"));
    return WWD_SUCCESS;
}
