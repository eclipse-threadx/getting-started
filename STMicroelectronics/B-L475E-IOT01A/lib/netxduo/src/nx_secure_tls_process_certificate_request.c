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
#include "nx_secure_x509.h"
#ifdef NX_SECURE_ENABLE_DTLS
#include "nx_secure_dtls.h"
#endif /* NX_SECURE_ENABLE_DTLS */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_process_certificate_request          PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an incoming TLS Certificate request         */
/*    message, typically sent by a TLS Server to request a client         */
/*    certificate for authentication.                                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    packet_buffer                         Pointer to message data       */
/*    message_length                        Length of message data (bytes)*/
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_client_handshake      DTLS client state machine     */
/*    _nx_secure_tls_client_handshake       TLS client state machine      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            supported DTLS, optimized   */
/*                                            the logic,                  */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s), added    */
/*                                            extension hook,             */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_process_certificate_request(NX_SECURE_TLS_SESSION *tls_session,
                                                UCHAR *packet_buffer, UINT message_length)
{
UINT  length;
UINT  cert_types_length;
UCHAR cert_type;
UINT  i;

#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
UINT sign_algs_length;
UINT sign_alg;
#endif

    NX_SECURE_PROCESS_CERTIFICATE_REQUEST_EXTENSION

    /* Structure:
     * |       1            |    <Cert types count>    |             2              |  <Sig algs length>        |
     * |  Cert types count  | Cert types (1 byte each) | Sig Hash algorithms length | Algorithms (2 bytes each) |
     */

    /* Use our length as an index into the buffer. */
    length = 0;

    /* Extract the count of certificate types from the incoming data. */
    cert_types_length = packet_buffer[length];
    length += 1;

    /* Make sure what we extracted makes sense. */
    if (cert_types_length > message_length)
    {
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }


    cert_type = NX_SECURE_TLS_CERT_TYPE_NONE;
    for (i = 0; i < cert_types_length; ++i)
    {
        if (packet_buffer[length] == NX_SECURE_TLS_CERT_TYPE_RSA_SIGN)
        {
            /* We found a type we support. */
            cert_type = packet_buffer[length];
        }
        length += 1;
    }

    /* Make sure our certificate type is one we support. */
    if (cert_type != NX_SECURE_TLS_CERT_TYPE_RSA_SIGN)
    {
        return(NX_SECURE_TLS_UNSUPPORTED_CERT_SIGN_TYPE);
    }

#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
    /* TLS 1.2 CertificateRequest contains a list of signature algorithms that
       is not included in earlier TLS versions. */
#ifdef NX_SECURE_ENABLE_DTLS
    if (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_2 ||
        tls_session -> nx_secure_tls_protocol_version == NX_SECURE_DTLS_VERSION_1_2)
#else
    if (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_2)
#endif /* NX_SECURE_ENABLE_DTLS */
    {
        /* Extract the count of algorithms from the incoming data. */
        sign_algs_length = (UINT)((packet_buffer[length] << 8) + packet_buffer[length + 1]);
        length = length + 2;

        /* Make sure what we extracted makes sense. */
        if ((length + sign_algs_length) > message_length)
        {
            return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
        }

        /* Extract the signature algorithms. */
        sign_alg = NX_SECURE_TLS_HASH_ALGORITHM_NONE;
        for (i = 0; i < sign_algs_length; i += 2)
        {
            /* Look for a type we support. */
            if ((UINT)((packet_buffer[length] << 8) + packet_buffer[length + 1]) == NX_SECURE_TLS_SIGNATURE_RSA_SHA256)
            {
                sign_alg = (UINT)((packet_buffer[length] << 8) + packet_buffer[length + 1]);
            }
            length = length + 2;
        }

        /* Make sure we are using the right signature algorithm! */
        if (sign_alg != NX_SECURE_TLS_SIGNATURE_RSA_SHA256)
        {
            return(NX_SECURE_TLS_UNSUPPORTED_CERT_SIGN_ALG);
        }
    }
#endif

    /* The remainder of the message is the Certificate Authorities list. It can be used
       to select a certificate in a particular authorization chain. In general, if there
       is only one device certificate we will send that one and if the server doesn't like
       it the connection will be dropped. */

#ifdef NX_SECURE_TLS_CLIENT_DISABLED
    /* If TLS Client is disabled and we have processed a CertificateRequest, something is wrong... */
    tls_session -> nx_secure_tls_server_state = NX_SECURE_TLS_SERVER_STATE_ERROR;

    return(NX_SECURE_TLS_INVALID_STATE);
#else
    /* Set our state to indicate we received a certificate request. */
    tls_session -> nx_secure_tls_client_state = NX_SECURE_TLS_CLIENT_STATE_CERTIFICATE_REQUEST;

    return(NX_SUCCESS);
#endif
}

