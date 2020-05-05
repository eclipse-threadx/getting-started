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
/*    _nx_secure_tls_send_server_key_exchange             PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function generates a ServerKeyExchange message, which is used  */
/*    when the chosen ciphersuite requires additional information for key */
/*    generation, such as when using Diffie-Hellman ciphers.              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    send_packet                           Packet used to send message   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nx_crypto_operation]                 Crypto operation              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_server_handshake      DTLS server state machine     */
/*    _nx_secure_tls_server_handshake       TLS server state machine      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            supported ECJPAKE, optimized*/
/*                                            the logic,                  */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s), added    */
/*                                            elliptic curve cryptography,*/
/*                                            support, fix swapped PSK    */
/*                                            parameter, improved packet  */
/*                                            length verification, fixed  */
/*                                            endian issue, removed       */
/*                                            cipher suite lookup,        */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_send_server_key_exchange(NX_SECURE_TLS_SESSION *tls_session,
                                             NX_PACKET *send_packet)
{
UINT                                  length;
const NX_SECURE_TLS_CIPHERSUITE_INFO *ciphersuite;

#if defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES) || defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE)
UCHAR *packet_buffer;
#endif /* defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES) || defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE) */
#ifdef NX_SECURE_ENABLE_PSK_CIPHERSUITES
USHORT identity_length;
UCHAR *identity;
#endif

#ifdef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
UINT              status;
USHORT            named_curve;
NX_CRYPTO_METHOD *crypto_method;
NX_CRYPTO_EXTENDED_OUTPUT extended_output;
#endif

    /* Build up the server key exchange message. Structure:
     * |        2        |  <key data length>  |
     * | Key data length |  Key data (opaque)  |
     */

    /* Figure out which ciphersuite we are using. */
    ciphersuite = tls_session -> nx_secure_tls_session_ciphersuite;
    if (ciphersuite == NX_NULL)
    {
        /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    length = 0;

    /* In the future, any Diffie-Hellman-based ciphersuites will populate this message with key
       data. RSA ciphersuites do not use this message. */

#ifdef NX_SECURE_ENABLE_PSK_CIPHERSUITES

    /* Get a pointer to fill our packet. */
    packet_buffer = send_packet -> nx_packet_append_ptr;

    /* Check for PSK ciphersuites. */
    if (ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_PSK)
    {
        /* If PSK is being used, the server sends an identity value to the client so
           the client can respond with the appropriate PSK. */

        /* Get identity hint and length. */
        identity = tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_store[0].nx_secure_tls_psk_id_hint;
        identity_length = (USHORT)tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_store[0].nx_secure_tls_psk_id_hint_size;

        if ((identity_length + 2u) > ((ULONG)(send_packet -> nx_packet_data_end) - (ULONG)packet_buffer))
        {

            /* Packet buffer too small. */
            return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
        }

        /* Use identity hint length for key data length. */
        packet_buffer[length]     = (UCHAR)((identity_length & 0xFF00) >> 8);
        packet_buffer[length + 1] = (UCHAR)(identity_length & 0x00FF);
        length = (USHORT)(length + 2);

        /* Extract the identity hint and put it into the packet buffer. */
        NX_SECURE_MEMCPY(&packet_buffer[length], identity, identity_length);

        /* Advance our total length. */
        length = (USHORT)(length + identity_length);
    }
#endif

#ifdef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE

    /* Check for ECJ-PAKE ciphersuites. */
    if (ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECJPAKE)
    {

        /* Get a pointer to fill our packet. */
        packet_buffer = send_packet -> nx_packet_append_ptr;

        if (((ULONG)(send_packet -> nx_packet_data_end) - (ULONG)(&packet_buffer[length])) < 3)
        {

            /* Packet buffer too small. */
            return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
        }

        /* ECCurveType: named_curve (3). */
        packet_buffer[length] = 3;
        length += 1;

        /* NamedCurve: secp256r1 (23) */
        named_curve = (USHORT)NX_CRYPTO_EC_SECP256R1;
        packet_buffer[length] = (UCHAR)((named_curve >> 8) & 0xFF);
        packet_buffer[length + 1] = (UCHAR)(named_curve & 0xFF);
        length += 2;

        extended_output.nx_crypto_extended_output_data = &packet_buffer[length];
        extended_output.nx_crypto_extended_output_length_in_byte =
            (ULONG)send_packet -> nx_packet_data_end - (ULONG)&packet_buffer[length];
        extended_output.nx_crypto_extended_output_actual_size = 0;

        crypto_method = ciphersuite -> nx_secure_tls_public_auth;
        status = crypto_method -> nx_crypto_operation(NX_CRYPTO_ECJPAKE_SERVER_KEY_EXCHANGE_GENERATE,
                                                      &tls_session -> nx_secure_public_auth_handler,
                                                      crypto_method,
                                                      NX_NULL, 0,
                                                      NX_NULL, 0, NX_NULL,
                                                      (UCHAR *)&extended_output,
                                                      sizeof(extended_output),
                                                      tls_session -> nx_secure_public_auth_metadata_area,
                                                      tls_session -> nx_secure_public_auth_metadata_size,
                                                      NX_NULL, NX_NULL);
        if (status)
        {
            return(status);
        }
        length += extended_output.nx_crypto_extended_output_actual_size;
    }
#endif


    /* Finally, we have a complete length and can adjust our packet accordingly. */
    send_packet -> nx_packet_append_ptr = send_packet -> nx_packet_append_ptr + length;
    send_packet -> nx_packet_length = send_packet -> nx_packet_length + length;

    return(NX_SECURE_TLS_SUCCESS);
}

