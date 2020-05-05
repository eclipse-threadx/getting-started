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
/**   SHA-512 Digest Algorithm (SHA5)                                     */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    nx_crypto_sha5.h                                    PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the NetX SHA512 component, derived primarily      */
/*    from NIST FIPS PUB 180-4 (Crypto Hash Standard).                    */
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



#ifndef SRC_NX_CRYPTO_SHA5_H_
#define SRC_NX_CRYPTO_SHA5_H_

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif


#include "nx_crypto.h"

#ifndef ULONG64
#define ULONG64                              unsigned long long
#endif /* ULONG64 */

#ifndef LONG64
#define LONG64                               long long
#endif /* LONG64 */

#define NX_CRYPTO_SHA512_BLOCK_SIZE_IN_BYTES    128
#define NX_CRYPTO_SHA384_ICV_LEN_IN_BITS        384
#define NX_CRYPTO_SHA512_ICV_LEN_IN_BITS        512
#define NX_CRYPTO_SHA512_224_ICV_LEN_IN_BITS    224
#define NX_CRYPTO_SHA512_256_ICV_LEN_IN_BITS    256

/* Define the control block structure for backward compatibility. */
#define NX_SHA512                               NX_CRYPTO_SHA512

typedef struct NX_CRYPTO_SHA512_STRUCT
{

    ULONG64 nx_sha512_states[8];                        /* Contains each state (A,B,C,D,E,F,G,H).   */
    ULONG64 nx_sha512_bit_count[2];                     /* Contains the 128-bit total bit            */
                                                        /*   count, where index 0 holds the         */
                                                        /*   least significant bit count and        */
                                                        /*   index 1 contains the most              */
                                                        /*   significant portion of the bit         */
                                                        /*   count.                                 */
    UCHAR nx_sha512_buffer[NX_CRYPTO_SHA512_BLOCK_SIZE_IN_BYTES];
                                                        /* Working buffer for SHA512 algorithm      */
                                                        /*   where partial buffers are              */
                                                        /*   accumulated until a full block         */
                                                        /*   can be processed.                      */
    ULONG64 nx_sha512_word_array[128];                  /* Working 64 word array.                   */
} NX_CRYPTO_SHA512;


UINT _nx_crypto_sha512_initialize(NX_CRYPTO_SHA512 *context, UINT algorithm);
UINT _nx_crypto_sha512_update(NX_CRYPTO_SHA512 *context, UCHAR *input_ptr, UINT input_length);
UINT _nx_crypto_sha512_digest_calculate(NX_CRYPTO_SHA512 *context, UCHAR *digest, UINT algorithm);
VOID _nx_crypto_sha512_process_buffer(NX_CRYPTO_SHA512 *context, UCHAR *buffer);

UINT _nx_crypto_method_sha512_init(struct  NX_CRYPTO_METHOD_STRUCT *method,
                                   UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                   VOID  **handle,
                                   VOID  *crypto_metadata,
                                   ULONG crypto_metadata_size);

UINT _nx_crypto_method_sha512_cleanup(VOID *crypto_metadata);

UINT _nx_crypto_method_sha512_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
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

#endif /* SRC_NX_CRYPTO_SHA5_H_ */

