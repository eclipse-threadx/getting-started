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
#ifdef NX_SECURE_ENABLE_DTLS
#include "nx_secure_dtls.h"
#endif /* NX_SECURE_ENABLE_DTLS */

/* This local static buffer needs to be large enough to hold both the server random and the client random. */
static UCHAR _nx_secure_tls_gen_keys_random[NX_SECURE_TLS_RANDOM_SIZE + NX_SECURE_TLS_RANDOM_SIZE];

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_generate_keys                        PORTABLE C      */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function generates the session keys used by TLS to encrypt     */
/*    the data being transmitted. It uses data gathered during the TLS    */
/*    handshake to generate a block of "key material" that is split into  */
/*    the various keys needed for each session.                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nx_crypto_init]                      Initialize crypto             */
/*    [nx_crypto_operation]                 Crypto operation              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_client_handshake      DTLS client state machine     */
/*    _nx_secure_dtls_server_handshake      DTLS server state machine     */
/*    _nx_secure_tls_client_handshake       TLS client state machine      */
/*    _nx_secure_tls_server_handshake       TLS server state machine      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_generate_keys(NX_SECURE_TLS_SESSION *tls_session)
{
UCHAR                                *pre_master_sec;
UINT                                  pre_master_sec_size;
UCHAR                                *master_sec;
UCHAR                                *key_block; /* Maximum ciphersuite key size - AES_256_CBC_SHA, 2x32 byte keys + 2x20 byte MAC secrets + 2x16 IVs. */
UINT                                  key_block_size;
UINT                                  key_size;
UINT                                  hash_size;
UINT                                  iv_size;
UINT                                  status;
const NX_CRYPTO_METHOD               *session_cipher_method = NX_NULL;
const NX_CRYPTO_METHOD               *session_prf_method = NX_NULL;
const NX_SECURE_TLS_CIPHERSUITE_INFO *ciphersuite;
VOID                                 *handler = NX_NULL;

    /* Generate the session keys using the parameters obtained in the handshake.
       By this point all the information needed to generate the TLS session key
       material should have been gathered and stored in the TLS socket structure. */

    key_block_size = 0;

    /* Working pointer into our new key material block since we are generating new keys. */
    key_block = tls_session -> nx_secure_tls_key_material.nx_secure_tls_new_key_material_data;


    /* Generate the Master Secret from the Pre-Master Secret.
       From the RFC (TLS 1.1, TLS 1.2):

        master_secret = PRF(pre_master_secret, "master secret",
                        ClientHello.random + ServerHello.random) [0..47];

        The master secret is always exactly 48 bytes in length.  The length
        of the premaster secret will vary depending on key exchange method.
     */

    /* Figure out which ciphersuite we are using. */
    ciphersuite = tls_session -> nx_secure_tls_session_ciphersuite;
    if (ciphersuite == NX_NULL)
    {

        /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    /* Get our session cipher method so we can get key sizes. */
    session_cipher_method = ciphersuite -> nx_secure_tls_session_cipher;

    /* The generation of key material is different between RSA and DH. */
    if (ciphersuite -> nx_secure_tls_public_cipher -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_RSA
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
        || ciphersuite -> nx_secure_tls_public_cipher -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECDHE
        || ciphersuite -> nx_secure_tls_public_cipher -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECDH
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */
#ifdef NX_SECURE_ENABLE_PSK_CIPHERSUITES
        || ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_PSK
#endif
#ifdef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
        || ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECJPAKE
#endif
        )
    {
        pre_master_sec = tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret;
        pre_master_sec_size = tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret_size;
        master_sec = tls_session -> nx_secure_tls_key_material.nx_secure_tls_master_secret;

        /* Concatenate random values to feed into PRF. */
        NX_SECURE_MEMCPY(_nx_secure_tls_gen_keys_random, tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_random,
                         NX_SECURE_TLS_RANDOM_SIZE);
        NX_SECURE_MEMCPY(&_nx_secure_tls_gen_keys_random[NX_SECURE_TLS_RANDOM_SIZE],
                         tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random, NX_SECURE_TLS_RANDOM_SIZE);

        /* Generate the master secret using the pre-master secret, the defined TLS label, and the concatenated
           random values. */
#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
#ifdef NX_SECURE_ENABLE_DTLS
        if (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_2 ||
            tls_session -> nx_secure_tls_protocol_version == NX_SECURE_DTLS_VERSION_1_2)
#else
        if (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_2)
#endif /* NX_SECURE_ENABLE_DTLS */
        {
            /* For TLS 1.2, the PRF is defined by the ciphersuite. However, if we are using an older ciphersuite,
             * default to the TLS 1.2 default PRF, which uses SHA-256-HMAC. */
            session_prf_method = ciphersuite -> nx_secure_tls_prf;
        }

#endif
#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
#ifdef NX_SECURE_ENABLE_DTLS
        if (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_0 ||
            tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_1 ||
            tls_session -> nx_secure_tls_protocol_version == NX_SECURE_DTLS_VERSION_1_0)
#else
        if (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_0 ||
            tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_1)
#endif /* NX_SECURE_ENABLE_DTLS */
        {
            /* TLS 1.0 and TLS 1.1 use the same PRF. */
            session_prf_method = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_prf_1_method;
        }
#endif
        /* If we don't have a PRF method, the version is wrong! */
        if (session_prf_method == NX_NULL)
        {
#ifdef NX_SECURE_KEY_CLEAR
            NX_SECURE_MEMSET(_nx_secure_tls_gen_keys_random, 0, sizeof(_nx_secure_tls_gen_keys_random));
#endif /* NX_SECURE_KEY_CLEAR  */

            return(NX_SECURE_TLS_PROTOCOL_VERSION_CHANGED);
        }

        /* Use the PRF to generate the master secret. */
        if (session_prf_method -> nx_crypto_init != NX_NULL)
        {
            status = session_prf_method -> nx_crypto_init((NX_CRYPTO_METHOD*)session_prf_method,
                                                 pre_master_sec, (NX_CRYPTO_KEY_SIZE)pre_master_sec_size,
                                                 &handler,
                                                 tls_session -> nx_secure_tls_prf_metadata_area,
                                                 tls_session -> nx_secure_tls_prf_metadata_size);

            if(status != NX_CRYPTO_SUCCESS)
            {
#ifdef NX_SECURE_KEY_CLEAR
                NX_SECURE_MEMSET(_nx_secure_tls_gen_keys_random, 0, sizeof(_nx_secure_tls_gen_keys_random));
#endif /* NX_SECURE_KEY_CLEAR  */

                return(status);
            }                                                     
        }

        if (session_prf_method -> nx_crypto_operation != NX_NULL)
        {
            status = session_prf_method -> nx_crypto_operation(NX_CRYPTO_PRF,
                                                      handler,
                                                      (NX_CRYPTO_METHOD*)session_prf_method,
                                                      (UCHAR *)"master secret",
                                                      13,
                                                      _nx_secure_tls_gen_keys_random,
                                                      64,
                                                      NX_NULL,
                                                      master_sec,
                                                      48,
                                                      tls_session -> nx_secure_tls_prf_metadata_area,
                                                      tls_session -> nx_secure_tls_prf_metadata_size,
                                                      NX_NULL,
                                                      NX_NULL);

#ifdef NX_SECURE_KEY_CLEAR
            NX_SECURE_MEMSET(_nx_secure_tls_gen_keys_random, 0, sizeof(_nx_secure_tls_gen_keys_random));
#endif /* NX_SECURE_KEY_CLEAR  */

            if(status != NX_CRYPTO_SUCCESS)
            {
                /* Secrets cleared above. */
                return(status);
            }
        }

        if (session_prf_method -> nx_crypto_cleanup)
        {
            status = session_prf_method -> nx_crypto_cleanup(tls_session -> nx_secure_tls_prf_metadata_area);

            if(status != NX_CRYPTO_SUCCESS)
            {
                /* All secrets cleared above. */
                return(status);
            }                                                     
        }
    }
    else
    {
        /* The chosen cipher is not supported. Likely an internal error since negotiation has already finished. */
        return(NX_SECURE_TLS_UNSUPPORTED_CIPHER);
    }

    /* Clear out the Pre-Master Secret (we don't need it anymore and keeping it in memory is dangerous). */
#ifdef NX_SECURE_KEY_CLEAR
    NX_SECURE_MEMSET(pre_master_sec, 0x0, sizeof(tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret));
#endif /* NX_SECURE_KEY_CLEAR  */

    /* Lookup ciphersuite data for key size. We need 2 keys for each session. */
    key_size = session_cipher_method -> nx_crypto_key_size_in_bits >> 3;

    /* Lookup ciphersuite data for hash size - used for the MAC secret. */
    hash_size = ciphersuite -> nx_secure_tls_hash_size;

    /* Lookup initialization vector size.  */
    iv_size = session_cipher_method -> nx_crypto_IV_size_in_bits >> 3;

    /* Now calculate block size. Need client and server for each key, hash, and iv. Note that
       key size and iv size may be zero depending on the ciphersuite, particularly when using
       the NULL ciphers. */
    key_block_size = 2 * (key_size + hash_size + iv_size);

    /* Now generate keys from the master secret we obtained above. */
    /* From the RFC (TLS 1.1):
       To generate the key material, compute

       key_block = PRF(SecurityParameters.master_secret,
                          "key expansion",
                          SecurityParameters.server_random +
             SecurityParameters.client_random);

       until enough output has been generated.  Then the key_block is
       partitioned as follows:

       client_write_MAC_secret[SecurityParameters.hash_size]
       server_write_MAC_secret[SecurityParameters.hash_size]
       client_write_key[SecurityParameters.key_material_length]
       server_write_key[SecurityParameters.key_material_length]
     */

    /* The order of the randoms is reversed from that used for the master secret
       when generating the key block.  */
    NX_SECURE_MEMCPY(_nx_secure_tls_gen_keys_random, tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random,
                     NX_SECURE_TLS_RANDOM_SIZE);
    NX_SECURE_MEMCPY(&_nx_secure_tls_gen_keys_random[NX_SECURE_TLS_RANDOM_SIZE],
                     tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_random, NX_SECURE_TLS_RANDOM_SIZE);

    /* Key expansion uses the PRF to generate a block of key material from the master secret (generated
       above) and the client and server random values transmitted during the initial hello negotiation. */
    if (session_prf_method -> nx_crypto_init != NX_NULL)
    {
        status = session_prf_method -> nx_crypto_init((NX_CRYPTO_METHOD*)session_prf_method,
                                             master_sec, 48,
                                             &handler,
                                             tls_session -> nx_secure_tls_prf_metadata_area,
                                             tls_session -> nx_secure_tls_prf_metadata_size);

        if(status != NX_CRYPTO_SUCCESS)
        {
#ifdef NX_SECURE_KEY_CLEAR
            NX_SECURE_MEMSET(_nx_secure_tls_gen_keys_random, 0, sizeof(_nx_secure_tls_gen_keys_random));
#endif /* NX_SECURE_KEY_CLEAR  */

            return(status);
        }                                                     
    }

    if (session_prf_method -> nx_crypto_operation != NX_NULL)
    {
        status = session_prf_method -> nx_crypto_operation(NX_CRYPTO_PRF,
                                                  handler,
                                                  (NX_CRYPTO_METHOD*)session_prf_method,
                                                  (UCHAR *)"key expansion",
                                                  13,
                                                  _nx_secure_tls_gen_keys_random,
                                                  64,
                                                  NX_NULL,
                                                  key_block,
                                                  key_block_size,
                                                  tls_session -> nx_secure_tls_prf_metadata_area,
                                                  tls_session -> nx_secure_tls_prf_metadata_size,
                                                  NX_NULL,
                                                  NX_NULL);

#ifdef NX_SECURE_KEY_CLEAR
        /* We now have a key block, clear our temporary secrets buffer. */
        NX_SECURE_MEMSET(_nx_secure_tls_gen_keys_random, 0, sizeof(_nx_secure_tls_gen_keys_random));
#endif /* NX_SECURE_KEY_CLEAR  */

        if(status != NX_CRYPTO_SUCCESS)
        {
            /* Secrets cleared above. */
            return(status);
        }
    }

    if (session_prf_method -> nx_crypto_cleanup)
    {
        status = session_prf_method -> nx_crypto_cleanup(tls_session -> nx_secure_tls_prf_metadata_area);

        if(status != NX_CRYPTO_SUCCESS)
        {
            /* Secrets cleared above. */
            return(status);
        }                                                     
    }

    return(NX_SECURE_TLS_SUCCESS);
}

