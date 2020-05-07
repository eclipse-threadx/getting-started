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
/**   HMAC Mode                                                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nx_crypto_hmac.h                                    PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the basic Application Interface (API) to the      */
/*    NetX Crypto HMAC module.                                            */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko              Initial Version 5.11       */
/*  08-15-2019     Timothy Stapko           Modified comment(s),          */
/*                                            added logic so NetX Crypto  */
/*                                            is FIPS 140-2 compliant,    */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/

#ifndef NX_CRYPTO_HMAC_H
#define NX_CRYPTO_HMAC_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

/* Include the ThreadX and port-specific data type file.  */

#include "nx_api.h"
#include "nx_crypto.h"

typedef struct NX_CRYPTO_HMAC_STRUCT
{
    VOID  *context;
    UCHAR *k_ipad;
    UCHAR *k_opad;
    UINT   algorithm;
    UINT   block_size;
    UINT   output_length;
    UINT   (*crypto_initialize)(VOID *, UINT);
    UINT   (*crypto_update)(VOID *, UCHAR *, UINT);
    UINT   (*crypto_digest_calculate)(VOID *, UCHAR *, UINT);
} NX_CRYPTO_HMAC;

UINT _nx_crypto_hmac(NX_CRYPTO_HMAC *crypto_matadata,
                     UCHAR *input_ptr, UINT input_length,
                     UCHAR *key_ptr, UINT key_length,
                     UCHAR *digest_ptr, UINT digest_length);

UINT _nx_crypto_hmac_initialize(NX_CRYPTO_HMAC *crypto_matadata, UCHAR *key_ptr, UINT key_length);

UINT _nx_crypto_hmac_update(NX_CRYPTO_HMAC *crypto_matadata, UCHAR *input_ptr, UINT input_length);

UINT _nx_crypto_hmac_digest_calculate(NX_CRYPTO_HMAC *crypto_matadata, UCHAR *digest_ptr, UINT digest_length);

VOID _nx_crypto_hmac_metadata_set(NX_CRYPTO_HMAC *hmac_metadata,
                                  VOID *context,  UCHAR *k_ipad, UCHAR  *k_opad,
                                  UINT algorithm, UINT block_size, UINT output_length,
                                  UINT (*crypto_initialize)(VOID *, UINT),
                                  UINT (*crypto_update)(VOID *, UCHAR *, UINT),
                                  UINT (*crypto_digest_calculate)(VOID *, UCHAR *, UINT));


#ifdef __cplusplus
}
#endif


#endif /* NX_CRYPTO_HAMC_H */

