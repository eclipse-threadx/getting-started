/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
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
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
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
/*    _nx_secure_tlx_ecc_generate_keys      Generate keys for ECC exchange*/
/*    [nx_crypto_init]                      Initialize Crypto Method      */
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
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_send_server_key_exchange(NX_SECURE_TLS_SESSION *tls_session,
                                             NX_PACKET *send_packet)
{
UINT                                  length;
const NX_SECURE_TLS_CIPHERSUITE_INFO *ciphersuite;
#if defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE) || defined(NX_SECURE_ENABLE_ECC_CIPHERSUITE)
UINT                                  status;
#endif /* defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE) || defined(NX_SECURE_ENABLE_ECC_CIPHERSUITE) */

#if defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES) || defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE) || defined(NX_SECURE_ENABLE_ECC_CIPHERSUITE)
UCHAR *packet_buffer;
#endif /* defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES) || defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE) || defined(NX_SECURE_ENABLE_ECC_CIPHERSUITE) */

#ifdef NX_SECURE_ENABLE_PSK_CIPHERSUITES
USHORT identity_length;
UCHAR *identity;
#endif

#if defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE)
USHORT            named_curve;
NX_CRYPTO_METHOD *crypto_method;
NX_CRYPTO_EXTENDED_OUTPUT extended_output;
#endif /* defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE) */

#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
NX_SECURE_TLS_ECDHE_HANDSHAKE_DATA   *ecdhe_data;
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

        crypto_method = (NX_CRYPTO_METHOD*)ciphersuite -> nx_secure_tls_public_auth;
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

#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
    /* Check for ECDHE ciphersuites. */
    if (ciphersuite -> nx_secure_tls_public_cipher -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECDHE)
    {
        ecdhe_data = (NX_SECURE_TLS_ECDHE_HANDSHAKE_DATA *)tls_session -> nx_secure_tls_key_material.nx_secure_tls_new_key_material_data;

        packet_buffer = send_packet -> nx_packet_append_ptr;
        length = (UINT)(send_packet->nx_packet_data_end - send_packet->nx_packet_append_ptr);

        status = _nx_secure_tls_ecc_generate_keys(tls_session, ecdhe_data -> nx_secure_tls_ecdhe_named_curve, NX_TRUE,
                                                  packet_buffer, &length, ecdhe_data);

        if (status != NX_SUCCESS)
        {
            return(status);
        }
    }
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */


    /* Finally, we have a complete length and can adjust our packet accordingly. */
    send_packet -> nx_packet_append_ptr = send_packet -> nx_packet_append_ptr + length;
    send_packet -> nx_packet_length = send_packet -> nx_packet_length + length;

    return(NX_SECURE_TLS_SUCCESS);
}

