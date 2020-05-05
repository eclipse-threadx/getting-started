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
#ifdef NX_SECURE_ENABLE_DTLS
#include "nx_secure_dtls.h"
#endif /* NX_SECURE_ENABLE_DTLS */

static UCHAR handshake_hash[16 + 20]; /* We concatenate MD5 and SHA-1 hashes into this buffer, OR SHA-256. */
static UCHAR _nx_secure_decrypted_signature[600];

#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
static const UCHAR _NX_SECURE_OID_SHA256[] = {0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0x04, 0x20};
#endif

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_process_certificate_verify           PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an incoming CertificateVerify message,      */
/*    which is sent by the remote client as a response to a               */
/*    CertificateRequest message sent by this TLS server.                 */
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
/*    [nx_crypto_operation]                 Public-key operation (eg RSA) */
/*                                            used to verify keys         */
/*    [nx_crypto_init]                      Initialize the public-key     */
/*                                            operation                   */
/*    _nx_secure_x509_local_device_certificate_get                        */
/*                                          Get the local certificate     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*  08-15-2019     Timothy Stapko           Modified comment(s), and      */
/*                                            added flexibility of using  */
/*                                            macros instead of direct C  */
/*                                            library function calls,     */
/*                                            added extension hook, added */
/*                                            logic to clear encryption   */
/*                                            key and other secret data,  */
/*                                            passed crypto handle into   */
/*                                            crypto internal functions,  */
/*                                            fix issues with proper      */
/*                                            CertificateVerify handling, */
/*                                            update buffer for 4096 RSA, */
/*                                            fixed compiler warnings,    */
/*                                            updated error return checks,*/
/*                                            removed cipher suite lookup,*/
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_process_certificate_verify(NX_SECURE_TLS_SESSION *tls_session,
                                               UCHAR *packet_buffer, UINT message_length)
{
UINT                                  length = 0;
UINT                                  data_size;
UINT                                  signature_length = 0;
UINT                                  i;
INT                                   compare_value;
UCHAR                                *received_signature = NX_NULL;
UCHAR                                *working_ptr;
NX_CRYPTO_METHOD                     *public_cipher_method;
NX_CRYPTO_METHOD                     *hash_method = NX_NULL;
NX_SECURE_X509_CERT                  *client_certificate;
UINT                                  status;
VOID                                 *handler = NX_NULL;

    NX_SECURE_PROCESS_CERTIFICATE_VERIFY_EXTENSION

    /*
       ==== TLS 1.0/1.1 structure (hashes are of all handshake messages to this point) ====
        struct {
            Signature signature;
        } CertificateVerify;

        struct {
          select (SignatureAlgorithm) {
              case anonymous: struct { };
              case rsa:
                  digitally-signed struct {
                      opaque md5_hash[16];
                      opaque sha_hash[20];
                  };
              case dsa:
                  digitally-signed struct {
                      opaque sha_hash[20];
                  };
              };
          };
        } Signature;

       ==== TLS 1.2 structure (signature is generally PKCS#1 encoded) ====
        struct {
            digitally-signed struct {
                opaque handshake_messages[handshake_messages_length];
            }
        } CertificateVerify;

        struct {
             SignatureAndHashAlgorithm algorithm;
             opaque signature<0..2^16-1>;
        } DigitallySigned
     */


    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {

        /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    /* Get reference to remote device certificate so we can get the public key for signature verification. */
    status = _nx_secure_x509_remote_endpoint_certificate_get(&tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store,
                                                             &client_certificate);

    if (status || client_certificate == NX_NULL)
    {
        /* No certificate found, error! */
        return(NX_SECURE_TLS_CERTIFICATE_NOT_FOUND);
    }


    /* Generate the handshake message hash that will need to match the received signature. */
#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
#ifdef NX_SECURE_ENABLE_DTLS
    if (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_2 ||
        tls_session -> nx_secure_tls_protocol_version == NX_SECURE_DTLS_VERSION_1_2)
#else
    if (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_2)
#endif /* NX_SECURE_ENABLE_DTLS */
    {
        /* Calculate our final signature length for later offset calculations. */
        signature_length = 19 + 32; /* DER encoding (19) + SHA-256 hash size (32) */

        /* Generate a hash of all sent and received handshake messages to this point (not a Finished hash!). */
        /* Copy over the handshake hash state into a local structure to do the intermediate calculation. */
        NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch,
               tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata,
               tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata_size);

        /* Use SHA-256 for now... */
        hash_method = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_handshake_hash_sha256_method;
        if (hash_method -> nx_crypto_operation != NX_NULL)
        {
            status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_CALCULATE,
                                                        tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_handler,
                                                        hash_method,
                                                        NX_NULL,
                                                        0,
                                                        NX_NULL,
                                                        0,
                                                        NX_NULL,
                                                        &handshake_hash[0],
                                                        sizeof(handshake_hash),
                                                        tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch,
                                                        tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata_size,
                                                        NX_NULL,
                                                        NX_NULL);

            if (status != NX_SUCCESS)
            {
                /* Something failed in the hash calculation. */
                return(status);
            }
        }
        
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
        /* Signature size is the size of SHA-1 (20) + MD5 (16). */
        signature_length = 36;

        /* Copy over the handshake hash metadata into scratch metadata area to do the intermediate calculation. Copy SHA-1 in
           first, then MD5. */
        NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch,
               tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha1_metadata,
               tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha1_metadata_size);
        NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch +
               tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha1_metadata_size,
               tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_md5_metadata,
               tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_md5_metadata_size);

        /* Finalize the handshake message hashes that we started at the beginning of the handshake. */
        hash_method = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_handshake_hash_md5_method;
        if (hash_method -> nx_crypto_operation != NX_NULL)
        {
            status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_CALCULATE,
                                                        tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_md5_handler,
                                                        hash_method,
                                                        NX_NULL,
                                                        0,
                                                        NX_NULL,
                                                        0,
                                                        NX_NULL,
                                                        &handshake_hash[0],
                                                        16,
                                                        tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch +
                                                        tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha1_metadata_size,
                                                        tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_md5_metadata_size,
                                                        NX_NULL,
                                                        NX_NULL);

            if (status != NX_SUCCESS)
            {
                /* Something failed in the hash calculation. */
                return(status);
            }
        }

        hash_method = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_handshake_hash_sha1_method;
        if (hash_method -> nx_crypto_operation != NX_NULL)
        {
            status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_CALCULATE,
                                                        tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha1_handler,
                                                        hash_method,
                                                        NX_NULL,
                                                        0,
                                                        NX_NULL,
                                                        0,
                                                        NX_NULL,
                                                        &handshake_hash[16],
                                                        sizeof(handshake_hash) - 16,
                                                        tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch,
                                                        tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha1_metadata_size,
                                                        NX_NULL,
                                                        NX_NULL);

            if (status != NX_SUCCESS)
            {
                /* Something failed in the hash calculation. */
                return(status);
            }
        }
    }
#endif


    /* Make sure we found a supported version (essentially an assertion check). */
    if (hash_method == NX_NULL)
    {
        return(NX_SECURE_TLS_UNSUPPORTED_TLS_VERSION);
    }

#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
#ifdef NX_SECURE_ENABLE_DTLS
    if (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_2 ||
        tls_session -> nx_secure_tls_protocol_version == NX_SECURE_DTLS_VERSION_1_2)
#else
    if (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_2)
#endif /* NX_SECURE_ENABLE_DTLS */
    {
        /* Check the signature method. */
        if(packet_buffer[0] != NX_SECURE_TLS_HASH_ALGORITHM_SHA256 ||
           packet_buffer[1] != NX_SECURE_TLS_SIGNATURE_ALGORITHM_RSA)
        {
            return(NX_SECURE_TLS_UNKNOWN_CERT_SIG_ALGORITHM);
        }

        /* Get the length of the encrypted signature data. */
        length = (UINT)((packet_buffer[2] << 8) + packet_buffer[3]);

        if(length != client_certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_modulus_length)
        {
            return(NX_SECURE_TLS_CERTIFICATE_SIG_CHECK_FAILED);
        }

        /* Pointer to the received signature that we need to check. */
        received_signature = &packet_buffer[4];
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
        /* Get the length of the encrypted signature data. */
        length = (UINT)((packet_buffer[0] << 8) + packet_buffer[1]);

        if(length != client_certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_modulus_length)
        {
            return(NX_SECURE_TLS_CERTIFICATE_SIG_CHECK_FAILED);
        }

        /* Pointer to the received signature that we need to check. */
        received_signature = &packet_buffer[2];
    }
#endif

    /* Length sanity check. */
    if (length > message_length )
    {
        /* Incoming message was too long! */
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    /* Start with a clear buffer for our decrypted signature data. */
    NX_SECURE_MEMSET(_nx_secure_decrypted_signature, 0x0, sizeof(_nx_secure_decrypted_signature));

    length = 0;

    /* Get our public-key crypto method. */
    public_cipher_method = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_cipher;


    /* Use RSA? */
    if (public_cipher_method -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_RSA &&
        client_certificate -> nx_secure_x509_public_algorithm == NX_SECURE_TLS_X509_TYPE_RSA)
    {
        /* If using RSA, the length is equal to the key size. */
        data_size = client_certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_modulus_length;

        if (public_cipher_method -> nx_crypto_init != NX_NULL)
        {
            /* Initialize the crypto method with public key. */
            status = public_cipher_method -> nx_crypto_init(public_cipher_method,
                                                   (UCHAR *)client_certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_modulus,
                                                   (NX_CRYPTO_KEY_SIZE)(client_certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_modulus_length << 3),
                                                   &handler,
                                                   tls_session -> nx_secure_public_cipher_metadata_area,
                                                   tls_session -> nx_secure_public_cipher_metadata_size);

            if (status != NX_SUCCESS)
            {
                /* Something failed in setting up the public cipher. */
                return(status);
            }
        }

        if (public_cipher_method -> nx_crypto_operation != NX_NULL)
        {
            /* Decrypt the hash we received using the remote host's public key. */
            status = public_cipher_method -> nx_crypto_operation(NX_CRYPTO_DECRYPT,
                                                                handler,
                                                                public_cipher_method,
                                                                (UCHAR *)client_certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_exponent,
                                                                (NX_CRYPTO_KEY_SIZE)(client_certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_exponent_length << 3),
                                                                received_signature,
                                                                data_size,
                                                                NX_NULL,
                                                                _nx_secure_decrypted_signature,
                                                                sizeof(_nx_secure_decrypted_signature),
                                                                tls_session -> nx_secure_public_cipher_metadata_area,
                                                                tls_session -> nx_secure_public_cipher_metadata_size,
                                                                NX_NULL, NX_NULL);

            if (status != NX_SUCCESS)
            {
                /* Something failed in the cipher operation. */
                return(status);
            }
        }

        if (public_cipher_method -> nx_crypto_cleanup)
        {
            status = public_cipher_method -> nx_crypto_cleanup(tls_session -> nx_secure_public_cipher_metadata_area);

            if (status != NX_SUCCESS)
            {
                /* Something failed in the cipher operation. */
                return(status);
            }
        }

        /* Check PKCS-1 Signature padding. The scheme is to start with the block type (0x00, 0x01 for signing)
           then pad with 0xFF bytes (for signing) followed with a single 0 byte right before the payload,
           which comes at the end of the RSA block. */

        /* Block type is 0x00, 0x01 for signatures */
        if (_nx_secure_decrypted_signature[0] != 0x0 && _nx_secure_decrypted_signature[1] != 0x1)
        {
            /* Unknown block type. */
            return(NX_SECURE_TLS_PADDING_CHECK_FAILED);
        }

        /* Check padding. */
        for (i = 2; i < (data_size - signature_length - 1); ++i)
        {
            if (_nx_secure_decrypted_signature[i] != (UCHAR)0xFF)
            {
                /* Bad padding value. */
                return(NX_SECURE_TLS_PADDING_CHECK_FAILED);
            }
        }
    }
    else
    {
        /* Unknown or unsupported public-key operation. */
        return(NX_SECURE_TLS_UNSUPPORTED_PUBLIC_CIPHER);
    }

    /* Check the received handshake hash against what we generated above. */

    /* Assure that we fail if anything goes wrong with the comparison. */
    compare_value = 1;

#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
#ifdef NX_SECURE_ENABLE_DTLS
    if (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_2 ||
        tls_session -> nx_secure_tls_protocol_version == NX_SECURE_DTLS_VERSION_1_2)
#else
    if (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_2)
#endif /* NX_SECURE_ENABLE_DTLS */
    {
        /* Get a working pointer into the padded signature buffer. All PKCS-1 encoded data
           comes at the end of the RSA encrypted block. */
        working_ptr = &_nx_secure_decrypted_signature[data_size - signature_length];

        /* Check the DER encoding. */
        compare_value = NX_SECURE_MEMCMP(&working_ptr[0], _NX_SECURE_OID_SHA256, 19);

        /* Check the handshake hash. */
        compare_value += NX_SECURE_MEMCMP(&working_ptr[19], handshake_hash, 32);
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
        /* Get a working pointer into the padded signature buffer. All PKCS-1 encoded data
           comes at the end of the RSA encrypted block. */
        working_ptr = &_nx_secure_decrypted_signature[data_size - signature_length];

        /* Now put the data into the padded buffer - must be at the end. */
        compare_value = NX_SECURE_MEMCMP(working_ptr, handshake_hash, 36);
    }
#endif

#ifdef NX_SECURE_KEY_CLEAR
    NX_SECURE_MEMSET(handshake_hash, 0, sizeof(handshake_hash));
    NX_SECURE_MEMSET(_nx_secure_decrypted_signature, 0, sizeof(_nx_secure_decrypted_signature));
#endif /* NX_SECURE_KEY_CLEAR  */

    if (compare_value)
    {
        /* The hash value did not compare, so something has gone wrong. */
        return(NX_SECURE_TLS_CERTIFICATE_VERIFY_FAILURE);
    }

    return(NX_SUCCESS);
}

