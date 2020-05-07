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
/**   Crypto                                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    nx_crypto.h                                         PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the NetX Security Encryption component.           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*  08-15-2019     Timothy Stapko           Modified comment(s),          */
/*                                            added operation method for  */
/*                                            elliptic curve cryptography,*/
/*                                            added logic so NetX Cryto   */
/*                                            is FIPS 140-2 compliant,    */
/*                                            add C++ extern wrapper,     */
/*                                            avoided unexpected          */
/*                                            optimization on memset,     */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/

#ifndef _NX_CRYPTO_H_
#define _NX_CRYPTO_H_

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif


#include "nx_api.h"
#include "nx_crypto_const.h"

#ifdef NX_CRYPTO_FIPS

VOID *_nx_crypto_fips_memcpy(void *dest, const void *src, size_t size);
VOID *_nx_crypto_fips_memmove(void *dest, const void *src, size_t size);
VOID *_nx_crypto_fips_memset(void *dest, int value, size_t size);
int   _nx_crypto_fips_memcmp(const void *dest, const void *src, size_t size);
UINT  _nx_crypto_drbg(UINT bits, UCHAR *result);

#ifdef _NX_CRYPTO_INITIALIZE_
VOID *(*volatile _nx_crypto_memset_ptr)(void *dest, int value, size_t size) = _nx_crypto_fips_memset;
VOID *(*volatile _nx_crypto_memcpy_ptr)(void *dest, const void *src, size_t size) = _nx_crypto_fips_memcpy;
#else
extern VOID *(*volatile _nx_crypto_memset_ptr)(void *dest, int value, size_t size);
extern VOID *(*volatile _nx_crypto_memcpy_ptr)(void *dest, const void *src, size_t size);
#endif

#ifndef NX_CRYPTO_MEMCPY
#define NX_CRYPTO_MEMCPY    _nx_crypto_memcpy_ptr
#endif

#ifndef NX_CRYPTO_MEMMOVE
#define NX_CRYPTO_MEMMOVE   _nx_crypto_fips_memmove
#endif

#ifndef NX_CRYPTO_MEMSET
#define NX_CRYPTO_MEMSET    _nx_crypto_memset_ptr
#endif

#ifndef NX_CRYPTO_MEMCMP
#define NX_CRYPTO_MEMCMP    _nx_crypto_fips_memcmp
#endif

#ifndef NX_CRYPTO_RBG
#define NX_CRYPTO_RBG       _nx_crypto_drbg
#endif

#define NX_CRYPTO_CONST

#else /* NON FIPS build. */

#ifdef _NX_CRYPTO_INITIALIZE_
VOID *(*volatile _nx_crypto_memset_ptr)(void *dest, int value, size_t size) = memset;
VOID *(*volatile _nx_crypto_memcpy_ptr)(void *dest, const void *src, size_t size) = memcpy;
#else
extern VOID *(*volatile _nx_crypto_memset_ptr)(void *dest, int value, size_t size);
extern VOID *(*volatile _nx_crypto_memcpy_ptr)(void *dest, const void *src, size_t size);
#endif

#ifndef NX_CRYPTO_MEMCPY
#define NX_CRYPTO_MEMCPY    _nx_crypto_memcpy_ptr
#endif

#ifndef NX_CRYPTO_MEMMOVE
#define NX_CRYPTO_MEMMOVE   memmove
#endif

#ifndef NX_CRYPTO_MEMSET
#define NX_CRYPTO_MEMSET    _nx_crypto_memset_ptr
#endif

#ifndef NX_CRYPTO_MEMCMP
#define NX_CRYPTO_MEMCMP    memcmp
#endif

#ifndef NX_CRYPTO_RBG
#define NX_CRYPTO_RBG       _nx_crypto_huge_number_rbg
#endif

#define NX_CRYPTO_CONST     const
#endif

#ifndef NX_CRYPTO_INTEGRITY_TEST
#define NX_CRYPTO_INTEGRITY_TEST
#endif

#ifndef NX_CRYPTO_RAND
#define NX_CRYPTO_RAND      NX_RAND
#endif

#ifdef NX_CRYPTO_FIPS

/* FIPS build forces NX_SECURE_KEY_CLEAR to be set */
#ifndef NX_SECURE_KEY_CLEAR
#define NX_SECURE_KEY_CLEAR
#endif /* NX_SECURE_KEY_CLEAR */

#ifdef _NX_CRYPTO_INITIALIZE_
unsigned int _nx_crypto_library_state = NX_CRYPTO_LIBRARY_STATE_UNINITIALIZED;
#else
extern unsigned int _nx_crypto_library_state;
#endif

#define NX_CRYPTO_STATE_CHECK  \
    if((_nx_crypto_library_state & (NX_CRYPTO_LIBRARY_STATE_OPERATIONAL | NX_CRYPTO_LIBRARY_STATE_POST_IN_PROGRESS)) == 0) \
        return(NX_CRYPTO_INVALID_LIBRARY);

#else

#define NX_CRYPTO_STATE_CHECK
#endif /* NX_CRYPTO_FIPS */

/* Keep functions not used which is compiler specific. */
#ifndef NX_CRYPTO_KEEP
#define NX_CRYPTO_KEEP
#endif /* NX_CRYPTO_KEEP */


/* Note that both input and output packets are prepared by the
   caller. For encryption/decryption operations, the callee shall
   use the output buffer for encrypted or decrypted data. For
   authentication operations, the callee shall use the output
   buffer for the digest.
   'crypto_metadata' Pointer to a storage space managed by the underlying crypto method.
   The content of this block is defined by each sa in use.
   'crypto_metadata_size' is the size of the crypto context block, in bytes. */
typedef struct NX_CRYPTO_INFO_STRUCT
{
    USHORT             nx_crypto_op;            /* Encrypt, Decrypt, Authenticate, Verify */
    NX_CRYPTO_KEY_SIZE nx_crypto_key_size_in_bits;
    UCHAR             *nx_crypto_key;
    UCHAR             *nx_crypto_iv_ptr;
    UCHAR             *nx_crypto_input;
    UCHAR             *nx_crypto_output;
    USHORT             nx_crypto_input_length_in_byte;
    USHORT             nx_crypto_output_length_in_byte;
    VOID              *nx_crypto_metadata;
    USHORT             nx_crypto_metadata_size;
    USHORT             nx_crypto_algorithm;
    volatile UINT      nx_crypto_status;
    VOID                (*nx_crypto_hw_process_callback)(VOID *packet_ptr, UINT status);
} NX_CRYPTO_INFO;

/* Notes on *nx_crypto_operation:
   The crypto method strcuture contains a function pointer: nx_crypto_operation.

   This function pointer should be set for the crypto method in use.  The arguments to this function are:
   nx_crypto_operation(UINT operation, UINT protocol, NX_CRYPTO_METHOD* crypto_method_ptr, NX_PACKET *packet_ptr)

   ESP/AH process invokes this function, specifies the operation to perform (encrypt, decrypt,
   digest computation, and the packet (data) to be performed on.

   This routine shall return SUCCESS/FAILURE on return.  */

typedef struct NX_CRYPTO_METHOD_STRUCT
{
    /* Name of the algorithm. For example:
       NX_CRYPTO_ENCRYPTOIN_3DES_CBC or NX_CRYPTO_AUTHENTICATION_HMAC_SHA1_96.
       Refer to nx_crypto.h for a list of symbols used in this field. User may
       wish to extend the list. */
    UINT nx_crypto_algorithm;

    /* Size of the key, in bits. */
    NX_CRYPTO_KEY_SIZE nx_crypto_key_size_in_bits;

    /* Size of the IV block, in bits. This is used for encryption. */
    USHORT nx_crypto_IV_size_in_bits;

    /* Size of the ICV block, in bits. This is used for authentication. */
    USHORT nx_crypto_ICV_size_in_bits;

    /* Size of the crypto block, in bytes. */
    ULONG nx_crypto_block_size_in_bytes;

    /* Size of the meta data area, in bytes. */
    ULONG nx_crypto_metadata_area_size;

    /* nx_cyrpto_init function initializes the underlying crypto
       method with the "key" information. If the crytpo method requires
       the storage of additional session information, this nx_crypto_init
       routine must allocate memory as needed, and pass a handle
       back to the caller in the parameter "handler". When NetX IPSec
       invokes this crypto method, the handle is passed to crypto operation.
       'crypto_metadata' Pointer to a storage space managed by the underlying crypto method.
       The content of this block is defined by each sa in use.
       'crypto_metadata_size' is the size of the crypto context block, in bytes. */
    UINT (*nx_crypto_init)(struct NX_CRYPTO_METHOD_STRUCT *method,
                           UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                           VOID **handler,
                           VOID *crypto_metadata,
                           ULONG crypto_metadata_size);

    /* When the SA is no longer needed, NetX IPSec calls nx_crypto_cleanup
       function and passes in the handler, so that the underlying
       method can clean up the resource, if needed.  */
    UINT (*nx_crypto_cleanup)(VOID *crypto_metadata);

    /* Function pointer to the actual crypto or hash operation.
       For crypto operation, this function returns NX_CRYPTO_SUCCESS or
       appropriate error code.
       Note that both input and output buffers are prepared by the
       caller. For encryption/decryption operations, the callee shall
       use the output buffer for encrypted or decrypted data. For
       authentication operations, the callee shall use the output
       buffer for the digest.
       'crypto_metadata' Pointer to a storage space managed by the underlying crypto method.
       The content of this block is defined by each sa in use.
       'crypto_metadata_size' is the size of the crypto context block, in bytes. */
    UINT (*nx_crypto_operation)(UINT op,       /* Encrypt, Decrypt, Authenticate */
                                VOID *handler, /* Crypto handler */
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
} NX_CRYPTO_METHOD;

/* Define structure for extended usage of output argument in nx_crypto_operation.
 * Crypto algorithm may return dynamic length of output, such as ECJPAKE.*/
typedef struct NX_CRYPTO_EXTENDED_OUTPUT_STRUCT
{

    /* Pointer to output buffer. */
    UCHAR *nx_crypto_extended_output_data;

    /* Length of output buffer. */
    ULONG  nx_crypto_extended_output_length_in_byte;

    /* Actual size of output buffer used. */
    ULONG  nx_crypto_extended_output_actual_size;
} NX_CRYPTO_EXTENDED_OUTPUT;


/* APIs. */
#define nx_crypto_initialize                            _nx_crypto_initialize

UINT  _nx_crypto_initialize(VOID);

#ifdef NX_CRYPTO_FIPS
#define nx_crypto_method_self_test                      _nx_crypto_method_self_test
#define nx_crypto_module_state_get                      _nx_crypto_module_state_get
/* int nx_crypto_rand(void); */

INT  _nx_crypto_method_self_test(INT);
UINT _nx_crypto_module_state_get(VOID);
#endif


#ifdef __cplusplus
}
#endif


#endif /* _NX_CRYPTO_H_ */

