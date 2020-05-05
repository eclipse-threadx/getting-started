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
/** NetX Crypto Component                                                 */
/**                                                                       */
/**   Transport Layer Security (TLS)                                      */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "nx_secure_tls.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_crypto_generic_ciphersuites                      PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*     This table of function pointers provides a mapping from TLS        */
/*     ciphersuites to the necessary cryptographic methods for a given    */
/*     platform. It can be used as a model to develop a hardware-specific */
/*     cryptography table for TLS.                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
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
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*  08-15-2019     Timothy Stapko           Modified comment(s), added    */
/*                                            operation method for ECC,   */
/*                                            internal logic clean up,    */
/*                                            removed CCM ciphersuites    */
/*                                            when AEAD was disabled,     */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/

/* Define cryptographic methods for use with TLS. */

extern NX_CRYPTO_METHOD crypto_method_null;
extern NX_CRYPTO_METHOD crypto_method_aes_cbc_128;
extern NX_CRYPTO_METHOD crypto_method_aes_cbc_256;
extern NX_CRYPTO_METHOD crypto_method_aes_ccm_8;
extern NX_CRYPTO_METHOD crypto_method_ecdsa;
extern NX_CRYPTO_METHOD crypto_method_ecdh;
extern NX_CRYPTO_METHOD crypto_method_ecdhe;
extern NX_CRYPTO_METHOD crypto_method_hmac_sha1;
extern NX_CRYPTO_METHOD crypto_method_hmac_sha256;
extern NX_CRYPTO_METHOD crypto_method_hmac_md5;
extern NX_CRYPTO_METHOD crypto_method_rsa;
extern NX_CRYPTO_METHOD crypto_method_auth_psk;
extern NX_CRYPTO_METHOD crypto_method_ec_secp192;
extern NX_CRYPTO_METHOD crypto_method_ec_secp224;
extern NX_CRYPTO_METHOD crypto_method_ec_secp256;
extern NX_CRYPTO_METHOD crypto_method_ec_secp384;
extern NX_CRYPTO_METHOD crypto_method_ec_secp521;
extern NX_CRYPTO_METHOD crypto_method_md5;
extern NX_CRYPTO_METHOD crypto_method_sha1;
extern NX_CRYPTO_METHOD crypto_method_sha224;
extern NX_CRYPTO_METHOD crypto_method_sha256;
extern NX_CRYPTO_METHOD crypto_method_sha384;
extern NX_CRYPTO_METHOD crypto_method_sha512;
extern NX_CRYPTO_METHOD crypto_method_tls_prf_1;
extern NX_CRYPTO_METHOD crypto_method_tls_prf_sha256;

/* Ciphersuite table without ECC. */
/* Lookup table used to map ciphersuites to cryptographic routines. */
NX_SECURE_TLS_CIPHERSUITE_INFO _nx_crypto_ciphersuite_lookup_table[] =
{
    /* Ciphersuite,                           public cipher,            public_auth,              session cipher & cipher mode,   iv size, key size,  hash method,                    hash size, TLS PRF */
    {TLS_RSA_WITH_AES_256_CBC_SHA256,         &crypto_method_rsa,       &crypto_method_rsa,       &crypto_method_aes_cbc_256,     16,      32,        &crypto_method_hmac_sha256,     32,        &crypto_method_tls_prf_sha256},
    {TLS_RSA_WITH_AES_256_CBC_SHA,            &crypto_method_rsa,       &crypto_method_rsa,       &crypto_method_aes_cbc_256,     16,      32,        &crypto_method_hmac_sha1,       20,        &crypto_method_tls_prf_sha256},
    {TLS_RSA_WITH_AES_128_CBC_SHA256,         &crypto_method_rsa,       &crypto_method_rsa,       &crypto_method_aes_cbc_128,     16,      16,        &crypto_method_hmac_sha256,     32,        &crypto_method_tls_prf_sha256},
    {TLS_RSA_WITH_AES_128_CBC_SHA,            &crypto_method_rsa,       &crypto_method_rsa,       &crypto_method_aes_cbc_128,     16,      16,        &crypto_method_hmac_sha1,       20,        &crypto_method_tls_prf_sha256},

#ifdef NX_SECURE_ENABLE_PSK_CIPHERSUITES
    {TLS_PSK_WITH_AES_128_CBC_SHA,            &crypto_method_null,      &crypto_method_auth_psk,  &crypto_method_aes_cbc_128,     16,      16,        &crypto_method_hmac_sha1,       20,        &crypto_method_tls_prf_sha256},
    {TLS_PSK_WITH_AES_256_CBC_SHA,            &crypto_method_null,      &crypto_method_auth_psk,  &crypto_method_aes_cbc_256,     16,      32,        &crypto_method_hmac_sha1,       20,        &crypto_method_tls_prf_sha256},
    {TLS_PSK_WITH_AES_128_CBC_SHA256,         &crypto_method_null,      &crypto_method_auth_psk,  &crypto_method_aes_cbc_128,     16,      16,        &crypto_method_hmac_sha256,     32,        &crypto_method_tls_prf_sha256},
#ifdef NX_SECURE_ENABLE_AEAD_CIPHER
    {TLS_PSK_WITH_AES_128_CCM_8,              &crypto_method_null,      &crypto_method_auth_psk,  &crypto_method_aes_ccm_8,       16,      16,        &crypto_method_null,            0,         &crypto_method_tls_prf_sha256},
#endif
#endif /* NX_SECURE_ENABLE_PSK_CIPHERSUITES */

    {TLS_RSA_WITH_NULL_SHA,                   &crypto_method_rsa,       &crypto_method_rsa,       &crypto_method_null,            0,       0,         &crypto_method_hmac_sha1,       20,        &crypto_method_tls_prf_sha256},
    {TLS_RSA_WITH_NULL_MD5,                   &crypto_method_rsa,       &crypto_method_rsa,       &crypto_method_null,            0,       0,         &crypto_method_hmac_md5,        16,        &crypto_method_tls_prf_sha256},
};

const UINT _nx_crypto_ciphersuite_lookup_table_size = sizeof(_nx_crypto_ciphersuite_lookup_table) / sizeof(NX_SECURE_TLS_CIPHERSUITE_INFO);

/* Lookup table for X.509 digital certificates - they need a public-key algorithm and a hash routine for verification. */
NX_SECURE_X509_CRYPTO _nx_crypto_x509_cipher_lookup_table[] =
{
    /* OID identifier,                        public cipher,            hash method */
    {NX_SECURE_TLS_X509_TYPE_RSA_MD5,        &crypto_method_rsa,       &crypto_method_md5},
    {NX_SECURE_TLS_X509_TYPE_RSA_SHA_1,      &crypto_method_rsa,       &crypto_method_sha1},
    {NX_SECURE_TLS_X509_TYPE_RSA_SHA_256,    &crypto_method_rsa,       &crypto_method_sha256},
    {NX_SECURE_TLS_X509_TYPE_RSA_SHA_384,    &crypto_method_rsa,       &crypto_method_sha384},
    {NX_SECURE_TLS_X509_TYPE_RSA_SHA_512,    &crypto_method_rsa,       &crypto_method_sha512},
};

const UINT _nx_crypto_x509_cipher_lookup_table_size = sizeof(_nx_crypto_x509_cipher_lookup_table) / sizeof(NX_SECURE_X509_CRYPTO);

/* Define the object we can pass into TLS. */
const NX_SECURE_TLS_CRYPTO nx_crypto_tls_ciphers =
{
    /* Ciphersuite lookup table and size. */
    _nx_crypto_ciphersuite_lookup_table,
    sizeof(_nx_crypto_ciphersuite_lookup_table) / sizeof(NX_SECURE_TLS_CIPHERSUITE_INFO),

#ifndef NX_SECURE_DISABLE_X509
    /* X.509 certificate cipher table and size. */
    _nx_crypto_x509_cipher_lookup_table,
    sizeof(_nx_crypto_x509_cipher_lookup_table) / sizeof(NX_SECURE_X509_CRYPTO),
#endif

    /* TLS version-specific methods. */
#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
    &crypto_method_md5,
    &crypto_method_sha1,
    &crypto_method_tls_prf_1,
#endif

#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
    &crypto_method_sha256,
    &crypto_method_tls_prf_sha256
#endif
};


#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
/* Ciphersuite table with ECC. */
/* Lookup table used to map ciphersuites to cryptographic routines. */
NX_SECURE_TLS_CIPHERSUITE_INFO _nx_crypto_ciphersuite_lookup_table_ecc[] =
{
    /* Ciphersuite,                           public cipher,            public_auth,              session cipher & cipher mode,   iv size, key size,  hash method,                    hash size, TLS PRF */
    {TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256, &crypto_method_ecdhe,     &crypto_method_ecdsa,     &crypto_method_aes_cbc_128,     16,      16,        &crypto_method_hmac_sha256,     32,        &crypto_method_tls_prf_sha256},
    {TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA,    &crypto_method_ecdhe,     &crypto_method_ecdsa,     &crypto_method_aes_cbc_128,     16,      16,        &crypto_method_hmac_sha1,       20,        &crypto_method_tls_prf_sha256},
    {TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA,    &crypto_method_ecdhe,     &crypto_method_ecdsa,     &crypto_method_aes_cbc_256,     16,      32,        &crypto_method_hmac_sha1,       20,        &crypto_method_tls_prf_sha256},
    {TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256,   &crypto_method_ecdhe,     &crypto_method_rsa,       &crypto_method_aes_cbc_128,     16,      16,        &crypto_method_hmac_sha256,     32,        &crypto_method_tls_prf_sha256},
    {TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA,      &crypto_method_ecdhe,     &crypto_method_rsa,       &crypto_method_aes_cbc_128,     16,      16,        &crypto_method_hmac_sha1,       20,        &crypto_method_tls_prf_sha256},
    {TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA,      &crypto_method_ecdhe,     &crypto_method_rsa,       &crypto_method_aes_cbc_256,     16,      32,        &crypto_method_hmac_sha1,       20,        &crypto_method_tls_prf_sha256},

    {TLS_RSA_WITH_AES_256_CBC_SHA256,         &crypto_method_rsa,       &crypto_method_rsa,       &crypto_method_aes_cbc_256,     16,      32,        &crypto_method_hmac_sha256,     32,        &crypto_method_tls_prf_sha256},
    {TLS_RSA_WITH_AES_256_CBC_SHA,            &crypto_method_rsa,       &crypto_method_rsa,       &crypto_method_aes_cbc_256,     16,      32,        &crypto_method_hmac_sha1,       20,        &crypto_method_tls_prf_sha256},
    {TLS_RSA_WITH_AES_128_CBC_SHA256,         &crypto_method_rsa,       &crypto_method_rsa,       &crypto_method_aes_cbc_128,     16,      16,        &crypto_method_hmac_sha256,     32,        &crypto_method_tls_prf_sha256},
    {TLS_RSA_WITH_AES_128_CBC_SHA,            &crypto_method_rsa,       &crypto_method_rsa,       &crypto_method_aes_cbc_128,     16,      16,        &crypto_method_hmac_sha1,       20,        &crypto_method_tls_prf_sha256},

#ifdef NX_SECURE_ENABLE_PSK_CIPHERSUITES
    {TLS_PSK_WITH_AES_128_CBC_SHA,            &crypto_method_null,      &crypto_method_auth_psk,  &crypto_method_aes_cbc_128,     16,      16,        &crypto_method_hmac_sha1,       20,        &crypto_method_tls_prf_sha256},
    {TLS_PSK_WITH_AES_256_CBC_SHA,            &crypto_method_null,      &crypto_method_auth_psk,  &crypto_method_aes_cbc_256,     16,      32,        &crypto_method_hmac_sha1,       20,        &crypto_method_tls_prf_sha256},
    {TLS_PSK_WITH_AES_128_CBC_SHA256,         &crypto_method_null,      &crypto_method_auth_psk,  &crypto_method_aes_cbc_128,     16,      16,        &crypto_method_hmac_sha256,     32,        &crypto_method_tls_prf_sha256},
#ifdef NX_SECURE_ENABLE_AEAD_CIPHER
    {TLS_PSK_WITH_AES_128_CCM_8,              &crypto_method_null,      &crypto_method_auth_psk,  &crypto_method_aes_ccm_8,       16,      16,        &crypto_method_null,            0,         &crypto_method_tls_prf_sha256},
#endif
#endif /* NX_SECURE_ENABLE_PSK_CIPHERSUITES */

    {TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256,  &crypto_method_ecdh,      &crypto_method_ecdsa,     &crypto_method_aes_cbc_128,     16,      16,        &crypto_method_hmac_sha256,     32,        &crypto_method_tls_prf_sha256},
    {TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA,     &crypto_method_ecdh,      &crypto_method_ecdsa,     &crypto_method_aes_cbc_128,     16,      16,        &crypto_method_hmac_sha1,       20,        &crypto_method_tls_prf_sha256},
    {TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA,     &crypto_method_ecdh,      &crypto_method_ecdsa,     &crypto_method_aes_cbc_256,     16,      32,        &crypto_method_hmac_sha1,       20,        &crypto_method_tls_prf_sha256},
    {TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256,    &crypto_method_ecdh,      &crypto_method_rsa,       &crypto_method_aes_cbc_128,     16,      16,        &crypto_method_hmac_sha256,     32,        &crypto_method_tls_prf_sha256},
    {TLS_ECDH_RSA_WITH_AES_128_CBC_SHA,       &crypto_method_ecdh,      &crypto_method_rsa,       &crypto_method_aes_cbc_128,     16,      16,        &crypto_method_hmac_sha1,       20,        &crypto_method_tls_prf_sha256},
    {TLS_ECDH_RSA_WITH_AES_256_CBC_SHA,       &crypto_method_ecdh,      &crypto_method_rsa,       &crypto_method_aes_cbc_256,     16,      32,        &crypto_method_hmac_sha1,       20,        &crypto_method_tls_prf_sha256},

    {TLS_RSA_WITH_NULL_SHA,                   &crypto_method_rsa,       &crypto_method_rsa,       &crypto_method_null,            0,       0,         &crypto_method_hmac_sha1,       20,        &crypto_method_tls_prf_sha256},
    {TLS_RSA_WITH_NULL_MD5,                   &crypto_method_rsa,       &crypto_method_rsa,       &crypto_method_null,            0,       0,         &crypto_method_hmac_md5,        16,        &crypto_method_tls_prf_sha256},
};

const UINT _nx_crypto_ciphersuite_lookup_table_ecc_size = sizeof(_nx_crypto_ciphersuite_lookup_table_ecc) / sizeof(NX_SECURE_TLS_CIPHERSUITE_INFO);

/* Lookup table for X.509 digital certificates - they need a public-key algorithm and a hash routine for verification. */
NX_SECURE_X509_CRYPTO _nx_crypto_x509_cipher_lookup_table_ecc[] =
{
    /* OID identifier,                        public cipher,            hash method */
    {NX_SECURE_TLS_X509_TYPE_RSA_MD5,        &crypto_method_rsa,       &crypto_method_md5},
    {NX_SECURE_TLS_X509_TYPE_RSA_SHA_1,      &crypto_method_rsa,       &crypto_method_sha1},
    {NX_SECURE_TLS_X509_TYPE_RSA_SHA_256,    &crypto_method_rsa,       &crypto_method_sha256},
    {NX_SECURE_TLS_X509_TYPE_RSA_SHA_384,    &crypto_method_rsa,       &crypto_method_sha384},
    {NX_SECURE_TLS_X509_TYPE_RSA_SHA_512,    &crypto_method_rsa,       &crypto_method_sha512},
    {NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_1,    &crypto_method_ecdsa,     &crypto_method_sha1},
    {NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_224,  &crypto_method_ecdsa,     &crypto_method_sha224},
    {NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_256,  &crypto_method_ecdsa,     &crypto_method_sha256},
    {NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_384,  &crypto_method_ecdsa,     &crypto_method_sha384},
    {NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_512,  &crypto_method_ecdsa,     &crypto_method_sha512},
};

const UINT _nx_crypto_x509_cipher_lookup_table_ecc_size = sizeof(_nx_crypto_x509_cipher_lookup_table_ecc) / sizeof(NX_SECURE_X509_CRYPTO);

/* Define the object we can pass into TLS. */
const NX_SECURE_TLS_CRYPTO nx_crypto_tls_ciphers_ecc =
{
    /* Ciphersuite lookup table and size. */
    _nx_crypto_ciphersuite_lookup_table_ecc,
    sizeof(_nx_crypto_ciphersuite_lookup_table_ecc) / sizeof(NX_SECURE_TLS_CIPHERSUITE_INFO),

#ifndef NX_SECURE_DISABLE_X509
    /* X.509 certificate cipher table and size. */
    _nx_crypto_x509_cipher_lookup_table_ecc,
    sizeof(_nx_crypto_x509_cipher_lookup_table_ecc) / sizeof(NX_SECURE_X509_CRYPTO),
#endif

    /* TLS version-specific methods. */
#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
    &crypto_method_md5,
    &crypto_method_sha1,
    &crypto_method_tls_prf_1,
#endif

#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
    &crypto_method_sha256,
    &crypto_method_tls_prf_sha256
#endif
};

const USHORT nx_crypto_ecc_supported_groups[] =
{
    (USHORT)NX_CRYPTO_EC_SECP256R1,
    (USHORT)NX_CRYPTO_EC_SECP384R1,
    (USHORT)NX_CRYPTO_EC_SECP521R1,
    (USHORT)NX_CRYPTO_EC_SECP224R1,
    (USHORT)NX_CRYPTO_EC_SECP192R1,
};

const NX_CRYPTO_METHOD *nx_crypto_ecc_curves[] =
{
    &crypto_method_ec_secp256,
    &crypto_method_ec_secp384,
    &crypto_method_ec_secp521,
    &crypto_method_ec_secp224,
    &crypto_method_ec_secp192,
};

const UINT nx_crypto_ecc_supported_groups_size = sizeof(nx_crypto_ecc_supported_groups) / sizeof(USHORT);
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */
