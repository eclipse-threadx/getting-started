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
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nx_secure_tls_api.h                                 PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the basic Application Interface (API) to the      */
/*    high-performance TLS implementation for the NetXDuo TCP/IP          */
/*    protocol.                                                           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            sorted the API by alphabet  */
/*                                            order,                      */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s), added    */
/*                                            APIs for crypto test and to */
/*                                            get alert value and level,  */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/

#ifndef SRC_NX_SECURE_TLS_API_H_
#define SRC_NX_SECURE_TLS_API_H_

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

/* Include the ThreadX and port-specific data type file.  */



#include "tx_api.h"
#include "nx_port.h"
#include "nx_api.h"
#include "nx_secure_tls.h"

#ifndef NX_SECURE_SOURCE_CODE

#ifdef NX_SECURE_DISABLE_ERROR_CHECKING
#define nx_secure_tls_active_certificate_set               _nx_secure_tls_active_certificate_set
#define nx_secure_tls_initialize                           _nx_secure_tls_initialize
#define nx_secure_tls_local_certificate_add                _nx_secure_tls_local_certificate_add
#define nx_secure_tls_local_certificate_find               _nx_secure_tls_local_certificate_find
#define nx_secure_tls_local_certificate_remove             _nx_secure_tls_local_certificate_remove
#define nx_secure_tls_metadata_size_calculate              _nx_secure_tls_metadata_size_calculate
#define nx_secure_tls_remote_certificate_allocate          _nx_secure_tls_remote_certificate_allocate
#define nx_secure_tls_remote_certificate_buffer_allocate   _nx_secure_tls_remote_certificate_buffer_allocate
#define nx_secure_tls_remote_certificate_free_all          _nx_secure_tls_remote_certificate_free_all
#define nx_secure_tls_server_certificate_add               _nx_secure_tls_server_certificate_add
#define nx_secure_tls_server_certificate_find              _nx_secure_tls_server_certificate_find
#define nx_secure_tls_server_certificate_remove            _nx_secure_tls_server_certificate_remove
#define nx_secure_tls_session_alert_value_get              _nx_secure_tls_session_alert_value_get
#define nx_secure_tls_session_certificate_callback_set     _nx_secure_tls_session_certificate_callback_set
#define nx_secure_tls_session_client_callback_set          _nx_secure_tls_session_client_callback_set
#define nx_secure_tls_session_client_verify_disable        _nx_secure_tls_session_client_verify_disable
#define nx_secure_tls_session_client_verify_enable         _nx_secure_tls_session_client_verify_enable
#define nx_secure_tls_session_x509_client_verify_configure _nx_secure_tls_session_x509_client_verify_configure
#define nx_secure_tls_session_create                       _nx_secure_tls_session_create
#define nx_secure_tls_session_delete                       _nx_secure_tls_session_delete
#define nx_secure_tls_session_end                          _nx_secure_tls_session_end
#define nx_secure_tls_session_packet_buffer_set            _nx_secure_tls_session_packet_buffer_set
#define nx_secure_tls_session_protocol_version_override    _nx_secure_tls_session_protocol_version_override
#define nx_secure_tls_session_receive                      _nx_secure_tls_session_receive
#define nx_secure_tls_session_renegotiate                  _nx_secure_tls_session_renegotiate
#define nx_secure_tls_session_renegotiate_callback_set     _nx_secure_tls_session_renegotiate_callback_set
#define nx_secure_tls_session_reset                        _nx_secure_tls_session_reset
#define nx_secure_tls_session_send                         _nx_secure_tls_session_send
#define nx_secure_tls_session_server_callback_set          _nx_secure_tls_session_server_callback_set
#define nx_secure_tls_session_sni_extension_parse          _nx_secure_tls_session_sni_extension_parse
#define nx_secure_tls_session_sni_extension_set            _nx_secure_tls_session_sni_extension_set
#define nx_secure_tls_session_start                        _nx_secure_tls_session_start
#define nx_secure_tls_session_time_function_set            _nx_secure_tls_session_time_function_set
#define nx_secure_tls_trusted_certificate_add              _nx_secure_tls_trusted_certificate_add
#define nx_secure_tls_trusted_certificate_remove           _nx_secure_tls_trusted_certificate_remove
#define nx_secure_tls_packet_allocate                      _nx_secure_tls_packet_allocate
#if defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES) || defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE)
#define nx_secure_tls_client_psk_set                       _nx_secure_tls_client_psk_set
#define nx_secure_tls_psk_add                              _nx_secure_tls_psk_add
#endif /* defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES) || defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE) */
#else /* !NX_SEURE_DISABLE_ERROR_CHECKING */
#define nx_secure_tls_active_certificate_set               _nxe_secure_tls_active_certificate_set
#define nx_secure_tls_initialize                           _nx_secure_tls_initialize
#define nx_secure_tls_local_certificate_add                _nxe_secure_tls_local_certificate_add
#define nx_secure_tls_local_certificate_find               _nxe_secure_tls_local_certificate_find
#define nx_secure_tls_local_certificate_remove             _nxe_secure_tls_local_certificate_remove
#define nx_secure_tls_metadata_size_calculate              _nxe_secure_tls_metadata_size_calculate
#define nx_secure_tls_remote_certificate_allocate          _nxe_secure_tls_remote_certificate_allocate
#define nx_secure_tls_remote_certificate_buffer_allocate   _nxe_secure_tls_remote_certificate_buffer_allocate
#define nx_secure_tls_remote_certificate_free_all          _nxe_secure_tls_remote_certificate_free_all
#define nx_secure_tls_server_certificate_add               _nxe_secure_tls_server_certificate_add
#define nx_secure_tls_server_certificate_find              _nxe_secure_tls_server_certificate_find
#define nx_secure_tls_server_certificate_remove            _nxe_secure_tls_server_certificate_remove
#define nx_secure_tls_session_alert_value_get              _nxe_secure_tls_session_alert_value_get
#define nx_secure_tls_session_certificate_callback_set     _nxe_secure_tls_session_certificate_callback_set
#define nx_secure_tls_session_client_callback_set          _nxe_secure_tls_session_client_callback_set
#define nx_secure_tls_session_client_verify_disable        _nxe_secure_tls_session_client_verify_disable
#define nx_secure_tls_session_client_verify_enable         _nxe_secure_tls_session_client_verify_enable
#define nx_secure_tls_session_x509_client_verify_configure _nxe_secure_tls_session_x509_client_verify_configure
#define nx_secure_tls_session_create                       _nxe_secure_tls_session_create
#define nx_secure_tls_session_delete                       _nxe_secure_tls_session_delete
#define nx_secure_tls_session_end                          _nxe_secure_tls_session_end
#define nx_secure_tls_session_packet_buffer_set            _nxe_secure_tls_session_packet_buffer_set
#define nx_secure_tls_session_protocol_version_override    _nxe_secure_tls_session_protocol_version_override
#define nx_secure_tls_session_receive                      _nxe_secure_tls_session_receive
#define nx_secure_tls_session_renegotiate                  _nxe_secure_tls_session_renegotiate
#define nx_secure_tls_session_renegotiate_callback_set     _nxe_secure_tls_session_renegotiate_callback_set
#define nx_secure_tls_session_reset                        _nxe_secure_tls_session_reset
#define nx_secure_tls_session_send                         _nxe_secure_tls_session_send
#define nx_secure_tls_session_server_callback_set          _nxe_secure_tls_session_server_callback_set
#define nx_secure_tls_session_sni_extension_parse          _nxe_secure_tls_session_sni_extension_parse
#define nx_secure_tls_session_sni_extension_set            _nxe_secure_tls_session_sni_extension_set
#define nx_secure_tls_session_start                        _nxe_secure_tls_session_start
#define nx_secure_tls_session_time_function_set            _nxe_secure_tls_session_time_function_set
#define nx_secure_tls_trusted_certificate_add              _nxe_secure_tls_trusted_certificate_add
#define nx_secure_tls_trusted_certificate_remove           _nxe_secure_tls_trusted_certificate_remove
#define nx_secure_tls_packet_allocate                      _nxe_secure_tls_packet_allocate
#if defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES) || defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE)
#define nx_secure_tls_client_psk_set                       _nxe_secure_tls_client_psk_set
#define nx_secure_tls_psk_add                              _nxe_secure_tls_psk_add
#endif /* defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES) || defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE) */
#endif /* NX_SECURE_DISABLE_ERROR_CHECKING */
#define nx_secure_crypto_table_self_test                   _nx_secure_crypto_table_self_test
#define nx_secure_crypto_rng_self_test                     _nx_secure_crypto_rng_self_test

UINT nx_secure_crypto_table_self_test(const NX_SECURE_TLS_CRYPTO *crypto_table,
                                      VOID *metadata, UINT metadata_size);
UINT nx_secure_crypto_rng_self_test();
UINT nx_secure_module_hash_compute(NX_CRYPTO_METHOD *hmac_ptr,
                                   UINT start_address,
                                   UINT end_address,
                                   UCHAR *key, UINT key_length,
                                   VOID *metadata, UINT metadata_size,
                                   UCHAR *output_buffer, UINT output_buffer_size, UINT *actual_size);


UINT nx_secure_tls_active_certificate_set(NX_SECURE_TLS_SESSION *tls_session,
                                          NX_SECURE_X509_CERT *certificate);
VOID nx_secure_tls_initialize(VOID);
UINT nx_secure_tls_local_certificate_add(NX_SECURE_TLS_SESSION *tls_session,
                                         NX_SECURE_X509_CERT *certificate);
UINT nx_secure_tls_local_certificate_find(NX_SECURE_TLS_SESSION *tls_session,
                                          NX_SECURE_X509_CERT **certificate, UCHAR *common_name,
                                          UINT name_length);
UINT nx_secure_tls_local_certificate_remove(NX_SECURE_TLS_SESSION *tls_session, UCHAR *common_name,
                                            UINT common_name_length);
UINT nx_secure_tls_metadata_size_calculate(const NX_SECURE_TLS_CRYPTO *cipher_table,
                                           ULONG *metadata_size);
UINT nx_secure_tls_remote_certificate_allocate(NX_SECURE_TLS_SESSION *tls_session,
                                               NX_SECURE_X509_CERT *certificate,
                                               UCHAR *raw_certificate_buffer, UINT buffer_size);
UINT nx_secure_tls_remote_certificate_buffer_allocate(NX_SECURE_TLS_SESSION *tls_session,
                                                    UINT certs_number, VOID *certificate_buffer, ULONG buffer_size);
UINT nx_secure_tls_remote_certificate_free_all(NX_SECURE_TLS_SESSION *tls_session);
UINT nx_secure_tls_server_certificate_add(NX_SECURE_TLS_SESSION *tls_session,
                                          NX_SECURE_X509_CERT *certificate, UINT cert_id);
UINT nx_secure_tls_server_certificate_find(NX_SECURE_TLS_SESSION *tls_session,
                                           NX_SECURE_X509_CERT **certificate, UINT cert_id);
UINT nx_secure_tls_server_certificate_remove(NX_SECURE_TLS_SESSION *tls_session, UINT cert_id);
UINT  nx_secure_tls_session_alert_value_get(NX_SECURE_TLS_SESSION *tls_session,
                                            UINT *alert_level, UINT *alert_value);
UINT nx_secure_tls_session_certificate_callback_set(NX_SECURE_TLS_SESSION *tls_session,
                                                    ULONG (*func_ptr)(NX_SECURE_TLS_SESSION *session,
                                                                      NX_SECURE_X509_CERT *certificate));
UINT nx_secure_tls_session_client_callback_set(NX_SECURE_TLS_SESSION *tls_session,
                                               ULONG (*func_ptr)(NX_SECURE_TLS_SESSION *tls_session,
                                                                 NX_SECURE_TLS_HELLO_EXTENSION *extensions,
                                                                 UINT num_extensions));
UINT nx_secure_tls_session_client_verify_disable(NX_SECURE_TLS_SESSION *tls_session);
UINT nx_secure_tls_session_client_verify_enable(NX_SECURE_TLS_SESSION *tls_session);
UINT nx_secure_tls_session_x509_client_verify_configure(NX_SECURE_TLS_SESSION *tls_session, UINT certs_number,
                                                          VOID *certificate_buffer, ULONG buffer_size);

UINT nx_secure_tls_session_create(NX_SECURE_TLS_SESSION *session_ptr,
                                  const NX_SECURE_TLS_CRYPTO *cipher_table,
                                  VOID *metadata_area,
                                  ULONG metadata_size);
UINT nx_secure_tls_session_delete(NX_SECURE_TLS_SESSION *tls_session);
UINT nx_secure_tls_session_end(NX_SECURE_TLS_SESSION *tls_session, UINT wait_option);
UINT nx_secure_tls_session_packet_buffer_set(NX_SECURE_TLS_SESSION *session_ptr,
                                             UCHAR *buffer_ptr, ULONG buffer_size);
UINT nx_secure_tls_session_protocol_version_override(NX_SECURE_TLS_SESSION *tls_session,
                                                     USHORT protocol_version);
UINT nx_secure_tls_session_receive(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET **packet_ptr_ptr,
                                   ULONG wait_option);
UINT nx_secure_tls_session_renegotiate(NX_SECURE_TLS_SESSION *tls_session, UINT wait_option);
UINT nx_secure_tls_session_renegotiate_callback_set(NX_SECURE_TLS_SESSION *tls_session,
                                                    ULONG (*func_ptr)(NX_SECURE_TLS_SESSION *session));
UINT nx_secure_tls_session_reset(NX_SECURE_TLS_SESSION *tls_session);
UINT nx_secure_tls_session_send(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET *packet_ptr,
                                ULONG wait_option);
UINT nx_secure_tls_session_server_callback_set(NX_SECURE_TLS_SESSION *tls_session,
                                               ULONG (*func_ptr)(NX_SECURE_TLS_SESSION *tls_session,
                                                                 NX_SECURE_TLS_HELLO_EXTENSION *extensions,
                                                                 UINT num_extensions));
UINT nx_secure_tls_session_sni_extension_parse(NX_SECURE_TLS_SESSION *tls_session,
                                               NX_SECURE_TLS_HELLO_EXTENSION *extensions,
                                               UINT num_extensions, NX_SECURE_X509_DNS_NAME *dns_name);
UINT nx_secure_tls_session_sni_extension_set(NX_SECURE_TLS_SESSION *tls_session,
                                             NX_SECURE_X509_DNS_NAME *dns_name);
UINT nx_secure_tls_session_start(NX_SECURE_TLS_SESSION *tls_session, NX_TCP_SOCKET *tcp_socket,
                                 UINT wait_option);
UINT nx_secure_tls_session_time_function_set(NX_SECURE_TLS_SESSION *tls_session,
                                             ULONG (*time_func_ptr)(VOID));
UINT nx_secure_tls_trusted_certificate_add(NX_SECURE_TLS_SESSION *tls_session,
                                           NX_SECURE_X509_CERT *certificate);
UINT nx_secure_tls_trusted_certificate_remove(NX_SECURE_TLS_SESSION *tls_session, UCHAR *common_name,
                                              UINT common_name_length);
UINT nx_secure_tls_packet_allocate(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET_POOL *pool_ptr,
                                   NX_PACKET **packet_ptr, ULONG wait_option);
#ifdef NX_SECURE_ENABLE_PSK_CIPHERSUITES
UINT nx_secure_tls_psk_add(NX_SECURE_TLS_SESSION *tls_session, UCHAR *pre_shared_key, UINT psk_length,
                           UCHAR *psk_identity, UINT identity_length, UCHAR *hint, UINT hint_length);

UINT nx_secure_tls_client_psk_set(NX_SECURE_TLS_SESSION *tls_session, UCHAR *pre_shared_key, UINT psk_length,
                                  UCHAR *psk_identity, UINT identity_length, UCHAR *hint, UINT hint_length);
#endif
#endif /* NX_SECURE_SOURCE_CODE */


#ifdef __cplusplus
}
#endif

#endif /* SRC_NX_SECURE_TLS_H_ */

