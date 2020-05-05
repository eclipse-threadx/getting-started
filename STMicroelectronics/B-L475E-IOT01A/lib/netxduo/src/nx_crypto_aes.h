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
/**   AES Encryption                                                      */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nx_crypto_aes.h                                     PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the basic Application Interface (API) to the      */
/*    NetX Crypto AES module.                                             */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*  08-15-2019     Yuxin Zhou               Modified comment(s),          */
/*                                            split operation functions   */
/*                                            for different modes,        */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/

#ifndef NX_CRYPTO_AES_H
#define NX_CRYPTO_AES_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

/* Include the ThreadX and port-specific data type file.  */

#include "nx_api.h"
#include "nx_crypto.h"



/* Constants. */
#define NX_CRYPTO_BITS_IN_UCHAR                  ((UINT)0x8)

/* Helper macros for bit indexing (used by the division operation). */
#define NX_CRYTPO_BIT_POSITION_BYTE_INDEX(x)     (x >> 3)                 /* Divide the bit position by 8 to get the byte array index.   */
#define NX_CRYPTO_BIT_POSITION_BIT_VALUE(x)      ((UINT)0x1 << (x & 0x7)) /* The bit to set (OR with the byte) is at (bit position % 8). */

/* Word-aligned versions. */
#define NX_CRYPTO_BIT_POSITION_WORD_INDEX(x)     (x >> 4)                 /* Divide the bit position by 16 to get the word array index.   */
#define NX_CRYPTO_BIT_POSITION_WORD_BIT_VALUE(x) ((UINT)0x1 << (x & 0xF)) /* The bit to set (OR with the byte) is at (bit position % 16). */


/* AES Support */

#define NX_CRYPTO_AES_STATE_ROWS                 (4)
#define NX_CRYPTO_AES_STATE_NB_BYTES             (4)

/* AES expects key sizes in the number of 32-bit words each key takes. */
#define NX_CRYPTO_AES_KEY_SIZE_128_BITS          (4)
#define NX_CRYPTO_AES_KEY_SIZE_192_BITS          (6)
#define NX_CRYPTO_AES_KEY_SIZE_256_BITS          (8)

#define NX_CRYPTO_AES_256_KEY_LEN_IN_BITS        (256)
#define NX_CRYPTO_AES_192_KEY_LEN_IN_BITS        (192)
#define NX_CRYPTO_AES_128_KEY_LEN_IN_BITS        (128)
#define NX_CRYPTO_AES_XCBC_MAC_KEY_LEN_IN_BITS   (128)

#define NX_CRYPTO_AES_MAX_KEY_SIZE               (NX_CRYPTO_AES_KEY_SIZE_256_BITS) /* Maximum key size in bytes. */

#define NX_CRYPTO_AES_BLOCK_SIZE                 (16)                              /* The AES block size for all NetX Crypto operations, in bytes. */
#define NX_CRYPTO_AES_BLOCK_SIZE_IN_BITS         (128)
#define NX_CRYPTO_AES_IV_LEN_IN_BITS             (128)
#define NX_CRYPTO_AES_CTR_IV_LEN_IN_BITS         (64)

#define NX_CRYPTO_AES_KEY_SCHEDULE_UNKNOWN       0
#define NX_CRYPTO_AES_KEY_SCHEDULE_ENCRYPT       1
#define NX_CRYPTO_AES_KEY_SCHEDULE_DECRYPT       2

/* Define the control block structure for backward compatibility. */
#define NX_AES                                   NX_CRYPTO_AES

typedef struct NX_CRYPTO_AES_STRUCT
{
    /* UINT nb; Number of bytes per column, equal to block length / 32  - ALWAYS == 4 */

    /* AES internal state, 4 rows (32 bits each) of nb bytes */
    UINT nx_crypto_aes_state[NX_CRYPTO_AES_STATE_NB_BYTES];

    /* Number of *words* in the cipher key - can be 4 (128 bits), 6 (192 bits), or 8 (256 bits). */
    USHORT nx_crypto_aes_key_size;

    /* Number of AES rounds for the current key. */
    UCHAR nx_crypto_aes_rounds;

    UCHAR nx_crypto_aes_reserved;

    /* The key schedule is as large as the key size (max = 256 bits) with expansion, total 64 UINT words. */
    UINT nx_crypto_aes_key_schedule[NX_CRYPTO_AES_MAX_KEY_SIZE * 8];
    UINT nx_crypto_aes_decrypt_key_schedule[NX_CRYPTO_AES_MAX_KEY_SIZE * 8];

    /* Pointer of additional data. */
    VOID *nx_crypto_aes_additional_data;

    /* Length of additional data. */
    UINT nx_crypto_aes_additional_data_len;
} NX_CRYPTO_AES;




UINT _nx_crypto_aes_encrypt(NX_CRYPTO_AES *aes_ptr, UCHAR *input, UCHAR *output, UINT length);
UINT _nx_crypto_aes_decrypt(NX_CRYPTO_AES *aes_ptr, UCHAR *input, UCHAR *output, UINT length);

UINT _nx_crypto_aes_key_set(NX_CRYPTO_AES *aes_ptr, UCHAR *key, UINT key_size);

UINT _nx_crypto_method_aes_init(struct NX_CRYPTO_METHOD_STRUCT *method,
                                UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                VOID **handle,
                                VOID *crypto_metadata,
                                ULONG crypto_metadata_size);

UINT _nx_crypto_method_aes_cleanup(VOID *crypto_metadata);

UINT _nx_crypto_method_aes_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
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

UINT  _nx_crypto_method_aes_cbc_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
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

UINT  _nx_crypto_method_aes_ccm_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
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

UINT  _nx_crypto_method_aes_ctr_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
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

UINT  _nx_crypto_method_aes_xcbc_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
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

#ifdef __cplusplus
}
#endif


#endif /* NX_CRYPTO_AES_H_ */

