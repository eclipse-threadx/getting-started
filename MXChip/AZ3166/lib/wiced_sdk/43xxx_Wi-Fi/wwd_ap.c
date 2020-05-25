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
 *  Provides an APSTA functionality specific to the 43362A2
 */

#include <string.h>
#include "wwd_rtos.h"
#include "wwd_events.h"
#include "wwd_assert.h"
#include "wwd_management.h"
#include "wwd_wifi.h"
#include "network/wwd_buffer_interface.h"
#include "internal/wwd_sdpcm.h"
#include "internal/wwd_internal.h"
#include "internal/wwd_ap.h"
#include "platform_toolchain.h"
#include "internal/wwd_ap_common.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 * @cond               Constants
 ******************************************************/

#define WLC_EVENT_MSG_LINK      (0x01)
#define RATE_SETTING_11_MBPS    (11000000 / 500000)

/** @endcond */

/* HT/AMPDU specific define */
#define AMPDU_AP_DEFAULT_BA_WSIZE   2   /* AP default BA window size */
#define AMPDU_STA_DEFAULT_BA_WSIZE  8   /* STA default BA window size */
#define AMPDU_STA_DEFAULT_MPDU      4   /* STA default num MPDU per AMPDU */
#define WEP40_KEY_LENGTH                     5
#define WEP104_KEY_LENGTH                    13
#define FORMATTED_ASCII_WEP40_KEY_LENGTH     28 /* For 5  bytes key */
#define FORMATTED_ASCII_WEP104_KEY_LENGTH    60 /* For 13 bytes key */

/******************************************************
 *                   Enumerations
 ******************************************************/

typedef enum
{
    BSS_AP   = 3,
    BSS_STA  = 2,
    BSS_UP   = 1,
    BSS_DOWN = 0
} bss_arg_option_t;

typedef enum
{
    WEP_OPEN_SYSTEM_AUTHENTICATION  = 0,
    WEP_SHARED_KEY_AUTHENTICATION   = 1
} wep_authentication_type_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

static void*        wwd_handle_apsta_event ( const wwd_event_header_t* event_header, const uint8_t* event_data, /*@returned@*/ void* handler_user_data );
static wwd_result_t internal_ap_init       ( wiced_ssid_t* ssid, wiced_security_t auth_type, const uint8_t* security_key, uint8_t key_length, uint8_t channel );
wwd_result_t        wwd_wifi_ap_init       ( wiced_ssid_t* ssid, wiced_security_t auth_type, const uint8_t* security_key, uint8_t key_length, uint8_t channel );
wwd_result_t        wwd_wifi_ap_up         ( void );

/******************************************************
 *               Variables Definitions
 ******************************************************/

static host_semaphore_type_t wwd_wifi_sleep_flag;
static const wwd_event_num_t apsta_events[] = { WLC_E_IF, WLC_E_LINK, WLC_E_NONE };

/******************************************************
 *               Function Definitions
 ******************************************************/

static void* wwd_handle_apsta_event( const wwd_event_header_t* event_header, const uint8_t* event_data, /*@returned@*/ void* handler_user_data )
{
    UNUSED_PARAMETER( event_header );
    UNUSED_PARAMETER( event_data );
    UNUSED_PARAMETER( handler_user_data );

    if ( (wwd_interface_t) event_header->interface != WWD_AP_INTERFACE)
    {
        return handler_user_data;
    }

    if ( ( ( event_header->event_type == (wwd_event_num_t) WLC_E_LINK ) &&
           ( event_header->interface == WWD_AP_INTERFACE ) ) ||
           ( event_header->event_type == WLC_E_IF ) )
    {
        wwd_result_t result;
        result = host_rtos_set_semaphore( &wwd_wifi_sleep_flag, WICED_FALSE );
        wiced_assert( "failed to post AP link semaphore", result == WWD_SUCCESS );
        REFERENCE_DEBUG_ONLY_VARIABLE( result );
    }
    return handler_user_data;
}

static wwd_result_t internal_ap_init( wiced_ssid_t* ssid, wiced_security_t auth_type, const uint8_t* security_key, uint8_t key_length, uint8_t channel )
{
    wiced_bool_t   wait_for_interface = WICED_FALSE;
    wwd_result_t   result;
    wiced_buffer_t response;
    wiced_buffer_t buffer;
    uint32_t*      data;
    uint32_t       bss_index = WWD_AP_INTERFACE;
    wiced_mac_t    mac;

#ifdef WICED_WIFI_SOFT_AP_WEP_SUPPORT_ENABLED
    uint32_t* auth;
    uint16_t length;
#endif

    if ( ( ( auth_type == WICED_SECURITY_WPA_TKIP_PSK ) || ( auth_type == WICED_SECURITY_WPA2_AES_PSK ) || ( auth_type == WICED_SECURITY_WPA2_MIXED_PSK ) ) &&
         ( ( key_length < (uint8_t) 8 ) || ( key_length > (uint8_t) 64 ) ) )
    {
        WPRINT_APP_INFO(( "Error: WPA security key length must be between 8 and 64\n" ));
        return WWD_WPA_KEYLEN_BAD;
    }

#ifdef WICED_WIFI_SOFT_AP_WEP_SUPPORT_ENABLED
    else if( (( auth_type == WICED_SECURITY_WEP_PSK ) || ( auth_type == WICED_SECURITY_WEP_SHARED )) &&
             (( key_length != FORMATTED_ASCII_WEP40_KEY_LENGTH ) && ( key_length != FORMATTED_ASCII_WEP104_KEY_LENGTH )) )
    {
        WPRINT_APP_INFO(( "Error: WEP security Key length must be either 5 / 13 bytes\n" ));
        return WWD_WEP_KEYLEN_BAD;
    }
#endif

    if ( ( wwd_wifi_p2p_is_go_up( ) == WICED_TRUE ) || ( wwd_wifi_get_ap_is_up( ) == WICED_TRUE ) )
    {
        WPRINT_APP_INFO(( "Error: Soft AP or Wi-Fi Direct group owner already up\n" ));
        return WWD_AP_ALREADY_UP;
    }

    /* Query bss state (does it exist? if so is it UP?) */
    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 4, IOVAR_STR_BSS );
    CHECK_IOCTL_BUFFER( data );
    *data = (uint32_t) bss_index;
    if ( wwd_sdpcm_send_iovar( SDPCM_GET, buffer, &response, WWD_STA_INTERFACE ) != WWD_SUCCESS )
    {
        /* Note: We don't need to release the response packet since the iovar failed */
        wait_for_interface = WICED_TRUE;
    }
    else
    {
        /* Check if the BSS is already UP, if so return */
        uint32_t* data2 = (uint32_t*) host_buffer_get_current_piece_data_pointer( response );
        if ( *data2 == (uint32_t) BSS_UP )
        {
            host_buffer_release( response, WWD_NETWORK_RX );
            wwd_wifi_set_ap_is_up( WICED_TRUE );
            return WWD_SUCCESS;
        }
        else
        {
            host_buffer_release( response, WWD_NETWORK_RX );
        }
    }

    CHECK_RETURN( host_rtos_init_semaphore( &wwd_wifi_sleep_flag ) );

    /* Register for interested events */
    CHECK_RETURN_WITH_SEMAPHORE( wwd_management_set_event_handler( apsta_events, wwd_handle_apsta_event, NULL, WWD_AP_INTERFACE ), &wwd_wifi_sleep_flag );

    /* Check if we need to wait for interface to be created */
    if ( wait_for_interface == WICED_TRUE )
    {
        CHECK_RETURN_WITH_SEMAPHORE( host_rtos_get_semaphore( &wwd_wifi_sleep_flag, (uint32_t) 10000, WICED_FALSE ), &wwd_wifi_sleep_flag );
    }

    if ( wwd_wifi_set_block_ack_window_size( WWD_AP_INTERFACE ) != WWD_SUCCESS )
    {
        return WWD_SET_BLOCK_ACK_WINDOW_FAIL;
    }

    /* Change the AP MAC address to be different from STA MAC */
    if( (result = wwd_wifi_get_mac_address(&mac, WWD_STA_INTERFACE)) != WWD_SUCCESS)
    {
        WPRINT_APP_INFO (( " Get STA MAC address failed result=%d\n", result ));
        return result;
    }
    mac.octet[0] |= (1<<1); /* Set locally administered bit 1 of first byte */

    if( (result = wwd_wifi_set_mac_address(mac, WWD_AP_INTERFACE)) != WWD_SUCCESS)
    {
        WPRINT_APP_INFO (( " Set AP MAC address failed result=%d\n", result ));
        return result;
    }

    /* Set the SSID */
    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 40, "bsscfg:" IOVAR_STR_SSID );
    CHECK_IOCTL_BUFFER_WITH_SEMAPHORE( data, &wwd_wifi_sleep_flag );
    data[0] = bss_index; /* Set the bsscfg index */
    data[1] = ssid->length; /* Set the ssid length */
    memcpy( &data[2], (uint8_t*) ssid->value, ssid->length );
    CHECK_RETURN_WITH_SEMAPHORE( wwd_sdpcm_send_iovar( SDPCM_SET, buffer, 0, WWD_STA_INTERFACE ), &wwd_wifi_sleep_flag );

    if ( (result = wwd_wifi_set_chanspec( WWD_AP_INTERFACE, channel, &wwd_wifi_sleep_flag )) != WWD_SUCCESS )
    {
          WPRINT_APP_INFO (( " Set chanspec IOVAR failed result=%d\n", result ));
          return result;
    }

#ifdef WICED_WIFI_SOFT_AP_WEP_SUPPORT_ENABLED
    if ( ( auth_type == WICED_SECURITY_WEP_PSK ) || ( auth_type == WICED_SECURITY_WEP_SHARED ) )
    {
        for ( length = 0; length < key_length; length = (uint16_t) ( length + 2 + security_key[ 1 ] ) )
        {
            const wiced_wep_key_t* in_key = (const wiced_wep_key_t*) &security_key[ length ];
            wl_wsec_key_t* out_key = (wl_wsec_key_t*) wwd_sdpcm_get_ioctl_buffer( &buffer, sizeof(wl_wsec_key_t) );
            CHECK_IOCTL_BUFFER_WITH_SEMAPHORE( out_key, &wwd_wifi_sleep_flag );
            memset( out_key, 0, sizeof(wl_wsec_key_t) );
            out_key->index = in_key->index;
            out_key->len = in_key->length;
            memcpy( out_key->data, in_key->data, in_key->length );
            switch ( in_key->length )
            {
                case WEP40_KEY_LENGTH:
                    out_key->algo = (uint32_t) CRYPTO_ALGO_WEP1;
                    break;
                case WEP104_KEY_LENGTH:
                    out_key->algo = (uint32_t) CRYPTO_ALGO_WEP128;
                    break;
                default:
                    host_buffer_release( buffer, WWD_NETWORK_TX );
                    return WWD_INVALID_KEY;
            }
            /* Set the first entry as primary key by default */
            if ( length == 0 )
            {
                out_key->flags |= WL_PRIMARY_KEY;
            }
            out_key->index = htod32(out_key->index);
            out_key->len = htod32(out_key->len);
            out_key->algo = htod32(out_key->algo);
            out_key->flags = htod32(out_key->flags);
            CHECK_RETURN_WITH_SEMAPHORE( wwd_sdpcm_send_ioctl( SDPCM_SET, WLC_SET_KEY, buffer, NULL, WWD_AP_INTERFACE ), &wwd_wifi_sleep_flag );
        }

        /* Set authentication type */
        auth = (uint32_t*) wwd_sdpcm_get_ioctl_buffer( &buffer, (uint16_t) 4 );
        CHECK_IOCTL_BUFFER_WITH_SEMAPHORE( auth, &wwd_wifi_sleep_flag );
        if ( auth_type == WICED_SECURITY_WEP_SHARED )
        {
            *auth = WEP_SHARED_KEY_AUTHENTICATION; /* 1 = Shared Key authentication */
        }
        else
        {
            *auth = WEP_OPEN_SYSTEM_AUTHENTICATION; /*  0 = Open System authentication */
        }
        CHECK_RETURN_WITH_SEMAPHORE( wwd_sdpcm_send_ioctl( SDPCM_SET, WLC_SET_AUTH, buffer, 0, WWD_AP_INTERFACE ), &wwd_wifi_sleep_flag );
    }
#endif

    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 8, "bsscfg:" IOVAR_STR_WSEC );
    CHECK_IOCTL_BUFFER_WITH_SEMAPHORE( data, &wwd_wifi_sleep_flag );
    data[0] = bss_index;
    if ((auth_type & WPS_ENABLED) != 0)
    {
        data[1] = (uint32_t) ( ( auth_type & ( ~WPS_ENABLED ) ) | SES_OW_ENABLED );
    }
    else
    {
        data[1] = (uint32_t) auth_type;
    }
    CHECK_RETURN_WITH_SEMAPHORE( wwd_sdpcm_send_iovar( SDPCM_SET, buffer, 0, WWD_STA_INTERFACE ), &wwd_wifi_sleep_flag );

    if ( ( auth_type != WICED_SECURITY_OPEN ) && ( auth_type != WICED_SECURITY_WEP_PSK ) && ( auth_type != WICED_SECURITY_WEP_SHARED ) )
    {
        wsec_pmk_t* psk;

        /* Set the wpa auth */
        data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 8, "bsscfg:" IOVAR_STR_WPA_AUTH );
        CHECK_IOCTL_BUFFER_WITH_SEMAPHORE( data, &wwd_wifi_sleep_flag );
        data[0] = bss_index;
        data[1] = (uint32_t) (auth_type == WICED_SECURITY_WPA_TKIP_PSK) ? ( WPA_AUTH_PSK ) : ( WPA2_AUTH_PSK | WPA_AUTH_PSK );
        CHECK_RETURN_WITH_SEMAPHORE( wwd_sdpcm_send_iovar( SDPCM_SET, buffer, 0, WWD_STA_INTERFACE ), &wwd_wifi_sleep_flag );

        /* Set the passphrase */
        psk = (wsec_pmk_t*) wwd_sdpcm_get_ioctl_buffer( &buffer, sizeof(wsec_pmk_t) );
        CHECK_IOCTL_BUFFER_WITH_SEMAPHORE( psk, &wwd_wifi_sleep_flag );
        memcpy( psk->key, security_key, key_length );
        psk->key_len = key_length;
        psk->flags   = (uint16_t) WSEC_PASSPHRASE;
        host_rtos_delay_milliseconds( 1 ); /* Delay required to allow radio firmware to be ready to receive PMK and avoid intermittent failure */
        CHECK_RETURN_WITH_SEMAPHORE( wwd_sdpcm_send_ioctl( SDPCM_SET, WLC_SET_WSEC_PMK, buffer, 0, WWD_AP_INTERFACE ), &wwd_wifi_sleep_flag );
    }

    /* Set the GMode */
    data = (uint32_t*) wwd_sdpcm_get_ioctl_buffer( &buffer, (uint16_t) 4 );
    CHECK_IOCTL_BUFFER_WITH_SEMAPHORE( data, &wwd_wifi_sleep_flag );
    *data = (uint32_t) GMODE_AUTO;

    result =  wwd_sdpcm_send_ioctl( SDPCM_SET, WLC_SET_GMODE, buffer, 0, WWD_AP_INTERFACE );
    if ( ( result != WWD_SUCCESS ) && ( result != WWD_WLAN_ASSOCIATED ) )
    {
        wiced_assert("start_ap: Failed to set GMode\n", 0 == 1 );

        (void) host_rtos_deinit_semaphore( &wwd_wifi_sleep_flag );
        return result;
    }

    /* Set the multicast transmission rate to 11 Mbps rather than the default 1 Mbps */
    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 4, IOVAR_STR_2G_MULTICAST_RATE );
    CHECK_IOCTL_BUFFER( data );
    *data = (uint32_t) RATE_SETTING_11_MBPS;
    CHECK_RETURN_WITH_SEMAPHORE( wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, WWD_AP_INTERFACE ), &wwd_wifi_sleep_flag );

    /* Set DTIM period */
    data = (uint32_t*) wwd_sdpcm_get_ioctl_buffer( &buffer, (uint16_t) 4 );
    CHECK_IOCTL_BUFFER_WITH_SEMAPHORE( data, &wwd_wifi_sleep_flag );
    *data = (uint32_t) WICED_DEFAULT_SOFT_AP_DTIM_PERIOD;
    CHECK_RETURN_WITH_SEMAPHORE( wwd_sdpcm_send_ioctl( SDPCM_SET, WLC_SET_DTIMPRD, buffer, 0, WWD_AP_INTERFACE ), &wwd_wifi_sleep_flag );

#ifdef WICED_DISABLE_SSID_BROADCAST
    /* Make the AP "hidden" */
    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 4, IOVAR_STR_CLOSEDNET );
    CHECK_IOCTL_BUFFER_WITH_SEMAPHORE( data, &wwd_wifi_sleep_flag );
    data[0] = (uint32_t) 1;
    CHECK_RETURN_WITH_SEMAPHORE( wwd_sdpcm_send_iovar( SDPCM_SET, buffer, 0, WWD_AP_INTERFACE ), &wwd_wifi_sleep_flag );
#endif

#ifdef WICED_WIFI_ISOLATE_AP_CLIENTS
    result = wwd_wifi_enable_ap_isolate( WWD_AP_INTERFACE, WICED_TRUE );
    wiced_assert("start_ap: Failed to disable intra BSS routing\r\n", result == WWD_SUCCESS );
#endif /* WICED_WIFI_ISOLATE_AP_CLIENTS */

#ifdef WWD_WIFI_CONFIG_AP_MAX_ASSOC
    /* Configuring the max number of associated STA in SoftAP mode. */
    result = wwd_wifi_set_iovar_value( IOVAR_STR_MAX_ASSOC, WWD_WIFI_CONFIG_AP_MAX_ASSOC, WWD_AP_INTERFACE );
    if ( result == WWD_WLAN_RANGE )
    {
        WPRINT_WWD_INFO(("configure maxassoc : Not In Range!( value : %d )\n", WWD_WIFI_CONFIG_AP_MAX_ASSOC));
        return result;
    } else if ( result != WWD_SUCCESS )
    {
        WPRINT_WWD_INFO(("configure maxassoc : ioctl failed!( err code : %d )\n", result));
        wiced_assert("start_ap: Failed to configure maxassoc\r\n", result == WWD_SUCCESS );
        return result;
    }
#endif /* WWD_WIFI_CONFIG_AP_MAX_ASSOC */

    return WWD_SUCCESS;
}

wwd_result_t wwd_wifi_ap_init( wiced_ssid_t* ssid, wiced_security_t auth_type, const uint8_t* security_key, uint8_t key_length, uint8_t channel )
{
    wwd_result_t result;

    /* Keep WLAN awake while setting up softAP */
    WWD_WLAN_KEEP_AWAKE( );

    result = internal_ap_init( ssid, auth_type, security_key, key_length, channel );

    WWD_WLAN_LET_SLEEP( );

    return result;
}

wwd_result_t wwd_wifi_ap_up( void )
{
    wiced_buffer_t buffer;
    uint32_t*      data;

    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 8, IOVAR_STR_BSS );
    CHECK_IOCTL_BUFFER_WITH_SEMAPHORE( data, &wwd_wifi_sleep_flag );
    data[0] = wwd_get_bss_index( WWD_AP_INTERFACE );
    data[1] = (uint32_t) BSS_UP;
    CHECK_RETURN_WITH_SEMAPHORE( wwd_sdpcm_send_iovar( SDPCM_SET, buffer, 0, WWD_STA_INTERFACE ), &wwd_wifi_sleep_flag );

    /* Wait until AP is brought up */
    CHECK_RETURN_WITH_SEMAPHORE( host_rtos_get_semaphore( &wwd_wifi_sleep_flag, (uint32_t) 10000, WICED_FALSE ), &wwd_wifi_sleep_flag );

    wwd_wifi_set_ap_is_up( WICED_TRUE );
    return WWD_SUCCESS;
}

/** Starts an infrastructure WiFi network
 * @param ssid      : A null terminated string containing the SSID name of the network to join
 * @param auth_type  : Authentication type:
 *                    - WICED_SECURITY_OPEN - Open Security
 *                    - WICED_SECURITY_WPA_TKIP_PSK   - WPA Security
 *                    - WICED_SECURITY_WPA2_AES_PSK   - WPA2 Security using AES cipher
 *                    - WICED_SECURITY_WPA2_MIXED_PSK - WPA2 Security using AES and/or TKIP ciphers
 *                    - WEP security is currently unimplemented due to lack of security
 * @param security_key : A byte array containing the cleartext security key for the network
 * @param key_length   : The length of the security_key in bytes.
 * @param channel     : 802.11 Channel number
 *
 * @return    WWD_SUCCESS : if successfully creates an AP
 *            Error code  : if an error occurred
 */
wwd_result_t wwd_wifi_start_ap( wiced_ssid_t* ssid, wiced_security_t auth_type, const uint8_t* security_key, uint8_t key_length, uint8_t channel )
{
    CHECK_RETURN( wwd_wifi_ap_init( ssid, auth_type, security_key, key_length, channel ) );

    CHECK_RETURN( wwd_wifi_ap_up() );

    return WWD_SUCCESS;
}

wwd_result_t wwd_wifi_stop_ap( void )
{
    uint32_t* data;
    wiced_buffer_t buffer;
    wiced_buffer_t response;
    wwd_result_t result;
    wwd_result_t result2;

    /* Query bss state (does it exist? if so is it UP?) */
    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 4, IOVAR_STR_BSS );
    CHECK_IOCTL_BUFFER( data );
    *data = wwd_get_bss_index( WWD_AP_INTERFACE );
    result = wwd_sdpcm_send_iovar( SDPCM_GET, buffer, &response, WWD_STA_INTERFACE );
    if ( result == WWD_WLAN_NOTFOUND )
    {
        /* AP interface does not exist - i.e. it is down */
        wwd_wifi_set_ap_is_up( WICED_FALSE );
        return WWD_SUCCESS;
    }

    CHECK_RETURN( result );

    data = (uint32_t*) host_buffer_get_current_piece_data_pointer( response );
    if ( data[0] != (uint32_t) BSS_UP )
    {
        /* AP interface indicates it is not up - i.e. it is down */
        host_buffer_release( response, WWD_NETWORK_RX );
        wwd_wifi_set_ap_is_up( WICED_FALSE );
        return WWD_SUCCESS;
    }

    host_buffer_release( response, WWD_NETWORK_RX );

    /* set BSS down */
    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 8, IOVAR_STR_BSS );
    CHECK_IOCTL_BUFFER( data );
    data[0] = wwd_get_bss_index( WWD_AP_INTERFACE );
    data[1] = (uint32_t) BSS_DOWN;
    CHECK_RETURN( wwd_sdpcm_send_iovar( SDPCM_SET, buffer, 0, WWD_STA_INTERFACE ) );

    /* Wait until AP is brought down */
    result = host_rtos_get_semaphore( &wwd_wifi_sleep_flag, (uint32_t) 10000, WICED_FALSE );
    result2 = host_rtos_deinit_semaphore( &wwd_wifi_sleep_flag );
    if ( result != WWD_SUCCESS )
    {
        return result;
    }
    if ( result2 != WWD_SUCCESS )
    {
        return result2;
    }

    CHECK_RETURN( wwd_management_set_event_handler( apsta_events, NULL, NULL, WWD_AP_INTERFACE ) );

    wwd_wifi_set_ap_is_up( WICED_FALSE );
    return WWD_SUCCESS;

}

/** Sets the chip specific AMPDU parameters for AP and STA
 *  For SDK 3.0, and beyond, each chip will need it's own function for setting AMPDU parameters.
 */

wwd_result_t wwd_wifi_set_ampdu_parameters( void )
{
    return wwd_wifi_set_ampdu_parameters_common( WWD_STA_INTERFACE, AMPDU_STA_DEFAULT_BA_WSIZE, AMPDU_STA_DEFAULT_MPDU, AMPDU_RX_FACTOR_8K );
}

/** Sets the chip specific AMPDU parameters for AP and STA
 *  For SDK 3.0, and beyond, each chip will need it's own function for setting AMPDU parameters.
 */
wwd_result_t wwd_wifi_set_block_ack_window_size( wwd_interface_t interface )
{
    return wwd_wifi_set_block_ack_window_size_common( interface, AMPDU_AP_DEFAULT_BA_WSIZE, AMPDU_STA_DEFAULT_BA_WSIZE );
}
