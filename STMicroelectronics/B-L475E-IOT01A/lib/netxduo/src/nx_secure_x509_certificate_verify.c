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

#include "nx_secure_tls.h"
#include "nx_secure_x509.h"

static UCHAR generated_hash[64];       /* We need to be able to hold the entire generated hash - SHA-512 = 64 bytes. */
static UCHAR decrypted_signature[512]; /* This needs to hold the entire decrypted data - RSA 2048-bit key = 256 bytes. */



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_x509_certificate_verify                  PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function verifies a certificate by checking its signature      */
/*    against its issuer's public key.                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    store                                 Pointer to certificate store  */
/*    certificate                           Pointer to certificate        */
/*    issuer_certificate                    Pointer to issuer certificate */
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
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_x509_certificate_chain_verify                            */
/*                                          Verify cert against stores    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s), added    */
/*                                            logic to support vendor-    */
/*                                            defined private key type,   */
/*                                            fixed compiler warnings,    */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s), and      */
/*                                            added flexibility of using  */
/*                                            macros instead of direct C  */
/*                                            library function calls, add */
/*                                            logic to properly initialize*/
/*                                            the crypto control blcok,   */
/*                                            fixed the usage of crypto   */
/*                                            metadata for hash method,   */
/*                                            updated error return checks,*/
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_x509_certificate_verify(NX_SECURE_X509_CERTIFICATE_STORE *store,
                                        NX_SECURE_X509_CERT *certificate,
                                        NX_SECURE_X509_CERT *issuer_certificate)
{
UINT                   status;
UINT                   oid_length;
const UCHAR           *oid;
UINT                   decrypted_hash_length;
const UCHAR           *decrypted_hash;
const UCHAR           *certificate_verify_data;
UINT                   verify_data_length;
const UCHAR           *signature_data;
UINT                   signature_length;
UINT                   compare_result;
UINT                   hash_length;
NX_CRYPTO_METHOD      *hash_method;
NX_CRYPTO_METHOD      *public_cipher_method;
NX_SECURE_X509_CRYPTO *crypto_methods;
VOID                  *handler = NX_NULL;

    NX_PARAMETER_NOT_USED(store);

    NX_SECURE_X509_CERTIFICATE_VERIFY_EXTENSION

    /* Get working pointers to relevant data. */
    certificate_verify_data = certificate -> nx_secure_x509_certificate_data;
    verify_data_length = certificate -> nx_secure_x509_certificate_data_length;
    signature_data = certificate -> nx_secure_x509_signature_data;
    signature_length = certificate -> nx_secure_x509_signature_data_length;

    /* Find certificate crypto methods for this certificate. */
    status = _nx_secure_x509_find_certificate_methods(certificate, (USHORT)certificate -> nx_secure_x509_signature_algorithm, &crypto_methods);
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Assign local pointers for the crypto methods. */
    hash_method = crypto_methods -> nx_secure_x509_hash_method;
    public_cipher_method = crypto_methods -> nx_secure_x509_public_cipher_method;

    NX_SECURE_MEMSET(generated_hash, 0, sizeof(generated_hash));
    NX_SECURE_MEMSET(decrypted_signature, 0, sizeof(decrypted_signature));

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

    /* We need to generate a hash of this certificate in order to verify it against our trusted store. */
    if (hash_method -> nx_crypto_operation != NX_NULL)
    {
        status = hash_method -> nx_crypto_operation(NX_CRYPTO_VERIFY,
                                           handler,
                                           hash_method,
                                           NX_NULL,
                                           0,
                                           (UCHAR *)certificate_verify_data,
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



    /* Perform a public-key decryption operation on the extracted signature from the certificate.
     * In this case, the operation is doing a "reverse decryption", using the public key to decrypt, rather
     * than the private. This allows us to tie a trusted root certificate to a signature of a certificate
     * signed by that root CA's private key. when combined with a hash method, this is the basic digital
     * signature operation. */

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
        /* Clear secrets state on errors. */
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

    /* Comparison failed, return error. */
    return(NX_SECURE_TLS_CERTIFICATE_SIG_CHECK_FAILED);
}

