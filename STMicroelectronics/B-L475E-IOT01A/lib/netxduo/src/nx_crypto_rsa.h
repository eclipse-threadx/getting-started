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
/**   RSA public-key encryption algorithm                                 */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nx_crypto_rsa.h                                     PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the basic Application Interface (API) to the      */
/*    NetX Crypto RSA module.                                             */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*  08-15-2019     Timothy Stapko           Modified comment(s),          */
/*                                            modified to support 4096    */
/*                                            bits by default,            */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/

#ifndef NX_CRYPTO_RSA_H
#define NX_CRYPTO_RSA_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

/* Include the ThreadX and port-specific data type file.  */

#include "nx_api.h"
#include "nx_crypto.h"

/* Define the maximum size of an RSA modulus supported in bits. */
#ifndef NX_CRYPTO_MAX_RSA_MODULUS_SIZE
#define NX_CRYPTO_MAX_RSA_MODULUS_SIZE    (4096) /* Default is to support 4096-bit RSA keys. */
#endif


/* Scratch buffer for RSA calculations.
    Size must be no less than 10 * sizeof(modulus) + 24. 2584 bytes for 2048 bits cryption.
    If CRT algorithm is not used, size must be no less than (7 * sizeof(modulus) + 8). 1800 bytes for 2048 bits cryption. */
#define NX_CRYPTO_RSA_SCRATCH_BUFFER_SIZE (((10 * (NX_CRYPTO_MAX_RSA_MODULUS_SIZE / 8)) + 24) / sizeof(USHORT))

/* Control block for RSA cryptographic operations. */
typedef struct NX_CRYPTO_RSA_STRUCT
{
    /* Pointer to the rsa modulus. */
    UCHAR *nx_crypto_rsa_modulus;

    /* RSA modulus length in bytes */
    UINT nx_crypto_rsa_modulus_length;

    /* Pointer to prime p. */
    UCHAR *nx_crypto_rsa_prime_p;

    /* Length of prime p in bytes. */
    UINT nx_crypto_rsa_prime_p_length;

    /* Pointer to prime q. */
    UCHAR *nx_crypto_rsa_prime_q;

    /* Length of prime q in bytes. */
    UINT nx_crypto_rsa_prime_q_length;

    /* Scratch buffer for RSA calculations. */
    USHORT nx_crypto_rsa_scratch_buffer[NX_CRYPTO_RSA_SCRATCH_BUFFER_SIZE];
} NX_CRYPTO_RSA;


/* Function prototypes */

UINT _nx_crypto_rsa_operation(const UCHAR *exponent, UINT exponent_length, const UCHAR *modulus, UINT modulus_length,
                              const UCHAR *p, UINT p_length, UCHAR *q, UINT q_length,
                              const UCHAR *input, UINT input_length, UCHAR *output,
                              USHORT *scratch_buf_ptr, UINT scratch_buf_length);

UINT _nx_crypto_method_rsa_cleanup(VOID *crypto_metadata);

UINT _nx_crypto_method_rsa_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
                                     VOID *handle, /* Crypto handler */
                                     struct NX_CRYPTO_METHOD_STRUCT *method,
                                     UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                     UCHAR *input, ULONG input_length_in_byte,
                                     UCHAR *iv_ptr,
                                     UCHAR *output, ULONG output_length_in_byte,
                                     VOID *crypto_metadata, ULONG crypto_metadata_size,
                                     VOID *packet_ptr,
                                     VOID (*nx_crypto_hw_process_callback)(VOID *packet_ptr, UINT status));

UINT _nx_crypto_method_rsa_init(struct NX_CRYPTO_METHOD_STRUCT *method,
                                UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                VOID **handle,
                                VOID *crypto_metadata, ULONG crypto_metadata_size);

#ifdef __cplusplus
}
#endif

#endif /* NX_CRYPTO_RSA_H */

