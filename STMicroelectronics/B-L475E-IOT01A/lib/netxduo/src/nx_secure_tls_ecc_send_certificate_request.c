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


#include "nx_secure_tls_ecc.h"
#include "nx_secure_tls.h"
#include "nx_secure_x509.h"
#ifdef NX_SECURE_ENABLE_DTLS
#include "nx_secure_dtls.h"
#endif /* NX_SECURE_ENABLE_DTLS */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_ecc_send_certificate_request         PORTABLE C      */
/*                                                          5.12          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function populates an NX_PACKET with a CertificateRequest      */
/*    message which is used to indicate to the remote client that we wish */
/*    to do a Client certificate verification. The Client should respond  */
/*    with an appropriate certificate and CertificateVerify message.      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    send_packet                           Packet to be filled           */
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
/*    _nx_secure_tls_server_handshake       TLS server state machine      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-15-2019     Timothy Stapko           Initial Version 5.12          */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_ecc_send_certificate_request(struct NX_SECURE_TLS_SESSION_STRUCT *tls_session,
                                                 NX_PACKET *send_packet)
{
UINT   length;
UCHAR *packet_buffer;


    /* Structure:
     * |       1            |    <Cert types count>    |             2              |  <Sig algs length>        |
     * |  Cert types count  | Cert types (1 byte each) | Sig Hash algorithms length | Algorithms (2 bytes each) |
     */

    if (((ULONG)(send_packet -> nx_packet_data_end) - (ULONG)(send_packet -> nx_packet_append_ptr)) < 11u)
    {

        /* Packet buffer is too small to hold random and ID. */
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    packet_buffer = send_packet -> nx_packet_append_ptr;

    /* Use our length as an index into the buffer. */
    length = 0;

    if (tls_session -> nx_secure_tls_ecc.nx_secure_tls_ecc_supported_groups_count != 0)
    {
        /* If ECC is initialized, we support RSA and ECDSA signatures. */
        packet_buffer[length] = 0x2;
        length += 1;

        packet_buffer[length] = NX_SECURE_TLS_CERT_TYPE_RSA_SIGN;
        length += 1;

        packet_buffer[length] = NX_SECURE_TLS_CERT_TYPE_ECDSA_SIGN;
        length += 1;
    }
    else
    {
        /* ECC is not initialized, we only support RSA signatures for now... */
        packet_buffer[length] = 0x1;
        length += 1;

        packet_buffer[length] = NX_SECURE_TLS_CERT_TYPE_RSA_SIGN;
        length += 1;
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
        if (tls_session -> nx_secure_tls_ecc.nx_secure_tls_ecc_supported_groups_count != 0)
        {
            /* ECC is initialized. */
            /* Length of supported signature algorithms - each entry is 2 bytes so length
               is number of entries * 2. */
            packet_buffer[length]  = (UCHAR)0x0;
            packet_buffer[length + 1] = (UCHAR)0x4;
            length = length + 2;

            /* Extract the signature algorithms. */
            packet_buffer[length] =     (UCHAR)(NX_SECURE_TLS_SIGNATURE_RSA_SHA256 >> 8);
            packet_buffer[length + 1] = (UCHAR)(NX_SECURE_TLS_SIGNATURE_RSA_SHA256);
            length = length + 2;

            packet_buffer[length] =     (UCHAR)(NX_SECURE_TLS_SIGNATURE_ECDSA_SHA256 >> 8);
            packet_buffer[length + 1] = (UCHAR)(NX_SECURE_TLS_SIGNATURE_ECDSA_SHA256);
            length = length + 2;
        }
        else
        {
            /* Length of supported signature algorithms - each entry is 2 bytes so length
               is number of entries * 2. */
            /* Note: We only support 1 one signature algorithm for now... */
            packet_buffer[length]  = (UCHAR)0x0;
            packet_buffer[length + 1] = (UCHAR)0x2;
            length = length + 2;

            /* Extract the signature algorithms. */
            packet_buffer[length] =     (UCHAR)(NX_SECURE_TLS_SIGNATURE_RSA_SHA256 >> 8);
            packet_buffer[length + 1] = (UCHAR)(NX_SECURE_TLS_SIGNATURE_RSA_SHA256);
            length = length + 2;
        }


    }
#endif

    /* Distinguished names length - right now we do not support this. */
    packet_buffer[length]  = (UCHAR)0x0;
    packet_buffer[length + 1] = (UCHAR)0x0;
    length = length + 2;

    /* The remainder of the message is the Certificate Authorities list. It can be used
       to select a certificate in a particular authorization chain.  */

    /* Let the caller know how many bytes we wrote. */
    send_packet -> nx_packet_append_ptr = send_packet -> nx_packet_append_ptr + (length);
    send_packet -> nx_packet_length = send_packet -> nx_packet_length + (length);

    return(NX_SUCCESS);
}

