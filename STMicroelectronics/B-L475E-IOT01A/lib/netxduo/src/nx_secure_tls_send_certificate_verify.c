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
static UCHAR _nx_secure_padded_signature[600];

#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
static const UCHAR _NX_SECURE_OID_SHA256[] = {0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0x04, 0x20};
#endif


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_send_certificate_verify              PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function populates an NX_PACKET with the TLS Certificate       */
/*    Verify message, which is used when a server requests a Client       */
/*    certificate for verification. The client encrypts a hash of         */
/*    received messages using its private key to verify that it is the    */
/*    actual owner of the certificate.                                    */
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
/*    [nx_crypto_operation]                 Public-key operation (eg RSA) */
/*                                            used to verify keys         */
/*    [nx_crypto_init]                      Initialize the public-key     */
/*                                            operation                   */
/*    _nx_secure_x509_local_device_certificate_get                        */
/*                                          Get the local certificate     */
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
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            added logic to support      */
/*                                            vendor-defined private key  */
/*                                            type, supported empty       */
/*                                            certificate messages,       */
/*                                            supported more ciphersuites,*/
/*                                            supported DTLS, optimize    */
/*                                            the logic,                  */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s), and      */
/*                                            optimized internl logic,    */
/*                                            added flexibility of using  */
/*                                            macros instead of direct C  */
/*                                            library function calls,     */
/*                                            update buffer for 4096 RSA, */
/*                                            improved packet length      */
/*                                            verification, updated error */
/*                                            return checks, removed      */
/*                                            cipher suite lookup,        */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_send_certificate_verify(NX_SECURE_TLS_SESSION *tls_session,
                                            NX_PACKET *send_packet)
{
UINT                                  length = 0;
UINT                                  data_size = 0;
UINT                                  signature_length = 0;
UINT                                  i;
UCHAR                                *current_buffer;
UCHAR                                *working_ptr;
NX_CRYPTO_METHOD                     *public_cipher_method;
NX_CRYPTO_METHOD                     *hash_method = NX_NULL;
NX_SECURE_X509_CERT                  *local_certificate;
UINT                                  status;
UINT                                  user_defined_key;
VOID                                 *handler = NX_NULL;

    NX_SECURE_SEND_CERTIFICATE_VERIFY_EXTENSION

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

    /* Get reference to local device certificate. NX_NULL is passed for name to get default entry. */
    status = _nx_secure_x509_local_device_certificate_get(&tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store,
                                                          NX_NULL, &local_certificate);

    /* In order to send a certificateVerify message, we must have a certificate. If
       no certificate is supplied for the TLS client, we send an empty certificate in response to the server request,
       but in that case we shouldn't be trying to send a CertificateVerify. */
    if (status)
    {
        /* The local certificate was not found. */
        return(status);
    }

#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
#ifdef NX_SECURE_ENABLE_DTLS
    if (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_2 ||
        tls_session -> nx_secure_tls_protocol_version == NX_SECURE_DTLS_VERSION_1_2)
#else
    if (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_2)
#endif /* NX_SECURE_ENABLE_DTLS */
    {

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
        }

        if(status != NX_SUCCESS)
        {
            return(status);
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

            if(status != NX_SUCCESS)
            {
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

            if(status != NX_SUCCESS)
            {
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

    /* Check for user-defined key types. */
    user_defined_key = NX_FALSE;
    if ((local_certificate -> nx_secure_x509_private_key_type & NX_SECURE_X509_KEY_TYPE_USER_DEFINED_MASK) != 0x0)
    {
        user_defined_key = NX_TRUE;
    }

    /* Get our actual public cipher method. */
    public_cipher_method = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_cipher;

    /* See if we are using RSA. Separate from other methods (e.g. ECC, DH) for proper handling of padding. */
    if (public_cipher_method -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_RSA &&
        local_certificate -> nx_secure_x509_public_algorithm == NX_SECURE_TLS_X509_TYPE_RSA)
    {
        /* If using RSA, the length is equal to the key size. */
        data_size = local_certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_modulus_length;

        if (((ULONG)(send_packet -> nx_packet_data_end) - (ULONG)(send_packet -> nx_packet_append_ptr)) < (4u + data_size))
        {
#ifdef NX_SECURE_KEY_CLEAR
            NX_SECURE_MEMSET(handshake_hash, 0, sizeof(handshake_hash));
#endif /* NX_SECURE_KEY_CLEAR  */

            /* Packet buffer is too small to hold random and ID. */
            return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
        }

        /* Pointer to where we are going to place our data. */
        current_buffer = send_packet -> nx_packet_append_ptr;

        /* Start with a clear buffer. */
        NX_SECURE_MEMSET(_nx_secure_padded_signature, 0x0, sizeof(_nx_secure_padded_signature));

        length = 0;

#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
#ifdef NX_SECURE_ENABLE_DTLS
        if (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_2 ||
            tls_session -> nx_secure_tls_protocol_version == NX_SECURE_DTLS_VERSION_1_2)
#else
        if (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_2)
#endif /* NX_SECURE_ENABLE_DTLS */
        {
            /* Signature algorithm used. */
            current_buffer[length]     = NX_SECURE_TLS_HASH_ALGORITHM_SHA256;   /* We only support SHA-256 right now. */
            current_buffer[length + 1] = NX_SECURE_TLS_SIGNATURE_ALGORITHM_RSA; /* RSA */
            length += 2;

            /* Length of signature data - size of RSA operation output. */
            current_buffer[length] =     (UCHAR)(data_size >> 8);
            current_buffer[length + 1] = (UCHAR)(data_size);
            length += 2;

            /* Calculate our final signature length for later offset calculations. */
            signature_length = 19 + 32; /* DER encoding (19) + SHA-256 hash size (32) */

            if (data_size < signature_length)
            {
#ifdef NX_SECURE_KEY_CLEAR
                NX_SECURE_MEMSET(handshake_hash, 0, sizeof(handshake_hash));
#endif /* NX_SECURE_KEY_CLEAR  */

                /* Invalid certificate. */
                return(NX_SECURE_TLS_INVALID_CERTIFICATE);
            }

            /* Get a working pointer into the padded signature buffer. All PKCS-1 encoded data
               comes at the end of the RSA encrypted block. */
            working_ptr = &_nx_secure_padded_signature[data_size - signature_length];

            /* Copy in the DER encoding. */
            NX_SECURE_MEMCPY(&working_ptr[0], _NX_SECURE_OID_SHA256, 19);

            /* Now put the data into the padded buffer - must be at the end. */
            NX_SECURE_MEMCPY(&working_ptr[19], handshake_hash, 32);
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
            /* Length of signature data - size of RSA operation output. */
            current_buffer[length] =     (UCHAR)(data_size >> 8);
            current_buffer[length + 1] = (UCHAR)(data_size);
            length += 2;

            /* Signature size is the size of SHA-1 (20) + MD5 (16). */
            signature_length = 36;

            if (data_size < signature_length)
            {
#ifdef NX_SECURE_KEY_CLEAR
                NX_SECURE_MEMSET(handshake_hash, 0, sizeof(handshake_hash));
#endif /* NX_SECURE_KEY_CLEAR  */
                /* Invalid certificate. */
                return(NX_SECURE_TLS_INVALID_CERTIFICATE);
            }

            /* Get a working pointer into the padded signature buffer. All PKCS-1 encoded data
               comes at the end of the RSA encrypted block. */
            working_ptr = &_nx_secure_padded_signature[data_size - signature_length];

            /* Now put the data into the padded buffer - must be at the end. */
            NX_SECURE_MEMCPY(working_ptr, handshake_hash, 36);
        }
#endif

#ifdef NX_SECURE_KEY_CLEAR
        /* At this point, the handshake_hash has been copied into _nx_secure_padded_signature and 
        is no longer needed so we can clear it here. */
        NX_SECURE_MEMSET(handshake_hash, 0, sizeof(handshake_hash));
#endif /* NX_SECURE_KEY_CLEAR  */

        /* PKCS-1 Signature padding. The scheme is to start with the block type (0x00, 0x01 for signing)
           then pad with 0xFF bytes (for signing) followed with a single 0 byte right before the payload,
           which comes at the end of the RSA block. */

        _nx_secure_padded_signature[1] = 0x1; /* Block type is 0x00, 0x01 for signatures */
        for (i = 2; i < (data_size - signature_length - 1); ++i)
        {
            _nx_secure_padded_signature[i] = (UCHAR)0xFF;
        }

        /* Check for user-defined keys. */
        if (user_defined_key)
        {
            /* A user-defined key is passed directly into the crypto routine. */
            status = public_cipher_method -> nx_crypto_operation(local_certificate -> nx_secure_x509_private_key_type,
                                                        NX_NULL,
                                                        public_cipher_method,
                                                        (UCHAR *)local_certificate -> nx_secure_x509_private_key.user_key.key_data,
                                                        (NX_CRYPTO_KEY_SIZE)(local_certificate -> nx_secure_x509_private_key.user_key.key_length),
                                                        _nx_secure_padded_signature,
                                                        length,
                                                        NX_NULL,
                                                        &current_buffer[length],
                                                        sizeof(_nx_secure_padded_signature),
                                                        tls_session -> nx_secure_public_cipher_metadata_area,
                                                        tls_session -> nx_secure_public_cipher_metadata_size,
                                                        NX_NULL, NX_NULL);

            if(status != NX_SUCCESS)
            {
#ifdef NX_SECURE_KEY_CLEAR
                NX_SECURE_MEMSET(_nx_secure_padded_signature, 0, sizeof(_nx_secure_padded_signature));
#endif /* NX_SECURE_KEY_CLEAR  */                
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
#ifdef NX_SECURE_KEY_CLEAR
                    NX_SECURE_MEMSET(_nx_secure_padded_signature, 0, sizeof(_nx_secure_padded_signature));
#endif /* NX_SECURE_KEY_CLEAR  */                    
                    return(status);
                }                                                     
            }

            if (public_cipher_method -> nx_crypto_operation != NX_NULL)
            {
                /* Sign the hash we just generated using our local RSA private key (associated with our local cert). */
                status = public_cipher_method -> nx_crypto_operation(NX_CRYPTO_DECRYPT,
                                                            handler,
                                                            public_cipher_method,
                                                            (UCHAR *)local_certificate -> nx_secure_x509_private_key.rsa_private_key.nx_secure_rsa_private_exponent,
                                                            (NX_CRYPTO_KEY_SIZE)(local_certificate -> nx_secure_x509_private_key.rsa_private_key.nx_secure_rsa_private_exponent_length << 3),
                                                            _nx_secure_padded_signature,
                                                            data_size,
                                                            NX_NULL,
                                                            &current_buffer[length],
                                                            sizeof(_nx_secure_padded_signature),
                                                            tls_session -> nx_secure_public_cipher_metadata_area,
                                                            tls_session -> nx_secure_public_cipher_metadata_size,
                                                            NX_NULL, NX_NULL);

                if(status != NX_SUCCESS)
                {
#ifdef NX_SECURE_KEY_CLEAR
                    NX_SECURE_MEMSET(_nx_secure_padded_signature, 0, sizeof(_nx_secure_padded_signature));
#endif /* NX_SECURE_KEY_CLEAR  */                    
                    return(status);
                }                                                     
            }

            if (public_cipher_method -> nx_crypto_cleanup)
            {
                status = public_cipher_method -> nx_crypto_cleanup(tls_session -> nx_secure_public_cipher_metadata_area);

                if(status != NX_SUCCESS)
                {
#ifdef NX_SECURE_KEY_CLEAR
                    NX_SECURE_MEMSET(_nx_secure_padded_signature, 0, sizeof(_nx_secure_padded_signature));
#endif /* NX_SECURE_KEY_CLEAR  */                    
                    return(status);
                }                                                     
            }
        }
    }

#ifdef NX_SECURE_KEY_CLEAR
    NX_SECURE_MEMSET(_nx_secure_padded_signature, 0, sizeof(_nx_secure_padded_signature));
#endif /* NX_SECURE_KEY_CLEAR  */

    /* Add the length of our encrypted signature to the total. */
    length += data_size;

    /* Set the return size and adjust length. */
    send_packet -> nx_packet_append_ptr = send_packet -> nx_packet_append_ptr + (USHORT)(length);
    send_packet -> nx_packet_length = send_packet -> nx_packet_length + (USHORT)(length);

    return(NX_SECURE_TLS_SUCCESS);
}

