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
/**    X509 Digital Certificates                                          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SECURE_SOURCE_CODE

#include "nx_secure_tls_ecc.h"
#include "nx_secure_tls.h"
#include "nx_secure_x509.h"

#ifndef NX_SECURE_X509_DISABLE_CRL
static UCHAR generated_hash[64];       /* We need to be able to hold the entire generated hash - SHA-512 = 64 bytes. */
static UCHAR decrypted_signature[512]; /* This needs to hold the entire decrypted data - RSA 2048-bit key = 256 bytes. */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_x509_ecc_crl_verify                      PORTABLE C      */
/*                                                          5.12          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function verifies a CRL by checking its signature against its  */
/*    issuer's public key. Note that a CRL does not have any crypto       */
/*    methods assigned to it, so the certificate being checked is also    */
/*    passed into this function so that its crypto routines may be        */
/*    utilized.                                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    certificate                           Certificate to be checked     */
/*    crl                                   Pointer to parsed CRL         */
/*    issuer_certificate                    CRL issuer's certificate      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nx_crypto_init]                      Crypto initialization         */
/*    [nx_crypto_operation]                 Crypto operation              */
/*    _nx_secure_x509_pkcs7_decode          Decode the PKCS#7 signature   */
/*    _nx_secure_x509_find_certificate_methods                            */
/*                                          Find certificate methods      */
/*    _nx_secure_tls_find_curve_method      Find named curve used         */
/*    _nx_secure_x509_asn1_tlv_block_parse  Parse ASN.1 block             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_x509_crl_revocation_check  Check revocation in crl       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-15-2019     Timothy Stapko           Initial Version 5.12          */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_x509_ecc_crl_verify(struct NX_SECURE_X509_CERT_STRUCT *certificate,
                                    struct NX_SECURE_X509_CRL_STRUCT *crl,
                                    struct NX_SECURE_X509_CERT_STRUCT *issuer_certificate)
{
UINT                   status;
UINT                   oid_length;
const UCHAR           *oid;
UINT                   decrypted_hash_length;
const UCHAR           *decrypted_hash;
const UCHAR           *crl_verify_data;
UINT                   verify_data_length;
const UCHAR           *signature_data;
UINT                   signature_length;
UINT                   compare_result;
UINT                   hash_length;
NX_CRYPTO_METHOD      *hash_method;
NX_CRYPTO_METHOD      *public_cipher_method;
NX_SECURE_X509_CRYPTO *crypto_methods;
VOID                  *handler = NX_NULL;
NX_SECURE_EC_PUBLIC_KEY *ec_pubkey;
NX_CRYPTO_METHOD      *curve_method;


    /* Get working pointers to relevant data. */
    crl_verify_data = crl -> nx_secure_x509_crl_verify_data;
    verify_data_length = crl -> nx_secure_x509_crl_verify_data_length;
    signature_data = crl -> nx_secure_x509_crl_signature_data;
    signature_length = crl -> nx_secure_x509_crl_signature_data_length;

    /* Find certificate crypto methods for this certificate. */
    status = _nx_secure_x509_find_certificate_methods(certificate, crl -> nx_secure_x509_crl_signature_algorithm, &crypto_methods);
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Assign local pointers for the crypto methods. */
    hash_method = crypto_methods -> nx_secure_x509_hash_method;
    public_cipher_method = crypto_methods -> nx_secure_x509_public_cipher_method;

    if (hash_method -> nx_crypto_init)
    {
        status = hash_method -> nx_crypto_init(hash_method,
                                      NX_NULL,
                                      0,
                                      &handler,
                                      certificate -> nx_secure_x509_hash_metadata_area,
                                      certificate -> nx_secure_x509_hash_metadata_size);

        if(status != NX_SUCCESS)
        {
            return(status);
        }                                                     
    }

    /* We need to generate a hash of this CRL in order to verify it against our trusted store. */
    if (hash_method -> nx_crypto_operation != NX_NULL)
    {
        status = hash_method -> nx_crypto_operation(NX_CRYPTO_VERIFY,
                                           handler,
                                           hash_method,
                                           NX_NULL,
                                           0,
                                           (UCHAR *)crl_verify_data,
                                           verify_data_length,
                                           NX_NULL,
                                           generated_hash,
                                           sizeof(generated_hash),
                                           certificate -> nx_secure_x509_hash_metadata_area,
                                           certificate -> nx_secure_x509_hash_metadata_size,
                                           NX_NULL, NX_NULL);

        if(status != NX_SUCCESS)
        {
            return(status);
        }                                                     
    }

    if (hash_method -> nx_crypto_cleanup)
    {
        status = hash_method -> nx_crypto_cleanup(certificate -> nx_secure_x509_hash_metadata_area);

        if(status != NX_SUCCESS)
        {
#ifdef NX_SECURE_KEY_CLEAR
            NX_SECURE_MEMSET(generated_hash, 0, sizeof(generated_hash));
#endif /* NX_SECURE_KEY_CLEAR  */

            return(status);
        }                                                     
    }

    hash_length = (hash_method -> nx_crypto_ICV_size_in_bits >> 3);

    /* Perform a public-key decryption operation on the extracted signature from the CRL.
     * In this case, the operation is doing a "reverse decryption", using the public key to decrypt, rather
     * than the private. This allows us to tie a trusted root certificate to a signature of a CRL
     * signed by that root CA's private key. when combined with a hash method, this is the basic digital
     * signature operation. */
    if (public_cipher_method -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_RSA ||
        public_cipher_method -> nx_crypto_algorithm == NX_CRYPTO_DIGITAL_SIGNATRUE_RSA)
    {
        /* Make sure the public algorithm of the issuer certificate is RSA. */
        if (issuer_certificate -> nx_secure_x509_public_algorithm != NX_SECURE_TLS_X509_TYPE_RSA)
        {
#ifdef NX_SECURE_KEY_CLEAR
            NX_SECURE_MEMSET(generated_hash, 0, sizeof(generated_hash));
#endif /* NX_SECURE_KEY_CLEAR  */

            return(NX_SECURE_X509_WRONG_SIGNATURE_METHOD);
        }

        if (public_cipher_method -> nx_crypto_init != NX_NULL)
        {
            /* Initialize the crypto method with public key. */
            status = public_cipher_method -> nx_crypto_init(public_cipher_method,
                                                   (UCHAR *)issuer_certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_modulus,
                                                   (NX_CRYPTO_KEY_SIZE)(issuer_certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_modulus_length << 3),
                                                   &handler,
                                                   certificate -> nx_secure_x509_public_cipher_metadata_area,
                                                   certificate -> nx_secure_x509_public_cipher_metadata_size);

            if(status != NX_SUCCESS)
            {
#ifdef NX_SECURE_KEY_CLEAR
                NX_SECURE_MEMSET(generated_hash, 0, sizeof(generated_hash));
#endif /* NX_SECURE_KEY_CLEAR  */

                return(status);
            }                                                     
        }

        if (public_cipher_method -> nx_crypto_operation != NX_NULL)
        {
            status = public_cipher_method -> nx_crypto_operation(NX_CRYPTO_DECRYPT,
                                                        handler,
                                                        public_cipher_method,
                                                        (UCHAR *)issuer_certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_exponent,
                                                        (NX_CRYPTO_KEY_SIZE)(issuer_certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_exponent_length << 3),
                                                        (UCHAR *)signature_data,
                                                        signature_length,
                                                        NX_NULL,
                                                        decrypted_signature,
                                                        sizeof(decrypted_signature),
                                                        certificate -> nx_secure_x509_public_cipher_metadata_area,
                                                        certificate -> nx_secure_x509_public_cipher_metadata_size,
                                                        NX_NULL, NX_NULL);

            if(status != NX_SUCCESS)
            {
#ifdef NX_SECURE_KEY_CLEAR
     		    NX_SECURE_MEMSET(generated_hash, 0, sizeof(generated_hash));
#endif /* NX_SECURE_KEY_CLEAR  */

                return(status);
            }                                                     
        }

        if (public_cipher_method -> nx_crypto_cleanup)
        {
            status = public_cipher_method -> nx_crypto_cleanup(certificate -> nx_secure_x509_public_cipher_metadata_area);

            if(status != NX_SUCCESS)
            {
#ifdef NX_SECURE_KEY_CLEAR
                NX_SECURE_MEMSET(generated_hash, 0, sizeof(generated_hash));
                NX_SECURE_MEMSET(decrypted_signature, 0, sizeof(decrypted_signature));
#endif /* NX_SECURE_KEY_CLEAR  */
                return(status);
            }                                                     
        }

        /* Decode the decrypted signature, which should be in PKCS#7 format. */
        status = _nx_secure_x509_pkcs7_decode(decrypted_signature, signature_length, &oid, &oid_length,
                                              &decrypted_hash, &decrypted_hash_length);

#ifdef NX_SECURE_KEY_CLEAR
        if(status != NX_SUCCESS || decrypted_hash_length != hash_length)
        {
            /* Clear secrets on errors. */            
            NX_SECURE_MEMSET(generated_hash, 0, sizeof(generated_hash));
            NX_SECURE_MEMSET(decrypted_signature, 0, sizeof(decrypted_signature));
        }
#endif /* NX_SECURE_KEY_CLEAR  */


        if (status != NX_SUCCESS)
        {
            return(status);
        }

        if (decrypted_hash_length != hash_length)
        {
            return(NX_SECURE_X509_WRONG_SIGNATURE_METHOD);
        }

        /* Compare generated hash with decrypted hash. */
        compare_result = (UINT)NX_SECURE_MEMCMP(generated_hash, decrypted_hash, decrypted_hash_length);

#ifdef NX_SECURE_KEY_CLEAR
        NX_SECURE_MEMSET(generated_hash, 0, sizeof(generated_hash));
        NX_SECURE_MEMSET(decrypted_signature, 0, sizeof(decrypted_signature));
#endif /* NX_SECURE_KEY_CLEAR  */

        /* If the comparision worked, return success. */
        if (compare_result == 0)
        {
            return(NX_SUCCESS);
        }
    }
    else if (public_cipher_method -> nx_crypto_algorithm == NX_CRYPTO_DIGITAL_SIGNATRUE_ECDSA)
    {
        /* Make sure the public algorithm of the issuer certificate is EC. */
        if (issuer_certificate -> nx_secure_x509_public_algorithm != NX_SECURE_TLS_X509_TYPE_EC)
        {
#ifdef NX_SECURE_KEY_CLEAR
            NX_SECURE_MEMSET(generated_hash, 0, sizeof(generated_hash));
#endif /* NX_SECURE_KEY_CLEAR  */            
            return(NX_SECURE_X509_WRONG_SIGNATURE_METHOD);
        }

        /* Verify the ECDSA signature. */

        ec_pubkey = (NX_SECURE_EC_PUBLIC_KEY *)&issuer_certificate -> nx_secure_x509_public_key;

        /* Find out which named curve the remote certificate is using. */
        status = _nx_secure_tls_find_curve_method(NX_NULL, (USHORT)(ec_pubkey -> nx_secure_ec_named_curve), (VOID **)&curve_method);

#ifdef NX_SECURE_KEY_CLEAR
        if(status != NX_SUCCESS || curve_method == NX_NULL)
        {
            /* Clear secrets on errors. */
            NX_SECURE_MEMSET(generated_hash, 0, sizeof(generated_hash));
        }
#endif /* NX_SECURE_KEY_CLEAR  */

        if(status != NX_SUCCESS)
        {
            return(status);
        }

        if (curve_method == NX_NULL)
        {
            /* The issuer certificate is using an unsupported curve. */
            return(NX_SECURE_TLS_UNSUPPORTED_PUBLIC_CIPHER);
        }

        if (public_cipher_method -> nx_crypto_init != NX_NULL)
        {
            status = public_cipher_method -> nx_crypto_init(public_cipher_method,
                                                            (UCHAR *)ec_pubkey -> nx_secure_ec_public_key,
                                                            (NX_CRYPTO_KEY_SIZE)(ec_pubkey -> nx_secure_ec_public_key_length << 3),
                                                            &handler,
                                                            certificate -> nx_secure_x509_public_cipher_metadata_area,
                                                            certificate -> nx_secure_x509_public_cipher_metadata_size);
            if (status != NX_CRYPTO_SUCCESS)
            {
#ifdef NX_SECURE_KEY_CLEAR
                NX_SECURE_MEMSET(generated_hash, 0, sizeof(generated_hash));
#endif /* NX_SECURE_KEY_CLEAR  */                
                return(status);
            }
        }
        if (public_cipher_method -> nx_crypto_operation == NX_NULL)
        {
#ifdef NX_SECURE_KEY_CLEAR
            NX_SECURE_MEMSET(generated_hash, 0, sizeof(generated_hash));
#endif /* NX_SECURE_KEY_CLEAR  */            
            return(NX_SECURE_TLS_MISSING_CRYPTO_ROUTINE);
        }

        status = public_cipher_method -> nx_crypto_operation(NX_CRYPTO_EC_CURVE_SET, handler,
                                                             public_cipher_method, NX_NULL, 0,
                                                             (UCHAR *)curve_method, sizeof(NX_CRYPTO_METHOD *), NX_NULL,
                                                             NX_NULL, 0,
                                                             certificate -> nx_secure_x509_public_cipher_metadata_area,
                                                             certificate -> nx_secure_x509_public_cipher_metadata_size,
                                                             NX_NULL, NX_NULL);
        if (status != NX_CRYPTO_SUCCESS)
        {
#ifdef NX_SECURE_KEY_CLEAR
            NX_SECURE_MEMSET(generated_hash, 0, sizeof(generated_hash));
#endif /* NX_SECURE_KEY_CLEAR  */            

            return(status);
        }

        status = public_cipher_method -> nx_crypto_operation(NX_CRYPTO_VERIFY, handler,
                                                             public_cipher_method,
                                                             (UCHAR *)ec_pubkey -> nx_secure_ec_public_key,
                                                             (NX_CRYPTO_KEY_SIZE)(ec_pubkey -> nx_secure_ec_public_key_length << 3),
                                                             generated_hash,
                                                             hash_method -> nx_crypto_ICV_size_in_bits >> 3,
                                                             NX_NULL,
                                                             (UCHAR *)signature_data,
                                                             signature_length,
                                                             certificate -> nx_secure_x509_public_cipher_metadata_area,
                                                             certificate -> nx_secure_x509_public_cipher_metadata_size,
                                                             NX_NULL, NX_NULL);
#ifdef NX_SECURE_KEY_CLEAR
        NX_SECURE_MEMSET(generated_hash, 0, sizeof(generated_hash));
#endif /* NX_SECURE_KEY_CLEAR  */

        if (status == NX_CRYPTO_SUCCESS)
        {
            return(NX_SUCCESS);
        }
    }
    else
    {
        return(NX_SECURE_TLS_UNSUPPORTED_PUBLIC_CIPHER);
    }

#ifdef NX_SECURE_KEY_CLEAR
        NX_SECURE_MEMSET(generated_hash, 0, sizeof(generated_hash));
#endif /* NX_SECURE_KEY_CLEAR  */

    /* Comparison failed, return error. */
    return(NX_SECURE_X509_CRL_SIGNATURE_CHECK_FAILED);
}
#endif /* NX_SECURE_X509_DISABLE_CRL */
