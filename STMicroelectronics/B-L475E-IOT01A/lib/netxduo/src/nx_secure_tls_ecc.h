/**************************************************************************/
/*                                                                        */
/*            Copyright (c) 1996-2019 by Express Logic Inc.               */
/*                                                                        */
/*  This software is copyrighted by and is the sole property of Express   */
/*  Logic, Inc.  All rights, title, ownership, or other interests         */
/*  in the software remain the property of Express Logic, Inc.  This      */
/*  software may only be used in accordance with the corresponding        */
/*  license agreement.  Any unauthorized use, duplication, transmission,  */
/*  distribution, or disclosure of this software is expressly forbidden.  */
/*                                                                        */
/*  This Copyright notice may not be removed or modified without prior    */
/*  written consent of Express Logic, Inc.                                */
/*                                                                        */
/*  Express Logic, Inc. reserves the right to modify this software        */
/*  without notice.                                                       */
/*                                                                        */
/*  Express Logic, Inc.                     info@expresslogic.com         */
/*  11423 West Bernardo Court               http://www.expresslogic.com   */
/*  San Diego, CA  92127                                                  */
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** NetX Secure Component                                                 */
/**                                                                       */
/**    Transport Layer Security (TLS)                                     */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    nx_secure_tls_ecc.h                                 PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines all service prototypes and data structure         */
/*    definitions for TLS Elliptic Curve Cryptography (ECC) Cipher Suites.*/
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-15-2019     Timothy Stapko           Initial Version 5.12          */
/*                                                                        */
/**************************************************************************/

#ifndef SRC_NX_SECURE_TLS_ECC_H_
#define SRC_NX_SECURE_TLS_ECC_H_

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include "nx_api.h"


#ifndef NX_SECURE_TLS_PREMASTER_SIZE
#define NX_SECURE_TLS_PREMASTER_SIZE      (68)  /* The pre-master secret should be at least 66 bytes for ECDH/ECDHE with secp521r1. */
#endif /* NX_SECURE_TLS_PREMASTER_SIZE */

#define NX_SECURE_TLS_RSA_PREMASTER_SIZE  (48)  /* The size of RSA encrypted pre-master secret. */

/* Perform error checking. The following macros must not be defined. */
#ifdef NX_SECURE_PROCESS_CLIENTHELLO_EXTENSION
#error "NX_SECURE_PROCESS_CLIENTHELLO_EXTENSION must not be defined"
#endif /* NX_SECURE_PROCESS_CLIENTHELLO_EXTENSION */

#ifdef NX_SECURE_SEND_CLIENTHELLO_EXTENSIONS_EXTENSION
#error "NX_SECURE_SEND_CLIENTHELLO_EXTENSIONS_EXTENSION must not be defined"
#endif /* NX_SECURE_SEND_CLIENTHELLO_EXTENSIONS_EXTENSION */

#ifdef NX_SECURE_PROCESS_CLIENT_KEY_EXCHANGE_EXTENSION
#error "NX_SECURE_PROCESS_CLIENT_KEY_EXCHANGE_EXTENSION must not be defined"
#endif /* NX_SECURE_PROCESS_CLIENT_KEY_EXCHANGE_EXTENSION */

#ifdef NX_SECURE_PROCESS_SERVER_KEY_EXCHANGE_EXTENSION
#error "NX_SECURE_PROCESS_SERVER_KEY_EXCHANGE_EXTENSION must not be defined"
#endif /* NX_SECURE_PROCESS_SERVER_KEY_EXCHANGE_EXTENSION */

#ifdef NX_SECURE_SEND_CLIENT_KEY_EXCHANGE_EXTENSION
#error "NX_SECURE_SEND_CLIENT_KEY_EXCHANGE_EXTENSION must not be defined"
#endif /* NX_SECURE_SEND_CLIENT_KEY_EXCHANGE_EXTENSION */

#ifdef NX_SECURE_SEND_SERVER_KEY_EXCHANGE_EXTENSION
#error "NX_SECURE_SEND_SERVER_KEY_EXCHANGE_EXTENSION must not be defined"
#endif /* NX_SECURE_SEND_SERVER_KEY_EXCHANGE_EXTENSION */

#ifdef NX_SECURE_PROCESS_CERTIFICATE_REQUEST_EXTENSION
#error "NX_SECURE_PROCESS_CERTIFICATE_REQUEST_EXTENSION must not be defined"
#endif /* NX_SECURE_PROCESS_CERTIFICATE_REQUEST_EXTENSION */

#ifdef NX_SECURE_SEND_CERTIFICATE_REQUEST_EXTENSION
#error "NX_SECURE_SEND_CERTIFICATE_REQUEST_EXTENSION must not be defined"
#endif /* NX_SECURE_SEND_CERTIFICATE_REQUEST_EXTENSION */

#ifdef NX_SECURE_PROCESS_CERTIFICATE_VERIFY_EXTENSION
#error "NX_SECURE_PROCESS_CERTIFICATE_VERIFY_EXTENSION must not be defined"
#endif /* NX_SECURE_PROCESS_CERTIFICATE_VERIFY_EXTENSION */

#ifdef NX_SECURE_SEND_CERTIFICATE_VERIFY_EXTENSION
#error "NX_SECURE_SEND_CERTIFICATE_VERIFY_EXTENSION must not be defined"
#endif /* NX_SECURE_SEND_CERTIFICATE_VERIFY_EXTENSION */

#ifdef NX_SECURE_GENERATE_PREMASTER_SECRET_EXTENSION
#error "NX_SECURE_GENERATE_PREMASTER_SECRET_EXTENSION must not be defined"
#endif /* NX_SECURE_GENERATE_PREMASTER_SECRET_EXTENSION */

#ifdef NX_SECURE_GENERATE_KEYS_EXTENSION
#error "NX_SECURE_GENERATE_KEYS_EXTENSION must not be defined"
#endif /* NX_SECURE_GENERATE_KEYS_EXTENSION */

#ifdef NX_SECURE_X509_PARSE_CERTIFICATE_EXTENSION
#error "NX_SECURE_X509_PARSE_CERTIFICATE_EXTENSION must not be defined"
#endif /* NX_SECURE_X509_PARSE_CERTIFICATE_EXTENSION */

#ifdef NX_SECURE_X509_CERTIFICATE_VERIFY_EXTENSION
#error "NX_SECURE_X509_CERTIFICATE_VERIFY_EXTENSION must not be defined"
#endif /* NX_SECURE_X509_CERTIFICATE_VERIFY_EXTENSION */

#ifdef NX_SECURE_X509_PARSE_CRL_EXTENSION
#error "NX_SECURE_X509_PARSE_CRL_EXTENSION must not be defined"
#endif /* NX_SECURE_X509_PARSE_CRL_EXTENSION */

#ifdef NX_SECURE_X509_CRL_VERIFY_EXTENSION
#error "NX_SECURE_X509_CRL_VERIFY_EXTENSION must not be defined"
#endif /* NX_SECURE_X509_CRL_VERIFY_EXTENSION */

#ifdef NX_SECURE_X509_CERTIFICATE_INITIALIZE_EXTENSION
#error "NX_SECURE_X509_CERTIFICATE_INITIALIZE_EXTENSION must not be defined"
#endif /* NX_SECURE_X509_CERTIFICATE_INITIALIZE_EXTENSION */

#ifdef NX_SECURE_TLS_SESSION_EXTENSION
#error "NX_SECURE_TLS_SESSION_EXTENSION must not be defined"
#endif /* NX_SECURE_TLS_SESSION_EXTENSION */

#define NX_SECURE_PROCESS_CLIENTHELLO_EXTENSION         NX_SECURE_PROCESS_CLIENTHELLO_EXTENSION_ECC
#define NX_SECURE_SEND_CLIENTHELLO_EXTENSIONS_EXTENSION NX_SECURE_SEND_CLIENTHELLO_EXTENSIONS_EXTENSION_ECC
#define NX_SECURE_PROCESS_CLIENT_KEY_EXCHANGE_EXTENSION NX_SECURE_PROCESS_CLIENT_KEY_EXCHANGE_EXTENSION_ECC
#define NX_SECURE_PROCESS_SERVER_KEY_EXCHANGE_EXTENSION NX_SECURE_PROCESS_SERVER_KEY_EXCHANGE_EXTENSION_ECC
#define NX_SECURE_SEND_CLIENT_KEY_EXCHANGE_EXTENSION    NX_SECURE_SEND_CLIENT_KEY_EXCHANGE_EXTENSION_ECC
#define NX_SECURE_SEND_SERVER_KEY_EXCHANGE_EXTENSION    NX_SECURE_SEND_SERVER_KEY_EXCHANGE_EXTENSION_ECC
#define NX_SECURE_PROCESS_CERTIFICATE_REQUEST_EXTENSION NX_SECURE_PROCESS_CERTIFICATE_REQUEST_EXTENSION_ECC
#define NX_SECURE_SEND_CERTIFICATE_REQUEST_EXTENSION    NX_SECURE_SEND_CERTIFICATE_REQUEST_EXTENSION_ECC
#define NX_SECURE_PROCESS_CERTIFICATE_VERIFY_EXTENSION  NX_SECURE_PROCESS_CERTIFICATE_VERIFY_EXTENSION_ECC
#define NX_SECURE_SEND_CERTIFICATE_VERIFY_EXTENSION     NX_SECURE_SEND_CERTIFICATE_VERIFY_EXTENSION_ECC
#define NX_SECURE_GENERATE_PREMASTER_SECRET_EXTENSION   NX_SECURE_GENERATE_PREMASTER_SECRET_EXTENSION_ECC
#define NX_SECURE_GENERATE_KEYS_EXTENSION               NX_SECURE_GENERATE_KEYS_EXTENSION_ECC
#define NX_SECURE_X509_PARSE_CERTIFICATE_EXTENSION      NX_SECURE_X509_PARSE_CERTIFICATE_EXTENSION_ECC
#define NX_SECURE_X509_CERTIFICATE_VERIFY_EXTENSION     NX_SECURE_X509_CERTIFICATE_VERIFY_EXTENSION_ECC
#define NX_SECURE_X509_PARSE_CRL_EXTENSION              NX_SECURE_X509_PARSE_CRL_EXTENSION_ECC
#define NX_SECURE_X509_CRL_VERIFY_EXTENSION             NX_SECURE_X509_CRL_VERIFY_EXTENSION_ECC
#define NX_SECURE_X509_CERTIFICATE_INITIALIZE_EXTENSION NX_SECURE_X509_CERTIFICATE_INITIALIZE_EXTENSION_ECC

#define NX_SECURE_SEND_CLIENTHELLO_EXTENSIONS_EXTENSION_ECC                                 \
    do {                                                                                    \
        if (NX_SUCCESS == _nx_secure_tls_ecc_send_clienthello_extensions(tls_session,       \
                                                                         packet_buffer,     \
                                                                         packet_offset,     \
                                                                         available_size))   \
        {                                                                                   \
            return(NX_SUCCESS);                                                             \
        }                                                                                   \
    } while(0);

#if defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES) || defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE)
#define NX_SECURE_PROCESS_SERVER_KEY_EXCHANGE_EXTENSION_ECC                                 \
    do {                                                                                    \
        status = _nx_secure_tls_ecc_process_server_key_exchange(tls_session,                \
                                                                packet_buffer,              \
                                                                message_length);            \
        if (status != NX_SUCCESS)                                                           \
        {                                                                                   \
            return(status);                                                                 \
        }                                                                                   \
    } while(0);
#else
#define NX_SECURE_PROCESS_SERVER_KEY_EXCHANGE_EXTENSION_ECC                                 \
    do {                                                                                    \
        message_length = _nx_secure_tls_ecc_process_server_key_exchange(tls_session,        \
                                                                        packet_buffer,      \
                                                                        message_length);    \
        if (message_length != NX_SUCCESS)                                                   \
        {                                                                                   \
            return(message_length);                                                         \
        }                                                                                   \
    } while(0);
#endif

#define NX_SECURE_SEND_CLIENT_KEY_EXCHANGE_EXTENSION_ECC                                    \
    do {                                                                                    \
        status = _nx_secure_tls_ecc_send_client_key_exchange(tls_session,                   \
                                                             send_packet);                  \
        if (status != NX_CONTINUE)                                                          \
        {                                                                                   \
            return(status);                                                                 \
        }                                                                                   \
    } while(0);

#define NX_SECURE_GENERATE_PREMASTER_SECRET_EXTENSION_ECC                                   \
    do {                                                                                    \
        status = _nx_secure_tls_ecc_generate_premaster_secret(tls_session);                 \
        if (status != NX_CONTINUE)                                                          \
        {                                                                                   \
            return(status);                                                                 \
        }                                                                                   \
        status = NX_SECURE_TLS_SUCCESS;                                                     \
    } while(0);

#define NX_SECURE_GENERATE_KEYS_EXTENSION_ECC                                               \
    do {                                                                                    \
        status = _nx_secure_tls_ecc_generate_keys(tls_session);                             \
        if (status != NX_CONTINUE)                                                          \
        {                                                                                   \
            return(status);                                                                 \
        }                                                                                   \
    } while(0);

#define NX_SECURE_X509_PARSE_CERTIFICATE_EXTENSION_ECC                                      \
    do {                                                                                    \
        status = _nx_secure_x509_ecc_certificate_parse(buffer, length, bytes_processed,     \
                                                       cert);                               \
        if (status != NX_CONTINUE)                                                          \
        {                                                                                   \
            return(status);                                                                 \
        }                                                                                   \
    } while(0);

#define NX_SECURE_X509_CERTIFICATE_VERIFY_EXTENSION_ECC                                     \
    do {                                                                                    \
        status = _nx_secure_x509_ecc_certificate_verify(certificate,                        \
                                                        issuer_certificate);                \
        if (status != NX_CONTINUE)                                                          \
        {                                                                                   \
            return(status);                                                                 \
        }                                                                                   \
    } while(0);

#define NX_SECURE_X509_CERTIFICATE_INITIALIZE_EXTENSION_ECC                                 \
    do {                                                                                    \
        status = _nx_secure_x509_ecc_certificate_initialize(certificate,                    \
                                                            certificate_data, length,       \
                                                            raw_data_buffer, buffer_size,   \
                                                            private_key, priv_len,          \
                                                            private_key_type);              \
        if (status != NX_CONTINUE)                                                          \
        {                                                                                   \
            return(status);                                                                 \
        }                                                                                   \
    } while(0);

#define NX_SECURE_PROCESS_CLIENTHELLO_EXTENSION_ECC                                         \
    do {                                                                                    \
        status = _nx_secure_tls_ecc_process_clienthello(tls_session,                        \
                                                        packet_buffer, message_length);     \
        if (status != NX_CONTINUE)                                                          \
        {                                                                                   \
            return(status);                                                                 \
        }                                                                                   \
    } while(0);

#define NX_SECURE_SEND_SERVER_KEY_EXCHANGE_EXTENSION_ECC                                    \
    status = _nx_secure_tls_ecc_send_server_key_exchange(tls_session,                       \
                                                         packet_pool, wait_option);         \
    if (status != NX_SUCCESS)                                                               \
    {                                                                                       \
        break;                                                                              \
    }

#define NX_SECURE_PROCESS_CLIENT_KEY_EXCHANGE_EXTENSION_ECC                                 \
    do {                                                                                    \
        status = _nx_secure_tls_ecc_process_client_key_exchange(tls_session,                \
                                                                packet_buffer,              \
                                                                message_length);            \
        if (status != NX_CONTINUE)                                                          \
        {                                                                                   \
            return(status);                                                                 \
        }                                                                                   \
    } while(0);

#define NX_SECURE_SEND_CERTIFICATE_VERIFY_EXTENSION_ECC                                     \
    do {                                                                                    \
        status = _nx_secure_tls_ecc_send_certificate_verify(tls_session,                    \
                                                             send_packet);                  \
        if (status != NX_CONTINUE)                                                          \
        {                                                                                   \
            return(status);                                                                 \
        }                                                                                   \
    } while(0);

#define NX_SECURE_PROCESS_CERTIFICATE_VERIFY_EXTENSION_ECC                                  \
    do {                                                                                    \
        status = _nx_secure_tls_ecc_process_certificate_verify(tls_session,                 \
                                                               packet_buffer,               \
                                                               message_length);             \
        if (status != NX_CONTINUE)                                                          \
        {                                                                                   \
            return(status);                                                                 \
        }                                                                                   \
    } while(0);

#define NX_SECURE_SEND_CERTIFICATE_REQUEST_EXTENSION_ECC                                    \
    do {                                                                                    \
        if (NX_SUCCESS != _nx_secure_tls_ecc_send_certificate_request(tls_session,          \
                                                                      send_packet))         \
        {                                                                                   \
            break;                                                                          \
        }                                                                                   \
        return(NX_SUCCESS);                                                                 \
    } while(0);

#define NX_SECURE_PROCESS_CERTIFICATE_REQUEST_EXTENSION_ECC                                 \
    do {                                                                                    \
        if (tls_session -> nx_secure_tls_ecc.nx_secure_tls_ecc_supported_groups_count != 0) \
        {                                                                                   \
            return(_nx_secure_tls_ecc_process_certificate_request(tls_session,              \
                                                                  packet_buffer,            \
                                                                  message_length));         \
        }                                                                                   \
    } while(0);

#define NX_SECURE_X509_PARSE_CRL_EXTENSION_ECC                                              \
    do {                                                                                    \
        status = _nx_secure_x509_ecc_certificate_revocation_list_parse(buffer, length,      \
                                                                       bytes_processed,     \
                                                                       crl);                \
        if (status != NX_CONTINUE)                                                          \
        {                                                                                   \
            return(status);                                                                 \
        }                                                                                   \
    } while(0);

#define NX_SECURE_X509_CRL_VERIFY_EXTENSION_ECC                                             \
    do {                                                                                    \
        status = _nx_secure_x509_ecc_crl_verify(certificate, crl,                           \
                                                issuer_certificate);                        \
        if (status != NX_CONTINUE)                                                          \
        {                                                                                   \
            return(status);                                                                 \
        }                                                                                   \
    } while(0);

#define NX_SECURE_TLS_SESSION_EXTENSION  NX_SECURE_TLS_ECC nx_secure_tls_ecc;

/* API return values.  */

struct NX_SECURE_TLS_SESSION_STRUCT;
struct NX_SECURE_TLS_HELLO_EXTENSION_STRUCT;
struct NX_SECURE_X509_CERT_STRUCT;
struct NX_SECURE_X509_CRL_STRUCT;

#define NX_SECURE_TLS_X509_TYPE_EC                        57
#define NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_1               58
#define NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_224             59
#define NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_256             60
#define NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_384             61
#define NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_512             62
#define NX_SECURE_TLS_X509_EC_SECT163K1                   0x00060001
#define NX_SECURE_TLS_X509_EC_SECT163R1                   0x00060002
#define NX_SECURE_TLS_X509_EC_SECT163R2                   0x00060003
#define NX_SECURE_TLS_X509_EC_SECT193R1                   0x00060004
#define NX_SECURE_TLS_X509_EC_SECT193R2                   0x00060005
#define NX_SECURE_TLS_X509_EC_SECT233K1                   0x00060006
#define NX_SECURE_TLS_X509_EC_SECT233R1                   0x00060007
#define NX_SECURE_TLS_X509_EC_SECT239K1                   0x00060008
#define NX_SECURE_TLS_X509_EC_SECT283K1                   0x00060009
#define NX_SECURE_TLS_X509_EC_SECT283R1                   0x0006000A
#define NX_SECURE_TLS_X509_EC_SECT409K1                   0x0006000B
#define NX_SECURE_TLS_X509_EC_SECT409R1                   0x0006000C
#define NX_SECURE_TLS_X509_EC_SECT571K1                   0x0006000D
#define NX_SECURE_TLS_X509_EC_SECT571R1                   0x0006000E
#define NX_SECURE_TLS_X509_EC_SECP160K1                   0x0006000F
#define NX_SECURE_TLS_X509_EC_SECP160R1                   0x00060010
#define NX_SECURE_TLS_X509_EC_SECP160R2                   0x00060011
#define NX_SECURE_TLS_X509_EC_SECP192K1                   0x00060012
#define NX_SECURE_TLS_X509_EC_SECP192R1                   0x00060013
#define NX_SECURE_TLS_X509_EC_SECP224K1                   0x00060014
#define NX_SECURE_TLS_X509_EC_SECP224R1                   0x00060015
#define NX_SECURE_TLS_X509_EC_SECP256K1                   0x00060016
#define NX_SECURE_TLS_X509_EC_SECP256R1                   0x00060017
#define NX_SECURE_TLS_X509_EC_SECP384R1                   0x00060018
#define NX_SECURE_TLS_X509_EC_SECP521R1                   0x00060019

#define NX_SECURE_X509_KEY_TYPE_EC_DER                    0x00000002 /* DER-encoded EC private key. */

/* Client Certificate Types for Certificate Request messages.
   Values taken directly from RFC 4492, section 5.5. */
#define NX_SECURE_TLS_CERT_TYPE_ECDSA_SIGN                64
#define NX_SECURE_TLS_CERT_TYPE_RSA_FIXED_ECDH            65
#define NX_SECURE_TLS_CERT_TYPE_ECDSA_FIXED_ECDH          66

/* Packed algorithm values (as seen over the wire). */
#define NX_SECURE_TLS_SIGNATURE_ECDSA_SHA256                 (((UINT)NX_SECURE_TLS_HASH_ALGORITHM_SHA256 << 8) + (UINT)NX_SECURE_TLS_SIGNATURE_ALGORITHM_ECDSA)


/* EC public key information. */
typedef struct NX_SECURE_EC_PUBLIC_KEY_STRUCT
{
    /* Public key for EC. */
    const UCHAR *nx_secure_ec_public_key;

    /* Size of the key used by the algorithm. */
    USHORT nx_secure_ec_public_key_length;

    /* Named curve used. */
    UINT nx_secure_ec_named_curve;

} NX_SECURE_EC_PUBLIC_KEY;

/* EC private key information. */
typedef struct NX_SECURE_EC_PRIVATE_KEY_STRUCT
{
    /* Private key for EC. */
    const UCHAR *nx_secure_ec_private_key;

    /* Size of the EC private key. */
    USHORT nx_secure_ec_private_key_length;

    /* Public key for EC. */
    /* This field is optional and it can be NX_NULL. */
    const UCHAR *nx_secure_ec_public_key;

    /* Size of the key used by the algorithm. */
    USHORT nx_secure_ec_public_key_length;

    /* Named curve used. */
    UINT nx_secure_ec_named_curve;

} NX_SECURE_EC_PRIVATE_KEY;

/* EC handshake information. */
typedef struct NX_SECURE_TLS_ECDHE_HANDSHAKE_DATA_STRUCT
{
    /* Named curve used. */
    UINT nx_secure_tls_ecdhe_named_curve;

    /* Signature Algorithm for ECDHE. */
    USHORT nx_secure_tls_ecdhe_signature_algorithm;

    /* Length of the private key. */
    USHORT nx_secure_tls_ecdhe_private_key_length;

    /* Private key for ECDHE. */
    UCHAR nx_secure_tls_ecdhe_private_key[NX_SECURE_TLS_PREMASTER_SIZE];

} NX_SECURE_TLS_ECDHE_HANDSHAKE_DATA;

/* ECC information. */
typedef struct NX_SECURE_TLS_ECC_STRUCT
{
    /* Supported named curves. */
    const USHORT *nx_secure_tls_ecc_supported_groups;

    /* Number of supported named curves. */
    USHORT  nx_secure_tls_ecc_supported_groups_count;

    /* Corresponding crypto methods for the supported named curve. */
    const VOID *nx_secure_tls_ecc_curves;
} NX_SECURE_TLS_ECC;

/* Declare internal functions. */
UINT _nx_secure_tls_ecc_initialize(struct NX_SECURE_TLS_SESSION_STRUCT *tls_session,
                                   const USHORT *supported_groups, USHORT supported_group_count,
                                   const VOID *curves);
UINT _nx_secure_tls_find_curve_method(struct NX_SECURE_TLS_SESSION_STRUCT *tls_session,
                                      USHORT named_curve, VOID **curve_method);
UINT _nx_secure_tls_ecc_send_clienthello_extensions(struct NX_SECURE_TLS_SESSION_STRUCT *tls_session,
                                                    UCHAR *packet_buffer, ULONG *packet_offset,
                                                    ULONG available_size);
UINT _nx_secure_tls_ecc_process_server_key_exchange(struct NX_SECURE_TLS_SESSION_STRUCT *tls_session,
                                                    UCHAR *packet_buffer, UINT message_length);
UINT _nx_secure_tls_ecc_send_client_key_exchange(struct NX_SECURE_TLS_SESSION_STRUCT *tls_session,
                                                 NX_PACKET *send_packet);
UINT _nx_secure_tls_ecc_generate_premaster_secret(struct NX_SECURE_TLS_SESSION_STRUCT *tls_session);
UINT _nx_secure_tls_ecc_generate_keys(struct NX_SECURE_TLS_SESSION_STRUCT *tls_session);

UINT _nx_secure_x509_ecc_certificate_parse(const UCHAR *buffer, UINT length, UINT *bytes_processed,
                                           struct NX_SECURE_X509_CERT_STRUCT *certificate);
VOID _nx_secure_x509_ecc_oid_parse(const UCHAR *oid, ULONG length, UINT *oid_value);
UINT _nx_secure_x509_ecc_certificate_verify(struct NX_SECURE_X509_CERT_STRUCT *certificate,
                                            struct NX_SECURE_X509_CERT_STRUCT *issuer_certificate);

UINT _nx_secure_tls_ecc_process_clienthello_extensions(struct NX_SECURE_TLS_SESSION_STRUCT *tls_session,
                                                       struct NX_SECURE_TLS_HELLO_EXTENSION_STRUCT *exts,
                                                       UINT num_extensions,
                                                       UINT *selected_curve, USHORT cert_curve,
                                                       UINT *cert_curve_supported,
                                                       USHORT *ecdhe_signature_algorithm);

UINT _nx_secure_x509_ec_private_key_parse(const UCHAR *buffer, UINT length,
                                          UINT *bytes_processed,
                                          NX_SECURE_EC_PRIVATE_KEY *ec_key);

UINT _nx_secure_x509_ecc_certificate_initialize(struct NX_SECURE_X509_CERT_STRUCT *certificate,
                                                UCHAR *certificate_data, USHORT length,
                                                UCHAR *raw_data_buffer, USHORT buffer_size,
                                                const UCHAR *private_key, USHORT priv_len,
                                                UINT private_key_type);

UINT _nx_secure_tls_ecc_process_clienthello(struct NX_SECURE_TLS_SESSION_STRUCT *tls_session,
                                            UCHAR *packet_buffer,
                                            UINT message_length);

UINT _nx_secure_tls_ecc_send_server_key_exchange(struct NX_SECURE_TLS_SESSION_STRUCT *tls_session,
                                                 NX_PACKET_POOL *packet_pool, ULONG wait_option);
UINT _nx_secure_tls_ecc_process_client_key_exchange(struct NX_SECURE_TLS_SESSION_STRUCT *tls_session,
                                                    UCHAR *packet_buffer, UINT message_length);
UINT _nx_secure_tls_ecc_send_certificate_verify(struct NX_SECURE_TLS_SESSION_STRUCT *tls_session,
                                                NX_PACKET *send_packet);
UINT _nx_secure_tls_ecc_process_certificate_verify(struct NX_SECURE_TLS_SESSION_STRUCT *tls_session,
                                                   UCHAR *packet_buffer, UINT message_length);
UINT _nx_secure_tls_ecc_send_certificate_request(struct NX_SECURE_TLS_SESSION_STRUCT *tls_session,
                                                 NX_PACKET *send_packet);
UINT _nx_secure_tls_ecc_process_certificate_request(struct NX_SECURE_TLS_SESSION_STRUCT *tls_session,
                                                    UCHAR *packet_buffer, UINT message_length);
UINT _nx_secure_x509_ecc_certificate_revocation_list_parse(const UCHAR *buffer, UINT length,
                                                           UINT *bytes_processed,
                                                           struct NX_SECURE_X509_CRL_STRUCT *crl);
UINT _nx_secure_x509_ecc_crl_verify(struct NX_SECURE_X509_CERT_STRUCT *certificate,
                                    struct NX_SECURE_X509_CRL_STRUCT *crl,
                                    struct NX_SECURE_X509_CERT_STRUCT *issuer_certificate);


/* Actual API functions .*/
#define nx_secure_tls_ecc_initialize    _nx_secure_tls_ecc_initialize

/* Functions for error checking .*/



#ifdef __cplusplus
}
#endif

#endif /* SRC_NX_SECURE_TLS_H_ */

