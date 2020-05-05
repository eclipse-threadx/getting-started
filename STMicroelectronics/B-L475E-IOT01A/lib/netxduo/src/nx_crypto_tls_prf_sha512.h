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

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include "nx_crypto_phash.h"

/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    nx_crypto_tls_prf_sha256.h                          PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the TLS Pseudo-Random Function (PRF) as described */
/*    in RFC 5246. This PRF is used for default key generation in TLS     */
/*    version 1.2. Ciphersuites may choose their own PRF in TLS version   */
/*    1.2 as well.                                                        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*  08-15-2019     Timothy Stapko           Modified comment(s),          */
/*                                            added logic so NetX Crypto  */
/*                                            is FIPS 140-2 compliant,    */
/*                                            add C++ extern wrapper,     */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/

typedef struct NX_CRYPTO_TLS_PRF_SHA512_STRUCT
{
    NX_CRYPTO_PHASH nx_secure_tls_prf_phash_info;
    UCHAR nx_secure_tls_prf_label_seed_buffer[80]; /* phash_seed = label(13 bytes) || prf_seed(64 bytes) */
    UCHAR nx_secure_tls_prf_temp_A_buffer[144]; /* The temp_A buffer needs to be large enough to holdthe lable(13 bytes) || prf_seed(64 bytes) || hash_size(48 bytes for SHA512) */
    UCHAR nx_secure_tls_prf_temp_hmac_output_buffer[64]; /* The temp buffer for the output buffer of hmac(secret, A(i) + seed) */
    UCHAR nx_secure_tls_prf_hmac_metadata_area[sizeof(NX_CRYPTO_SHA512_HMAC)]; /* metadata buffer for the hmac function */
} NX_CRYPTO_TLS_PRF_SHA512;

UINT _nx_crypto_method_prf_sha512_init(struct NX_CRYPTO_METHOD_STRUCT *method,
                                       UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                       VOID **handle,
                                       VOID *crypto_metadata,
                                       ULONG crypto_metadata_size);

UINT _nx_crypto_method_prf_sha512_cleanup(VOID *crypto_metadata);

UINT _nx_crypto_method_prf_sha512_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
                                            VOID *handle, /* Crypto handler */
                                            struct NX_CRYPTO_METHOD_STRUCT *method,
                                            UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                            UCHAR *input, ULONG input_length_in_byte,
                                            UCHAR *iv_ptr,
                                            UCHAR *output, ULONG output_length_in_byte,
                                            VOID *crypto_metadata, ULONG crypto_metadata_size,
                                            VOID *packet_ptr,
                                            VOID (*nx_crypto_hw_process_callback)(VOID *packet_ptr, UINT status));



#ifdef __cplusplus
}
#endif

