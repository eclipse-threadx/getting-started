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

#ifndef NX_SECURE_TLS_CLIENT_DISABLED

static UCHAR hash[16 + 20]; /* We concatenate MD5 and SHA-1 hashes into this buffer, OR SHA-256. */
static UCHAR decrypted_signature[512];

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_ecc_process_server_key_exchange      PORTABLE C      */
/*                                                          5.12          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an incoming ServerKeyExchange message,      */
/*    which is sent by the remote TLS Server host when the ECDHE_ECDSA,   */
/*    ECDHE_RSA key exchange algorithms are used.                         */
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
/*    _nx_secure_tls_find_curve_method      Find named curve used         */
/*    _nx_secure_x509_remote_endpoint_certificate_get                     */
/*                                          Get cert for remote host      */
/*    _nx_secure_x509_find_certificate_methods                            */
/*                                          Find crypto methods for       */
/*                                            signature hashing           */
/*    [nx_crypto_operation]                 Crypto operation              */
/*    _nx_secure_x509_pkcs7_decode          Decode the PKCS#7 signature   */
/*    _nx_secure_x509_asn1_tlv_block_parse  Parse ASN.1 block             */
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
/*  08-15-2019     Timothy Stapko           Initial Version 5.12          */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_ecc_process_server_key_exchange(struct NX_SECURE_TLS_SESSION_STRUCT *tls_session,
                                                    UCHAR *packet_buffer, UINT message_length)
{
UINT                                  status;
NX_CRYPTO_METHOD                     *curve_method;
NX_CRYPTO_METHOD                     *curve_method_cert;
NX_CRYPTO_METHOD                     *ecdhe_method;
NX_CRYPTO_METHOD                     *hash_method;
NX_CRYPTO_METHOD                     *auth_method;
VOID                                 *handler = NX_NULL;
UCHAR                                 pubkey_length;
UCHAR                                *pubkey;
USHORT                                signature_length;
NX_SECURE_X509_CERT                  *server_certificate;
NX_SECURE_X509_CRYPTO                *crypto_methods;
UINT                                  sig_oid_length;
const UCHAR                          *sig_oid;
UINT                                  decrypted_hash_length;
const UCHAR                          *decrypted_hash;
UINT                                  compare_result;
NX_SECURE_EC_PUBLIC_KEY              *ec_pubkey;
NX_CRYPTO_EXTENDED_OUTPUT             extended_output;

    NX_PARAMETER_NOT_USED(message_length);

    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {

        /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    /* FIXME: Add support for tls version 1.0 and 1.1. */

    if (tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_cipher -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECDHE)
    {
        tls_session -> nx_secure_tls_client_state = NX_SECURE_TLS_CLIENT_STATE_SERVER_KEY_EXCHANGE;

        /* Make sure curve type is named_curve (3). */
        if (packet_buffer[0] != 3)
        {
            return(NX_SECURE_TLS_UNSUPPORTED_ECC_FORMAT);
        }

        /* Find out which named curve the server is using. */
        status = _nx_secure_tls_find_curve_method(tls_session, (USHORT)((packet_buffer[1] << 8) + packet_buffer[2]), (VOID **)&curve_method);

        if(status != NX_SUCCESS)
        {
            return(status);
        }

        if (curve_method == NX_NULL)
        {
            /* The remote server is using an unsupported curve. */
            return(NX_SECURE_TLS_UNSUPPORTED_ECC_CURVE);
        }

        /* Get reference to remote server certificate so we can get the public key for signature verification. */
        status = _nx_secure_x509_remote_endpoint_certificate_get(&tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store,
                                                                 &server_certificate);
        if (status || server_certificate == NX_NULL)
        {
            /* No certificate found, error! */
            return(NX_SECURE_TLS_CERTIFICATE_NOT_FOUND);
        }

        pubkey_length = packet_buffer[3];
        pubkey = &packet_buffer[4];

        signature_length = (USHORT)((packet_buffer[6 + pubkey_length] << 8) + packet_buffer[7 + pubkey_length]);

        /* Find out the hash algorithm used for the signature. */
        switch (packet_buffer[4 + pubkey_length])
        {
        case NX_SECURE_TLS_HASH_ALGORITHM_SHA1:
            if (server_certificate -> nx_secure_x509_public_algorithm == NX_SECURE_TLS_X509_TYPE_EC)
            {
                status = _nx_secure_x509_find_certificate_methods(server_certificate,
                                                                  NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_1,
                                                                  &crypto_methods);
            }
            else
            {
                status = _nx_secure_x509_find_certificate_methods(server_certificate,
                                                                  NX_SECURE_TLS_X509_TYPE_RSA_SHA_1,
                                                                  &crypto_methods);
            }
            if (status)
            {
                return(status);
            }
            break;

        case NX_SECURE_TLS_HASH_ALGORITHM_SHA256:
            if (server_certificate -> nx_secure_x509_public_algorithm == NX_SECURE_TLS_X509_TYPE_EC)
            {
                status = _nx_secure_x509_find_certificate_methods(server_certificate,
                                                                  NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_256,
                                                                  &crypto_methods);
            }
            else
            {
                status = _nx_secure_x509_find_certificate_methods(server_certificate,
                                                                  NX_SECURE_TLS_X509_TYPE_RSA_SHA_256,
                                                                  &crypto_methods);
            }
            if (status)
            {
                return(status);
            }
            break;

        default:
            /* The hash algorithm is not supported. */
            return(NX_SECURE_TLS_UNSUPPORTED_SIGNATURE_ALGORITHM);
            break;
        }

        hash_method = crypto_methods -> nx_secure_x509_hash_method;

        /* Calculate the hash: SHA(ClientHello.random + ServerHello.random +
                                   ServerKeyExchange.params); */
        if (hash_method -> nx_crypto_init)
        {
            status = hash_method -> nx_crypto_init(hash_method,
                                          NX_NULL,
                                          0,
                                          &handler,
                                          tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch,
                                          tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch_size);

            if(status != NX_SUCCESS)
            {
                return(status);
            }                                                     
        }

        if (hash_method -> nx_crypto_operation != NX_NULL)
        {
            status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_INITIALIZE,
                                               handler,
                                               hash_method,
                                               NX_NULL,
                                               0,
                                               NX_NULL,
                                               0,
                                               NX_NULL,
                                               NX_NULL,
                                               0,
                                               tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch,
                                               tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch_size,
                                               NX_NULL,
                                               NX_NULL);

            if(status != NX_SUCCESS)
            {
                return(status);
            }                                                    
        }
        else
        {
            return(NX_SECURE_TLS_MISSING_CRYPTO_ROUTINE);
        }

        status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                           handler,
                                           hash_method,
                                           NX_NULL,
                                           0,
                                           tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_random,
                                           32,
                                           NX_NULL,
                                           NX_NULL,
                                           0,
                                           tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch,
                                           tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch_size,
                                           NX_NULL,
                                           NX_NULL);

        if(status != NX_SUCCESS)
        {
            return(status);
        }                                                     
    
        status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                           handler,
                                           hash_method,
                                           NX_NULL,
                                           0,
                                           tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random,
                                           32,
                                           NX_NULL,
                                           NX_NULL,
                                           0,
                                           tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch,
                                           tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch_size,
                                           NX_NULL,
                                           NX_NULL);

        if(status != NX_SUCCESS)
        {
            return(status);
        }                                                     
        
        status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                           handler,
                                           hash_method,
                                           NX_NULL,
                                           0,
                                           packet_buffer,
                                           (ULONG)(4 + pubkey_length),
                                           NX_NULL,
                                           NX_NULL,
                                           0,
                                           tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch,
                                           tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch_size,
                                           NX_NULL,
                                           NX_NULL);

        if(status != NX_SUCCESS)
        {
            return(status);
        }                                                     
   
        status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_CALCULATE,
                                           handler,
                                           hash_method,
                                           NX_NULL,
                                           0,
                                           NX_NULL,
                                           0,
                                           NX_NULL,
                                           hash,
                                           hash_method ->nx_crypto_ICV_size_in_bits >> 3,
                                           tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch,
                                           tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch_size,
                                           NX_NULL,
                                           NX_NULL);

        if(status != NX_SUCCESS)
        {
            return(status);
        }                                                     
        
        if (hash_method -> nx_crypto_cleanup)
        {
            status = hash_method -> nx_crypto_cleanup(tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch);

            if(status != NX_SUCCESS)
            {
                return(status);
            }                                                     
        }
        handler = NX_NULL;

        /* Verify the signature. */
        auth_method = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_auth;

        if (packet_buffer[5 + pubkey_length] == NX_SECURE_TLS_SIGNATURE_ALGORITHM_RSA &&
            (auth_method -> nx_crypto_algorithm == NX_CRYPTO_DIGITAL_SIGNATRUE_RSA ||
            auth_method -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_RSA))
        {
            /* Verify the RSA signature. */

            if (auth_method -> nx_crypto_init != NX_NULL)
            {
                /* Initialize the crypto method with public key. */
                status = auth_method -> nx_crypto_init(auth_method,
                                                       (UCHAR *)server_certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_modulus,
                                                       (NX_CRYPTO_KEY_SIZE)(server_certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_modulus_length << 3),
                                                       &handler,
                                                       tls_session -> nx_secure_public_auth_metadata_area,
                                                       tls_session -> nx_secure_public_auth_metadata_size);
            }

            if (auth_method -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_RSA)
            {
                status = auth_method -> nx_crypto_operation(NX_CRYPTO_DECRYPT,
                                                            handler,
                                                            auth_method,
                                                            (UCHAR *)server_certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_exponent,
                                                            (NX_CRYPTO_KEY_SIZE)(server_certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_exponent_length << 3),
                                                            &packet_buffer[8 + pubkey_length],
                                                            signature_length,
                                                            NX_NULL,
                                                            decrypted_signature,
                                                            sizeof(decrypted_signature),
                                                            tls_session -> nx_secure_public_auth_metadata_area,
                                                            tls_session -> nx_secure_public_auth_metadata_size,
                                                            NX_NULL, NX_NULL);

            }

            if (auth_method -> nx_crypto_cleanup)
            {
                status = auth_method -> nx_crypto_cleanup(tls_session -> nx_secure_public_auth_metadata_area);
                if(status != NX_SUCCESS)
                {
                    return(status);
                }
            }
            handler = NX_NULL;

            /* Decode the decrypted signature. */
            status = _nx_secure_x509_pkcs7_decode(decrypted_signature, signature_length, &sig_oid, &sig_oid_length,
                                                  &decrypted_hash, &decrypted_hash_length);
            if (status != NX_SUCCESS)
            {
                return(NX_SECURE_TLS_SIGNATURE_VERIFICATION_ERROR);
            }

            if (decrypted_hash_length != (hash_method -> nx_crypto_ICV_size_in_bits >> 3))
            {
                return(NX_SECURE_TLS_SIGNATURE_VERIFICATION_ERROR);
            }

            /* Compare generated hash with decrypted hash. */
            compare_result = (UINT)NX_SECURE_MEMCMP(hash, decrypted_hash, decrypted_hash_length);

#ifdef NX_SECURE_KEY_CLEAR
            NX_SECURE_MEMSET(hash, 0, sizeof(hash));
            NX_SECURE_MEMSET(decrypted_signature, 0, sizeof(decrypted_signature));
#endif /* NX_SECURE_KEY_CLEAR  */

            if (compare_result != 0)
            {
                return(NX_SECURE_TLS_SIGNATURE_VERIFICATION_ERROR);
            }
        }
        else if (packet_buffer[5 + pubkey_length] == NX_SECURE_TLS_SIGNATURE_ALGORITHM_ECDSA &&
                 auth_method -> nx_crypto_algorithm == NX_CRYPTO_DIGITAL_SIGNATRUE_ECDSA)
        {
            /* Verify the ECDSA signature. */

            ec_pubkey = (NX_SECURE_EC_PUBLIC_KEY *)&server_certificate -> nx_secure_x509_public_key;

            /* Find out which named curve the remote certificate is using. */
            status = _nx_secure_tls_find_curve_method(tls_session, (USHORT)(ec_pubkey -> nx_secure_ec_named_curve), (VOID **)&curve_method_cert);

            if(status != NX_SUCCESS)
            {
                return(status);
            }

            if (curve_method_cert == NX_NULL)
            {
                /* The remote certificate is using an unsupported curve. */
                return(NX_SECURE_TLS_UNSUPPORTED_ECC_CURVE);
            }

            if (auth_method -> nx_crypto_init != NX_NULL)
            {
                status = auth_method -> nx_crypto_init(auth_method,
                                                       (UCHAR *)ec_pubkey -> nx_secure_ec_public_key,
                                                       (NX_CRYPTO_KEY_SIZE)(ec_pubkey -> nx_secure_ec_public_key_length << 3),
                                                       &handler,
                                                       tls_session -> nx_secure_public_auth_metadata_area,
                                                       tls_session -> nx_secure_public_auth_metadata_size);
                if (status != NX_CRYPTO_SUCCESS)
                {
                    return(status);
                }
            }
            if (auth_method -> nx_crypto_operation == NX_NULL)
            {
                return(NX_SECURE_TLS_MISSING_CRYPTO_ROUTINE);
            }

            status = auth_method -> nx_crypto_operation(NX_CRYPTO_EC_CURVE_SET, handler,
                                                        auth_method, NX_NULL, 0,
                                                        (UCHAR *)curve_method_cert, sizeof(NX_CRYPTO_METHOD *), NX_NULL,
                                                        NX_NULL, 0,
                                                        tls_session -> nx_secure_public_auth_metadata_area,
                                                        tls_session -> nx_secure_public_auth_metadata_size,
                                                        NX_NULL, NX_NULL);
            if (status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }

            if (message_length < ((UINT)signature_length + pubkey_length + 8))
            {
                return(NX_SECURE_TLS_SIGNATURE_VERIFICATION_ERROR);
            }

            status = auth_method -> nx_crypto_operation(NX_CRYPTO_VERIFY, handler,
                                                        auth_method,
                                                        (UCHAR *)ec_pubkey -> nx_secure_ec_public_key,
                                                        (NX_CRYPTO_KEY_SIZE)(ec_pubkey -> nx_secure_ec_public_key_length << 3),
                                                        hash,
                                                        hash_method ->nx_crypto_ICV_size_in_bits >> 3,
                                                        NX_NULL,
                                                        &packet_buffer[8 + pubkey_length],
                                                        signature_length,
                                                        tls_session -> nx_secure_public_auth_metadata_area,
                                                        tls_session -> nx_secure_public_auth_metadata_size,
                                                        NX_NULL, NX_NULL);

            if (status == NX_CRYPTO_AUTHENTICATION_FAILED)
            {
                return(NX_SECURE_TLS_SIGNATURE_VERIFICATION_ERROR);
            }

            if (status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }

            if (auth_method -> nx_crypto_cleanup)
            {
                status = auth_method -> nx_crypto_cleanup(tls_session -> nx_secure_public_auth_metadata_area);
                if(status != NX_SUCCESS)
                {
                    return(status);
                }
            }
        }
        else
        {
            /* The signature hash algorithm used by the server is not supported. */
            return(NX_SECURE_TLS_UNSUPPORTED_SIGNATURE_ALGORITHM);
        }

        ecdhe_method = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_cipher;
        if (ecdhe_method -> nx_crypto_operation == NX_NULL)
        {
            return(NX_SECURE_TLS_MISSING_CRYPTO_ROUTINE);
        }

        if (ecdhe_method -> nx_crypto_init != NX_NULL)
        {
            status = ecdhe_method -> nx_crypto_init(ecdhe_method,
                                           NX_NULL,
                                           0,
                                           &handler,
                                           tls_session -> nx_secure_public_cipher_metadata_area,
                                           tls_session -> nx_secure_public_cipher_metadata_size);

            if(status != NX_SUCCESS)
            {
                return(status);
            }                                                     
        }

        status = ecdhe_method -> nx_crypto_operation(NX_CRYPTO_EC_CURVE_SET, handler,
                                                     ecdhe_method, NX_NULL, 0,
                                                     (UCHAR *)curve_method, sizeof(NX_CRYPTO_METHOD *), NX_NULL,
                                                     NX_NULL, 0,
                                                     tls_session -> nx_secure_public_cipher_metadata_area,
                                                     tls_session -> nx_secure_public_cipher_metadata_size,
                                                     NX_NULL, NX_NULL);
        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        /* Store public key in the nx_secure_tls_new_key_material_data. */
        extended_output.nx_crypto_extended_output_data = &tls_session -> nx_secure_tls_key_material.nx_secure_tls_new_key_material_data[1];
        extended_output.nx_crypto_extended_output_length_in_byte = sizeof(tls_session -> nx_secure_tls_key_material.nx_secure_tls_new_key_material_data) - 1;
        extended_output.nx_crypto_extended_output_actual_size = 0;
        status = ecdhe_method -> nx_crypto_operation(NX_CRYPTO_DH_SETUP, handler,
                                                     ecdhe_method, NX_NULL, 0,
                                                     NX_NULL, 0, NX_NULL,
                                                     (UCHAR *)&extended_output,
                                                     sizeof(extended_output),
                                                     tls_session -> nx_secure_public_cipher_metadata_area,
                                                     tls_session -> nx_secure_public_cipher_metadata_size,
                                                     NX_NULL, NX_NULL);
        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        tls_session -> nx_secure_tls_key_material.nx_secure_tls_new_key_material_data[0] = (UCHAR)extended_output.nx_crypto_extended_output_actual_size;

        extended_output.nx_crypto_extended_output_data = tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret;
        extended_output.nx_crypto_extended_output_length_in_byte = sizeof(tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret);
        extended_output.nx_crypto_extended_output_actual_size = 0;
        status = ecdhe_method -> nx_crypto_operation(NX_CRYPTO_DH_CALCULATE, handler,
                                                     ecdhe_method, NX_NULL, 0,
                                                     pubkey, pubkey_length, NX_NULL,
                                                     (UCHAR *)&extended_output,
                                                     sizeof(extended_output),
                                                     tls_session -> nx_secure_public_cipher_metadata_area,
                                                     tls_session -> nx_secure_public_cipher_metadata_size,
                                                     NX_NULL, NX_NULL);
        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret_size = extended_output.nx_crypto_extended_output_actual_size;

        if (ecdhe_method -> nx_crypto_cleanup)
        {
            status = ecdhe_method -> nx_crypto_cleanup(tls_session -> nx_secure_public_cipher_metadata_area);

            if(status != NX_SUCCESS)
            {
                return(status);
            }                                                     
        }
    }

    return(NX_SUCCESS);
}



#endif /* NX_SECURE_TLS_CLIENT_DISABLED */

