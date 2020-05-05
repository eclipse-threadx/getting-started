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
/*    nx_secure_tls.h                                     PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines all service prototypes and data structure         */
/*    definitions for TLS implementation.                                 */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            supported ECJPAKE, supported*/
/*                                            renegotiation, supported    */
/*                                            more ciphersuites, supported*/
/*                                            to configure symbols for PSK*/
/*                                            buffers, supported multiple */
/*                                            extensions, sorted the      */
/*                                            functions by alphabet order,*/
/*                                            added session callbacks,    */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s), added    */
/*                                            wait_option to _nx_secure_  */
/*                                            tls_send_certificate, added */
/*                                            a new error for renegotia-  */
/*                                            tion issues and changed the */
/*                                            name of the CSCV renegotia- */
/*                                            tion flag, added new error  */
/*                                            return for improperly       */
/*                                            initialized crypto routines,*/
/*                                            added support for AEAD      */
/*                                            cipher, added flexibility of*/
/*                                            using macros instead of     */
/*                                            direct C library function   */
/*                                            calls, added extension hook,*/
/*                                            used macros from crypto     */
/*                                            library, added macros for   */
/*                                            elliptic curve cryptography,*/
/*                                            passed crypto handle into   */
/*                                            crypto internal functions,  */
/*                                            added support for passing   */
/*                                            alert information to user,  */
/*                                            supported ECC ciphersuites, */
/*                                            disabled AEAD by default,   */
/*                                            improved packet length      */
/*                                            verification, supported TLS */
/*                                            Fallback SCSV, updated      */
/*                                            error return codes, added   */
/*                                            new API to get alert value  */
/*                                            and level, add remote cert  */
/*                                            memory optimization,        */
/*                                            improved internal logic,    */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/

#ifndef SRC_NX_SECURE_TLS_H_
#define SRC_NX_SECURE_TLS_H_

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#ifdef ECLIPSE_TEST
#define __REV16(x) x
#endif

/* Include the ThreadX and port-specific data type file.  */

#ifdef NX_SECURE_SOURCE_CODE
#ifndef TX_DISABLE_ERROR_CHECKING
#define TX_DISABLE_ERROR_CHECKING
#endif
#ifndef NX_DISABLE_ERROR_CHECKING
#define NX_DISABLE_ERROR_CHECKING
#endif
#endif

#include "nx_api.h"
#include "nx_secure_port.h"
#include "nx_crypto.h"

/* Enable ECC by default. */
#ifndef NX_SECURE_DISABLE_ECC_CIPHERSUITE
#ifndef NX_SECURE_ENABLE_ECC_CIPHERSUITE
#define NX_SECURE_ENABLE_ECC_CIPHERSUITE
#endif
#else
#undef NX_SECURE_ENABLE_ECC_CIPHERSUITE
#endif

#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
#include "nx_secure_tls_ecc.h"
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */
#include "nx_secure_x509.h"

#if defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE)
#ifndef NX_SECURE_ENABLE_AEAD_CIPHER
#define NX_SECURE_ENABLE_AEAD_CIPHER
#endif /* NX_SECURE_ENABLE_AEAD_CIPHER */
#endif
#ifndef NX_SECURE_AEAD_CIPHER_CHECK
#define NX_SECURE_AEAD_CIPHER_CHECK(a)                  NX_FALSE
#endif /* NX_SECURE_AEAD_CIPHER_CHECK */

/* ID is used to determine if a TLS session has been initialized. */
#define NX_SECURE_TLS_ID                                ((ULONG)0x544c5320)

#define EL_PRODUCT_NETX_SECURE
#define NETX_SECURE_MAJOR_VERSION                       5
#define NETX_SECURE_MINOR_VERSION                      12
#define NETX_SECURE_SERVICE_PACK_VERSION                0

/* The following symbols are defined for backward compatibility reasons. */
#define __PRODUCT_NETX_SECURE__
#define __NETX_SECURE_MAJOR_VERSION__                   NETX_SECURE_MAJOR_VERSION
#define __NETX_SECURE_MINOR_VERSION__                   NETX_SECURE_MINOR_VERSION
#define __NETX_SECURE_SERVICE_PACK_VERSION__            NETX_SECURE_SERVICE_PACK_VERSION

/* Define memcpy, memset and memcmp functions used internal. */
#ifndef NX_SECURE_MEMCPY
#define NX_SECURE_MEMCPY                                memcpy
#endif /* NX_SECURE_MEMCPY */

#ifndef NX_SECURE_MEMCMP
#define NX_SECURE_MEMCMP                                memcmp
#endif /* NX_SECURE_MEMCMP */

#ifndef NX_SECURE_MEMSET
#define NX_SECURE_MEMSET                                memset
#endif /* NX_SECURE_MEMSET */

#ifndef NX_SECURE_MEMMOVE
#define NX_SECURE_MEMMOVE                               memmove
#endif /* NX_SECURE_MEMMOVE */

/* Define extensions used for user defined actions during handshake. */
#ifndef NX_SECURE_PROCESS_CLIENTHELLO_EXTENSIONS_EXTENSION
#define NX_SECURE_PROCESS_CLIENTHELLO_EXTENSIONS_EXTENSION
#endif /* NX_SECURE_PROCESS_CLIENTHELLO_EXTENSIONS_EXTENSION */

#ifndef NX_SECURE_PROCESS_CLIENTHELLO_EXTENSION
#define NX_SECURE_PROCESS_CLIENTHELLO_EXTENSION
#endif /* NX_SECURE_PROCESS_CLIENTHELLO_EXTENSION */

#ifndef NX_SECURE_PROCESS_SERVERHELLO_EXTENSIONS_EXTENSION
#define NX_SECURE_PROCESS_SERVERHELLO_EXTENSIONS_EXTENSION
#endif /* NX_SECURE_PROCESS_SERVERHELLO_EXTENSIONS_EXTENSION */

#ifndef NX_SECURE_SEND_CLIENTHELLO_EXTENSIONS_EXTENSION
#define NX_SECURE_SEND_CLIENTHELLO_EXTENSIONS_EXTENSION
#endif /* NX_SECURE_SEND_CLIENTHELLO_EXTENSIONS_EXTENSION */

#ifndef NX_SECURE_SEND_SERVERHELLO_EXTENSIONS_EXTENSION
#define NX_SECURE_SEND_SERVERHELLO_EXTENSIONS_EXTENSION
#endif /* NX_SECURE_SEND_SERVERHELLO_EXTENSIONS_EXTENSION */

#ifndef NX_SECURE_PROCESS_CLIENT_KEY_EXCHANGE_EXTENSION
#define NX_SECURE_PROCESS_CLIENT_KEY_EXCHANGE_EXTENSION
#endif /* NX_SECURE_PROCESS_CLIENT_KEY_EXCHANGE_EXTENSION */

#ifndef NX_SECURE_PROCESS_SERVER_KEY_EXCHANGE_EXTENSION
#define NX_SECURE_PROCESS_SERVER_KEY_EXCHANGE_EXTENSION
#endif /* NX_SECURE_PROCESS_SERVER_KEY_EXCHANGE_EXTENSION */

#ifndef NX_SECURE_SEND_CLIENT_KEY_EXCHANGE_EXTENSION
#define NX_SECURE_SEND_CLIENT_KEY_EXCHANGE_EXTENSION
#endif /* NX_SECURE_SEND_CLIENT_KEY_EXCHANGE_EXTENSION */

#ifndef NX_SECURE_SEND_SERVER_KEY_EXCHANGE_EXTENSION
#define NX_SECURE_SEND_SERVER_KEY_EXCHANGE_EXTENSION
#endif /* NX_SECURE_SEND_SERVER_KEY_EXCHANGE_EXTENSION */

#ifndef NX_SECURE_PROCESS_CERTIFICATE_REQUEST_EXTENSION
#define NX_SECURE_PROCESS_CERTIFICATE_REQUEST_EXTENSION
#endif /* NX_SECURE_PROCESS_CERTIFICATE_REQUEST_EXTENSION */

#ifndef NX_SECURE_SEND_CERTIFICATE_REQUEST_EXTENSION
#define NX_SECURE_SEND_CERTIFICATE_REQUEST_EXTENSION
#endif /* NX_SECURE_SEND_CERTIFICATE_REQUEST_EXTENSION */

#ifndef NX_SECURE_PROCESS_CERTIFICATE_VERIFY_EXTENSION
#define NX_SECURE_PROCESS_CERTIFICATE_VERIFY_EXTENSION
#endif /* NX_SECURE_PROCESS_CERTIFICATE_VERIFY_EXTENSION */

#ifndef NX_SECURE_SEND_CERTIFICATE_VERIFY_EXTENSION
#define NX_SECURE_SEND_CERTIFICATE_VERIFY_EXTENSION
#endif /* NX_SECURE_SEND_CERTIFICATE_VERIFY_EXTENSION */

#ifndef NX_SECURE_GENERATE_PREMASTER_SECRET_EXTENSION
#define NX_SECURE_GENERATE_PREMASTER_SECRET_EXTENSION
#endif /* NX_SECURE_GENERATE_PREMASTER_SECRET_EXTENSION */

#ifndef NX_SECURE_GENERATE_KEYS_EXTENSION
#define NX_SECURE_GENERATE_KEYS_EXTENSION
#endif /* NX_SECURE_GENERATE_KEYS_EXTENSION */

#ifndef NX_SECURE_TLS_SESSION_EXTENSION
#define NX_SECURE_TLS_SESSION_EXTENSION
#endif /* NX_SECURE_TLS_SESSION_EXTENSION */

/* Map NX_SECURE_CALLER_CHECKING_EXTERNS to NX_CALLER_CHECKING_EXTERNS, which is defined
   in nx_port.h.*/

#define NX_SECURE_CALLER_CHECKING_EXTERNS               NX_CALLER_CHECKING_EXTERNS

/* Configuration macros - define these to disable TLS client or server.
   #define NX_SECURE_TLS_SERVER_DISABLED
   #define NX_SECURE_TLS_CLIENT_DISABLED
 */

/* Configuration macro: allow self-signed certificates to be used to identify a remote host. */
/* #define NX_SECURE_ALLOW_SELF_SIGNED_CERTIFICATES */

/* Configuration macro: enable secure session renegotiation extension (RFC 5746). */
#define NX_SECURE_TLS_ENABLE_SECURE_RENEGOTIATION

/* API return values.  */

#define NX_SECURE_TLS_SUCCESS                           0x00        /* Function returned successfully. */
#define NX_SECURE_TLS_SESSION_UNINITIALIZED             0x101       /* TLS main loop called with uninitialized socket. */
#define NX_SECURE_TLS_UNRECOGNIZED_MESSAGE_TYPE         0x102       /* TLS record layer received an unrecognized message type. */
#define NX_SECURE_TLS_INVALID_STATE                     0x103       /* Internal error - state not recognized. */
#define NX_SECURE_TLS_INVALID_PACKET                    0x104       /* Internal error - received packet did not contain TLS data. */
#define NX_SECURE_TLS_UNKNOWN_CIPHERSUITE               0x105       /* The chosen ciphersuite is not supported - internal error for server,
                                                                       for client it means the remote host sent a bad ciphersuite (error or attack). */
#define NX_SECURE_TLS_UNSUPPORTED_CIPHER                0x106       /* In doing an encryption or decryption, the chosen cipher is disabled or unavailable. */
#define NX_SECURE_TLS_HANDSHAKE_FAILURE                 0x107       /* Something in message processing during the handshake has failed. */
#define NX_SECURE_TLS_HASH_MAC_VERIFY_FAILURE           0x108       /* An incoming record had a MAC that did not match the one we generated. */
#define NX_SECURE_TLS_TCP_SEND_FAILED                   0x109       /* The outgoing TCP send of a record failed for some reason. */
#define NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH          0x10A       /* An incoming message had a length that was incorrect (usually a length other
                                                                       than one in the header, as in certificate messages) */
#define NX_SECURE_TLS_BAD_CIPHERSPEC                    0x10B       /* An incoming ChangeCipherSpec message was incorrect. */
#define NX_SECURE_TLS_INVALID_SERVER_CERT               0x10C       /* An incoming server certificate did not parse correctly. */
#define NX_SECURE_TLS_UNSUPPORTED_PUBLIC_CIPHER         0x10D       /* A certificate provided by a server specified a public-key operation we do not support. */
#define NX_SECURE_TLS_NO_SUPPORTED_CIPHERS              0x10E       /* Received a ClientHello with no supported ciphersuites. */
#define NX_SECURE_TLS_UNKNOWN_TLS_VERSION               0x10F       /* An incoming record had a TLS version that isn't recognized. */
#define NX_SECURE_TLS_UNSUPPORTED_TLS_VERSION           0x110       /* An incoming record had a valid TLS version, but one that isn't supported. */
#define NX_SECURE_TLS_ALLOCATE_PACKET_FAILED            0x111       /* An internal packet allocation for a TLS message failed. */
#define NX_SECURE_TLS_INVALID_CERTIFICATE               0x112       /* An X509 certificate did not parse correctly. */
#define NX_SECURE_TLS_NO_CLOSE_RESPONSE                 0x113       /* During a TLS session close, did not receive a CloseNotify from the remote host. */
#define NX_SECURE_TLS_ALERT_RECEIVED                    0x114       /* The remote host sent an alert, indicating an error and closing the connection. */
#define NX_SECURE_TLS_FINISHED_HASH_FAILURE             0x115       /* The Finish message hash received does not match the local generated hash - handshake corruption. */
#define NX_SECURE_TLS_UNKNOWN_CERT_SIG_ALGORITHM        0x116       /* A certificate during verification had an unsupported signature algorithm. */
#define NX_SECURE_TLS_CERTIFICATE_SIG_CHECK_FAILED      0x117       /* A certificate signature verification check failed - certificate data did not match signature. */
#define NX_SECURE_TLS_BAD_COMPRESSION_METHOD            0x118       /* Received a Hello message with an unsupported compression method. */
#define NX_SECURE_TLS_CERTIFICATE_NOT_FOUND             0x119       /* In an operation on a certificate list, no matching certificate was found. */
#define NX_SECURE_TLS_INVALID_SELF_SIGNED_CERT          0x11A       /* The remote host sent a self-signed certificate and NX_SECURE_ALLOW_SELF_SIGNED_CERTIFICATES is not defined. */
#define NX_SECURE_TLS_ISSUER_CERTIFICATE_NOT_FOUND      0x11B       /* A remote certificate was received with an issuer not in the local trusted store. */
#define NX_SECURE_TLS_OUT_OF_ORDER_MESSAGE              0x11C       /* A DTLS message was received in the wrong order - a dropped datagram is the likely culprit. */
#define NX_SECURE_TLS_INVALID_REMOTE_HOST               0x11D       /* A packet was received from a remote host that we do not recognize. */
#define NX_SECURE_TLS_INVALID_EPOCH                     0x11E       /* A DTLS message was received and matched to a DTLS session but it had the wrong epoch and should be ignored. */
#define NX_SECURE_TLS_REPEAT_MESSAGE_RECEIVED           0x11F       /* A DTLS message was received with a sequence number we have already seen, ignore it. */
#define NX_SECURE_TLS_NEED_DTLS_SESSION                 0x120       /* A TLS session was used in a DTLS API that was not initialized for DTLS. */
#define NX_SECURE_TLS_NEED_TLS_SESSION                  0x121       /* A TLS session was used in a TLS API that was initialized for DTLS and not TLS. */
#define NX_SECURE_TLS_SEND_ADDRESS_MISMATCH             0x122       /* Caller attempted to send data over a DTLS session with an IP address or port that did not match the session. */
#define NX_SECURE_TLS_NO_FREE_DTLS_SESSIONS             0x123       /* A new connection tried to get a DTLS session from the cache, but there were none free. */
#define NX_SECURE_DTLS_SESSION_NOT_FOUND                0x124       /* The caller searched for a DTLS session, but the given IP address and port did not match any entries in the cache. */
#define NX_SECURE_TLS_NO_MORE_PSK_SPACE                 0x125       /* The caller attempted to add a PSK to a TLS session but there was no more space in the given session. */
#define NX_SECURE_TLS_NO_MATCHING_PSK                   0x126       /* A remote host provided a PSK identity hint that did not match any in our local store. */
#define NX_SECURE_TLS_CLOSE_NOTIFY_RECEIVED             0x127       /* A TLS session received a CloseNotify alert from the remote host indicating the session is complete. */
#define NX_SECURE_TLS_NO_AVAILABLE_SESSIONS             0x128       /* No TLS sessions in a TLS object are available to handle a connection. */
#define NX_SECURE_TLS_NO_CERT_SPACE_ALLOCATED           0x129       /* No certificate space was allocated for incoming remote certificates. */
#define NX_SECURE_TLS_PADDING_CHECK_FAILED              0x12A       /* Encryption padding in an incoming message was not correct. */
#define NX_SECURE_TLS_UNSUPPORTED_CERT_SIGN_TYPE        0x12B       /* In processing a CertificateVerifyRequest, no supported certificate type was provided by the remote server. */
#define NX_SECURE_TLS_UNSUPPORTED_CERT_SIGN_ALG         0x12C       /* In processing a CertificateVerifyRequest, no supported signature algorithm was provided by the remote server. */
#define NX_SECURE_TLS_INSUFFICIENT_CERT_SPACE           0x12D       /* Not enough certificate buffer space allocated for a certificate. */
#define NX_SECURE_TLS_PROTOCOL_VERSION_CHANGED          0x12E       /* The protocol version in an incoming TLS record did not match the version of the established session. */
#define NX_SECURE_TLS_NO_RENEGOTIATION_ERROR            0x12F       /* A HelloRequest message was received, but we are not re-negotiating. */
#define NX_SECURE_TLS_UNSUPPORTED_FEATURE               0x130       /* A feature that was disabled was encountered during a TLS session or handshake. */
#define NX_SECURE_TLS_CERTIFICATE_VERIFY_FAILURE        0x131       /* A CertificateVerify message from a remote Client failed to verify the Client certificate. */
#define NX_SECURE_TLS_EMPTY_REMOTE_CERTIFICATE_RECEIVED 0x132       /* The remote host sent an empty certificate message. */
#define NX_SECURE_TLS_RENEGOTIATION_EXTENSION_ERROR     0x133       /* An error occurred in processing or sending the Secure Renegotiation Indication Extension. */
#define NX_SECURE_TLS_RENEGOTIATION_SESSION_INACTIVE    0x134       /* A server attempted to re-establish a connection that was already closed. */
#define NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL           0x135       /* A TLS record was received which has a size that exceeds the allocated packet buffer space. */
#define NX_SECURE_TLS_EXTENSION_NOT_FOUND               0x136       /* A TLS extension parsing function did not find the intended extension in the hello extension data. */
#define NX_SECURE_TLS_SNI_EXTENSION_INVALID             0x137       /* Received a ClientHello containing invalid SNI extension data. */
#define NX_SECURE_TLS_CERT_ID_INVALID                   0x138       /* Tried to add a certificate with a numeric ID that was invalid (likely 0). */
#define NX_SECURE_TLS_CERT_ID_DUPLICATE                 0x139       /* Tried to add a certificate with a numeric ID that was already used - needs to be unique. */
#define NX_SECURE_TLS_RENEGOTIATION_FAILURE             0x13A       /* Attempted a renegotiation with a remote host that did not supply the SCSV or renegotiation extension. */
#define NX_SECURE_TLS_MISSING_CRYPTO_ROUTINE            0x13B       /* In attempting to perform a cryptographic operation, an entry in the ciphersuite table (or one of its function pointers) was NULL. */
#define NX_SECURE_TLS_EMPTY_EC_GROUP                    0x13C       /* ECC ciphersuite is set but no supported EC group. */
#define NX_SECURE_TLS_EMPTY_EC_POINT_FORMAT             0x13D       /* ECC ciphersuite is set but no supported EC point format. */
#define NX_SECURE_TLS_INAPPROPRIATE_FALLBACK            0x146       /* Remote Client attempted an inappropriate TLS version downgrade. */
#define NX_SECURE_TLS_CRYPTO_KEYS_TOO_LARGE             0x147       /* In attempting to generate TLS session keys, the key buffer was too small - increase NX_SECURE_TLS_KEY_MATERIAL_SIZE. */
#define NX_SECURE_TLS_UNSUPPORTED_ECC_CURVE             0x148       /* The remote host provided a certificate or chose a ciphersuite with an ECC curve that isn't supported. */
#define NX_SECURE_TLS_UNSUPPORTED_ECC_FORMAT            0x149       /* Encountered a curve type or ECC format that is not supported. */
#define NX_SECURE_TLS_UNSUPPORTED_SIGNATURE_ALGORITHM   0x14A       /* An unsupported signature algorithm was encountered (used in key exchange or other non-certificate situations). */
#define NX_SECURE_TLS_SIGNATURE_VERIFICATION_ERROR      0x14B       /* A signature verification check failed (used in key exchange or other non-cert situations). */
#define NX_SECURE_TLS_UNEXPECTED_MESSAGE                0x14C       /* TLS received an unexpected message from the remote host. */
#define NX_SECURE_TLS_AEAD_DECRYPT_FAIL                 0x14D       /* An incoming record did not pass integrity check with AEAD ciphers. */


/* NX_CONTINUE is a symbol defined in NetX Duo 5.10.  For backward compatibility, this symbol is defined here */
#if ((__NETXDUO_MAJOR_VERSION__ == 5) && (__NETXDUO_MINOR_VERSION__ == 9))
#define NX_CONTINUE                                     0x55
#endif

#ifdef NX_SECURE_TLS_SERVER_DISABLED
#ifdef NX_SECURE_TLS_CLIENT_DISABLED
#error "Must enable either TLS Client or TLS Server!"
#endif
#endif

/* Define TLS and DTLS identity */
#define NX_SECURE_TLS                                   0
#define NX_SECURE_DTLS                                  1

/* For proper handshake processing, we save off what type of socket we have.
 * These values represent the possible types. */
#define NX_SECURE_TLS_SESSION_TYPE_NONE                 0 /* No socket type assigned yet. Should produce an error if the wrong function is called. */
#define NX_SECURE_TLS_SESSION_TYPE_CLIENT               1 /* Client TLS socket - assigned when nx_secure_tls_client_socket_connect is called. */
#define NX_SECURE_TLS_SESSION_TYPE_SERVER               2 /* Server TLS socket - assigned when nx_secure_tls_server_socket_listen is called. */

/* TLS handshake states - Server. */
#define NX_SECURE_TLS_SERVER_STATE_IDLE                 0  /* TLS server is waiting for a ClientHello. */
#define NX_SECURE_TLS_SERVER_STATE_ERROR                1  /* TLS server encountered an internal (non-alert) error. */
#define NX_SECURE_TLS_SERVER_STATE_ALERT_SENT           2  /* TLS server encountered an issue and sent an alert to the remote client. */
#define NX_SECURE_TLS_SERVER_STATE_SEND_HELLO           3  /* A ClientHello has been received and we need to respond. */
#define NX_SECURE_TLS_SERVER_STATE_SEND_HELLO_VERIFY    4  /* In DTLS, send a HelloVerifyRequest message back to the client. */
#define NX_SECURE_TLS_SERVER_STATE_HELLO_SENT           5  /* ServerHelloDone response has been sent to client. */
#define NX_SECURE_TLS_SERVER_STATE_CLIENT_CERTIFICATE   6  /* A Client Certificate message has been received. */
#define NX_SECURE_TLS_SERVER_STATE_KEY_EXCHANGE         7  /* Server needs to send a key exchange message. */
#define NX_SECURE_TLS_SERVER_STATE_CERTIFICATE_VERIFY   8  /* A Client CertificateVerify message has been received. */
#define NX_SECURE_TLS_SERVER_STATE_FINISH_HANDSHAKE     9  /* Server received a client Finished message and needs to respond. */
#define NX_SECURE_TLS_SERVER_STATE_HANDSHAKE_FINISHED   10 /* Server has completed the handshake. */
#define NX_SECURE_TLS_SERVER_STATE_HELLO_REQUEST        11 /* A HelloRequest has been sent. */

/* TLS handshake states - Client. */
#define NX_SECURE_TLS_CLIENT_STATE_IDLE                 0  /* Client socket is not connected, waiting for connection request from application. */
#define NX_SECURE_TLS_CLIENT_STATE_ERROR                1  /* Client socket has encountered an error (separate from alerts). */
#define NX_SECURE_TLS_CLIENT_STATE_ALERT_SENT           2  /* TLS Client sent an alert to the remote server. */
#define NX_SECURE_TLS_CLIENT_STATE_HELLO_REQUEST        3  /* TLS server sent a hello request, we need to re-negotiate the session. */
#define NX_SECURE_TLS_CLIENT_STATE_HELLO_VERIFY         4  /* A HelloVerifyRequest was received - need to re-send ClientHello (DTLS). */
#define NX_SECURE_TLS_CLIENT_STATE_SERVERHELLO          5  /* A ServerHello has been received. */
#define NX_SECURE_TLS_CLIENT_STATE_SERVER_CERTIFICATE   6  /* A Server Certificate message has been received. */
#define NX_SECURE_TLS_CLIENT_STATE_SERVER_KEY_EXCHANGE  7  /* A ServerKeyExchange message has been received. */
#define NX_SECURE_TLS_CLIENT_STATE_CERTIFICATE_REQUEST  8  /* A Server CertificateRequest message has been received. */
#define NX_SECURE_TLS_CLIENT_STATE_SERVERHELLO_DONE     9  /* A ServerHelloDone message has been received. */
#define NX_SECURE_TLS_CLIENT_STATE_HANDSHAKE_FINISHED   10 /* Client has received a Finished message to end the handshake. */
#define NX_SECURE_TLS_CLIENT_STATE_RENEGOTIATING        11 /* Client is renegotiating a handshake. Only used to kick off a renegotiation. */

/* TLS Alert message numbers from RFC 5246. */
#define NX_SECURE_TLS_ALERT_CLOSE_NOTIFY                0
#define NX_SECURE_TLS_ALERT_UNEXPECTED_MESSAGE          10
#define NX_SECURE_TLS_ALERT_BAD_RECORD_MAC              20
#define NX_SECURE_TLS_ALERT_DECRYPTION_FAILED_RESERVED  21
#define NX_SECURE_TLS_ALERT_RECORD_OVERFLOW             22
#define NX_SECURE_TLS_ALERT_DECOMPRESSION_FAILURE       30
#define NX_SECURE_TLS_ALERT_HANDSHAKE_FAILURE           40
#define NX_SECURE_TLS_ALERT_NO_CERTIFICATE_RESERVED     41
#define NX_SECURE_TLS_ALERT_BAD_CERTIFICATE             42
#define NX_SECURE_TLS_ALERT_UNSUPPORTED_CERTIFICATE     43
#define NX_SECURE_TLS_ALERT_CERTIFICATE_REVOKED         44
#define NX_SECURE_TLS_ALERT_CERTIFICATE_EXPIRED         45
#define NX_SECURE_TLS_ALERT_CERTIFICATE_UNKNOWN         46
#define NX_SECURE_TLS_ALERT_ILLEGAL_PARAMETER           47
#define NX_SECURE_TLS_ALERT_UNKNOWN_CA                  48
#define NX_SECURE_TLS_ALERT_ACCESS_DENIED               49
#define NX_SECURE_TLS_ALERT_DECODE_ERROR                50
#define NX_SECURE_TLS_ALERT_DECRYPT_ERROR               51
#define NX_SECURE_TLS_ALERT_EXPORT_RESTRICTION_RESERVED 60
#define NX_SECURE_TLS_ALERT_PROTOCOL_VERSION            70
#define NX_SECURE_TLS_ALERT_INSUFFICIENT_SECURITY       71
#define NX_SECURE_TLS_ALERT_INTERNAL_ERROR              80
#define NX_SECURE_TLS_ALERT_INAPPROPRIATE_FALLBACK      86
#define NX_SECURE_TLS_ALERT_USER_CANCELED               90
#define NX_SECURE_TLS_ALERT_NO_RENEGOTIATION            100
#define NX_SECURE_TLS_ALERT_UNSUPPORTED_EXTENSION       110
#define NX_SECURE_TLS_ALERT_UNKNOWN_PSK_IDENTITY        115

/* TLS alert levels. */
#define NX_SECURE_TLS_ALERT_LEVEL_WARNING               0x1
#define NX_SECURE_TLS_ALERT_LEVEL_FATAL                 0x2


/* TLS protocol versions - TLS version 1.2 has protocol version 3.3 (for legacy reasons). */
#define NX_SECURE_TLS_VERSION_MAJOR_3                   0x3
#define NX_SECURE_SSL_VERSION_MINOR_3_0                 0x0
#define NX_SECURE_TLS_VERSION_MINOR_1_0                 0x1
#define NX_SECURE_TLS_VERSION_MINOR_1_1                 0x2
#define NX_SECURE_TLS_VERSION_MINOR_1_2                 0x3

#define NX_SECURE_TLS_VERSION_SSL_3_0                   ((NX_SECURE_TLS_VERSION_MAJOR_3 << 8) | NX_SECURE_SSL_VERSION_MINOR_3_0)
#define NX_SECURE_TLS_VERSION_TLS_1_0                   ((NX_SECURE_TLS_VERSION_MAJOR_3 << 8) | NX_SECURE_TLS_VERSION_MINOR_1_0)
#define NX_SECURE_TLS_VERSION_TLS_1_1                   ((NX_SECURE_TLS_VERSION_MAJOR_3 << 8) | NX_SECURE_TLS_VERSION_MINOR_1_1)
#define NX_SECURE_TLS_VERSION_TLS_1_2                   ((NX_SECURE_TLS_VERSION_MAJOR_3 << 8) | NX_SECURE_TLS_VERSION_MINOR_1_2)


/* The number of TLS versions actually recognized by the NetX Secure TLS stack. */
#define NX_SECURE_TLS_NUM_VERSIONS                      (4)

/* Configuration macros for supported TLS versions. */
#ifdef NX_SECURE_TLS_ENABLE_SSL_3_0
#define NX_SECURE_TLS_SSL_3_0_ENABLED                   (1) /* SSLv3 supported. */
#else
#define NX_SECURE_TLS_SSL_3_0_ENABLED                   (0) /* SSLv3 not currently supported. */
#endif

#ifdef NX_SECURE_TLS_ENABLE_TLS_1_0
#define NX_SECURE_TLS_TLS_1_0_ENABLED                   (1) /* TLS 1.0 supported. */
#else
#define NX_SECURE_TLS_TLS_1_0_ENABLED                   (0) /* TLS 1.0 not currently supported. */
#endif

#ifdef NX_SECURE_TLS_DISABLE_TLS_1_1
#define NX_SECURE_TLS_TLS_1_1_ENABLED                   (0) /* TLS 1.1 supported. */
#else
#define NX_SECURE_TLS_TLS_1_1_ENABLED                   (1) /* TLS 1.1 not currently supported. */
#endif

#ifndef NX_SECURE_TLS_TLS_1_2_ENABLED
#define NX_SECURE_TLS_TLS_1_2_ENABLED                   (1)
#endif

/* Define a structure to keep track of which versions of TLS are enabled and supported. */
typedef struct NX_SECURE_TLS_VERSIONS_STRUCT
{
    /* The protocol version in network byte-order format for use in TLS messages. */
    USHORT nx_secure_tls_protocol_version;

    /* Flag indicating that the associated TLS protocol version is supported/enabled. */
    USHORT nx_secure_tls_is_supported;
} NX_SECURE_TLS_VERSIONS;

/* Define a structure to keep track of supported versions for TLS and DTLS */
typedef struct NX_SECURE_VERSIONS_LIST_STRUCT
{
    const NX_SECURE_TLS_VERSIONS *nx_secure_versions_list;
    UINT                          nx_secure_versions_list_count;
} NX_SECURE_VERSIONS_LIST;

/* TLS ciphersuite definitions. */
#define TLS_NULL_WITH_NULL_NULL                            0x0000
#define TLS_RSA_WITH_NULL_MD5                              0x0001
#define TLS_RSA_WITH_NULL_SHA                              0x0002
#define TLS_RSA_WITH_AES_128_CBC_SHA                       0x002F
#define TLS_DH_DSS_WITH_AES_128_CBC_SHA                    0x0030
#define TLS_DH_RSA_WITH_AES_128_CBC_SHA                    0x0031
#define TLS_DHE_DSS_WITH_AES_128_CBC_SHA                   0x0032
#define TLS_DHE_RSA_WITH_AES_128_CBC_SHA                   0x0033
#define TLS_DH_anon_WITH_AES_128_CBC_SHA                   0x0034
#define TLS_RSA_WITH_AES_256_CBC_SHA                       0x0035
#define TLS_DH_DSS_WITH_AES_256_CBC_SHA                    0x0036
#define TLS_DH_RSA_WITH_AES_256_CBC_SHA                    0x0037
#define TLS_DHE_DSS_WITH_AES_256_CBC_SHA                   0x0038
#define TLS_DHE_RSA_WITH_AES_256_CBC_SHA                   0x0039
#define TLS_DH_anon_WITH_AES_256_CBC_SHA                   0x003A
#define TLS_RSA_WITH_AES_128_CBC_SHA256                    0x003C
#define TLS_RSA_WITH_AES_256_CBC_SHA256                    0x003D
#define TLS_PSK_WITH_AES_128_CBC_SHA                       0x008C
#define TLS_PSK_WITH_AES_256_CBC_SHA                       0x008D
#define TLS_RSA_WITH_AES_128_GCM_SHA256                    0x009C
#define TLS_RSA_WITH_AES_256_GCM_SHA384                    0x009D
#define TLS_PSK_WITH_AES_128_CBC_SHA256                    0x00AE
#define TLS_PSK_WITH_AES_128_CCM_8                         0xC0A8

/* EC Ciphersuites. */
#define TLS_ECDH_ECDSA_WITH_NULL_SHA                       0xC001
#define TLS_ECDH_ECDSA_WITH_RC4_128_SHA                    0xC002
#define TLS_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA               0xC003
#define TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA                0xC004
#define TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA                0xC005
#define TLS_ECDHE_ECDSA_WITH_NULL_SHA                      0xC006
#define TLS_ECDHE_ECDSA_WITH_RC4_128_SHA                   0xC007
#define TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA              0xC008
#define TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA               0xC009
#define TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA               0xC00A
#define TLS_ECDH_RSA_WITH_NULL_SHA                         0xC00B
#define TLS_ECDH_RSA_WITH_RC4_128_SHA                      0xC00C
#define TLS_ECDH_RSA_WITH_3DES_EDE_CBC_SHA                 0xC00D
#define TLS_ECDH_RSA_WITH_AES_128_CBC_SHA                  0xC00E
#define TLS_ECDH_RSA_WITH_AES_256_CBC_SHA                  0xC00F
#define TLS_ECDHE_RSA_WITH_NULL_SHA                        0xC010
#define TLS_ECDHE_RSA_WITH_RC4_128_SHA                     0xC011
#define TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA                0xC012
#define TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA                 0xC013
#define TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA                 0xC014
#define TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256            0xC023
#define TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384            0xC024
#define TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256             0xC025
#define TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384             0xC026
#define TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256              0xC027
#define TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384              0xC028
#define TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256               0xC029
#define TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384               0xC02A
#define TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256            0xC02B
#define TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384            0xC02C
#define TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256             0xC02D
#define TLS_ECDH_ECDSA_WITH_AES_256_GCM_SHA384             0xC02E
#define TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256              0xC02F
#define TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384              0xC030
#define TLS_ECDH_RSA_WITH_AES_128_GCM_SHA256               0xC031
#define TLS_ECDH_RSA_WITH_AES_256_GCM_SHA384               0xC032

#define TLS_EMPTY_RENEGOTIATION_INFO_SCSV                  0x00FF /* Secure renegotiation extension Signalling Ciphersuite Value (SCSV). */
#define TLS_FALLBACK_NOTIFY_SCSV                           0x5600 /* TLS version fallback notification SCSV. */

/* Cipher and hash methods for look up in determining what methods to use for a particular ciphersuite. */
#define TLS_CIPHER_NULL                                    0x00
#define TLS_CIPHER_AES                                     0x01
#define TLS_CIPHER_RC4                                     0x02
#define TLS_ECJPAKE_WITH_AES_128_CCM_8                     0xC0FF

/* Define the key exchange algorithm for backward compatibility. */
#define TLS_CIPHER_RSA                                     NX_CRYPTO_KEY_EXCHANGE_RSA
#define TLS_PUBLIC_AUTH_PSK                                NX_CRYPTO_KEY_EXCHANGE_PSK
#define TLS_PUBLIC_AUTH_ECJPAKE                            NX_CRYPTO_KEY_EXCHANGE_ECJPAKE
#define TLS_PRF_1                                          NX_CRYPTO_PRF_HMAC_SHA1 /* TLSv1.0/1.1 PRF */
#define TLS_PRF_SHA_256                                    NX_CRYPTO_PRF_HMAC_SHA2_256 /* TLS PRF, SHA-256 based for TLSv1.2. */
#define TLS_HASH_SHA_1                                     NX_CRYPTO_HASH_SHA1 
#define TLS_HASH_SHA_256                                   NX_CRYPTO_HASH_SHA256 
#define TLS_HASH_SHA_384                                   NX_CRYPTO_HASH_SHA384 
#define TLS_HASH_SHA_512                                   NX_CRYPTO_HASH_SHA512 

/* Client Certificate Types for Certificate Request messages.
   Values taken directly from RFC 5246, section 7.4.4. */
#define NX_SECURE_TLS_CERT_TYPE_NONE                       0
#define NX_SECURE_TLS_CERT_TYPE_RSA_SIGN                   1
#define NX_SECURE_TLS_CERT_TYPE_DSS_SIGN                   2
#define NX_SECURE_TLS_CERT_TYPE_RSA_FIXED_DH               3
#define NX_SECURE_TLS_CERT_TYPE_DSS_FIXED_DH               4
#define NX_SECURE_TLS_CERT_TYPE_RSA_EPHEMERAL_DH           5
#define NX_SECURE_TLS_CERT_TYPE_DSS_EPHEMERAL_DH           6
#define NX_SECURE_TLS_CERT_TYPE_FORTEZZA_DMS               20

/* Algorithm identifiers for signature methods used in CertificateVerify messages and
   in the "signature_algorithms" extension, from RFC 5246, section 7.4.1.4.1.*/
#define NX_SECURE_TLS_HASH_ALGORITHM_NONE                  0
#define NX_SECURE_TLS_HASH_ALGORITHM_MD5                   1
#define NX_SECURE_TLS_HASH_ALGORITHM_SHA1                  2
#define NX_SECURE_TLS_HASH_ALGORITHM_SHA224                3
#define NX_SECURE_TLS_HASH_ALGORITHM_SHA256                4
#define NX_SECURE_TLS_HASH_ALGORITHM_SHA384                5
#define NX_SECURE_TLS_HASH_ALGORITHM_SHA512                6

/* Signature algorithms paired with the hash algorithms above. */
#define NX_SECURE_TLS_SIGNATURE_ALGORITHM_ANONYMOUS        0
#define NX_SECURE_TLS_SIGNATURE_ALGORITHM_RSA              1
#define NX_SECURE_TLS_SIGNATURE_ALGORITHM_DSA              2
#define NX_SECURE_TLS_SIGNATURE_ALGORITHM_ECDSA            3

/* Packed algorithm values (as seen over the wire). */
#define NX_SECURE_TLS_SIGNATURE_RSA_SHA256                 (((UINT)NX_SECURE_TLS_HASH_ALGORITHM_SHA256 << 8) + (UINT)NX_SECURE_TLS_SIGNATURE_ALGORITHM_RSA)

/* Session key generation and assignment constants. */
#define NX_SECURE_TLS_KEY_SET_LOCAL                        0
#define NX_SECURE_TLS_KEY_SET_REMOTE                       1

/* TLS extension definitions from RFC 5246, 5746, 6066, and others. */
#define NX_SECURE_TLS_EXTENSION_SERVER_NAME_INDICATION     (0x0000)
#define NX_SECURE_TLS_EXTENSION_MAX_FRAGMENT_LENGTH        (0x0001)
#define NX_SECURE_TLS_EXTENSION_CLIENT_CERTIFICATE_URL     (0x0002)
#define NX_SECURE_TLS_EXTENSION_TRUSTED_CA_INDICATION      (0x0003)
#define NX_SECURE_TLS_EXTENSION_TRUNCATED_HMAC             (0x0004)
#define NX_SECURE_TLS_EXTENSION_CERTIFICATE_STATUS_REQUEST (0x0005)
#define NX_SECURE_TLS_EXTENSION_EC_GROUPS                  (0x000A)
#define NX_SECURE_TLS_EXTENSION_EC_POINT_FORMATS           (0x000B)
#define NX_SECURE_TLS_EXTENSION_SIGNATURE_ALGORITHMS       (0x000D)
#define NX_SECURE_TLS_EXTENSION_ECJPAKE_KEY_KP_PAIR        (0x0100)
#define NX_SECURE_TLS_EXTENSION_SECURE_RENEGOTIATION       (0xFF01)

/* Extension-specific values. */
#define NX_SECURE_TLS_SNI_NAME_TYPE_DNS                    (0x0)

/* Define the maximum number of structures allocated for TLS ClientHello and ServerHello extension data. */
#define NX_SECURE_TLS_HELLO_EXTENSIONS_MAX                 (10)

/* Some constants for use in defining buffers for crypto and hash operations. */
#define NX_SECURE_TLS_RANDOM_SIZE                          (32)  /* Size of the server and client random values, in bytes. */
#define NX_SECURE_TLS_MAX_HASH_SIZE                        (32)  /* This is the largest size a single hash/MAC for ANY session *might* be, in bytes. */
#ifndef NX_SECURE_TLS_PREMASTER_SIZE
#define NX_SECURE_TLS_PREMASTER_SIZE                       (48)  /* The pre-master secret is 48 bytes, except for PSK ciphersuites for which it may be more. */
#endif
#define NX_SECURE_TLS_MASTER_SIZE                          (48)  /* The master secret is also 48 bytes. */
#define NX_SECURE_TLS_MAX_KEY_SIZE                         (32)  /* Maximum size of a session key in bytes. */
#define NX_SECURE_TLS_MAX_IV_SIZE                          (16)  /* Maximum size of a session initialization vector in bytes. */
#define NX_SECURE_TLS_SESSION_ID_SIZE                      (256) /* Maximum size of a session ID value used for renegotiation in bytes. */
#define NX_SECURE_TLS_SEQUENCE_NUMBER_SIZE                 (2)   /* Size of sequence numbers for TLS records in 32-bit words. */
#define NX_SECURE_TLS_RECORD_HEADER_SIZE                   (5)   /* Size of the TLS record header in bytes. */
#define NX_SECURE_TLS_HANDSHAKE_HEADER_SIZE                (4)   /* Size of the TLS handshake record header in bytes. */
#define NX_SECURE_TLS_FINISHED_HASH_SIZE                   (12)  /* Size of the TLS handshake Finished hash in bytes. If SSLv3 is added, the hash size will need to
                                                                    be revisited because it is different. */
#define NX_SECURE_TLS_MAX_CIPHER_BLOCK_SIZE                (128) /* Size of the largest block used by session ciphers (in block mode). */

/* The minimum size for the TLS message buffer is determined by a number of factors, but primarily
 * the expected size of the TLS handshake Certificate message (sent by the TLS server) that may
 * contain multiple certificates of 1-2KB each. The upper limit is determined by the length field
 * in the TLS header (16 bit), and is 64KB.
 */
#ifndef NX_SECURE_TLS_MINIMUM_MESSAGE_BUFFER_SIZE
#define NX_SECURE_TLS_MINIMUM_MESSAGE_BUFFER_SIZE          (4000)
#endif

/* Define a minimum reasonable size for a TLS X509 certificate. This is used in checking for
 * errors in allocating certificate space. The size is determined by assuming a 512-bit RSA
 * key, MD5 hash, and a rough estimate of other data. It is theoretically possible for a real
 * certificate to be smaller, but in that case, bypass the error checking by re-defining this
 * macro.
 *    Approximately: 64(RSA) + 16(MD5) + 176(ASN.1 + text data, common name, etc)
 */
#ifndef NX_SECURE_TLS_MINIMUM_CERTIFICATE_SIZE
#define NX_SECURE_TLS_MINIMUM_CERTIFICATE_SIZE               (256)
#endif

/* We store the key material in a single contiguous block in the TLS control block, using offsets to
 * get the actual key values. We need to size the key material according to the maximum amount of
 * key material needed by any of the supported ciphersuites, times 2 because there are separate keys for
 * client and server. */
#define NX_SECURE_TLS_KEY_MATERIAL_SIZE                    (2 * (NX_SECURE_TLS_MAX_HASH_SIZE + NX_SECURE_TLS_MAX_KEY_SIZE + NX_SECURE_TLS_MAX_IV_SIZE))

/* PSK-specific defines. If PSK is disabled, don't bring PSK types into the build. */
#if defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES) || defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE)


#ifndef NX_SECURE_TLS_MAX_PSK_SIZE
#define NX_SECURE_TLS_MAX_PSK_SIZE                         (20)
#endif /* NX_SECURE_TLS_MAX_PSK_SIZE */

#ifndef NX_SECURE_TLS_MAX_PSK_KEYS
#define NX_SECURE_TLS_MAX_PSK_KEYS                         (5)
#endif /* NX_SECURE_TLS_MAX_PSK_KEYS */

#ifndef NX_SECURE_TLS_MAX_PSK_ID_SIZE
#define NX_SECURE_TLS_MAX_PSK_ID_SIZE                      (20)
#endif /* NX_SECURE_TLS_MAX_PSK_ID_SIZE */

/* ECC-specific defines. */
#ifndef NX_SECURE_TLS_EC_MAX_REMOTE_GROUPS
#define NX_SECURE_TLS_EC_MAX_REMOTE_GROUPS                 (4)
#endif /* NX_SECURE_TLS_EC_MAX_REMOTE_GROUPS */

#ifndef NX_SECURE_TLS_EC_MAX_REMOTE_POINT_FORMATS
#define NX_SECURE_TLS_EC_MAX_REMOTE_POINT_FORMATS          (4)
#endif /* NX_SECURE_TLS_EC_MAX_REMOTE_POINT_FORMATS */

/* This structure holds the data for Pre-Shared Keys (PSKs) for use with
   the TLS PSK ciphersuites. The actual keys are generated from this data
   as part of the TLS handshake, but the user must provide this seed and
   an "identity" to match a remote host to a known key. */
typedef struct NX_SECURE_TLS_PSK_STORE_STRUCT
{
    /* This holds the actual key data for the PSK. */
    UCHAR nx_secure_tls_psk_data[NX_SECURE_TLS_MAX_PSK_SIZE];
    UINT  nx_secure_tls_psk_data_size;

    /* This holds the identity information for the key in this PSK entry. */
    UCHAR nx_secure_tls_psk_id[NX_SECURE_TLS_MAX_PSK_ID_SIZE];
    UINT  nx_secure_tls_psk_id_size;

    /* This holds the "identity hint" sent to a TLS server during the handshake.
       The hint tells the server how to choose a PSK/identity pair. */
    UCHAR nx_secure_tls_psk_id_hint[NX_SECURE_TLS_MAX_PSK_ID_SIZE];
    UINT  nx_secure_tls_psk_id_hint_size;
} NX_SECURE_TLS_PSK_STORE;
#endif /* defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES) || defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE) */


/* TLS Ciphersuite lookup table. Contains all pertinent information for ciphersuites used in TLS operations.
 * The lookup is based on the first field, which will contain the defined TLS value for the ciphersuite. */
typedef struct NX_SECURE_TLS_CIPHERSUITE_INFO_STRUCT
{
    /* The value of the ciphersuite "on the wire" as defined by the TLS spec. */
    USHORT nx_secure_tls_ciphersuite;

    /* The Public Key operation in this suite - RSA or DH. */
    NX_CRYPTO_METHOD *nx_secure_tls_public_cipher;

    /* The Public Authentication method used for signing data. */
    NX_CRYPTO_METHOD *nx_secure_tls_public_auth;

    /* NOTE: The Public Key size is determined by the public keys used and cannot be determined at compile time. */

    /* The session cipher being used - AES, RC4, etc. */
    NX_CRYPTO_METHOD *nx_secure_tls_session_cipher;

    /* The size of the initialization vectors needed for the session cipher. N/A for all session ciphers (enter "NONE"). */
    USHORT nx_secure_tls_iv_size;

    /* The key size for the session cipher. */
    UCHAR nx_secure_tls_session_key_size;

    /* The hash being used - MD5, SHA-1, SHA-256, etc. */
    NX_CRYPTO_METHOD *nx_secure_tls_hash;

    /* The size of the hash being used. This is for convenience as the size is determined
       by the hash, e.g. SHA-1 is 20 bytes, MD5 is 16 bytes. */
    USHORT nx_secure_tls_hash_size;

    /* The TLS PRF being used - for TLSv1.0 and TLSv1.1 this is a single function. For TLSv1.2,
       the PRF is determined by the ciphersuite. */
    NX_CRYPTO_METHOD *nx_secure_tls_prf;
} NX_SECURE_TLS_CIPHERSUITE_INFO;


typedef USHORT NX_SECURE_TLS_STATE;
typedef USHORT NX_SECURE_TLS_SERVER_STATE;
typedef USHORT NX_SECURE_TLS_CLIENT_STATE;

typedef struct NX_SECURE_TLS_KEY_MATERIAL_STRUCT
{
    /* Client random bytes - generated during the handshake. */
    UCHAR nx_secure_tls_client_random[NX_SECURE_TLS_RANDOM_SIZE];

    /* Server random bytes - generated during the handshake. */
    UCHAR nx_secure_tls_server_random[NX_SECURE_TLS_RANDOM_SIZE];

    /* The pre-master-secret length is dependent upon the public key
       algorithm chosen - the RSA pre-master-secret is 48 bytes.
       THIS MUST BE DELETED FROM MEMORY ONCE KEYS ARE GENERATED. */
    UCHAR nx_secure_tls_pre_master_secret[NX_SECURE_TLS_PREMASTER_SIZE];
    UINT  nx_secure_tls_pre_master_secret_size;

    /* The master secret is always 48 bytes in length, regardless of the
       length of the pre-master-secret. */
    UCHAR nx_secure_tls_master_secret[NX_SECURE_TLS_MASTER_SIZE];

    /* We store generate the session key material into this buffer,
       thus needing no copying of data (using the pointers to actual data below). */
    UCHAR nx_secure_tls_key_material_data[NX_SECURE_TLS_KEY_MATERIAL_SIZE];

    /* During a session renegotiation, there will be the current set of session keys
     * in use, and a new set of keys that will be generated during the renegotiation
     * handshake. However, there is a period of time where the local and remote keys
     * are out of sync (after a ChangeCipherSpec is sent/received but before the second
     * CCS message is sent) so we need to keep a separate buffer for new keys until
     * we are fully using the new keys.
     */
    UCHAR nx_secure_tls_new_key_material_data[NX_SECURE_TLS_KEY_MATERIAL_SIZE];

    /* The TLS protocol requires a "secret" used in the hash of each message,
       and one secret each for client and server. */
    UCHAR *nx_secure_tls_client_write_mac_secret;
    UCHAR *nx_secure_tls_server_write_mac_secret;

    /* The actual TLS Session keys used to encrypt session data (e.g. using AES.).
       There is one key for each direction, so the client encrypts with the "client_write"
       key and the server decrypts incoming data using the same key. */
    UCHAR *nx_secure_tls_client_write_key;
    UCHAR *nx_secure_tls_server_write_key;

    /* Some algorithms used in the TLS session require initialization vectors. */
    UCHAR *nx_secure_tls_client_iv;
    UCHAR *nx_secure_tls_server_iv;
} NX_SECURE_TLS_KEY_MATERIAL;

/* This structure contains the metadata for the TLS handshake hash - the state
 * of the hash must persist through the entire handshake process so it is stored
 * separately from the rest of the crypto metadata.
 */
typedef struct NX_SECURE_TLS_HANDSHAKE_HASH_STRUCT
{
    /* Handshake verification hash context - we need MD5 and SHA-1 for TLS 1.0 and 1.1. */
    CHAR *nx_secure_tls_handshake_hash_md5_metadata;
    ULONG nx_secure_tls_handshake_hash_md5_metadata_size;
    VOID *nx_secure_tls_handshake_hash_md5_handler;

    /* SHA-1 handshake hash context. */
    CHAR *nx_secure_tls_handshake_hash_sha1_metadata;
    ULONG nx_secure_tls_handshake_hash_sha1_metadata_size;
    VOID *nx_secure_tls_handshake_hash_sha1_handler;

    /* SHA-256 handshake hash context. */
    CHAR *nx_secure_tls_handshake_hash_sha256_metadata;
    ULONG nx_secure_tls_handshake_hash_sha256_metadata_size;
    VOID *nx_secure_tls_handshake_hash_sha256_handler;

    /* Scratch metadata space for copying one of the above states when
       generating the final hash. */
    CHAR *nx_secure_tls_handshake_hash_scratch;
    ULONG nx_secure_tls_handshake_hash_scratch_size;
} NX_SECURE_TLS_HANDSHAKE_HASH;


/* Top-level structure that contains all the relevant cryptographic method
   information for all TLS versions. */
typedef struct NX_SECURE_TLS_CRYPTO_STRUCT
{
    /* Table that maps ciphersuites to crypto methods. */
    NX_SECURE_TLS_CIPHERSUITE_INFO *nx_secure_tls_ciphersuite_lookup_table;
    USHORT                          nx_secure_tls_ciphersuite_lookup_table_size;

    /* Table that maps X.509 cipher identifiers to crypto methods. */
#ifndef NX_SECURE_DISABLE_X509
    NX_SECURE_X509_CRYPTO *nx_secure_tls_x509_cipher_table;
    USHORT                 nx_secure_tls_x509_cipher_table_size;
#endif

    /* Specific routines needed for specific TLS versions. */
#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
    NX_CRYPTO_METHOD *nx_secure_tls_handshake_hash_md5_method;
    NX_CRYPTO_METHOD *nx_secure_tls_handshake_hash_sha1_method;
    NX_CRYPTO_METHOD *nx_secure_tls_prf_1_method;
#endif

#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
    NX_CRYPTO_METHOD *nx_secure_tls_handshake_hash_sha256_method;
    NX_CRYPTO_METHOD *nx_secure_tls_prf_sha256_method;
#endif
} NX_SECURE_TLS_CRYPTO;


typedef struct NX_SECURE_TLS_CREDENTIALS_STRUCT
{
    /* X509 certificates are enabled by default. Disable them using this macro. */
#ifndef NX_SECURE_DISABLE_X509
    /* X509 Certificate store. */
    NX_SECURE_X509_CERTIFICATE_STORE nx_secure_tls_certificate_store;

    /* Pointer to the active local certificate, if non-NULL it overrides the store
       when sending out a certificate. */
    NX_SECURE_X509_CERT *nx_secure_tls_active_certificate;

#endif

#ifdef NX_SECURE_ENABLE_PSK_CIPHERSUITES

    /* Server identity value (received from remote host). */
    UCHAR nx_secure_tls_remote_psk_id[NX_SECURE_TLS_MAX_PSK_ID_SIZE];
    UINT  nx_secure_tls_remote_psk_id_size;

    /* Client PSK for use with a specific server. */
    NX_SECURE_TLS_PSK_STORE nx_secure_tls_client_psk;
#endif

#if defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES) || defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE)
    /* Store for PSK ciphersuite keys. Used for TLS servers and PSK. */
    NX_SECURE_TLS_PSK_STORE nx_secure_tls_psk_store[NX_SECURE_TLS_MAX_PSK_KEYS];

    /* Current count/index into PSK store. */
    UINT nx_secure_tls_psk_count;
#endif /* defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES) || defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE) */
} NX_SECURE_TLS_CREDENTIALS;

/* This structure encapsulates a single extension and its associated data. The
   structure is used to pass opaque data in and out of the TLS stack. Helper
   functions are used to extract/fill extension-specific data. */
typedef struct NX_SECURE_TLS_HELLO_EXTENSION_STRUCT
{

    /* Identifier for the extension - used to identify the data in the buffer. */
    USHORT nx_secure_tls_extension_id;

    /* Length of data in the buffer. */
    USHORT nx_secure_tls_extension_data_length;

    /* Data for the extensions. Pointer to a buffer containing the data which
       is formatted according to the particular extension. */
    const UCHAR *nx_secure_tls_extension_data;
} NX_SECURE_TLS_HELLO_EXTENSION;


/* Definition of the top-level TLS session control block used by the application. */
typedef struct NX_SECURE_TLS_SESSION_STRUCT
{
    /* Identifier to determine if TLS session has been properly initialized. */
    ULONG nx_secure_tls_id;

    /* Underlying TCP socket. */
    NX_TCP_SOCKET *nx_secure_tls_tcp_socket;

    /* Queue the incoming packets for one record. */
    NX_PACKET *nx_secure_record_queue_header;
    NX_PACKET *nx_secure_record_queue_tail;
    NX_PACKET *nx_secure_record_decrypted_packet;
    ULONG      nx_secure_record_queue_length;

    /* Packet pool used by TLS stack to allocate outgoing packets used in TLS handshake. */
    NX_PACKET_POOL *nx_secure_tls_packet_pool;

    /* Packet/message buffer for re-assembling TLS messages. */
    UCHAR *nx_secure_tls_packet_buffer;
    ULONG  nx_secure_tls_packet_buffer_size;
    ULONG  nx_secure_tls_packet_buffer_original_size;

    /* What type of socket is this? Client or server? */
    UINT nx_secure_tls_socket_type;

    /* Protocol version used for the current session. Actual version depends on
     * user preference and the remote host. */
    USHORT nx_secure_tls_protocol_version;

    /* This field overrides the version returned by _nx_secure_tls_newest_supported_version. */
    USHORT nx_secure_tls_protocol_version_override;

    /* State of local and remote encryption - post ChangeCipherSpec. */
    UCHAR nx_secure_tls_remote_session_active;
    UCHAR nx_secure_tls_local_session_active;

    /* Chosen ciphersuite. */
    const NX_SECURE_TLS_CIPHERSUITE_INFO *nx_secure_tls_session_ciphersuite;

    /* Chosen ciphersuite table, passed in during the session create call. */
    const NX_SECURE_TLS_CRYPTO *nx_secure_tls_crypto_table;

    /* Key material (master secret, session keys, etc.) is stored here. */
    NX_SECURE_TLS_KEY_MATERIAL nx_secure_tls_key_material;

    /* Session ID length. */
    UCHAR nx_secure_tls_session_id_length;

    /* Session ID used for session re-negotiation. */
    UCHAR nx_secure_tls_session_id[NX_SECURE_TLS_SESSION_ID_SIZE];

#ifdef NX_SECURE_TLS_ENABLE_SECURE_RENEGOTIATION
    /* This flag indicates whether the remote host supports secure renegotiation
       as indicated in the initial Hello messages (SCSV or the renegotiation
       extension were provided). */
    USHORT nx_secure_tls_secure_renegotiation;

    /* The verify data is named "remote" and "local" since it can be used by
       both TLS Client and TLS Server instances. */
    UCHAR nx_secure_tls_remote_verify_data[NX_SECURE_TLS_FINISHED_HASH_SIZE];
    UCHAR nx_secure_tls_local_verify_data[NX_SECURE_TLS_FINISHED_HASH_SIZE];
#endif


    /* Sequence number for the current TLS session - local host. */
    ULONG nx_secure_tls_local_sequence_number[NX_SECURE_TLS_SEQUENCE_NUMBER_SIZE];

    /* Sequence number for the current TLS session - remote host. For verification of incoming records. */
    ULONG nx_secure_tls_remote_sequence_number[NX_SECURE_TLS_SEQUENCE_NUMBER_SIZE];

    /* Pointer to TLS credentials for this session - stores PSKs, certificates, and other identification. */
    NX_SECURE_TLS_CREDENTIALS nx_secure_tls_credentials;

    /* Handshake hash (for the Finished message) must be maintained for all handshake messages. The
     * TLS version determines the actual hash being used, so all hash context data is encapsulated in
     * the handshake hash structure. */
    NX_SECURE_TLS_HANDSHAKE_HASH nx_secure_tls_handshake_hash;

    /* If our TLS server wishes to verify the client certificate, the application
       will set this to true (non-zero). */
    USHORT nx_secure_tls_verify_client_certificate;

    /* This flag will be set to true when TLS has received credentials (e.g. certificate, PSK)
       from the remote host. If it is still false when we get to the end of the handshake,
       we have not received credentials from the remote host and should fail the handshake. */
    USHORT nx_secure_tls_received_remote_credentials;

    /* If we receive a hello message from the remote server during a session,
       we have a re-negotiation handshake we need to process. */
    USHORT nx_secure_tls_renegotiation_handshake;

    /* Flag to enable/disable session renegotiation at application's choosing. */
    USHORT nx_secure_tls_renegotation_enabled;

#ifndef NX_SECURE_TLS_SERVER_DISABLED
    /* The state of the server handshake if this is a server socket. */
    NX_SECURE_TLS_SERVER_STATE nx_secure_tls_server_state;
#endif

#ifndef NX_SECURE_TLS_CLIENT_DISABLED
    /* The state of the client handshake if this is a client socket. */
    NX_SECURE_TLS_CLIENT_STATE nx_secure_tls_client_state;

    /* If the remote TLS Server requests a certificate, save that state here so we can send the cert. */
    USHORT nx_secure_tls_client_certificate_requested;
#endif

    /* Define the link between other TLS structures created by the application.  */
    struct NX_SECURE_TLS_SESSION_STRUCT
        *nx_secure_tls_created_previous,
        *nx_secure_tls_created_next;

    /* Define the public cipher metadata area. */
    VOID *nx_secure_public_cipher_metadata_area;

    /* Define the public cipher metadata size. */
    ULONG nx_secure_public_cipher_metadata_size;

    /* Define the public authentication handler. */
    VOID *nx_secure_public_auth_handler;

    /* Define the public authentication metadata area. */
    VOID *nx_secure_public_auth_metadata_area;

    /* Define the public authentication metadata size. */
    ULONG nx_secure_public_auth_metadata_size;

    /* Define the session cipher handler for client. */
    VOID *nx_secure_session_cipher_handler_client;

    /* Define the session cipher handler for server. */
    VOID *nx_secure_session_cipher_handler_server;

    /* Define the session cipher metadata area for client. */
    VOID *nx_secure_session_cipher_metadata_area_client;

    /* Define the crypto metadata area for server. */
    VOID *nx_secure_session_cipher_metadata_area_server;

    /* Define the crypto metadata size. */
    ULONG nx_secure_session_cipher_metadata_size;

    /* Define the hash Message Authentication Code (MAC) handler. */
    VOID *nx_secure_hash_mac_handler;

    /* Define the hash Message Authentication Code (MAC) metadata area. */
    VOID *nx_secure_hash_mac_metadata_area;

    /* Define the hash Message Authentication Code (MAC) metadata size. */
    ULONG nx_secure_hash_mac_metadata_size;

    /* Define the TLS PRF metadata area. */
    VOID *nx_secure_tls_prf_metadata_area;

    /* Define the TLS PRF metadata size. */
    ULONG nx_secure_tls_prf_metadata_size;

    /* Function (set by user) to call when TLS needs the current time. */
    ULONG (*nx_secure_tls_session_time_function)(void);

    /* Function (set by application) to call when TLS has a certificate from the
       remote host that has passed basic validation but requires additional checks
       by the application before being accepted. */
    ULONG (*nx_secure_tls_session_certificate_callback)(struct NX_SECURE_TLS_SESSION_STRUCT *session, NX_SECURE_X509_CERT *certificate);

    /* Function (set by user) to call when TLS receives a re-negotiation request from the remote host. */
    ULONG (*nx_secure_tls_session_renegotiation_callback)(struct NX_SECURE_TLS_SESSION_STRUCT *session);

    /* Function (set by user) to call when a TLS Client receives a ServerHello message containing extensions
       that require specific actions. */
    ULONG (*nx_secure_tls_session_client_callback)(struct NX_SECURE_TLS_SESSION_STRUCT *tls_session, NX_SECURE_TLS_HELLO_EXTENSION *extensions, UINT num_extensions);

    /* Function (set by user) to call when a TLS Server receives a ClientHello message containing extensions
       that require specific actions. */
    ULONG (*nx_secure_tls_session_server_callback)(struct NX_SECURE_TLS_SESSION_STRUCT *tls_session, NX_SECURE_TLS_HELLO_EXTENSION *extensions, UINT num_extensions);

#ifndef NX_SECURE_TLS_SNI_EXTENSION_DISABLED
    /* Server Name Indication (SNI) extension. For TLS Client, this is a single DNS name.
       For TLS Server, this is unused. */
    NX_SECURE_X509_DNS_NAME *nx_secure_tls_sni_extension_server_name;
#endif

    /* These are used to store off the alert value and level when an alert is recevied. */
    UINT nx_secure_tls_received_alert_level;
    UINT nx_secure_tls_received_alert_value;

    NX_SECURE_TLS_SESSION_EXTENSION
} NX_SECURE_TLS_SESSION;

/* TLS record types. */
#define NX_SECURE_TLS_CHANGE_CIPHER_SPEC   20
#define NX_SECURE_TLS_ALERT                21
#define NX_SECURE_TLS_HANDSHAKE            22
#define NX_SECURE_TLS_APPLICATION_DATA     23

/* TLS handshake message values. */
#define NX_SECURE_TLS_HELLO_REQUEST        0
#define NX_SECURE_TLS_CLIENT_HELLO         1
#define NX_SECURE_TLS_SERVER_HELLO         2
#define NX_SECURE_TLS_HELLO_VERIFY_REQUEST 3
#define NX_SECURE_TLS_CERTIFICATE_MSG      11
#define NX_SECURE_TLS_SERVER_KEY_EXCHANGE  12
#define NX_SECURE_TLS_CERTIFICATE_REQUEST  13
#define NX_SECURE_TLS_SERVER_HELLO_DONE    14
#define NX_SECURE_TLS_CERTIFICATE_VERIFY   15
#define NX_SECURE_TLS_CLIENT_KEY_EXCHANGE  16
#define NX_SECURE_TLS_FINISHED             20
#define NX_SECURE_TLS_CERTIFICATE_URL      21
#define NX_SECURE_TLS_CERTIFICATE_STATUS   22
#define NX_SECURE_TLS_INVALID_MESSAGE      100


/* Cryptographic hashing - P-HASH and the TLS PRF. */
typedef UINT (*_nx_secure_hmac_function_pointer)(UCHAR *input_ptr, UINT input_length,
                                                 UCHAR *key_ptr, UINT key_length,
                                                 UCHAR *digest_ptr);

/* These are passed to the P-HASH function to tell it which digest length is being used. */
#define NX_SECURE_PHASH_HMAC_MD5           16
#define NX_SECURE_PHASH_HMAC_SHA1          20
#define NX_SECURE_PHASH_HMAC_SHA256        32


/* Declare internal functions. */
UINT _nx_secure_tls_allocate_handshake_packet(NX_SECURE_TLS_SESSION *tls_session,
                                              NX_PACKET_POOL *packet_pool,
                                              NX_PACKET **send_packet, ULONG wait_option);
UINT _nx_secure_tls_check_protocol_version(NX_SECURE_TLS_SESSION *tls_session,
                                           USHORT protocol_version, UINT id);
UINT _nx_secure_tls_ciphersuite_lookup(NX_SECURE_TLS_SESSION *tls_session, UINT ciphersuite,
                                       NX_SECURE_TLS_CIPHERSUITE_INFO const **info);
UINT _nx_secure_tls_client_handshake(NX_SECURE_TLS_SESSION *tls_session, UCHAR *packet_buffer,
                                     UINT data_length, ULONG wait_option);
UINT _nx_secure_tls_finished_hash_generate(NX_SECURE_TLS_SESSION *tls_session,
                                           UCHAR *finished_label, UCHAR *finished_hash);
UINT _nx_secure_tls_generate_keys(NX_SECURE_TLS_SESSION *tls_session);
UINT _nx_secure_tls_generate_premaster_secret(NX_SECURE_TLS_SESSION *tls_session);
UINT _nx_secure_tls_handshake_hash_init(NX_SECURE_TLS_SESSION *tls_session);
UINT _nx_secure_tls_handshake_hash_update(NX_SECURE_TLS_SESSION *tls_session, UCHAR *data,
                                          UINT length);
UINT _nx_secure_tls_handshake_process(NX_SECURE_TLS_SESSION *tls_session, UINT wait_option);
UINT _nx_secure_tls_hash_record(NX_SECURE_TLS_SESSION * tls_session,
                                ULONG sequence_num[NX_SECURE_TLS_SEQUENCE_NUMBER_SIZE],
                                UCHAR * header, UINT header_length, UCHAR * data, UINT length,
                                UCHAR * record_hash, UINT * hash_length, UCHAR * mac_secret);
UINT _nx_secure_tls_key_material_init(NX_SECURE_TLS_KEY_MATERIAL *key_material);
VOID _nx_secure_tls_map_error_to_alert(UINT error_number, UINT *alert_number,
                                       UINT *alert_level);
VOID _nx_secure_tls_newest_supported_version(NX_SECURE_TLS_SESSION *session_ptr,
                                             USHORT *protocol_version, UINT id);
VOID _nx_secure_tls_protocol_version_get(NX_SECURE_TLS_SESSION *session_ptr,
                                         USHORT *protocol_version, UINT id);
UINT _nx_secure_tls_process_certificate_request(NX_SECURE_TLS_SESSION *tls_session,
                                                UCHAR *packet_buffer, UINT message_length);
UINT _nx_secure_tls_process_certificate_verify(NX_SECURE_TLS_SESSION *tls_session,
                                               UCHAR *packet_buffer, UINT message_length);
UINT _nx_secure_tls_process_changecipherspec(NX_SECURE_TLS_SESSION *tls_session,
                                             UCHAR *packet_buffer, UINT message_length);
UINT _nx_secure_tls_process_client_key_exchange(NX_SECURE_TLS_SESSION *tls_session,
                                                UCHAR *packet_buffer, UINT message_length);
UINT _nx_secure_tls_process_clienthello(NX_SECURE_TLS_SESSION *tls_session,
                                        UCHAR *packet_buffer, UINT message_length);
UINT _nx_secure_tls_process_clienthello_extensions(NX_SECURE_TLS_SESSION *tls_session,
                                                   UCHAR *packet_buffer, UINT message_length,
                                                   NX_SECURE_TLS_HELLO_EXTENSION *extensions,
                                                   UINT *num_extensions);

UINT _nx_secure_tls_process_finished(NX_SECURE_TLS_SESSION *tls_session, UCHAR *packet_buffer,
                                     UINT message_length);
UINT _nx_secure_tls_process_header(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET *packet_ptr,
                                   ULONG record_offset, USHORT *message_type, UINT *length,
                                   UCHAR *header_data, USHORT *header_length);
UINT _nx_secure_tls_process_handshake_header(UCHAR *packet_buffer, USHORT *message_type,
                                             USHORT *header_size, UINT *message_length);
UINT _nx_secure_tls_process_record(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET *packet_ptr,
                                   ULONG *bytes_processed, ULONG wait_option);
UINT _nx_secure_tls_process_remote_certificate(NX_SECURE_TLS_SESSION *tls_session,
                                               UCHAR *packet_buffer,
                                               UINT message_length);
UINT _nx_secure_tls_process_server_key_exchange(NX_SECURE_TLS_SESSION *tls_session,
                                                UCHAR *packet_buffer, UINT message_length);
UINT _nx_secure_tls_process_serverhello(NX_SECURE_TLS_SESSION *tls_session, UCHAR *packet_buffer,
                                        UINT message_length);
UINT _nx_secure_tls_process_serverhello_extensions(NX_SECURE_TLS_SESSION *tls_session,
                                                   UCHAR *packet_buffer, UINT message_length,
                                                   NX_SECURE_TLS_HELLO_EXTENSION *extensions,
                                                   UINT *num_extensions);
UINT _nx_secure_tls_record_hash_calculate(NX_SECURE_TLS_SESSION *tls_session, UCHAR *record_hash,
                                          UINT *hash_length);
UINT _nx_secure_tls_record_hash_initialize(NX_SECURE_TLS_SESSION *tls_session,
                                           ULONG sequence_num[NX_SECURE_TLS_SEQUENCE_NUMBER_SIZE],
                                           UCHAR *header, UINT header_length, UINT *hash_length,
                                           UCHAR *mac_secret);
UINT _nx_secure_tls_record_hash_update(NX_SECURE_TLS_SESSION *tls_session, UCHAR *data,
                                       UINT length);
UINT _nx_secure_tls_record_payload_decrypt(NX_SECURE_TLS_SESSION *tls_session, UCHAR *data,
                                           UINT *length,
                                           ULONG sequence_num[NX_SECURE_TLS_SEQUENCE_NUMBER_SIZE],
                                           UCHAR record_type);
UINT _nx_secure_tls_record_payload_encrypt(NX_SECURE_TLS_SESSION *tls_session,
                                           NX_PACKET *send_packet,
                                           ULONG sequence_num[NX_SECURE_TLS_SEQUENCE_NUMBER_SIZE],
                                           UCHAR record_type);
UINT _nx_secure_tls_remote_certificate_free(NX_SECURE_TLS_SESSION *tls_session,
                                            NX_SECURE_X509_DISTINGUISHED_NAME *name);
UINT _nx_secure_tls_remote_certificate_verify(NX_SECURE_TLS_SESSION *tls_session);
VOID _nx_secure_tls_send_alert(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET *send_packet,
                               UCHAR alert_number, UCHAR alert_level);
UINT _nx_secure_tls_send_certificate(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET *send_packet,
                                     ULONG wait_option);
UINT _nx_secure_tls_send_certificate_request(NX_SECURE_TLS_SESSION *tls_session,
                                             NX_PACKET *send_packet);
UINT _nx_secure_tls_send_changecipherspec(NX_SECURE_TLS_SESSION *tls_session,
                                          NX_PACKET *send_packet);
UINT _nx_secure_tls_send_clienthello(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET *send_packet);
UINT _nx_secure_tls_send_clienthello_extensions(NX_SECURE_TLS_SESSION *tls_session,
                                                UCHAR *packet_buffer, ULONG *packet_offset,
                                                ULONG available_size);
UINT _nx_secure_tls_send_client_key_exchange(NX_SECURE_TLS_SESSION *tls_session,
                                             NX_PACKET *send_packet);
UINT _nx_secure_tls_send_finished(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET *send_packet);
UINT _nx_secure_tls_send_handshake_record(NX_SECURE_TLS_SESSION *tls_session,
                                          NX_PACKET *send_packet, UCHAR handshake_type,
                                          ULONG wait_option);
UINT _nx_secure_tls_send_hellorequest(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET *send_packet);
UINT _nx_secure_tls_send_certificate_verify(NX_SECURE_TLS_SESSION *tls_session,
                                            NX_PACKET *send_packet);
UINT _nx_secure_tls_send_record(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET *send_packet,
                                UCHAR record_type, ULONG wait_option);
UINT _nx_secure_tls_send_server_key_exchange(NX_SECURE_TLS_SESSION *tls_session,
                                             NX_PACKET *send_packet);
UINT _nx_secure_tls_send_serverhello(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET *send_packet);
UINT _nx_secure_tls_send_serverhello_extensions(NX_SECURE_TLS_SESSION *tls_session,
                                                UCHAR *packet_buffer, ULONG *packet_offset,
                                                ULONG available_size);
UINT _nx_secure_tls_server_certificate_add(NX_SECURE_TLS_SESSION *tls_session,
                                           NX_SECURE_X509_CERT *certificate, UINT cert_id);
UINT _nx_secure_tls_server_certificate_find(NX_SECURE_TLS_SESSION *tls_session,
                                            NX_SECURE_X509_CERT **certificate, UINT cert_id);
UINT _nx_secure_tls_server_certificate_remove(NX_SECURE_TLS_SESSION *tls_session, UINT cert_id);
UINT _nx_secure_tls_server_handshake(NX_SECURE_TLS_SESSION *tls_session, UCHAR *packet_buffer,
                                     ULONG wait_option);
UINT _nx_secure_tls_session_iv_size_get(NX_SECURE_TLS_SESSION *tls_session, USHORT *iv_size);
UINT _nx_secure_tls_session_keys_set(NX_SECURE_TLS_SESSION *tls_session, USHORT key_set);
UINT _nx_secure_tls_session_receive_records(NX_SECURE_TLS_SESSION *tls_session,
                                            NX_PACKET **packet_ptr_ptr, ULONG wait_option);
UINT _nx_secure_tls_verify_mac(NX_SECURE_TLS_SESSION *tls_session, UCHAR *header_data,
                               USHORT header_length, UCHAR *data, UINT *length);


/* Actual API functions .*/
UINT nx_secure_module_hash_compute(NX_CRYPTO_METHOD *hmac_ptr,
                                   UINT start_address,
                                   UINT end_address,
                                   UCHAR *key, UINT key_length,
                                   VOID *metadata, UINT metadata_size,
                                   UCHAR *output_buffer, UINT output_buffer_size, UINT *actual_size);
UINT _nx_secure_tls_active_certificate_set(NX_SECURE_TLS_SESSION *tls_session,
                                           NX_SECURE_X509_CERT *certificate);
VOID _nx_secure_tls_initialize(VOID);

UINT _nx_secure_tls_local_certificate_add(NX_SECURE_TLS_SESSION *tls_session,
                                          NX_SECURE_X509_CERT *certificate);
UINT _nx_secure_tls_local_certificate_find(NX_SECURE_TLS_SESSION *tls_session,
                                           NX_SECURE_X509_CERT **certificate, UCHAR *common_name, UINT name_length);
UINT _nx_secure_tls_local_certificate_remove(NX_SECURE_TLS_SESSION *tls_session,
                                             UCHAR *common_name, UINT common_name_length);
UINT _nx_secure_tls_metadata_size_calculate(const NX_SECURE_TLS_CRYPTO *crypto_table,
                                            ULONG *metadata_size);
UINT _nx_secure_tls_remote_certificate_allocate(NX_SECURE_TLS_SESSION *tls_session,
                                                NX_SECURE_X509_CERT *certificate,
                                                UCHAR *raw_certificate_buffer, UINT buffer_size);
UINT _nx_secure_tls_remote_certificate_buffer_allocate(NX_SECURE_TLS_SESSION *tls_session,
                                                    UINT certs_number, VOID *certificate_buffer, ULONG buffer_size);
UINT _nx_secure_tls_remote_certificate_free_all(NX_SECURE_TLS_SESSION *tls_session);
UINT _nx_secure_tls_server_certificate_add(NX_SECURE_TLS_SESSION *tls_session,
                                           NX_SECURE_X509_CERT *certificate, UINT cert_id);
UINT _nx_secure_tls_server_certificate_find(NX_SECURE_TLS_SESSION *tls_session,
                                            NX_SECURE_X509_CERT **certificate, UINT cert_id);
UINT _nx_secure_tls_server_certificate_remove(NX_SECURE_TLS_SESSION *tls_session, UINT cert_id);
UINT _nx_secure_tls_session_alert_value_get(NX_SECURE_TLS_SESSION *tls_session,
                                            UINT *alert_level, UINT *alert_value);
UINT _nx_secure_tls_session_certificate_callback_set(NX_SECURE_TLS_SESSION *tls_session,
                                                     ULONG (*func_ptr)(NX_SECURE_TLS_SESSION *session,
                                                                       NX_SECURE_X509_CERT *certificate));
UINT _nx_secure_tls_session_client_callback_set(NX_SECURE_TLS_SESSION *tls_session,
                                                ULONG (*func_ptr)(NX_SECURE_TLS_SESSION *tls_session,
                                                                  NX_SECURE_TLS_HELLO_EXTENSION *extensions,
                                                                  UINT num_extensions));
UINT _nx_secure_tls_session_client_verify_disable(NX_SECURE_TLS_SESSION *tls_session);
UINT _nx_secure_tls_session_client_verify_enable(NX_SECURE_TLS_SESSION *tls_session);
UINT _nx_secure_tls_session_x509_client_verify_configure(NX_SECURE_TLS_SESSION *tls_session, UINT certs_number,
                                                           VOID *certificate_buffer, ULONG buffer_size);
UINT _nx_secure_tls_session_create(NX_SECURE_TLS_SESSION *session_ptr,
                                   const NX_SECURE_TLS_CRYPTO *cipher_table,
                                   VOID *metadata_area,
                                   ULONG metadata_size);
UINT _nx_secure_tls_session_delete(NX_SECURE_TLS_SESSION *tls_session);
UINT _nx_secure_tls_session_end(NX_SECURE_TLS_SESSION *tls_session, UINT wait_option);
UINT _nx_secure_tls_session_packet_buffer_set(NX_SECURE_TLS_SESSION *session_ptr,
                                              UCHAR *buffer_ptr, ULONG buffer_size);
UINT _nx_secure_tls_session_protocol_version_override(NX_SECURE_TLS_SESSION *tls_session,
                                                      USHORT protocol_version);
UINT _nx_secure_tls_session_receive(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET **packet_ptr_ptr,
                                    ULONG wait_option);
UINT _nx_secure_tls_session_renegotiate(NX_SECURE_TLS_SESSION *tls_session,
                                        UINT wait_option);
UINT _nx_secure_tls_session_renegotiate_callback_set(NX_SECURE_TLS_SESSION *tls_session,
                                                     ULONG (*func_ptr)(NX_SECURE_TLS_SESSION *session));
UINT _nx_secure_tls_session_reset(NX_SECURE_TLS_SESSION *tls_session);
UINT _nx_secure_tls_session_send(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET *packet_ptr,
                                 ULONG wait_option);
UINT _nx_secure_tls_session_server_callback_set(NX_SECURE_TLS_SESSION *tls_session,
                                                ULONG (*func_ptr)(NX_SECURE_TLS_SESSION *tls_session,
                                                                  NX_SECURE_TLS_HELLO_EXTENSION *extensions,
                                                                  UINT num_extensions));
UINT _nx_secure_tls_session_sni_extension_parse(NX_SECURE_TLS_SESSION *tls_session,
                                                NX_SECURE_TLS_HELLO_EXTENSION *extensions,
                                                UINT num_extensions, NX_SECURE_X509_DNS_NAME *dns_name);
UINT _nx_secure_tls_session_sni_extension_set(NX_SECURE_TLS_SESSION *tls_session,
                                              NX_SECURE_X509_DNS_NAME *dns_name);
UINT _nx_secure_tls_session_start(NX_SECURE_TLS_SESSION *tls_session, NX_TCP_SOCKET *tcp_socket,
                                  UINT wait_option);
UINT _nx_secure_tls_session_time_function_set(NX_SECURE_TLS_SESSION *tls_session,
                                              ULONG (*time_func_ptr)(void));
UINT _nx_secure_tls_trusted_certificate_add(NX_SECURE_TLS_SESSION *tls_session,
                                            NX_SECURE_X509_CERT *certificate);
UINT _nx_secure_tls_trusted_certificate_remove(NX_SECURE_TLS_SESSION *tls_session,
                                               UCHAR *common_name, UINT common_name_length);
UINT _nx_secure_tls_packet_allocate(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET_POOL *pool_ptr,
                                    NX_PACKET **packet_ptr, ULONG wait_option);
#if defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES) || defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE)
UINT _nx_secure_tls_psk_add(NX_SECURE_TLS_SESSION *tls_session, UCHAR *pre_shared_key, UINT psk_length,
                            UCHAR *psk_identity, UINT identity_length, UCHAR *hint, UINT hint_length);
UINT _nx_secure_tls_psk_find(NX_SECURE_TLS_SESSION *tls_session, UCHAR **psk_data, UINT *psk_length,
                             UCHAR *psk_identity, UINT identity_length);
UINT _nx_secure_tls_client_psk_set(NX_SECURE_TLS_SESSION *tls_session, UCHAR *pre_shared_key, UINT psk_length,
                                   UCHAR *psk_identity, UINT identity_length, UCHAR *hint, UINT hint_length);
#endif

/* Functions for error checking .*/
UINT _nxe_secure_tls_active_certificate_set(NX_SECURE_TLS_SESSION *tls_session,
                                            NX_SECURE_X509_CERT *certificate);
UINT _nxe_secure_tls_local_certificate_add(NX_SECURE_TLS_SESSION *tls_session,
                                           NX_SECURE_X509_CERT *certificate);
UINT _nxe_secure_tls_local_certificate_find(NX_SECURE_TLS_SESSION *tls_session,
                                            NX_SECURE_X509_CERT **certificate, UCHAR *common_name,
                                            UINT name_length);
UINT _nxe_secure_tls_local_certificate_remove(NX_SECURE_TLS_SESSION *tls_session,
                                              UCHAR *common_name, UINT common_name_length);
UINT _nxe_secure_tls_metadata_size_calculate(const NX_SECURE_TLS_CRYPTO *crypto_table,
                                             ULONG *metadata_size);
UINT _nxe_secure_tls_remote_certificate_allocate(NX_SECURE_TLS_SESSION *tls_session,
                                                 NX_SECURE_X509_CERT *certificate,
                                                 UCHAR *raw_certificate_buffer, UINT buffer_size);
UINT _nxe_secure_tls_remote_certificate_buffer_allocate(NX_SECURE_TLS_SESSION *tls_session,
                                                    UINT certs_number, VOID *certificate_buffer, ULONG buffer_size);
UINT _nxe_secure_tls_remote_certificate_free_all(NX_SECURE_TLS_SESSION *tls_session);
UINT _nxe_secure_tls_server_certificate_add(NX_SECURE_TLS_SESSION *tls_session,
                                            NX_SECURE_X509_CERT *certificate, UINT cert_id);
UINT _nxe_secure_tls_server_certificate_find(NX_SECURE_TLS_SESSION *tls_session,
                                             NX_SECURE_X509_CERT **certificate, UINT cert_id);
UINT _nxe_secure_tls_server_certificate_remove(NX_SECURE_TLS_SESSION *tls_session, UINT cert_id);
UINT  _nxe_secure_tls_session_alert_value_get(NX_SECURE_TLS_SESSION *tls_session,
                                                        UINT *alert_level, UINT *alert_value);
UINT _nxe_secure_tls_session_certificate_callback_set(NX_SECURE_TLS_SESSION *tls_session,
                                                      ULONG (*func_ptr)(NX_SECURE_TLS_SESSION *session,
                                                                        NX_SECURE_X509_CERT *certificate));
UINT _nxe_secure_tls_session_client_callback_set(NX_SECURE_TLS_SESSION *tls_session,
                                                 ULONG (*func_ptr)(NX_SECURE_TLS_SESSION *tls_session,
                                                                   NX_SECURE_TLS_HELLO_EXTENSION *extensions,
                                                                   UINT num_extensions));
UINT _nxe_secure_tls_session_client_verify_disable(NX_SECURE_TLS_SESSION *tls_session);
UINT _nxe_secure_tls_session_client_verify_enable(NX_SECURE_TLS_SESSION *tls_session);
UINT _nxe_secure_tls_session_x509_client_verify_configure(NX_SECURE_TLS_SESSION *tls_session, UINT certs_number,
                                                           VOID *certificate_buffer, ULONG buffer_size);
UINT _nxe_secure_tls_session_create(NX_SECURE_TLS_SESSION *session_ptr,
                                    const NX_SECURE_TLS_CRYPTO *cipher_table,
                                    VOID *metadata_area,
                                    ULONG metadata_size);
UINT _nxe_secure_tls_session_delete(NX_SECURE_TLS_SESSION *tls_session);
UINT _nxe_secure_tls_session_end(NX_SECURE_TLS_SESSION *tls_session, UINT wait_option);
UINT _nxe_secure_tls_session_packet_buffer_set(NX_SECURE_TLS_SESSION *session_ptr,
                                               UCHAR *buffer_ptr, ULONG buffer_size);
UINT _nxe_secure_tls_session_protocol_version_override(NX_SECURE_TLS_SESSION *tls_session,
                                                       USHORT protocol_version);
UINT _nxe_secure_tls_session_receive(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET **packet_ptr_ptr,
                                     ULONG wait_option);
UINT _nxe_secure_tls_session_renegotiate(NX_SECURE_TLS_SESSION *tls_session,
                                         UINT wait_option);
UINT _nxe_secure_tls_session_renegotiate_callback_set(NX_SECURE_TLS_SESSION *tls_session,
                                                      ULONG (*func_ptr)(NX_SECURE_TLS_SESSION *session));
UINT _nxe_secure_tls_session_reset(NX_SECURE_TLS_SESSION *tls_session);
UINT _nxe_secure_tls_session_send(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET *packet_ptr,
                                  ULONG wait_option);
UINT _nxe_secure_tls_session_server_callback_set(NX_SECURE_TLS_SESSION *tls_session,
                                                 ULONG (*func_ptr)(NX_SECURE_TLS_SESSION *tls_session,
                                                                   NX_SECURE_TLS_HELLO_EXTENSION *extensions,
                                                                   UINT num_extensions));
UINT _nxe_secure_tls_session_sni_extension_parse(NX_SECURE_TLS_SESSION *tls_session,
                                                 NX_SECURE_TLS_HELLO_EXTENSION *extensions,
                                                 UINT num_extensions, NX_SECURE_X509_DNS_NAME *dns_name);
UINT _nxe_secure_tls_session_sni_extension_set(NX_SECURE_TLS_SESSION *tls_session,
                                               NX_SECURE_X509_DNS_NAME *dns_name);
UINT _nxe_secure_tls_session_start(NX_SECURE_TLS_SESSION *tls_session, NX_TCP_SOCKET *tcp_socket,
                                   UINT wait_option);
UINT _nxe_secure_tls_session_time_function_set(NX_SECURE_TLS_SESSION *tls_session,
                                               ULONG (*time_func_ptr)(void));
UINT _nxe_secure_tls_trusted_certificate_add(NX_SECURE_TLS_SESSION *tls_session,
                                             NX_SECURE_X509_CERT *certificate);
UINT _nxe_secure_tls_trusted_certificate_remove(NX_SECURE_TLS_SESSION *tls_session,
                                                UCHAR *common_name, UINT common_name_length);
UINT _nxe_secure_tls_packet_allocate(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET_POOL *pool_ptr,
                                     NX_PACKET **packet_ptr, ULONG wait_option);
#if defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES) || defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE)
UINT _nxe_secure_tls_psk_add(NX_SECURE_TLS_SESSION *tls_session, UCHAR *pre_shared_key, UINT psk_length,
                             UCHAR *psk_identity, UINT identity_length, UCHAR *hint, UINT hint_length);
UINT _nxe_secure_tls_psk_find(NX_SECURE_TLS_SESSION *tls_session, UCHAR **psk_data, UINT *psk_length,
                              UCHAR *psk_identity, UINT identity_length);
UINT _nxe_secure_tls_client_psk_set(NX_SECURE_TLS_SESSION *tls_session, UCHAR *pre_shared_key, UINT psk_length,
                                    UCHAR *psk_identity, UINT identity_length, UCHAR *hint, UINT hint_length);
#endif

/* TLS component data declarations follow.  */

/* Determine if the initialization function of this component is including
   this file.  If so, make the data definitions really happen.  Otherwise,
   make them extern so other functions in the component can access them.  */

#ifdef NX_SECURE_TLS_INIT
#define TLS_DECLARE
#else
#define TLS_DECLARE extern
#endif


/* Define the head pointer of the created TLS list.  */
TLS_DECLARE  NX_SECURE_TLS_SESSION *_nx_secure_tls_created_ptr;
TLS_DECLARE  ULONG    _nx_secure_tls_created_count;
TLS_DECLARE  TX_MUTEX _nx_secure_tls_protection;

#ifdef __cplusplus
}
#endif

#endif /* SRC_NX_SECURE_TLS_H_ */

