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

static UCHAR _nx_secure_client_padded_pre_master[600];

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_send_client_key_exchange             PORTABLE C      */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function encrypts the Pre-Master Secret (generated earlier)    */
/*    and populates an NX_PACKET with the complete ClientKeyExchange      */
/*    message (to be sent by the caller). It also will send ephemeral     */
/*    keys for ciphersuites that require them.                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    send_packet                           Outgoing TLS packet           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_remote_endpoint_certificate_get                     */
/*                                          Get cert for remote host      */
/*    [nx_crypto_operation]                 Public-key ciphers            */
/*    [nx_crypto_init]                      Initialize Crypto Method      */
/*    _nx_secure_tls_remote_certificate_free_all                          */
/*                                          Free all remote certificates  */
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
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_send_client_key_exchange(NX_SECURE_TLS_SESSION *tls_session,
                                             NX_PACKET *send_packet)
{
UINT                                  status;
UINT                                  data_size;
UCHAR                                *encrypted_data_ptr;
UCHAR                                *packet_buffer;
UCHAR                                 rand_byte;
UINT                                  i;
const NX_CRYPTO_METHOD               *public_cipher_method;
NX_SECURE_X509_CERT                  *remote_certificate;
VOID                                 *handler = NX_NULL;
#ifdef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
NX_CRYPTO_EXTENDED_OUTPUT             extended_output;
#endif /* NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE */

    /* Send a ClientKeyExchange message.
     * Structure:
     * |   2    |          <Length>           |
     * | Length | Encrypted Pre-Master Secret |
     *
     * ECDHE ephemeral key structure:
     * |   2    |            <Length>             |
     * | Length | EC Diffie-Hellman Client Params |
     */

    packet_buffer = send_packet -> nx_packet_append_ptr;

    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {

        /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
    if (tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_cipher -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECDH ||
        tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_cipher -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECDHE)
    {
        data_size = (UINT)(1 + tls_session -> nx_secure_tls_key_material.nx_secure_tls_new_key_material_data[0]);

        if ((data_size > sizeof(tls_session -> nx_secure_tls_key_material.nx_secure_tls_new_key_material_data)) ||
            (data_size > ((ULONG)(send_packet -> nx_packet_data_end) - (ULONG)(send_packet -> nx_packet_append_ptr))))
        {
            _nx_secure_tls_remote_certificate_free_all(tls_session);
            /* Packet buffer too small. */
            return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
        }

        NX_SECURE_MEMCPY(packet_buffer, tls_session -> nx_secure_tls_key_material.nx_secure_tls_new_key_material_data, data_size);
    }
    else
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */
    {

        /* Pointer to the output encrypted pre-master secret. */
        encrypted_data_ptr = &packet_buffer[2];

#ifdef NX_SECURE_ENABLE_PSK_CIPHERSUITES
        /* Check for PSK ciphersuites. */
        if (tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_PSK)
        {
            if ((tls_session -> nx_secure_tls_credentials.nx_secure_tls_client_psk.nx_secure_tls_psk_id_hint_size >
                 sizeof(tls_session -> nx_secure_tls_credentials.nx_secure_tls_client_psk.nx_secure_tls_psk_id_hint)) ||
                (tls_session -> nx_secure_tls_credentials.nx_secure_tls_client_psk.nx_secure_tls_psk_id_hint_size >
                 (((ULONG)(send_packet -> nx_packet_data_end) - (ULONG)(send_packet -> nx_packet_append_ptr)) - 2)))
            {

                /* Packet buffer too small. */
                return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
            }

            /* Send the PSK Identity string to the remote server along with its length. */
            NX_SECURE_MEMCPY(encrypted_data_ptr, tls_session -> nx_secure_tls_credentials.nx_secure_tls_client_psk.nx_secure_tls_psk_id,
                   tls_session -> nx_secure_tls_credentials.nx_secure_tls_client_psk.nx_secure_tls_psk_id_size);

            /* Make sure our size is correct. */
            data_size = tls_session -> nx_secure_tls_credentials.nx_secure_tls_client_psk.nx_secure_tls_psk_id_size;

            /* Put the length into our outgoing packet buffer. */
            packet_buffer[0] = (UCHAR)((data_size & 0xFF00) >> 8);
            packet_buffer[1] = (UCHAR)(data_size & 0x00FF);

            data_size += 2;
        }
        else
#endif
#ifdef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
        if (tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECJPAKE)
        {
            data_size = 0;
            public_cipher_method = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_auth;

            extended_output.nx_crypto_extended_output_data = packet_buffer;
            extended_output.nx_crypto_extended_output_length_in_byte =
                (ULONG)send_packet -> nx_packet_data_end - (ULONG)packet_buffer;
            extended_output.nx_crypto_extended_output_actual_size = 0;
            status = public_cipher_method -> nx_crypto_operation(NX_CRYPTO_ECJPAKE_CLIENT_KEY_EXCHANGE_GENERATE,
                                                                 tls_session -> nx_secure_public_auth_handler,
                                                                 (NX_CRYPTO_METHOD*)public_cipher_method,
                                                                 NX_NULL, 0,
                                                                 NX_NULL, 0, NX_NULL,
                                                                 (UCHAR *)&extended_output,
                                                                 sizeof(extended_output),
                                                                 tls_session -> nx_secure_public_auth_metadata_area,
                                                                 tls_session -> nx_secure_public_auth_metadata_size,
                                                                 NX_NULL, NX_NULL);
            if (status)
            {
                _nx_secure_tls_remote_certificate_free_all(tls_session);
                return(status);
            }

            data_size += extended_output.nx_crypto_extended_output_actual_size;
        }
        else
#endif
        {

            /* Extract the data to be verified from the remote certificate processed earlier. */
            status = _nx_secure_x509_remote_endpoint_certificate_get(&tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store,
                                                                     &remote_certificate);

            if (status)
            {
                _nx_secure_tls_remote_certificate_free_all(tls_session);

                /* No certificate found, error! */
                return(NX_SECURE_TLS_NO_CERT_SPACE_ALLOCATED);
            }

            /* Get the public cipher method from the ciphersuite. */
            public_cipher_method = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_cipher;

            /* If using RSA, the length is equal to the key size. */
            data_size = remote_certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_modulus_length;

            /* PKCS#1 v1.5 padding. The scheme is to start with the block type (0x00, 0x02 for PKCS#1)
               then pad with non-zero bytes (random is cryptographically more secure), followed with
               a single 0 byte right before the payload, which comes at the end of the RSA block. */
            NX_SECURE_MEMSET(_nx_secure_client_padded_pre_master, 0x0, sizeof(_nx_secure_client_padded_pre_master));
            _nx_secure_client_padded_pre_master[1] = 0x2; /* Block type is 0x00, 0x02 */
            for (i = 2; i < (data_size - NX_SECURE_TLS_RSA_PREMASTER_SIZE - 1); ++i)
            {
                /* PKCS#1 padding must be random, but CANNOT be 0. */
                do
                {
                    rand_byte = (UCHAR)NX_RAND();
                } while (rand_byte == 0);
                _nx_secure_client_padded_pre_master[i] = rand_byte;
            }

            if ((data_size < NX_SECURE_TLS_RSA_PREMASTER_SIZE) ||
                (((ULONG)(send_packet -> nx_packet_data_end) - (ULONG)(send_packet -> nx_packet_append_ptr)) < (2u + data_size)))
            {
                _nx_secure_tls_remote_certificate_free_all(tls_session);

                /* Invalid certificate modulus length. */
                return(NX_SECURE_TLS_INVALID_CERTIFICATE);
            }

            /* Now put the pre-master data into the padded buffer - must be at the end. */
            NX_SECURE_MEMCPY(&_nx_secure_client_padded_pre_master[data_size - NX_SECURE_TLS_RSA_PREMASTER_SIZE],
                   tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret, NX_SECURE_TLS_RSA_PREMASTER_SIZE);

            if (public_cipher_method -> nx_crypto_init != NX_NULL)
            {
                /* Initialize the crypto method with public key. */
                status = public_cipher_method -> nx_crypto_init((NX_CRYPTO_METHOD*)public_cipher_method,
                                                       (UCHAR *)remote_certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_modulus,
                                                       (NX_CRYPTO_KEY_SIZE)(remote_certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_modulus_length << 3),
                                                       &handler,
                                                       tls_session -> nx_secure_public_cipher_metadata_area,
                                                       tls_session -> nx_secure_public_cipher_metadata_size);

                if(status != NX_CRYPTO_SUCCESS)
                {
                    _nx_secure_tls_remote_certificate_free_all(tls_session);
#ifdef NX_SECURE_KEY_CLEAR
                    NX_SECURE_MEMSET(_nx_secure_client_padded_pre_master, 0, sizeof(_nx_secure_client_padded_pre_master));
#endif /* NX_SECURE_KEY_CLEAR  */

                    return(status);
                }
            }

            if (public_cipher_method -> nx_crypto_operation != NX_NULL)
            {
                /* Now encrypt the pre-master secret using the public key provided by the remote host
                   and place the result in the outgoing packet buffer. */
                status = public_cipher_method -> nx_crypto_operation(NX_CRYPTO_DECRYPT,
                                                            handler,
                                                            (NX_CRYPTO_METHOD*)public_cipher_method,
                                                            (UCHAR *)remote_certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_exponent,
                                                            (NX_CRYPTO_KEY_SIZE)(remote_certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_exponent_length << 3),
                                                            _nx_secure_client_padded_pre_master,
                                                            data_size,
                                                            NX_NULL,
                                                            encrypted_data_ptr,
                                                            data_size,
                                                            tls_session -> nx_secure_public_cipher_metadata_area,
                                                            tls_session -> nx_secure_public_cipher_metadata_size,
                                                            NX_NULL, NX_NULL);

                if(status != NX_CRYPTO_SUCCESS)
                {
                     _nx_secure_tls_remote_certificate_free_all(tls_session);
#ifdef NX_SECURE_KEY_CLEAR
                    NX_SECURE_MEMSET(_nx_secure_client_padded_pre_master, 0, sizeof(_nx_secure_client_padded_pre_master));
#endif /* NX_SECURE_KEY_CLEAR  */

                    return(status);
                }
            }

            if (public_cipher_method -> nx_crypto_cleanup)
            {
                status = public_cipher_method -> nx_crypto_cleanup(tls_session -> nx_secure_public_cipher_metadata_area);

                if(status != NX_CRYPTO_SUCCESS)
                {
                     _nx_secure_tls_remote_certificate_free_all(tls_session);
#ifdef NX_SECURE_KEY_CLEAR
                    NX_SECURE_MEMSET(_nx_secure_client_padded_pre_master, 0, sizeof(_nx_secure_client_padded_pre_master));
#endif /* NX_SECURE_KEY_CLEAR  */

                    return(status);
                }
            }

            /* Put the length into our outgoing packet buffer. */
            packet_buffer[0] = (UCHAR)((data_size & 0xFF00) >> 8);
            packet_buffer[1] = (UCHAR)(data_size & 0x00FF);

            data_size += 2;
        }
    }

    /* Let the caller know how many bytes we wrote. +2 for the length we just added. */
    send_packet -> nx_packet_append_ptr = send_packet -> nx_packet_append_ptr + data_size;
    send_packet -> nx_packet_length = send_packet -> nx_packet_length + data_size;

#ifdef NX_SECURE_KEY_CLEAR
    NX_SECURE_MEMSET(_nx_secure_client_padded_pre_master, 0, sizeof(_nx_secure_client_padded_pre_master));
#endif /* NX_SECURE_KEY_CLEAR  */

    return(NX_SECURE_TLS_SUCCESS);
}

