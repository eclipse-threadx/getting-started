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
/**   CCM Mode                                                            */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nx_crypto_ccm.h                                     PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the basic Application Interface (API) to the      */
/*    NetX Crypto CCM module.                                             */
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

#ifndef NX_CRYPTO_CCM_H
#define NX_CRYPTO_CCM_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

/* Include the ThreadX and port-specific data type file.  */

#include "nx_api.h"
#include "nx_crypto.h"

#define NX_CRYPTO_CCM_BLOCK_SIZE 16

UINT _nx_crypto_ccm_authentication_add(VOID *crypto_metadata, UINT (*crypto_function)(VOID *, UCHAR *, UCHAR *, UINT),
                                       UINT (*key_set_function)(VOID *, UCHAR *, UINT),
                                       VOID *additional_data, UINT additional_len,
                                       UCHAR *input, UCHAR *output, UINT length,
                                       UCHAR *iv, UCHAR icv_len, UINT block_size);

UINT _nx_crypto_ccm_authentication_check(VOID *crypto_metadata, UINT (*crypto_function)(VOID *, UCHAR *, UCHAR *, UINT),
                                         UINT (*key_set_function)(VOID *, UCHAR *, UINT),
                                         VOID *additional_data, UINT additional_len,
                                         UCHAR *input, UCHAR *output, UINT length,
                                         UCHAR *iv, UCHAR icv_len, UINT block_size);

UINT _nx_crypto_ccm_encrypt(VOID *crypto_metadata, UINT (*crypto_function)(VOID *, UCHAR *, UCHAR *, UINT),
                            UINT (*key_set_function)(VOID *, UCHAR *, UINT),
                            VOID *additional_data, UINT additional_len,
                            UCHAR *input, UCHAR *output, UINT length,
                            UCHAR *iv, UCHAR icv_len, UINT block_size);

#define _nx_crypto_ccm_decrypt _nx_crypto_ccm_encrypt


#ifdef __cplusplus
}
#endif


#endif /* NX_CRYPTO_CCM_H */

