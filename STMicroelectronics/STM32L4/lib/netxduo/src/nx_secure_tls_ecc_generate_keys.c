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
#ifdef NX_SECURE_ENABLE_DTLS
#include "nx_secure_dtls.h"
#endif /* NX_SECURE_ENABLE_DTLS */

#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE

static UCHAR hash[64]; /* We concatenate MD5 and SHA-1 hashes into this buffer, OR SHA-256, SHA-384, SHA512. */
static UCHAR _nx_secure_padded_signature[512];
/* DER encodings (with OIDs for common algorithms) from RFC 8017.
 * NOTE: This is the equivalent DER-encoding for the value "T" described in RFC 8017 section 9.2. */
static const UCHAR _NX_CRYPTO_DER_OID_MD5[]         =  {0x30, 0x20, 0x30, 0x0c, 0x06, 0x08, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x02, 0x02, 0x05, 0x00, 0x04, 0x10};
static const UCHAR _NX_CRYPTO_DER_OID_SHA_1[]       =  {0x30, 0x21, 0x30, 0x09, 0x06, 0x05, 0x2b, 0x0e, 0x03, 0x02, 0x1a, 0x05, 0x00, 0x04, 0x14};
static const UCHAR _NX_CRYPTO_DER_OID_SHA_224[]     =  {0x30, 0x2d, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x04, 0x05, 0x00, 0x04, 0x1c};
static const UCHAR _NX_CRYPTO_DER_OID_SHA_256[]     =  {0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0x04, 0x20};
static const UCHAR _NX_CRYPTO_DER_OID_SHA_384[]     =  {0x30, 0x41, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x02, 0x05, 0x00, 0x04, 0x30};
static const UCHAR _NX_CRYPTO_DER_OID_SHA_512[]     =  {0x30, 0x51, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x03, 0x05, 0x00, 0x04, 0x40};



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_ecc_generate_keys                    PORTABLE C      */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is used to generate ECC key pairs (public, private)   */
/*    for use in TLS. TLS ECC keys need to be signed using a trusted      */
/*    certificate or other mechanism - this function handles the signature*/
/*    generation and outputs the public key and it's signature in the     */
/*    proper over-the-wire format for TLS. The private key is also        */
/*    exported (conditionally) for later use (usually when used to        */
/*    calculate the shared secret.                                        */
/*                                                                        */
/*    NOTE: The key sizes should contain the size of their respective     */
/*          buffers as input. The value will be replaced with the actual  */
/*          size of the generated key.                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    ecc_named_curve                       IANA ECC curve identifier     */
/*    sign_key                              True/False generate signature */
/*    public_key                            Signed ECC public key         */
/*    public_key_size                       Size of public key            */
/*    ecc_data                              ECC data (incl. private key)  */
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
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_ecc_generate_keys(NX_SECURE_TLS_SESSION *tls_session, UINT ecc_named_curve, USHORT sign_key,
                                      UCHAR *public_key, UINT *public_key_size, NX_SECURE_TLS_ECDHE_HANDSHAKE_DATA *ecc_data)
{
UINT                                  length;
UINT                                  output_size;
UINT                                  status;
NX_CRYPTO_EXTENDED_OUTPUT extended_output;
USHORT                                signature_length;
UINT                                  signature_offset;
const UCHAR                          *der_encoding = NX_NULL;
UINT                                  der_encoding_length = 0;
UINT                                  hash_length;
const NX_CRYPTO_METHOD               *curve_method;
const NX_CRYPTO_METHOD               *curve_method_cert;
const NX_CRYPTO_METHOD               *ecdhe_method;
const NX_CRYPTO_METHOD               *hash_method;
const NX_CRYPTO_METHOD               *auth_method;
VOID                                 *handler = NX_NULL;
NX_SECURE_X509_CERT                  *certificate;
NX_SECURE_X509_CRYPTO                *crypto_methods;
NX_SECURE_EC_PRIVATE_KEY             *ec_privkey;
NX_SECURE_EC_PUBLIC_KEY              *ec_pubkey;
USHORT                                signature_algorithm_id;


#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    if(tls_session->nx_secure_tls_1_3)
    {
        ecdhe_method = tls_session->nx_secure_tls_crypto_table->nx_secure_tls_ecdhe_method;
    }
    else
#endif
    {
        if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
        {

            /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
            return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
        }

        /* Generate ECDHE key pair using ECDHE crypto method. */
        ecdhe_method = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_cipher;
    }

    /* Make sure we have a method to use. */
    if (ecdhe_method == NX_NULL || ecdhe_method -> nx_crypto_operation == NX_NULL)
    {
        return(NX_SECURE_TLS_MISSING_CRYPTO_ROUTINE);
    }

    /* Set the curve we are using. */
    ecc_data->nx_secure_tls_ecdhe_named_curve = ecc_named_curve;

    /* Find out which named curve the we are using. */
    status = _nx_secure_tls_find_curve_method(tls_session, (USHORT)ecc_named_curve, &curve_method);
    if(status != NX_SUCCESS)
    {
        return(status);
    }
    if (curve_method == NX_NULL)
    {
        return(NX_SECURE_TLS_UNSUPPORTED_ECC_CURVE);
    }


    if (ecdhe_method -> nx_crypto_init != NX_NULL)
    {
        status = ecdhe_method -> nx_crypto_init((NX_CRYPTO_METHOD*)ecdhe_method,
                                       NX_NULL,
                                       0,
                                       &handler,
                                       tls_session -> nx_secure_public_cipher_metadata_area,
                                       tls_session -> nx_secure_public_cipher_metadata_size);
        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    status = ecdhe_method -> nx_crypto_operation(NX_CRYPTO_EC_CURVE_SET, handler,
                                                 (NX_CRYPTO_METHOD*)ecdhe_method, NX_NULL, 0,
                                                 (UCHAR *)curve_method, sizeof(NX_CRYPTO_METHOD *), NX_NULL,
                                                 NX_NULL, 0,
                                                 tls_session -> nx_secure_public_cipher_metadata_area,
                                                 tls_session -> nx_secure_public_cipher_metadata_size,
                                                 NX_NULL, NX_NULL);
    if (status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }


    /* Start to fill the public key buffer. */
    length = 0;
    output_size = *public_key_size;
    *public_key_size = 0;

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    if(!tls_session->nx_secure_tls_1_3)
#endif
    {
        /* ECCurveType: named_curve (3). */
        public_key[length] = 3;
        length += 1;

        /* NamedCurve */
        public_key[length] = (UCHAR)((ecc_data -> nx_secure_tls_ecdhe_named_curve & 0xFF00) >> 8);
        public_key[length + 1] = (UCHAR)(ecc_data -> nx_secure_tls_ecdhe_named_curve & 0x00FF);
        length += 2;
    }


    /* Generate the key pair and output the public key. */
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    if(tls_session->nx_secure_tls_1_3)
    {
        extended_output.nx_crypto_extended_output_data = &public_key[length];
        extended_output.nx_crypto_extended_output_length_in_byte = output_size - length;
    }
    else
#endif
    {
        extended_output.nx_crypto_extended_output_data = &public_key[length + 1];
        extended_output.nx_crypto_extended_output_length_in_byte = output_size - (length + 1);
    }
    extended_output.nx_crypto_extended_output_actual_size = 0;
    status = ecdhe_method -> nx_crypto_operation(NX_CRYPTO_DH_SETUP, handler,
                                                 (NX_CRYPTO_METHOD*)ecdhe_method, NX_NULL, 0,
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

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    if(!tls_session->nx_secure_tls_1_3)
#endif
    {
        /* Put the length into the buffer before the key data. */
        public_key[length] = (UCHAR)extended_output.nx_crypto_extended_output_actual_size;
        length += 1;

    }

    length += (UINT)(extended_output.nx_crypto_extended_output_actual_size);

    /* Export the private key for later use. */
    extended_output.nx_crypto_extended_output_data = ecc_data -> nx_secure_tls_ecdhe_private_key;
    extended_output.nx_crypto_extended_output_length_in_byte =
        sizeof(ecc_data -> nx_secure_tls_ecdhe_private_key);
    extended_output.nx_crypto_extended_output_actual_size = 0;
    status = ecdhe_method -> nx_crypto_operation(NX_CRYPTO_DH_PRIVATE_KEY_EXPORT, handler,
                                                 (NX_CRYPTO_METHOD*)ecdhe_method, NX_NULL, 0,
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

    /* Set the private key length. */
    ecc_data -> nx_secure_tls_ecdhe_private_key_length = (USHORT)extended_output.nx_crypto_extended_output_actual_size;

    /* Cleanup the ECC crypto state. */
    if (ecdhe_method -> nx_crypto_cleanup)
    {
        status = ecdhe_method -> nx_crypto_cleanup(tls_session -> nx_secure_public_cipher_metadata_area);
        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    /* If signing the key, generate the signature now using the local device certificate (if available). */
    if(sign_key == NX_TRUE)
    {
        /* Get the local certificate. */
        if (tls_session -> nx_secure_tls_credentials.nx_secure_tls_active_certificate != NX_NULL)
        {
            certificate = tls_session -> nx_secure_tls_credentials.nx_secure_tls_active_certificate;
        }
        else
        {
            /* Get reference to local device certificate. NX_NULL is passed for name to get default entry. */
            status = _nx_secure_x509_local_device_certificate_get(&tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store,
                                                                  NX_NULL, &certificate);
            if (status != NX_SUCCESS)
            {
                certificate = NX_NULL;
            }
        }

        if (certificate == NX_NULL)
        {
            /* No certificate found, error! */
            return(NX_SECURE_TLS_CERTIFICATE_NOT_FOUND);
        }


        /* Find out the hash algorithm used for the signature. */
        /* Map signature algorithm to internal ID. */
        _nx_secure_tls_get_signature_algorithm_id((UINT)(ecc_data -> nx_secure_tls_ecdhe_signature_algorithm),
                                                  &signature_algorithm_id);

        /* Get the crypto method. */
        status = _nx_secure_x509_find_certificate_methods(certificate,
                                                          signature_algorithm_id,
                                                          &crypto_methods);
        if (status)
        {
            return(NX_SECURE_TLS_UNSUPPORTED_SIGNATURE_ALGORITHM);
        }

#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
#ifdef NX_SECURE_ENABLE_DTLS
        if ((ecc_data -> nx_secure_tls_ecdhe_signature_algorithm & 0xFF) == NX_SECURE_TLS_SIGNATURE_ALGORITHM_RSA &&
           (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_0 ||
            tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_1 ||
            tls_session -> nx_secure_tls_protocol_version == NX_SECURE_DTLS_VERSION_1_0))
#else
        if ((ecc_data -> nx_secure_tls_ecdhe_signature_algorithm & 0xFF) == NX_SECURE_TLS_SIGNATURE_ALGORITHM_RSA &&
           (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_0 ||
            tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_1))
#endif /* NX_SECURE_ENABLE_DTLS */
        {

            /* TLS 1.0 and TLS 1.1 use MD5 + SHA1 hash for RSA signatures. */
            hash_method = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_handshake_hash_md5_method;
        }
        else
#endif /* NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED */
        {
            hash_method = crypto_methods -> nx_secure_x509_hash_method;
        }

        hash_length = hash_method -> nx_crypto_ICV_size_in_bits >> 3;

        /* Calculate the hash: SHA(ClientHello.random + ServerHello.random +
                                    ServerKeyExchange.params); */
        if (hash_method -> nx_crypto_init)
        {
            status = hash_method -> nx_crypto_init((NX_CRYPTO_METHOD*)hash_method,
                                          NX_NULL,
                                          0,
                                          &handler,
                                          tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch,
                                          tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch_size);

            if(status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }
        }

        if (hash_method -> nx_crypto_operation != NX_NULL)
        {
            status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_INITIALIZE,
                                               handler,
                                               (NX_CRYPTO_METHOD*)hash_method,
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

            if(status != NX_CRYPTO_SUCCESS)
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
                                           (NX_CRYPTO_METHOD*)hash_method,
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

        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                           handler,
                                           (NX_CRYPTO_METHOD*)hash_method,
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

        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                           handler,
                                           (NX_CRYPTO_METHOD*)hash_method,
                                           NX_NULL,
                                           0,
                                           public_key,
                                           length,
                                           NX_NULL,
                                           NX_NULL,
                                           0,
                                           tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch,
                                           tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch_size,
                                           NX_NULL,
                                           NX_NULL);

        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_CALCULATE,
                                           handler,
                                           (NX_CRYPTO_METHOD*)hash_method,
                                           NX_NULL,
                                           0,
                                           NX_NULL,
                                           0,
                                           NX_NULL,
                                           hash,
                                           hash_length,
                                           tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch,
                                           tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch_size,
                                           NX_NULL,
                                           NX_NULL);

        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        if (hash_method -> nx_crypto_cleanup)
        {
            status = hash_method -> nx_crypto_cleanup(tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch);

            if(status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }
        }
        handler = NX_NULL;

#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
#ifdef NX_SECURE_ENABLE_DTLS
        if ((ecc_data -> nx_secure_tls_ecdhe_signature_algorithm & 0xFF) == NX_SECURE_TLS_SIGNATURE_ALGORITHM_RSA &&
           (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_0 ||
            tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_1 ||
            tls_session -> nx_secure_tls_protocol_version == NX_SECURE_DTLS_VERSION_1_0))
#else
        if ((ecc_data -> nx_secure_tls_ecdhe_signature_algorithm & 0xFF) == NX_SECURE_TLS_SIGNATURE_ALGORITHM_RSA &&
           (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_0 ||
            tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_1))
#endif /* NX_SECURE_ENABLE_DTLS */
        {
            hash_method = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_handshake_hash_sha1_method;;

            /* Calculate the hash: SHA(ClientHello.random + ServerHello.random +
                                       ServerKeyExchange.params); */
            if (hash_method -> nx_crypto_init)
            {
                status = hash_method -> nx_crypto_init((NX_CRYPTO_METHOD*)hash_method,
                                              NX_NULL,
                                              0,
                                              &handler,
                                              tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch,
                                              tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch_size);

                if(status != NX_CRYPTO_SUCCESS)
                {
                    return(status);
                }
            }

            if (hash_method -> nx_crypto_operation != NX_NULL)
            {
                status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_INITIALIZE,
                                                   handler,
                                                   (NX_CRYPTO_METHOD*)hash_method,
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

                if(status != NX_CRYPTO_SUCCESS)
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
                                               (NX_CRYPTO_METHOD*)hash_method,
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

            if(status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }

            status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                               handler,
                                               (NX_CRYPTO_METHOD*)hash_method,
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

            if(status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }

            status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                               handler,
                                               (NX_CRYPTO_METHOD*)hash_method,
                                               NX_NULL,
                                               0,
                                               public_key,
                                               length,
                                               NX_NULL,
                                               NX_NULL,
                                               0,
                                               tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch,
                                               tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch_size,
                                               NX_NULL,
                                               NX_NULL);

            if(status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }

            status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_CALCULATE,
                                               handler,
                                               (NX_CRYPTO_METHOD*)hash_method,
                                               NX_NULL,
                                               0,
                                               NX_NULL,
                                               0,
                                               NX_NULL,
                                               &hash[16],
                                               hash_method -> nx_crypto_ICV_size_in_bits >> 3,
                                               tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch,
                                               tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch_size,
                                               NX_NULL,
                                               NX_NULL);

            if(status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }

            if (hash_method -> nx_crypto_cleanup)
            {
                status = hash_method -> nx_crypto_cleanup(tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch);

                if(status != NX_CRYPTO_SUCCESS)
                {
                    return(status);
                }
            }
            handler = NX_NULL;
        }
#endif /* NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED */

#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
#ifdef NX_SECURE_ENABLE_DTLS
        if (tls_session -> nx_secure_tls_protocol_version != NX_SECURE_TLS_VERSION_TLS_1_0 &&
            tls_session -> nx_secure_tls_protocol_version != NX_SECURE_TLS_VERSION_TLS_1_1 &&
            tls_session -> nx_secure_tls_protocol_version != NX_SECURE_DTLS_VERSION_1_0)
#else
        if (tls_session -> nx_secure_tls_protocol_version != NX_SECURE_TLS_VERSION_TLS_1_0 &&
            tls_session -> nx_secure_tls_protocol_version != NX_SECURE_TLS_VERSION_TLS_1_1)
#endif /* NX_SECURE_ENABLE_DTLS */
#endif /* NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED */
        {

            /* Signature Hash Algorithm. */
            public_key[length] = (UCHAR)((ecc_data -> nx_secure_tls_ecdhe_signature_algorithm & 0xFF00) >> 8);
            public_key[length + 1] = (UCHAR)(ecc_data -> nx_secure_tls_ecdhe_signature_algorithm & 0x00FF);
            length += 2;
        }

        /* Sign the hash. */
        auth_method = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_auth;
        if ((ecc_data -> nx_secure_tls_ecdhe_signature_algorithm & 0xFF) == NX_SECURE_TLS_SIGNATURE_ALGORITHM_RSA &&
            (auth_method -> nx_crypto_algorithm == NX_CRYPTO_DIGITAL_SIGNATURE_RSA ||
            auth_method -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_RSA))
        {
            signature_length = certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_modulus_length;

            /* Signature Length */
            public_key[length] = (UCHAR)((signature_length & 0xFF00) >> 8);
            public_key[length + 1] = (UCHAR)(signature_length & 0x00FF);
            length += 2;

#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
#ifdef NX_SECURE_ENABLE_DTLS
            if ((ecc_data -> nx_secure_tls_ecdhe_signature_algorithm & 0xFF) == NX_SECURE_TLS_SIGNATURE_ALGORITHM_RSA &&
               (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_0 ||
                tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_1 ||
                tls_session -> nx_secure_tls_protocol_version == NX_SECURE_DTLS_VERSION_1_0))
#else
            if ((ecc_data -> nx_secure_tls_ecdhe_signature_algorithm & 0xFF) == NX_SECURE_TLS_SIGNATURE_ALGORITHM_RSA &&
               (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_0 ||
                tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_1))
#endif /* NX_SECURE_ENABLE_DTLS */
            {
                hash_length += hash_method -> nx_crypto_ICV_size_in_bits >> 3;
                der_encoding_length = 0;
            }
            else
#endif /* NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED */
            {
                switch ((ecc_data -> nx_secure_tls_ecdhe_signature_algorithm & 0xFF00) >> 8)
                {
                case NX_SECURE_TLS_HASH_ALGORITHM_MD5:
                    der_encoding = _NX_CRYPTO_DER_OID_MD5;
                    der_encoding_length = sizeof(_NX_CRYPTO_DER_OID_MD5);
                    break;
                case NX_SECURE_TLS_HASH_ALGORITHM_SHA1:
                    der_encoding = _NX_CRYPTO_DER_OID_SHA_1;
                    der_encoding_length = sizeof(_NX_CRYPTO_DER_OID_SHA_1);
                    break;
                case NX_SECURE_TLS_HASH_ALGORITHM_SHA224:
                    der_encoding = _NX_CRYPTO_DER_OID_SHA_224;
                    der_encoding_length = sizeof(_NX_CRYPTO_DER_OID_SHA_224);
                    break;
                case NX_SECURE_TLS_HASH_ALGORITHM_SHA256:
                    der_encoding = _NX_CRYPTO_DER_OID_SHA_256;
                    der_encoding_length = sizeof(_NX_CRYPTO_DER_OID_SHA_256);
                    break;
                case NX_SECURE_TLS_HASH_ALGORITHM_SHA384:
                    der_encoding = _NX_CRYPTO_DER_OID_SHA_384;
                    der_encoding_length = sizeof(_NX_CRYPTO_DER_OID_SHA_384);
                    break;
                case NX_SECURE_TLS_HASH_ALGORITHM_SHA512:
                    der_encoding = _NX_CRYPTO_DER_OID_SHA_512;
                    der_encoding_length = sizeof(_NX_CRYPTO_DER_OID_SHA_512);
                    break;
                default:
                    return(NX_SECURE_TLS_UNSUPPORTED_SIGNATURE_ALGORITHM);
                }
            }

            /* Build the RSA signature. */
            /* C-STAT: If signature_length is ever exactly equal to (der_encoding_length + hash_length)
                       then signature_offset will be 0 and the (signature_offset - 1) expression below
                       would result in a negative array subscript. Thus, also check for equality in the
                       second condition (a zero-length signature offset). */
            signature_offset = signature_length - (der_encoding_length + hash_length);
            if ((signature_offset > sizeof(_nx_secure_padded_signature)) ||
                (signature_length > sizeof(_nx_secure_padded_signature)) ||
                (signature_offset == 0))
            {

                /* Buffer too small. */
                return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
            }
            NX_CRYPTO_MEMSET(_nx_secure_padded_signature, 0xff, signature_offset);
            _nx_secure_padded_signature[0] = 0x0;
            _nx_secure_padded_signature[1] = 0x1;
            _nx_secure_padded_signature[signature_offset - 1] = 0x0;
            if (der_encoding_length > 0)
            {
                NX_CRYPTO_MEMCPY(&_nx_secure_padded_signature[signature_offset], der_encoding, der_encoding_length);
                signature_offset += der_encoding_length;
            }
            NX_CRYPTO_MEMCPY(&_nx_secure_padded_signature[signature_offset], hash, hash_length);

            if (auth_method -> nx_crypto_init != NX_NULL)
            {
                /* Initialize the crypto method with public key. */
                status = auth_method -> nx_crypto_init((NX_CRYPTO_METHOD*)auth_method,
                                                       (UCHAR *)certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_modulus,
                                                       (NX_CRYPTO_KEY_SIZE)(certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_modulus_length << 3),
                                                       &handler,
                                                       tls_session -> nx_secure_public_auth_metadata_area,
                                                       tls_session -> nx_secure_public_auth_metadata_size);
                if (status != NX_CRYPTO_SUCCESS)
                {
                    return(status);
                }
            }

            if (auth_method -> nx_crypto_operation != NX_NULL)
            {
                /* Sign the hash we just generated using our local RSA private key (associated with our local cert). */
                status = auth_method -> nx_crypto_operation(NX_CRYPTO_DECRYPT,
                                                            handler,
                                                            (NX_CRYPTO_METHOD*)auth_method,
                                                            (UCHAR *)certificate -> nx_secure_x509_private_key.rsa_private_key.nx_secure_rsa_private_exponent,
                                                            (NX_CRYPTO_KEY_SIZE)(certificate -> nx_secure_x509_private_key.rsa_private_key.nx_secure_rsa_private_exponent_length << 3),
                                                            _nx_secure_padded_signature,
                                                            signature_length,
                                                            NX_NULL,
                                                            &public_key[length],
                                                            signature_length,
                                                            tls_session -> nx_secure_public_auth_metadata_area,
                                                            tls_session -> nx_secure_public_auth_metadata_size,
                                                            NX_NULL, NX_NULL);
                if (status != NX_CRYPTO_SUCCESS)
                {
                    return(status);
                }
            }

            length += signature_length;

            if (auth_method -> nx_crypto_cleanup)
            {
                status = auth_method -> nx_crypto_cleanup(tls_session -> nx_secure_public_auth_metadata_area);

                if(status != NX_CRYPTO_SUCCESS)
                {
                    return(status);
                }
            }
        }
        else if ((ecc_data -> nx_secure_tls_ecdhe_signature_algorithm & 0xFF) == NX_SECURE_TLS_SIGNATURE_ALGORITHM_ECDSA &&
                 auth_method -> nx_crypto_algorithm == NX_CRYPTO_DIGITAL_SIGNATURE_ECDSA)
        {
            ec_privkey = &certificate -> nx_secure_x509_private_key.ec_private_key;
            ec_pubkey = &certificate -> nx_secure_x509_public_key.ec_public_key;

            /* Find out which named curve the local certificate is using. */
            status = _nx_secure_tls_find_curve_method(tls_session, (USHORT)(ec_privkey -> nx_secure_ec_named_curve), &curve_method_cert);
            if(status != NX_SUCCESS)
            {
                return(status);
            }
            if (curve_method_cert == NX_NULL)
            {

                /* The local certificate is using an unsupported curve. */
                return(NX_SECURE_TLS_UNSUPPORTED_ECC_CURVE);
            }

            if (auth_method -> nx_crypto_init != NX_NULL)
            {
                status = auth_method -> nx_crypto_init((NX_CRYPTO_METHOD*)auth_method,
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
                                                        (NX_CRYPTO_METHOD*)auth_method, NX_NULL, 0,
                                                        (UCHAR *)curve_method_cert, sizeof(NX_CRYPTO_METHOD *), NX_NULL,
                                                        NX_NULL, 0,
                                                        tls_session -> nx_secure_public_auth_metadata_area,
                                                        tls_session -> nx_secure_public_auth_metadata_size,
                                                        NX_NULL, NX_NULL);
            if (status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }

            /* Generate the signature and put it in the packet. */
            extended_output.nx_crypto_extended_output_data = &public_key[length + 2];
            extended_output.nx_crypto_extended_output_length_in_byte = output_size - length;
            extended_output.nx_crypto_extended_output_actual_size = 0;
            status = auth_method -> nx_crypto_operation(NX_CRYPTO_AUTHENTICATE, handler,
                                                        (NX_CRYPTO_METHOD*)auth_method,
                                                        (UCHAR *)ec_privkey -> nx_secure_ec_private_key,
                                                        (NX_CRYPTO_KEY_SIZE)(ec_privkey -> nx_secure_ec_private_key_length << 3),
                                                        hash,
                                                        hash_method -> nx_crypto_ICV_size_in_bits >> 3, NX_NULL,
                                                        (UCHAR *)&extended_output,
                                                        sizeof(extended_output),
                                                        tls_session -> nx_secure_public_auth_metadata_area,
                                                        tls_session -> nx_secure_public_auth_metadata_size,
                                                        NX_NULL, NX_NULL);
            if (status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }

            if (auth_method -> nx_crypto_cleanup)
            {
                status = auth_method -> nx_crypto_cleanup(tls_session -> nx_secure_public_auth_metadata_area);

                if(status != NX_CRYPTO_SUCCESS)
                {
                    return(status);
                }
            }

            /* Signature Length */
            public_key[length] = (UCHAR)((extended_output.nx_crypto_extended_output_actual_size & 0xFF00) >> 8);
            public_key[length + 1] = (UCHAR)(extended_output.nx_crypto_extended_output_actual_size & 0x00FF);

            length += extended_output.nx_crypto_extended_output_actual_size + 2;
        }
        else
        {
            /* The signature algorithm is not supported. */
            *public_key_size = 0;
            return(NX_SECURE_TLS_UNSUPPORTED_SIGNATURE_ALGORITHM);
        }
    }

    /* Return the length of our generated data. */
    *public_key_size = length;

    return(NX_SUCCESS);
}
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */


