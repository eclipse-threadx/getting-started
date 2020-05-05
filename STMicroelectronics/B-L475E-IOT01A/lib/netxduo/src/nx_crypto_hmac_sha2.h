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
/**   HMAC SHA256 Digest Algorithm (SHA256)                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    nx_crypto_hmac_sha1.h                                PORTABLE C     */
/*                                                           5.12         */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the NetX HMAC SHA256 algorithm, derived from      */
/*    RFC2202. From a user-specified number of input bytes and key, this  */
/*    produces a 32-byte (256-bit) digest or sometimes called a hash      */
/*    value. The resulting digest is returned in a 32-byte array supplied */
/*    by the caller.                                                      */
/*                                                                        */
/*    It is assumed that nx_api.h and nx_port.h have already been         */
/*    included.                                                           */
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

#ifndef  NX_HMAC_SHA2_H
#define  NX_HMAC_SHA2_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include "nx_crypto.h"
#include "nx_crypto_sha2.h"

#define NX_CRYPTO_HMAC_SHA256_KEY_LEN_IN_BITS      256

#define NX_CRYPTO_HMAC_SHA224_ICV_FULL_LEN_IN_BITS NX_CRYPTO_SHA224_ICV_LEN_IN_BITS
#define NX_CRYPTO_HMAC_SHA256_ICV_FULL_LEN_IN_BITS NX_CRYPTO_SHA256_ICV_LEN_IN_BITS

/* Define the control block structure for backward compatibility. */
#define NX_SHA256_HMAC                          NX_CRYPTO_SHA256_HMAC

typedef struct NX_CRYPTO_SHA256_HMAC_STRUCT
{
    NX_CRYPTO_SHA256    nx_sha256_hmac_context;
    UCHAR               nx_sha256_hmac_k_ipad[64];
    UCHAR               nx_sha256_hmac_k_opad[64];
} NX_CRYPTO_SHA256_HMAC;

/* Define the function prototypes for HMAC SHA256.  */

UINT _nx_crypto_method_hmac_sha256_init(struct  NX_CRYPTO_METHOD_STRUCT *method,
                                        UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                        VOID  **handle,
                                        VOID  *crypto_metadata,
                                        ULONG crypto_metadata_size);

UINT _nx_crypto_method_hmac_sha256_cleanup(VOID *crypto_metadata);

UINT _nx_crypto_method_hmac_sha256_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
                                             VOID *handle, /* Crypto handler */
                                             struct NX_CRYPTO_METHOD_STRUCT *method,
                                             UCHAR *key,
                                             NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                             UCHAR *input,
                                             ULONG input_length_in_byte,
                                             UCHAR *iv_ptr,
                                             UCHAR *output,
                                             ULONG output_length_in_byte,
                                             VOID *crypto_metadata,
                                             ULONG crypto_metadata_size,
                                             VOID *packet_ptr,
                                             VOID (*nx_crypto_hw_process_callback)(VOID *packet_ptr, UINT status));

#endif

#ifdef __cplusplus
}
#endif
