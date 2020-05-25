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

#include "besl_structures.h"
#include "crypto_structures.h"
#include <time.h>
#include "dtls_cipher_suites.h"
#include "wwd_constants.h"
#include "tls_types.h"

#include "linked_list.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/
typedef uint32_t dtls_packet_t;

typedef enum
{
    DTLS_NO_VERIFICATION       = 0,
    DTLS_VERIFICATION_OPTIONAL = 1,
    DTLS_VERIFICATION_REQUIRED = 2,
} wiced_dtls_certificate_verification_t;

typedef enum
{
    WICED_DTLS_AS_CLIENT = 0,
    WICED_DTLS_AS_SERVER = 1
} wiced_dtls_endpoint_type_t;

typedef enum
{
    DTLS_TCP_TRANSPORT = 0,
    DTLS_EAP_TRANSPORT = 1,
    DTLS_UDP_TRANSPORT = 2,
} dtls_transport_protocol_t;

/**
 * Defines DTLS event types
 */
typedef enum wiced_dtls_event_s
{
    DTLS_EVENT_TYPE_CONNECTED = 1,      /* Event sent when new client connects with DTLS server and handshake has been finished */
    DTLS_EVENT_TYPE_DISCONNECTED,       /* Event sent when client DISCONNECT */
    DTLS_EVENT_TYPE_APP_DATA,           /* Event sent when Application data received from client */
} dtls_server_event_type_t;

typedef enum
{
    WICED_DTLS_SECURITY_TYPE_PSK       = 0,  /* security type PSK */
    WICED_DTLS_SECURITY_TYPE_NONPSK    = 1,  /* security type other than PSK */
} wiced_dtls_security_type_t;

typedef enum
{
    DTLS_RECEIVE_EVENT,
    DTLS_RETRANSMISSION_CHECK_EVENT,
    DTLS_STOP_EVENT
} dtls_event_type_t;

typedef struct
{
    uint8_t*            certificate_data;
    uint32_t            certificate_length;
    char*               private_key;
    uint32_t            key_length;
} wiced_dtls_nonpsk_info_t;

typedef struct
{
   dtls_server_event_type_t event;
   void*                    callback_args;
   tls_packet_t*            packet;
   dtls_session_t           session;
} dtls_peer_data;

#pragma  pack(1)

/** Generic header structure of the DTLS record layer. */
typedef struct
{
        uint8_t     content_type;      /**< content type of the included message */
        uint16_t    version;           /**< Protocol version */
        uint16_t    epoch;             /**< counter for cipher state changes */
        uint8_t     sequence_number[6];   /**< sequence number */
        uint8_t     length[2];            /**< length of the following fragment */
        /* fragment */
} dtls_record_header_t;

#pragma  pack()

typedef struct
{
    union
    {
        mbedtls_pk_context  private_key;
        wiced_tls_psk_key_t psk_key;
    };
    mbedtls_x509_crt        certificate;
} wiced_dtls_identity_t;

/**
 * Holds security parameters, local state and the transport address
 * for each peer. */
typedef struct dtls_peer_t
{
        linked_list_node_t           this_node;
        mbedtls_ssl_context          context;
        dtls_session_t               session;
} dtls_peer_t;

typedef struct context_t
{
    linked_list_t             peer_list;            /* peer list */
    void*                     timer_event;
    wiced_dtls_identity_t*    identity;
} dtls_context_t;

typedef struct
{
    dtls_event_type_t    event_type;
    void                *data;
    uint32_t             rcv_ev_count;   /* Valid only for RECEIVE EVENTS */
} dtls_event_message_t;
/******************************************************
 *                 Type Definitions
 ******************************************************/

typedef struct context_t wiced_dtls_workspace_t;

/* Callback registered with DTLS will be called when there is application data available or client disconnected. for application data, callback should free the packet after use.
 * Callback is called from high priority thread so please free as soon as possible */
typedef int (*wiced_dtls_callback_t)( void* socket, void* arg );

typedef struct
{
        wiced_dtls_workspace_t     context;
        wiced_dtls_identity_t*     identity;
        wiced_dtls_callback_t      callback;
        void*                      event_thread;
        void*                      event_queue;
        void*                      callback_arg;
}wiced_dtls_context_t;

typedef enum
{
    DTLS_RESULT_LIST ( DTLS_ )
    /* 5000 - 5999 */
}    dtls_result_t;

    /******************************************************
     *                    Structures
     ******************************************************/

    /******************************************************
     *                 Global Variables
     ******************************************************/

    /******************************************************
     *               Function Declarations
     ******************************************************/
    dtls_result_t dtls_close_notify( dtls_context_t* context );
    void          dtls_free ( dtls_context_t* context, dtls_session_t* session );



#ifdef __cplusplus
} /*extern "C" */
#endif
