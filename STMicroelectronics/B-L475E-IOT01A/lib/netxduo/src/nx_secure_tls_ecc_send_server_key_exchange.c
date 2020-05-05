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

static UCHAR hash[16 + 20]; /* We concatenate MD5 and SHA-1 hashes into this buffer, OR SHA-256. */
static UCHAR _nx_secure_padded_signature[512];
/* DER encodings (with OIDs for common algorithms) from RFC 8017.
 * NOTE: This is the equivalent DER-encoding for the value "T" described in RFC 8017 section 9.2. */
static const UCHAR _NX_CRYPTO_DER_OID_SHA_1[]       =  {0x30, 0x21, 0x30, 0x09, 0x06, 0x05, 0x2b, 0x0e, 0x03, 0x02, 0x1a, 0x05, 0x00, 0x04, 0x14};
static const UCHAR _NX_CRYPTO_DER_OID_SHA_256[]     =  {0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0x04, 0x20};

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_ecc_send_server_key_exchange         PORTABLE C      */
/*                                                          5.12          */
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
/*  08-15-2019     Timothy Stapko           Initial Version 5.12          */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_ecc_send_server_key_exchange(struct NX_SECURE_TLS_SESSION_STRUCT *tls_session,
                                                 NX_PACKET_POOL *packet_pool, ULONG wait_option)
{
NX_PACKET                            *send_packet;
UINT                                  status;
UINT                                  length;
UCHAR                                *packet_buffer;
USHORT                                signature_length;
UINT                                  signature_offset;
const UCHAR                          *der_encoding = NX_NULL;
UINT                                  der_encoding_length = 0;
UINT                                  hash_length;
NX_SECURE_TLS_ECDHE_HANDSHAKE_DATA   *ecdhe_data;
NX_CRYPTO_EXTENDED_OUTPUT             extended_output;
NX_CRYPTO_METHOD                     *curve_method;
NX_CRYPTO_METHOD                     *curve_method_cert;
NX_CRYPTO_METHOD                     *ecdhe_method;
NX_CRYPTO_METHOD                     *hash_method;
NX_CRYPTO_METHOD                     *auth_method;
VOID                                 *handler = NX_NULL;
NX_SECURE_X509_CERT                  *server_certificate;
NX_SECURE_X509_CRYPTO                *crypto_methods;
NX_SECURE_EC_PRIVATE_KEY             *ec_privkey;
NX_SECURE_EC_PUBLIC_KEY              *ec_pubkey;

    /* Build up the server key exchange message. Structure:
     * select (KeyExchangeAlgorithm) {
     *     case ec_diffie_hellman:
     *         ServerECDHParams params;
     *         Signature signed_params;
     *  } ServerKeyExchange
     */

    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {

        /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    /* Check for ECDHE ciphersuites. */
    if (tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_cipher -> nx_crypto_algorithm != NX_CRYPTO_KEY_EXCHANGE_ECDHE)
    {
        return(NX_SUCCESS);
    }


    ecdhe_data = (NX_SECURE_TLS_ECDHE_HANDSHAKE_DATA *)tls_session -> nx_secure_tls_key_material.nx_secure_tls_new_key_material_data;

    /* Find out which named curve the we are using. */
    status = _nx_secure_tls_find_curve_method(tls_session, (USHORT)ecdhe_data -> nx_secure_tls_ecdhe_named_curve, (VOID **)&curve_method);
    if(status != NX_SUCCESS)
    {
        return(status);
    }
    if (curve_method == NX_NULL)
    {
        return(NX_SECURE_TLS_UNSUPPORTED_ECC_CURVE);
    }

    /* Generate ECDHE key pair using ECDHE crypto method. */
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

    status = _nx_secure_tls_allocate_handshake_packet(tls_session, packet_pool, &send_packet, wait_option);
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    if (((ULONG)(send_packet -> nx_packet_data_end) - (ULONG)(send_packet -> nx_packet_append_ptr)) < 3u)
    {

        /* Packet buffer is too small to hold random and ID. */
        nx_packet_release(send_packet);
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    /* Get a pointer to fill our packet. */
    packet_buffer = send_packet -> nx_packet_append_ptr;
    length = 0;

    /* ECCurveType: named_curve (3). */
    packet_buffer[length] = 3;
    length += 1;

    /* NamedCurve */
    packet_buffer[length] = (UCHAR)((ecdhe_data -> nx_secure_tls_ecdhe_named_curve & 0xFF00) >> 8);
    packet_buffer[length + 1] = (UCHAR)(ecdhe_data -> nx_secure_tls_ecdhe_named_curve & 0x00FF);
    length += 2;

    /* Generate the key pair and put the public key in the packet. */
    extended_output.nx_crypto_extended_output_data = &packet_buffer[length + 1];
    extended_output.nx_crypto_extended_output_length_in_byte =
        (ULONG)send_packet -> nx_packet_data_end - (ULONG)&packet_buffer[length + 1];
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
        nx_packet_release(send_packet);
        return(status);
    }

    packet_buffer[length] = (UCHAR)extended_output.nx_crypto_extended_output_actual_size;
    length += (UINT)(1 + packet_buffer[length]);

    /* Export the private key for use when processing the ClientKeyExchange. */
    extended_output.nx_crypto_extended_output_data = ecdhe_data -> nx_secure_tls_ecdhe_private_key;
    extended_output.nx_crypto_extended_output_length_in_byte =
        sizeof(ecdhe_data -> nx_secure_tls_ecdhe_private_key);
    extended_output.nx_crypto_extended_output_actual_size = 0;
    status = ecdhe_method -> nx_crypto_operation(NX_CRYPTO_DH_PRIVATE_KEY_EXPORT, handler,
                                                 ecdhe_method, NX_NULL, 0,
                                                 NX_NULL, 0, NX_NULL,
                                                 (UCHAR *)&extended_output,
                                                 sizeof(extended_output),
                                                 tls_session -> nx_secure_public_cipher_metadata_area,
                                                 tls_session -> nx_secure_public_cipher_metadata_size,
                                                 NX_NULL, NX_NULL);
    if (status != NX_CRYPTO_SUCCESS)
    {
        nx_packet_release(send_packet);
        return(status);
    }

    ecdhe_data -> nx_secure_tls_ecdhe_private_key_length = (USHORT)extended_output.nx_crypto_extended_output_actual_size;

    if (ecdhe_method -> nx_crypto_cleanup)
    {
        status = ecdhe_method -> nx_crypto_cleanup(tls_session -> nx_secure_public_cipher_metadata_area);
        if(status != NX_SUCCESS)
        {
            return(status);
        }
    }

    /* Get the local certificate. */
    if (tls_session -> nx_secure_tls_credentials.nx_secure_tls_active_certificate != NX_NULL)
    {
        server_certificate = tls_session -> nx_secure_tls_credentials.nx_secure_tls_active_certificate;
    }
    else
    {
        /* Get reference to local device certificate. NX_NULL is passed for name to get default entry. */
        status = _nx_secure_x509_local_device_certificate_get(&tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store,
                                                              NX_NULL, &server_certificate);
        if (status != NX_SUCCESS)
        {
            server_certificate = NX_NULL;
        }
    }

    if (server_certificate == NX_NULL)
    {
        /* No certificate found, error! */
        nx_packet_release(send_packet);
        return(NX_SECURE_TLS_CERTIFICATE_NOT_FOUND);
    }


    /* Find out the hash algorithm used for the signature. */
    switch ((ecdhe_data -> nx_secure_tls_ecdhe_signature_algorithm & 0xFF00) >> 8)
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
            nx_packet_release(send_packet);
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
            nx_packet_release(send_packet);
            return(status);
        }
        break;

    default:
        /* The hash algorithm is not supported. */
        nx_packet_release(send_packet);
        return(NX_SECURE_TLS_UNSUPPORTED_SIGNATURE_ALGORITHM);
        break;
    }

    hash_method = crypto_methods -> nx_secure_x509_hash_method;

    hash_length = hash_method -> nx_crypto_ICV_size_in_bits >> 3;

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
        nx_packet_release(send_packet);
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
                                       length,
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
                                       hash_length,
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

    if (((ULONG)(send_packet -> nx_packet_data_end) - (ULONG)(send_packet -> nx_packet_append_ptr)) <
        (6u + length))
    {

        /* Packet buffer is too small to hold random and ID. */
        nx_packet_release(send_packet);
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    /* Signature Hash Algorithm. */
    packet_buffer[length] = (UCHAR)((ecdhe_data -> nx_secure_tls_ecdhe_signature_algorithm & 0xFF00) >> 8);
    packet_buffer[length + 1] = (UCHAR)(ecdhe_data -> nx_secure_tls_ecdhe_signature_algorithm & 0x00FF);
    length += 2;

    /* Sign the hash. */
    auth_method = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_auth;
    if ((ecdhe_data -> nx_secure_tls_ecdhe_signature_algorithm & 0xFF) == NX_SECURE_TLS_SIGNATURE_ALGORITHM_RSA &&
        (auth_method -> nx_crypto_algorithm == NX_CRYPTO_DIGITAL_SIGNATRUE_RSA ||
        auth_method -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_RSA))
    {
        signature_length = server_certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_modulus_length;

        /* Signature Length */
        packet_buffer[length] = (UCHAR)((signature_length & 0xFF00) >> 8);
        packet_buffer[length + 1] = (UCHAR)(signature_length & 0x00FF);
        length += 2;

        switch ((ecdhe_data -> nx_secure_tls_ecdhe_signature_algorithm & 0xFF00) >> 8)
        {
        case NX_SECURE_TLS_HASH_ALGORITHM_SHA1:
            der_encoding = _NX_CRYPTO_DER_OID_SHA_1;
            der_encoding_length = sizeof(_NX_CRYPTO_DER_OID_SHA_1);
            break;

        case NX_SECURE_TLS_HASH_ALGORITHM_SHA256:
            der_encoding = _NX_CRYPTO_DER_OID_SHA_256;
            der_encoding_length = sizeof(_NX_CRYPTO_DER_OID_SHA_256);
            break;
        default:
            /* This is basically an assert check, but if the signature algorithm is unknown, we have a problem. */        
            return(NX_SECURE_TLS_UNKNOWN_CERT_SIG_ALGORITHM);
        }

        /* Build the RSA signature. */
        /* C-STAT: If signature_length is ever exactly equal to (der_encoding_length + hash_length) 
                   then signature_offset will be 0 and the (signature_offset - 1) expression below
                   would result in a negative array subscript. Thus, also check for equality in the
                   second condition (a zero-length signature offset). */        
        if ((signature_length > sizeof(_nx_secure_padded_signature)) ||
            (signature_length <= (der_encoding_length + hash_length)))
        {

            /* Buffer too small. */
            nx_packet_release(send_packet);
            return(NX_SECURE_TLS_INVALID_CERTIFICATE);
        }

        signature_offset = signature_length - (der_encoding_length + hash_length);
        NX_CRYPTO_MEMSET(_nx_secure_padded_signature, 0xff, signature_offset);
        _nx_secure_padded_signature[0] = 0x0;
        _nx_secure_padded_signature[1] = 0x1;
        _nx_secure_padded_signature[signature_offset - 1] = 0x0;
        NX_CRYPTO_MEMCPY(&_nx_secure_padded_signature[signature_offset], der_encoding, der_encoding_length);
        signature_offset += der_encoding_length;
        NX_CRYPTO_MEMCPY(&_nx_secure_padded_signature[signature_offset], hash, hash_length);

        if (auth_method -> nx_crypto_init != NX_NULL)
        {
            /* Initialize the crypto method with public key. */
            status = auth_method -> nx_crypto_init(auth_method,
                                                   (UCHAR *)server_certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_modulus,
                                                   (NX_CRYPTO_KEY_SIZE)(server_certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_modulus_length << 3),
                                                   &handler,
                                                   tls_session -> nx_secure_public_auth_metadata_area,
                                                   tls_session -> nx_secure_public_auth_metadata_size);
            if (status != NX_CRYPTO_SUCCESS)
            {
                nx_packet_release(send_packet);
                return(status);
            }
        }

        if (((ULONG)(send_packet -> nx_packet_data_end) - (ULONG)(send_packet -> nx_packet_append_ptr)) <
            (length + signature_length))
        {

            /* Packet buffer is too small to hold random and ID. */
            nx_packet_release(send_packet);
            return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
        }

        if (auth_method -> nx_crypto_operation != NX_NULL)
        {
            /* Sign the hash we just generated using our local RSA private key (associated with our local cert). */
            status = auth_method -> nx_crypto_operation(NX_CRYPTO_DECRYPT,
                                                        handler,
                                                        auth_method,
                                                        (UCHAR *)server_certificate -> nx_secure_x509_private_key.rsa_private_key.nx_secure_rsa_private_exponent,
                                                        (NX_CRYPTO_KEY_SIZE)(server_certificate -> nx_secure_x509_private_key.rsa_private_key.nx_secure_rsa_private_exponent_length << 3),
                                                        _nx_secure_padded_signature,
                                                        signature_length,
                                                        NX_NULL,
                                                        &packet_buffer[length],
                                                        signature_length,
                                                        tls_session -> nx_secure_public_auth_metadata_area,
                                                        tls_session -> nx_secure_public_auth_metadata_size,
                                                        NX_NULL, NX_NULL);
            if (status != NX_CRYPTO_SUCCESS)
            {
                nx_packet_release(send_packet);
                return(status);
            }
        }

        length += signature_length;

        if (auth_method -> nx_crypto_cleanup)
        {
            status = auth_method -> nx_crypto_cleanup(tls_session -> nx_secure_public_auth_metadata_area);

            if(status != NX_SUCCESS)
            {
                return(status);
            }                                                     
        }
    }
    else if ((ecdhe_data -> nx_secure_tls_ecdhe_signature_algorithm & 0xFF) == NX_SECURE_TLS_SIGNATURE_ALGORITHM_ECDSA &&
             auth_method -> nx_crypto_algorithm == NX_CRYPTO_DIGITAL_SIGNATRUE_ECDSA)
    {
        ec_privkey = (NX_SECURE_EC_PRIVATE_KEY *)&server_certificate -> nx_secure_x509_private_key;
        ec_pubkey = (NX_SECURE_EC_PUBLIC_KEY *)&server_certificate -> nx_secure_x509_public_key;

        /* Find out which named curve the local certificate is using. */
        status = _nx_secure_tls_find_curve_method(tls_session, (USHORT)(ec_privkey -> nx_secure_ec_named_curve), (VOID **)&curve_method_cert);
        if(status != NX_SUCCESS)
        {
            return(status);
        }
        if (curve_method_cert == NX_NULL)
        {
            /* The local certificate is using an unsupported curve. */
            nx_packet_release(send_packet);
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
                nx_packet_release(send_packet);
                return(status);
            }
        }
        if (auth_method -> nx_crypto_operation == NX_NULL)
        {
            nx_packet_release(send_packet);
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
            nx_packet_release(send_packet);
            return(status);
        }

        /* Generate the signature and put it in the packet. */
        extended_output.nx_crypto_extended_output_data = &packet_buffer[length + 2];
        extended_output.nx_crypto_extended_output_length_in_byte =
            (ULONG)send_packet -> nx_packet_data_end - (ULONG)&packet_buffer[length + 2];
        extended_output.nx_crypto_extended_output_actual_size = 0;
        status = auth_method -> nx_crypto_operation(NX_CRYPTO_AUTHENTICATE, handler,
                                                    auth_method,
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
            nx_packet_release(send_packet);
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

        /* Signature Length */
        packet_buffer[length] = (UCHAR)((extended_output.nx_crypto_extended_output_actual_size & 0xFF00) >> 8);
        packet_buffer[length + 1] = (UCHAR)(extended_output.nx_crypto_extended_output_actual_size & 0x00FF);

        length += extended_output.nx_crypto_extended_output_actual_size + 2;
    }
    else
    {
        /* The signature algorithm is not supported. */
        nx_packet_release(send_packet);
        return(NX_SECURE_TLS_UNSUPPORTED_SIGNATURE_ALGORITHM);
    }


    /* Finally, we have a complete length and can adjust our packet accordingly. */
    send_packet -> nx_packet_append_ptr = send_packet -> nx_packet_append_ptr + length;
    send_packet -> nx_packet_length = send_packet -> nx_packet_length + length;

    status = _nx_secure_tls_send_handshake_record(tls_session, send_packet, NX_SECURE_TLS_SERVER_KEY_EXCHANGE, wait_option);

    return(status);
}

