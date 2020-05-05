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
/**   DES Encryption Standard (DES)                                       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    nx_crypto_des.h                                     PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the NetX DES encryption algorithm, derived        */
/*    principally from FIPS-46. From an 8 bytes of raw input, the DES     */
/*    encryption routine produces an 8-byte encryption of the input.      */
/*    Conversely, from an 8-byte encryption, the decryption routine       */
/*    produces the original 8 bytes of input. Note that the caller must   */
/*    ensure 8 bytes of input and output are provided.                    */
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
/*                                            fixed the compiler error for*/
/*                                            cast-function-type,         */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/

#ifndef  _NX_CRYPTO_DES_H_
#define  _NX_CRYPTO_DES_H_

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include "nx_crypto.h"


#define NX_CRYPTO_DES_KEY_LEN_IN_BITS    64
#define NX_CRYPTO_DES_BLOCK_SIZE_IN_BITS 64
#define NX_CRYPTO_DES_IV_LEN_IN_BITS     64

UINT _nx_crypto_method_des_init(struct NX_CRYPTO_METHOD_STRUCT *method,
                                UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                VOID **handle,
                                VOID *crypto_metadata,
                                ULONG crypto_metadata_size);

UINT _nx_crypto_method_des_cleanup(VOID *crypto_metadata);

UINT _nx_crypto_method_des_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
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

/* Define the DES context structure.  */

typedef struct NX_CRYPTO_DES_STRUCT
{

    ULONG nx_des_encryption_keys[32];           /* Contains the encryption keys     */
    ULONG nx_des_decryption_keys[32];           /* Contains the decryption keys     */
} NX_CRYPTO_DES;


/* Define the function prototypes for DES.  */

UINT _nx_crypto_des_key_set(NX_CRYPTO_DES * context, UCHAR key[8]);
UINT _nx_crypto_des_encrypt(NX_CRYPTO_DES * context, UCHAR source[8], UCHAR destination[8], UINT length);
UINT _nx_crypto_des_decrypt(NX_CRYPTO_DES * context, UCHAR source[8], UCHAR destination[8], UINT length);
VOID _nx_crypto_des_process_block(UCHAR source[8], UCHAR destination[8], ULONG keys[32]);

#ifdef __cplusplus
}
#endif

#endif

