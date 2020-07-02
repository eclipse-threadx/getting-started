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
#include "cipher_suites.h"
#include "wwd_constants.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "mbedtls/pk.h"
#include "mbedtls/aes.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/ssl.h"
#include "mbedtls/ssl_internal.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
/******************************************************
 *                      Macros
 ******************************************************/

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
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/
/* WICED_MBEDTLS Start */
#define WICED_TLS_CONTEXT_ID                     (0xd309c08b)
#define MAX_EXT_DATA_LENGTH                      256
#define  TLS_HANDSHAKE_PACKET_TIMEOUT_MS         (20000)

typedef struct mbedtls_ssl_context  wiced_tls_workspace_t;
typedef struct mbedtls_ssl_session  wiced_tls_session_t;
typedef struct mbedtls_x509_crt wiced_tls_certificate_t;
typedef uint32_t             tls_packet_t;

typedef int (*wiced_tls_sign_certificate_verify)(  void* key ,rsa_hash_id_t hash_id, int32_t hashlen, const unsigned char *hash, unsigned char *sign, uint32_t* key_length, wiced_tls_key_type_t type );

typedef enum
{
    TLS_CERTIFICATE_IN_PEM_FORMAT,
    TLS_CERTIFICATE_IN_DER_FORMAT,
} wiced_tls_certificate_format_t;

typedef enum
{
    TLS_NO_VERIFICATION       = 0,
    TLS_VERIFICATION_OPTIONAL = 1,
    TLS_VERIFICATION_REQUIRED = 2,
} wiced_tls_certificate_verification_t;

typedef enum
{
    WICED_TLS_SIMPLE_CONTEXT,
    WICED_TLS_ADVANCED_CONTEXT,
} wiced_tls_context_type_t;

typedef enum
{
    WICED_TLS_AS_CLIENT = 0,
    WICED_TLS_AS_SERVER = 1
} wiced_tls_endpoint_type_t;

/*
 * SSL state machine
 */
typedef enum
{
    SSL_HELLO_REQUEST,
    SSL_CLIENT_HELLO,
    SSL_SERVER_HELLO,
    SSL_SERVER_CERTIFICATE,
    SSL_SERVER_KEY_EXCHANGE,
    SSL_CERTIFICATE_REQUEST,
    SSL_SERVER_HELLO_DONE,
    SSL_CLIENT_CERTIFICATE,
    SSL_CLIENT_KEY_EXCHANGE,
    SSL_CERTIFICATE_VERIFY,
    SSL_CLIENT_CHANGE_CIPHER_SPEC,
    SSL_CLIENT_FINISHED,
    SSL_SERVER_CHANGE_CIPHER_SPEC,
    SSL_SERVER_FINISHED,
    SSL_FLUSH_BUFFERS,
    SSL_HANDSHAKE_OVER
} tls_states_t;

typedef struct
{
    linked_list_node_t      this_node;

    mbedtls_pk_context      private_key;
    mbedtls_x509_crt        certificate;
} wiced_tls_credentials_info_t;

typedef struct
{
    linked_list_t                      credentials;
    wiced_tls_sign_certificate_verify  custom_sign;
} wiced_tls_identity_t;

typedef struct
{
    uint32_t                 context_id;
    wiced_tls_workspace_t    context;
    /* This session pointer is only used to resume connection for client, If application/library wants to resume connection it needs to pass pointer of previous stored session */
    wiced_tls_session_t*     session;
    wiced_tls_identity_t*    identity;

    /*
     * Optional root CA certificate chain to be used for this context.
     * If specified, this certificate chain takes precedence over certificates
     * installed via the wiced_tls_init_root_ca_certificates() API.
     */
    mbedtls_x509_crt*        root_ca_certificates;

    mbedtls_entropy_context  entropy;
    mbedtls_ctr_drbg_context ctr_drbg;

} wiced_tls_context_t;

typedef enum
{
    TLS_RESULT_LIST     (  TLS_      )  /* 5000 - 5999 */
} tls_result_t;

#pragma pack(1)

typedef struct
{
 uint8_t type;
 uint8_t major_version;
 uint8_t minor_version;
 uint16_t length;
 uint8_t message[1];
 }tls_record_t;
/* Helper structure to create TLS record */

typedef struct
{
    uint8_t  type;
    uint8_t  major_version;
    uint8_t  minor_version;
    uint16_t length;
} tls_record_header_t;

#pragma pack()

typedef enum
{
    TLS_FRAGMENT_LENGTH_512   =  1,
    TLS_FRAGMENT_LENGTH_1024  =  2,
    TLS_FRAGMENT_LENGTH_2048  =  3,
    TLS_FRAGMENT_LENGTH_4096  =  4,
} wiced_tls_max_fragment_length_t;

/* Reference: http://www.iana.org/assignments/tls-extensiontype-values/tls-extensiontype-values.xhtml */
typedef enum
{
    TLS_EXTENSION_TYPE_SERVER_NAME                              =  0,
    TLS_EXTENSION_TYPE_MAX_FRAGMENT_LENGTH                      =  1,
    TLS_EXTENSION_TYPE_APPLICATION_LAYER_PROTOCOL_NEGOTIATION   = 16,
} wiced_tls_extension_type_t;

typedef struct
{
    wiced_tls_extension_type_t           type;
    union
    {
        uint8_t*                         server_name;
        wiced_tls_max_fragment_length_t  max_fragment_length;
        /* Pointer to a NULL terminated array of character pointers */
        const char**                     alpn_protocol_list;
    } extension_data;
} wiced_tls_extension_t;

struct _ssl_extension
{
    uint16_t id;
    uint16_t used;
    uint16_t sz;
    uint8_t data[MAX_EXT_DATA_LENGTH+1];
};

typedef struct _ssl_extension ssl_extension;

/* WICED_MBEDTLS End */

#ifdef __cplusplus
} /*extern "C" */
#endif
