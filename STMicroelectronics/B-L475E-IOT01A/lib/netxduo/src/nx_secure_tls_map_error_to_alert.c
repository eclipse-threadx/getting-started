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

#define NX_SECURE_SOURCE_CODE

#include "nx_secure_tls.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_map_error_to_alert                   PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function maps an internal error status to the appropriate TLS  */
/*    alert number to be sent to the remote host.                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*   error_number                           The error we are mapping      */
/*   alert_number                           Return the alert number       */
/*   alert_level                            Return the alert level        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_client_handshake      DTLS client state machine     */
/*    _nx_secure_dtls_server_handshake      DTLS server state machine     */
/*    _nx_secure_dtls_session_receive       Receive DTLS data             */
/*    _nx_secure_tls_client_handshake       TLS client state machine      */
/*    _nx_secure_tls_server_handshake       TLS server state machine      */
/*    _nx_secure_tls_session_receive_records                              */
/*                                          Receive TLS records           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  08-15-2019     Timothy Stapko           Modified comment(s), updated  */
/*                                            mapping for certificate     */
/*                                            verification failure,       */
/*                                            supported TLS Fallback SCSV,*/
/*                                            and error return codes,     */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
VOID _nx_secure_tls_map_error_to_alert(UINT error_number, UINT *alert_number, UINT *alert_level)
{
/* The following alerts are not currently sent by NetX Secure:
    NX_SECURE_TLS_ALERT_EXPORT_RESTRICTION_RESERVED  // MUST NOT be sent per RFC
    NX_SECURE_TLS_ALERT_INSUFFICIENT_SECURITY        // May be used if unsupported ciphersuites aren't strong enough (more specific than unsupported ciphers)
    NX_SECURE_TLS_ALERT_USER_CANCELED                // Only used if the application chooses to abort the connection during the handshake
    NX_SECURE_TLS_ALERT_ACCESS_DENIED                // Only used in systems with access control
    NX_SECURE_TLS_ALERT_DECRYPTION_FAILED_RESERVED   // MUST NOT be sent per RFC
    NX_SECURE_TLS_ALERT_RECORD_OVERFLOW              // This is handled by NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH, which is a decode alert
    NX_SECURE_TLS_ALERT_DECOMPRESSION_FAILURE        // No compression methods are used currently
    NX_SECURE_TLS_ALERT_NO_CERTIFICATE_RESERVED      // MUST NOT be sent per RFC
    NX_SECURE_TLS_ALERT_UNSUPPORTED_EXTENSION        // We ignore extensions currently
 */

    switch (error_number)
    {
    /* Unexpected message alerts. */
    case NX_SECURE_TLS_UNRECOGNIZED_MESSAGE_TYPE:
    case NX_SECURE_TLS_ALERT_RECEIVED:
    case NX_SECURE_TLS_BAD_CIPHERSPEC:               
    case NX_SECURE_TLS_UNEXPECTED_MESSAGE:           /* Deliberate fall-through. */
        *alert_number = NX_SECURE_TLS_ALERT_UNEXPECTED_MESSAGE;
        *alert_level = NX_SECURE_TLS_ALERT_LEVEL_FATAL;
        break;

    /* Hash or decryption failures. */
    case NX_SECURE_TLS_HASH_MAC_VERIFY_FAILURE:
    case NX_SECURE_TLS_AEAD_DECRYPT_FAIL:
    case NX_SECURE_TLS_PADDING_CHECK_FAILED:        /* Deliberate fall-through. */
        *alert_number = NX_SECURE_TLS_ALERT_BAD_RECORD_MAC;
        *alert_level = NX_SECURE_TLS_ALERT_LEVEL_FATAL;
        break;

    /* General handshake failures. */
    case NX_SECURE_TLS_UNKNOWN_CIPHERSUITE:
    case NX_SECURE_TLS_UNSUPPORTED_CIPHER:
    case NX_SECURE_TLS_HANDSHAKE_FAILURE:
    case NX_SECURE_TLS_NO_SUPPORTED_CIPHERS:   
    case NX_SECURE_TLS_UNSUPPORTED_FEATURE:    
    case NX_SECURE_TLS_UNSUPPORTED_ECC_CURVE:
    case NX_SECURE_TLS_UNSUPPORTED_ECC_FORMAT:
    case NX_SECURE_TLS_EXTENSION_NOT_FOUND:
    case NX_SECURE_TLS_SNI_EXTENSION_INVALID:
    case NX_SECURE_TLS_EMPTY_EC_GROUP:
    case NX_SECURE_TLS_EMPTY_EC_POINT_FORMAT:
    case NX_SECURE_TLS_UNSUPPORTED_SIGNATURE_ALGORITHM: /* Deliberate fall-through. */
        *alert_number = NX_SECURE_TLS_ALERT_HANDSHAKE_FAILURE;
        *alert_level = NX_SECURE_TLS_ALERT_LEVEL_FATAL;
        break;

    /* Invalid certificate issues. */
    case NX_SECURE_TLS_INVALID_SERVER_CERT:
    case NX_SECURE_TLS_INVALID_CERTIFICATE:
    case NX_SECURE_TLS_CERTIFICATE_SIG_CHECK_FAILED:
    case NX_SECURE_TLS_CERTIFICATE_NOT_FOUND:
    case NX_SECURE_TLS_CERTIFICATE_VERIFY_FAILURE:
    case NX_SECURE_TLS_EMPTY_REMOTE_CERTIFICATE_RECEIVED:
    case NX_SECURE_X509_WRONG_SIGNATURE_METHOD:
    case NX_SECURE_X509_INVALID_DATE_FORMAT:
    case NX_SECURE_X509_ASN1_LENGTH_TOO_LONG:
    case NX_SECURE_X509_PKCS7_PARSING_FAILED:         /* Deliberate fall-through. */
        *alert_number = NX_SECURE_TLS_ALERT_BAD_CERTIFICATE;
        *alert_level = NX_SECURE_TLS_ALERT_LEVEL_FATAL;
        break;

    /* Unsupported certificate issues (unsupported ciphers and signature types). */
    case NX_SECURE_TLS_UNSUPPORTED_PUBLIC_CIPHER:
    case NX_SECURE_TLS_UNSUPPORTED_CERT_SIGN_TYPE:
    case NX_SECURE_TLS_UNSUPPORTED_CERT_SIGN_ALG:     /* Deliberate fall-through. */
        *alert_number = NX_SECURE_TLS_ALERT_UNSUPPORTED_CERTIFICATE;
        *alert_level = NX_SECURE_TLS_ALERT_LEVEL_FATAL;
        break;

    /* A certificate was revoked by its signer. */
    case NX_SECURE_X509_CRL_CERTIFICATE_REVOKED:
        *alert_number = NX_SECURE_TLS_ALERT_CERTIFICATE_REVOKED;
        *alert_level = NX_SECURE_TLS_ALERT_LEVEL_FATAL;
        break;

    /* A certificate has expired or is not yet valid. */
    case NX_SECURE_X509_CERTIFICATE_EXPIRED:
    case NX_SECURE_X509_CERTIFICATE_NOT_YET_VALID:    /* Deliberate fall-through. */
        *alert_number = NX_SECURE_TLS_ALERT_CERTIFICATE_EXPIRED;
        *alert_level = NX_SECURE_TLS_ALERT_LEVEL_FATAL;
        break;

    /* Unknown certificate issues - the certificate was unsupported but for some odd reason (or it was self-signed). */
    case NX_SECURE_TLS_INVALID_SELF_SIGNED_CERT:
    case NX_SECURE_TLS_UNKNOWN_CERT_SIG_ALGORITHM:
        *alert_number = NX_SECURE_TLS_ALERT_CERTIFICATE_UNKNOWN;
        *alert_level = NX_SECURE_TLS_ALERT_LEVEL_FATAL;
        break;

    /*  Illegal parameters - bad compression method, etc. */
    case NX_SECURE_TLS_BAD_COMPRESSION_METHOD:        /* Deliberate fall-through. */
        *alert_number = NX_SECURE_TLS_ALERT_ILLEGAL_PARAMETER;
        *alert_level = NX_SECURE_TLS_ALERT_LEVEL_FATAL;
        break;

    /* The issuer for a received certificate was not found in our local store. */
    case NX_SECURE_TLS_ISSUER_CERTIFICATE_NOT_FOUND:
    case NX_SECURE_X509_CHAIN_VERIFY_FAILURE:
        *alert_number = NX_SECURE_TLS_ALERT_UNKNOWN_CA;
        *alert_level = NX_SECURE_TLS_ALERT_LEVEL_FATAL;
        break;

    /* Some type of decoding error happened with a received message. */
    case NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH:
        *alert_number = NX_SECURE_TLS_ALERT_DECODE_ERROR;
        *alert_level = NX_SECURE_TLS_ALERT_LEVEL_FATAL;
        break;

    /* Decryption error in processing a message. */
    case NX_SECURE_TLS_FINISHED_HASH_FAILURE:
    case NX_SECURE_TLS_SIGNATURE_VERIFICATION_ERROR:
        *alert_number = NX_SECURE_TLS_ALERT_DECRYPT_ERROR;
        *alert_level = NX_SECURE_TLS_ALERT_LEVEL_FATAL;
        break;

    /* We received a protocol version that we understand but that version is not supported/enabled. */
    case NX_SECURE_TLS_PROTOCOL_VERSION_CHANGED:
    case NX_SECURE_TLS_UNKNOWN_TLS_VERSION:
    case NX_SECURE_TLS_UNSUPPORTED_TLS_VERSION:
        *alert_number = NX_SECURE_TLS_ALERT_PROTOCOL_VERSION;
        *alert_level = NX_SECURE_TLS_ALERT_LEVEL_FATAL;
        break;

    /* Re-negotiation issues - the client may opt to decline a Hello Request message. */
    case NX_SECURE_TLS_NO_RENEGOTIATION_ERROR:
    case NX_SECURE_TLS_RENEGOTIATION_SESSION_INACTIVE:
    case NX_SECURE_TLS_RENEGOTIATION_FAILURE:
        *alert_number = NX_SECURE_TLS_ALERT_NO_RENEGOTIATION;
        *alert_level = NX_SECURE_TLS_ALERT_LEVEL_WARNING;
        break;

    /* Unknown PSK errors. */
    case NX_SECURE_TLS_NO_MATCHING_PSK:
        *alert_number = NX_SECURE_TLS_ALERT_UNKNOWN_PSK_IDENTITY;
        *alert_level = NX_SECURE_TLS_ALERT_LEVEL_FATAL;
        break;

    case NX_SECURE_TLS_INAPPROPRIATE_FALLBACK:
        *alert_number = NX_SECURE_TLS_ALERT_INAPPROPRIATE_FALLBACK;
        *alert_level = NX_SECURE_TLS_ALERT_LEVEL_FATAL;
        break;
        
    /* Internal errors. */

    case NX_SECURE_TLS_ALLOCATE_PACKET_FAILED:
    case NX_SECURE_TLS_SESSION_UNINITIALIZED:
    case NX_SECURE_TLS_INVALID_STATE:
    case NX_SECURE_TLS_INVALID_PACKET:
    case NX_SECURE_TLS_NEED_DTLS_SESSION:
    case NX_SECURE_TLS_NEED_TLS_SESSION:
    case NX_SECURE_TLS_INSUFFICIENT_CERT_SPACE:
    case NX_SECURE_TLS_TCP_SEND_FAILED:
    case NX_SECURE_TLS_NO_CLOSE_RESPONSE:
    case NX_SECURE_TLS_NO_MORE_PSK_SPACE:
    case NX_SECURE_TLS_NO_CERT_SPACE_ALLOCATED:
    case NX_SECURE_TLS_CLOSE_NOTIFY_RECEIVED:
    case NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL:
    case NX_SECURE_TLS_CERT_ID_INVALID:
    case NX_SECURE_TLS_CRYPTO_KEYS_TOO_LARGE:
    case NX_SECURE_TLS_CERT_ID_DUPLICATE: /* Deliberate fall-through. */

    /* DTLS errors. */
    case NX_SECURE_TLS_OUT_OF_ORDER_MESSAGE:
    case NX_SECURE_TLS_INVALID_REMOTE_HOST:
    case NX_SECURE_TLS_INVALID_EPOCH:
    case NX_SECURE_TLS_REPEAT_MESSAGE_RECEIVED:
    case NX_SECURE_TLS_SEND_ADDRESS_MISMATCH:
    case NX_SECURE_TLS_NO_FREE_DTLS_SESSIONS:
    case NX_SECURE_DTLS_SESSION_NOT_FOUND:
    case NX_SECURE_TLS_NO_AVAILABLE_SESSIONS: /* Deliberate fall-through. */

    case NX_SECURE_TLS_SUCCESS:               /* We should not be mapping success to an error! */
    default:
        *alert_number = NX_SECURE_TLS_ALERT_INTERNAL_ERROR;
        *alert_level = NX_SECURE_TLS_ALERT_LEVEL_FATAL;
        break;
    }
}

