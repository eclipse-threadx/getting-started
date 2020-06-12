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
/**    Transport Layer Security (TLS) - Generate Session Keys             */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SECURE_SOURCE_CODE


#include "nx_secure_tls.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_session_keys_set                     PORTABLE C      */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets the session keys for a TLS session following the */
/*    sending or receiving of a ChangeCipherSpec message. In              */
/*    renegotiation handshakes, two separate set of session keys will be  */
/*    in use simultaneously so we need this to be able to separate which  */
/*    keys are actually in use.                                           */
/*                                                                        */
/*    Once the keys are set, this function initializes the appropriate    */
/*    session cipher with the new key set.                                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    key_set                               Remote or local keys          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nx_crypto_init]                      Initialize crypto             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_client_handshake      DTLS client state machine     */
/*    _nx_secure_dtls_server_handshake      DTLS server state machine     */
/*    _nx_secure_tls_client_handshake       TLS client state machine      */
/*    _nx_secure_tls_server_handshake       TLS server state machine      */
/*    _nx_secure_tls_process_changecipherspec                             */
/*                                          Process ChangeCipherSpec      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*                                                                        */
/**************************************************************************/
#define NX_SECURE_SOURCE_CODE
UINT _nx_secure_tls_session_keys_set(NX_SECURE_TLS_SESSION *tls_session, USHORT key_set)
{
UINT                                  status;
UCHAR                                *key_block;     /* Maximum ciphersuite key size - AES_256_CBC_SHA, 2x32 byte keys + 2x20 byte MAC secrets + 2x16 IVs. */
UINT                                  key_size;
UINT                                  hash_size;
UINT                                  iv_size;
UINT                                  key_offset;
UINT                                  is_client;
const NX_CRYPTO_METHOD               *session_cipher_method = NX_NULL;

    /* The key material should have already been generated by nx_secure_tls_generate_keys once all
     * key generation data was available. This simply switches the appropriate key data over to the active
     * key block */

    if (key_set == NX_SECURE_TLS_KEY_SET_LOCAL)
    {
        tls_session -> nx_secure_tls_local_session_active = 1;
    }
    else
    {
        tls_session -> nx_secure_tls_remote_session_active = 1;
    }

    /* Working pointers into our key material blocks. */
    key_block = tls_session -> nx_secure_tls_key_material.nx_secure_tls_key_material_data;

    /* See if we are setting server or client keys. */
    if ((key_set == NX_SECURE_TLS_KEY_SET_REMOTE && tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_CLIENT) ||
        (key_set == NX_SECURE_TLS_KEY_SET_LOCAL  && tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_SERVER))
    {
        /* Setting remote keys for a client or local keys for a server: we are setting server keys. */
        is_client = NX_FALSE;
    }
    else
    {
        /* Local client/local keys or local server/remote keys. */
        is_client = NX_TRUE;
    }



    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {

        /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    /* Get our session cipher method so we can get key sizes. */
    session_cipher_method = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_session_cipher;

    /* Lookup ciphersuite data for key size. We need 2 keys for each session. */
    key_size = session_cipher_method -> nx_crypto_key_size_in_bits >> 3;

    /* Lookup ciphersuite data for hash size - used for the MAC secret. */
    hash_size = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_hash_size;

    /* Lookup initialization vector size.  */
    iv_size = session_cipher_method -> nx_crypto_IV_size_in_bits >> 3;

    /* Partition the key block into our keys and secrets following the TLS spec.*/
    key_offset = 0;

    if (((hash_size + key_size + iv_size) << 1) > sizeof(tls_session -> nx_secure_tls_key_material.nx_secure_tls_key_material_data))
    {

        /* No space for key material data. */
        return(NX_SECURE_TLS_CRYPTO_KEYS_TOO_LARGE);
    }

    /* First, the mac secrets. Check for non-zero in the (unlikely) event we are using a NULL hash. */
    if (hash_size > 0)
    {

        /* Copy new client mac secret over if setting client keys. */
        if (is_client)
        {
            NX_SECURE_MEMCPY(&tls_session -> nx_secure_tls_key_material.nx_secure_tls_key_material_data[key_offset],
                             &tls_session -> nx_secure_tls_key_material.nx_secure_tls_new_key_material_data[key_offset], hash_size);
        }
        tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_write_mac_secret = &key_block[key_offset];
        key_offset += hash_size;

        /* Copy new server mac secret if setting server keys. */
        if (!is_client)
        {
            NX_SECURE_MEMCPY(&tls_session -> nx_secure_tls_key_material.nx_secure_tls_key_material_data[key_offset],
                             &tls_session -> nx_secure_tls_key_material.nx_secure_tls_new_key_material_data[key_offset], hash_size);
        }
        tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_write_mac_secret = &key_block[key_offset];
        key_offset += hash_size;
    }

    /* Now the keys. Check for non-zero size in the event we are using a NULL cipher (usually for debugging). */
    if (key_size > 0)
    {
        /* Copy new client session key if setting client keys. */
        if (is_client)
        {
            NX_SECURE_MEMCPY(&tls_session -> nx_secure_tls_key_material.nx_secure_tls_key_material_data[key_offset],
                             &tls_session -> nx_secure_tls_key_material.nx_secure_tls_new_key_material_data[key_offset], key_size);
        }
        tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_write_key = &key_block[key_offset];
        key_offset += key_size;

        /* Copy new server session key if setting server keys. */
        if (!is_client)
        {
            NX_SECURE_MEMCPY(&tls_session -> nx_secure_tls_key_material.nx_secure_tls_key_material_data[key_offset],
                             &tls_session -> nx_secure_tls_key_material.nx_secure_tls_new_key_material_data[key_offset], key_size);
        }
        tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_write_key = &key_block[key_offset];
        key_offset += key_size;
    }

    /* Finally, the IVs. Many ciphers don't use IV's so the iv_size is often zero. */
    if (iv_size > 0)
    {
        /* Copy new client IV if setting client keys. */
        if (is_client)
        {
            NX_SECURE_MEMCPY(&tls_session -> nx_secure_tls_key_material.nx_secure_tls_key_material_data[key_offset],
                             &tls_session -> nx_secure_tls_key_material.nx_secure_tls_new_key_material_data[key_offset], iv_size);
        }
        tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_iv = &key_block[key_offset];
        key_offset += iv_size;

        /* Copy new server IV if setting server keys. */
        if (!is_client)
        {
            NX_SECURE_MEMCPY(&tls_session -> nx_secure_tls_key_material.nx_secure_tls_key_material_data[key_offset],
                             &tls_session -> nx_secure_tls_key_material.nx_secure_tls_new_key_material_data[key_offset], iv_size);
        }
        tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_iv = &key_block[key_offset];
        key_offset += iv_size;
    }

    /* Initialize the crypto method used in the session cipher. */
    if (session_cipher_method -> nx_crypto_init != NULL)
    {
        /* Set client write key. */
        if (is_client)
        {
            status = session_cipher_method -> nx_crypto_init((NX_CRYPTO_METHOD*)session_cipher_method,
                                                    tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_write_key,
                                                    session_cipher_method -> nx_crypto_key_size_in_bits,
                                                    &tls_session -> nx_secure_session_cipher_handler_client,
                                                    tls_session -> nx_secure_session_cipher_metadata_area_client,
                                                    tls_session -> nx_secure_session_cipher_metadata_size);
        }
        else
        {
            /* Set server write key. */
            status = session_cipher_method -> nx_crypto_init((NX_CRYPTO_METHOD*)session_cipher_method,
                                                    tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_write_key,
                                                    session_cipher_method -> nx_crypto_key_size_in_bits,
                                                    &tls_session -> nx_secure_session_cipher_handler_server,
                                                    tls_session -> nx_secure_session_cipher_metadata_area_server,
                                                    tls_session -> nx_secure_session_cipher_metadata_size);
        }
        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    return(NX_SECURE_TLS_SUCCESS);
}

