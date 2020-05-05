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
/**   NULL Cipher                                                         */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nx_crypto_null.h                                    PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the basic Application Interface (API) to the      */
/*    NetX Crypto NULL module.                                            */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*  08-15-2019     Timothy Stapko           Modified comment(s),          */
/*                                            added logic so NetX Crypto  */
/*                                            is FIPS 140-2 compliant,    */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/

#ifndef NX_CRYPTO_NULL_H
#define NX_CRYPTO_NULL_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

/* Include the ThreadX and port-specific data type file.  */

#include "nx_api.h"
#include "nx_crypto.h"

/* Function prototypes */

UINT _nx_crypto_method_null_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
                                      VOID *handle, /* Crypto handler */
                                      struct NX_CRYPTO_METHOD_STRUCT *method,
                                      UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                      UCHAR *input, ULONG input_length_in_byte,
                                      UCHAR *iv_ptr,
                                      UCHAR *output, ULONG output_length_in_byte,
                                      VOID *crypto_metadata, ULONG crypto_metadata_size,
                                      VOID *packet_ptr,
                                      VOID (*nx_crypto_hw_process_callback)(VOID *packet_ptr, UINT status));

UINT _nx_crypto_method_null_init(struct NX_CRYPTO_METHOD_STRUCT *method,
                                 UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                 VOID **handle,
                                 VOID *crypto_metadata, ULONG crypto_metadata_size);

UINT _nx_crypto_method_null_cleanup(VOID *crypto_metadata);

#ifdef __cplusplus
}
#endif

#endif /* NX_CRYPTO_NULL_H */

