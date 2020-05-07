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

static UCHAR _nx_secure_client_padded_pre_master[600];

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_process_client_key_exchange          PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an incoming ClientKeyExchange message,      */
/*    which contains the encrypted Pre-Master Secret. This function       */
/*    decrypts the Pre-Master Secret and saves it in the TLS session      */
/*    control block for use in generating session key material later.     */
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
/*    _nx_secure_tls_generate_premaster_secret                            */
/*                                          Generate the shared secret    */
/*                                            used to generate keys later */
/*    _nx_secure_x509_local_device_certificate_get                        */
/*                                          Get the local certificate     */
/*                                            for its keys                */
/*    [nx_crypto_init]                      Initialize crypto             */
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
/*  12-15-2017     Timothy Stapko           Modified comment(s), and      */
/*                                            updated to new crypto API,  */
/*                                            added logic to support      */
/*                                            vendor-defined private key  */
/*                                            type, supported ECJPAKE,    */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s), and      */
/*                                            added flexibility of using  */
/*                                            macros instead of direct C  */
/*                                            library function calls,     */
/*                                            passed crypto handle into   */
/*                                            crypto internal functions,  */
/*                                            added operation method for  */
/*                                            elliptic curve cryptography,*/
/*                                            removed server state        */
/*                                            processing, added extension */
/*                                            hook, added logic to clear  */
/*                                            encryption key and other    */
/*                                            secret data, updated buffer */
/*                                            for 4096 RSA, improved      */
/*                                            used random premaster for   */
/*                                            incorrect message, fixed    */
/*                                            endian issue, updated       */
/*                                            error return codes,         */
/*                                            updated error return checks,*/
/*                                            removed cipher suite lookup,*/
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_process_client_key_exchange(NX_SECURE_TLS_SESSION *tls_session,
                                                UCHAR *packet_buffer, UINT message_length)
{
USHORT                                length;
UINT                                  status;
UCHAR                                *encrypted_pre_master_secret;
NX_CRYPTO_METHOD                     *public_cipher_method;
NX_SECURE_X509_CERT                  *local_certificate;
UINT                                  user_defined_key;
VOID                                 *handler = NX_NULL;
UCHAR                                 rand_byte;
UINT                                  i;

    NX_SECURE_PROCESS_CLIENT_KEY_EXCHANGE_EXTENSION

    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {

        /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    /* Process key material. The contents of the handshake record differ according to the
       ciphersuite chosen in the Client/Server Hello negotiation. */

#ifdef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
    /* Check for ECJ-PAKE ciphersuites and generate the pre-master-secret. */
    if (tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECJPAKE)
    {

        tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret_size = 32;

        public_cipher_method = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_auth;
        status = public_cipher_method -> nx_crypto_operation(NX_CRYPTO_ECJPAKE_CLIENT_KEY_EXCHANGE_PROCESS,
                                                             tls_session -> nx_secure_public_auth_handler,
                                                             public_cipher_method,
                                                             NX_NULL, 0,
                                                             packet_buffer,
                                                             message_length,
                                                             NX_NULL,
                                                             tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret,
                                                             tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret_size,
                                                             tls_session -> nx_secure_public_auth_metadata_area,
                                                             tls_session -> nx_secure_public_auth_metadata_size,
                                                             NX_NULL, NX_NULL);
        if (status)
        {
            return(status);
        }

        if (public_cipher_method -> nx_crypto_cleanup)
        {
            status = public_cipher_method -> nx_crypto_cleanup(tls_session -> nx_secure_public_auth_metadata_area);

            if (status)
            {
                return(status);
            }
        }
    }
    else
#endif
    {
        /* Get pre-master-secret length. */
        length = (USHORT)((packet_buffer[0] << 8) + (USHORT)packet_buffer[1]);
        packet_buffer += 2;

        if (length > message_length)
        {
            /* The payload is larger than the header indicated. */
            return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
        }

        /* Pointer to the encrypted pre-master secret in our packet buffer. */
        encrypted_pre_master_secret = &packet_buffer[0];


        if (tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_ciphersuite == TLS_NULL_WITH_NULL_NULL)
        {
            /* Special case - NULL ciphersuite. No keys are generated. */
            if (length > sizeof(tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret))
            {
                length = sizeof(tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret);
            }
            NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret, encrypted_pre_master_secret, length);
            tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret_size = length;
        }

#ifdef NX_SECURE_ENABLE_PSK_CIPHERSUITES
        /* Check for PSK ciphersuites and generate the pre-master-secret. */
        if (tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_PSK)
        {
            status = _nx_secure_tls_generate_premaster_secret(tls_session);

            if (status != NX_SUCCESS)
            {
                return(status);
            }
        }
        else
#endif
        {       /* Certificate-based authentication. */

            /* Get reference to local device certificate. NX_NULL is passed for name to get default entry. */
            status = _nx_secure_x509_local_device_certificate_get(&tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store,
                                                                  NX_NULL, &local_certificate);

            if (status)
            {
                /* No certificate found, error! */
                return(NX_SECURE_TLS_CERTIFICATE_NOT_FOUND);
            }

            /* Get the public cipher method pointer for this session. */
            public_cipher_method = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_cipher;

            /* Check for user-defined key types. */
            user_defined_key = NX_FALSE;
            if ((local_certificate -> nx_secure_x509_private_key_type & NX_SECURE_X509_KEY_TYPE_USER_DEFINED_MASK) != 0x0)
            {
                user_defined_key = NX_TRUE;
            }

            /* See if we are using RSA. Separate from other methods (e.g. ECC, DH) for proper handling of padding. */
            if (public_cipher_method -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_RSA &&
                local_certificate -> nx_secure_x509_public_algorithm == NX_SECURE_TLS_X509_TYPE_RSA)
            {
                /* Check for user-defined keys. */
                if (user_defined_key)
                {
                    /* A user-defined key is passed directly into the crypto routine. */
                    status = public_cipher_method -> nx_crypto_operation(local_certificate -> nx_secure_x509_private_key_type,
                                                                NX_NULL,
                                                                public_cipher_method,
                                                                (UCHAR *)local_certificate -> nx_secure_x509_private_key.user_key.key_data,
                                                                (NX_CRYPTO_KEY_SIZE)(local_certificate -> nx_secure_x509_private_key.user_key.key_length),
                                                                encrypted_pre_master_secret,
                                                                length,
                                                                NX_NULL,
                                                                _nx_secure_client_padded_pre_master,
                                                                sizeof(_nx_secure_client_padded_pre_master),
                                                                tls_session -> nx_secure_public_cipher_metadata_area,
                                                                tls_session -> nx_secure_public_cipher_metadata_size,
                                                                NX_NULL, NX_NULL);

                    if(status != NX_SUCCESS)
                    {
                        return(status);
                    }                                                     
                }
                else
                {
                    /* Generic RSA operation, use pre-parsed RSA key data. */
                    if (public_cipher_method -> nx_crypto_init != NX_NULL)
                    {
                        /* Initialize the crypto method with public key. */
                        status = public_cipher_method -> nx_crypto_init(public_cipher_method,
                                                               (UCHAR *)local_certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_modulus,
                                                               (NX_CRYPTO_KEY_SIZE)(local_certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_modulus_length << 3),
                                                               &handler,
                                                               tls_session -> nx_secure_public_cipher_metadata_area,
                                                               tls_session -> nx_secure_public_cipher_metadata_size);

                        if(status != NX_SUCCESS)
                        {
                            return(status);
                        }                                                     
                    }

                    if (public_cipher_method -> nx_crypto_operation != NX_NULL)
                    {


                        /* Check for P and Q in the private key. If they are present, we can use them to
                           speed up RSA using the Chinese Remainder Theorem version of the algorithm. */
                        if (local_certificate -> nx_secure_x509_private_key.rsa_private_key.nx_secure_rsa_private_prime_p != NX_NULL &&
                            local_certificate -> nx_secure_x509_private_key.rsa_private_key.nx_secure_rsa_private_prime_q != NX_NULL)
                        {


                            status = public_cipher_method -> nx_crypto_operation(NX_CRYPTO_SET_PRIME_P,
                                                                        handler,
                                                                        public_cipher_method,
                                                                        NX_NULL,
                                                                        0,
                                                                        (VOID *)local_certificate -> nx_secure_x509_private_key.rsa_private_key.nx_secure_rsa_private_prime_p,
                                                                        local_certificate -> nx_secure_x509_private_key.rsa_private_key.nx_secure_rsa_private_prime_p_length,
                                                                        NX_NULL,
                                                                        NX_NULL,
                                                                        0,
                                                                        tls_session -> nx_secure_public_cipher_metadata_area,
                                                                        tls_session -> nx_secure_public_cipher_metadata_size,
                                                                        NX_NULL, NX_NULL);

                            if(status != NX_SUCCESS)
                            {
                                return(status);
                            }                                                     

                            status = public_cipher_method -> nx_crypto_operation(NX_CRYPTO_SET_PRIME_Q,
                                                                        handler,
                                                                        public_cipher_method,
                                                                        NX_NULL,
                                                                        0,
                                                                        (VOID *)local_certificate -> nx_secure_x509_private_key.rsa_private_key.nx_secure_rsa_private_prime_q,
                                                                        local_certificate -> nx_secure_x509_private_key.rsa_private_key.nx_secure_rsa_private_prime_q_length,
                                                                        NX_NULL,
                                                                        NX_NULL,
                                                                        0,
                                                                        tls_session -> nx_secure_public_cipher_metadata_area,
                                                                        tls_session -> nx_secure_public_cipher_metadata_size,
                                                                        NX_NULL, NX_NULL);

                            if(status != NX_SUCCESS)
                            {
                                return(status);
                            }                                                     
                    

                            /* Decrypt the pre-master-secret using the private key provided by the user
                                and place the result in the session key material space in our socket. */
                            status = public_cipher_method -> nx_crypto_operation(NX_CRYPTO_DECRYPT,
                                                                        handler,
                                                                        public_cipher_method,
                                                                        (UCHAR *)local_certificate -> nx_secure_x509_private_key.rsa_private_key.nx_secure_rsa_private_exponent,
                                                                        (NX_CRYPTO_KEY_SIZE)(local_certificate -> nx_secure_x509_private_key.rsa_private_key.nx_secure_rsa_private_exponent_length << 3),
                                                                        encrypted_pre_master_secret,
                                                                        length,
                                                                        NX_NULL,
                                                                        _nx_secure_client_padded_pre_master,
                                                                        sizeof(_nx_secure_client_padded_pre_master),
                                                                        tls_session -> nx_secure_public_cipher_metadata_area,
                                                                        tls_session -> nx_secure_public_cipher_metadata_size,
                                                                        NX_NULL, NX_NULL);

                            if(status != NX_SUCCESS)
                            {
                                return(status);
                            }                                                     
                        }
                    }
                    
                    if (public_cipher_method -> nx_crypto_cleanup)
                    {
                        status = public_cipher_method -> nx_crypto_cleanup(tls_session -> nx_secure_public_cipher_metadata_area);

                        if(status != NX_SUCCESS)
                        {
#ifdef NX_SECURE_KEY_CLEAR
                            NX_SECURE_MEMSET(_nx_secure_client_padded_pre_master, 0, sizeof(_nx_secure_client_padded_pre_master));
#endif /* NX_SECURE_KEY_CLEAR  */

                            return(status);
                        }                                                     
                    }
                }

                /* Check padding - first 2 bytes should be 0x00, 0x02 for PKCS#1 padding. A 0x00 byte should immediately
                   precede the data. */
                if (_nx_secure_client_padded_pre_master[0] != 0x00 ||
                    _nx_secure_client_padded_pre_master[1] != 0x02 ||
                    _nx_secure_client_padded_pre_master[length - NX_SECURE_TLS_PREMASTER_SIZE - 1] != 0x00)
                {

                    /* Invalid padding.  To avoid Bleichenbacher's attack, use random numbers to 
                       generate premaster secret and continue the operation (which will be properly 
                       taken care of later in _nx_secure_tls_process_finished()).  
                       
                       This is described in RFC 5246, section 7.4.7.1, page 58-59. */

                    /* Generate premaster secret using random numbers. */
                    for (i = 0; i < NX_SECURE_TLS_PREMASTER_SIZE; ++i)
                    {

                        /* PKCS#1 padding must be random, but CANNOT be 0. */
                        do
                        {
                            rand_byte = (UCHAR)NX_RAND();
                        } while (rand_byte == 0);
                        tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret[i] = rand_byte;
                    }
                }
                else
                {

                    /* Extract the 48 bytes of the actual pre-master secret from the data we just decrypted, stripping the padding, which
                       comes at the beginning of the decrypted block (the pre-master secret is the last 48 bytes. */
                    NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret, 
                           &_nx_secure_client_padded_pre_master[length - NX_SECURE_TLS_PREMASTER_SIZE], NX_SECURE_TLS_PREMASTER_SIZE);
                }
                tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret_size = NX_SECURE_TLS_PREMASTER_SIZE;
            }   /* End RSA-specific section. */
            else
            {
                /* Unknown or invalid public cipher. */
                return(NX_SECURE_TLS_UNSUPPORTED_PUBLIC_CIPHER);
            }
        }
    }
#ifdef NX_SECURE_KEY_CLEAR
    NX_SECURE_MEMSET(_nx_secure_client_padded_pre_master, 0, sizeof(_nx_secure_client_padded_pre_master));
#endif /* NX_SECURE_KEY_CLEAR  */

#ifdef NX_SECURE_TLS_SERVER_DISABLED
    /* If TLS Server is disabled and we have processed a ClientKeyExchange, something is wrong... */
    tls_session -> nx_secure_tls_client_state = NX_SECURE_TLS_CLIENT_STATE_ERROR;
    return(NX_SECURE_TLS_INVALID_STATE);
#else
    return(NX_SUCCESS);
#endif
}

