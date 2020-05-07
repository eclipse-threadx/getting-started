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
/**   HMAC MD5 Digest Algorithm (MD5)                                     */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "nx_api.h"
#include "nx_crypto_md5.h"
#include "nx_crypto_hmac_md5.h"
#include "nx_crypto_hmac.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_hmac_md5_init                     PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is the common crypto method init callback for         */
/*    Express Logic supported HMAC MD5 cryptograhic algorithm.            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    method                                Pointer to crypto method      */
/*    key                                   Pointer to key                */
/*    key_size_in_bits                      Length of key size in bits    */
/*    handler                               Returned crypto handler       */
/*    crypto_metadata                       Metadata area                 */
/*    crypto_metadata_size                  Size of the metadata area     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-15-2019     Timothy Stapko           Initial Version 5.12          */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT  _nx_crypto_method_hmac_md5_init(struct  NX_CRYPTO_METHOD_STRUCT *method,
                                                     UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                                     VOID  **handle,
                                                     VOID  *crypto_metadata,
                                                     ULONG crypto_metadata_size)
{

    NX_CRYPTO_STATE_CHECK

    NX_PARAMETER_NOT_USED(key_size_in_bits);
    NX_PARAMETER_NOT_USED(handle);

    if ((method == NX_NULL) || (key == NX_NULL) || (crypto_metadata == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }
  
    /* Verify the metadata addrsss is 4-byte aligned. */
    if((((ULONG)crypto_metadata) & 0x3) != 0)
    {
        return(NX_PTR_ERROR);
    }

    if(crypto_metadata_size < sizeof(NX_CRYPTO_MD5_HMAC))
    {
        return(NX_PTR_ERROR);
    }

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_hmac_md5_cleanup                  PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function cleans up the crypto metadata.                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    crypto_metadata                       Crypto metadata               */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NX_CRYPTO_MEMSET                      Set the memory                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-15-2019     Timothy Stapko           Initial Version 5.12          */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT  _nx_crypto_method_hmac_md5_cleanup(VOID *crypto_metadata)
{

    NX_CRYPTO_STATE_CHECK

#ifdef NX_SECURE_KEY_CLEAR
    if (!crypto_metadata)
        return (NX_CRYPTO_SUCCESS);

    /* Clean up the crypto metadata.  */
    NX_CRYPTO_MEMSET(crypto_metadata, 0, sizeof(NX_CRYPTO_MD5_HMAC));
#else
    NX_PARAMETER_NOT_USED(crypto_metadata);
#endif/* NX_SECURE_KEY_CLEAR  */

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_hmac_md5_operation                PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles HMAC MD5 Authentication operation.            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    Operation Type                */
/*                                          Encrypt, Decrypt, Authenticate*/
/*    handler                               Pointer to crypto context     */
/*    method                                Pointer to crypto method      */
/*    key                                   Pointer to key                */
/*    key_size_in_bits                      Length of key size in bits    */
/*    input                                 Input Stream                  */
/*    input_length_in_byte                  Input Stream Length           */
/*    iv_ptr                                Initialized Vector            */
/*    output                                Output Stream                 */
/*    output_length_in_byte                 Output Stream Length          */
/*    crypto_metadata                       Metadata area                 */
/*    crypto_metadata_size                  Size of the metadata area     */
/*    packet_ptr                            Pointer to packet             */
/*    nx_crypto_hw_process_callback         Callback function pointer     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_hmac                       Calculate the HMAC            */
/*    _nx_crypto_hmac_metadata_set          Set HMAC metadata             */
/*    _nx_crypto_hmac_initialize            Perform HMAC initialization   */
/*    _nx_crypto_hmac_update                Perform HMAC update           */
/*    _nx_crypto_hmac_digest_calculate      Calculate HMAC digest         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_hmac_md5_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
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
                                                          VOID (*nx_crypto_hw_process_callback)(VOID *packet_ptr, UINT status))
{
NX_CRYPTO_MD5_HMAC *ctx;
NX_CRYPTO_HMAC      hmac_metadata;

    NX_PARAMETER_NOT_USED(handle);
    NX_PARAMETER_NOT_USED(iv_ptr);
    NX_PARAMETER_NOT_USED(packet_ptr);
    NX_PARAMETER_NOT_USED(nx_crypto_hw_process_callback);

    NX_CRYPTO_STATE_CHECK

    /* Verify the metadata addrsss is 4-byte aligned. */
    if((method == NX_NULL) || (crypto_metadata == NX_NULL) || ((((ULONG)crypto_metadata) & 0x3) != 0))
    {
        return(NX_PTR_ERROR);
    }

    if(crypto_metadata_size < sizeof(NX_CRYPTO_MD5_HMAC))
    {
        return(NX_PTR_ERROR);
    }

    ctx = (NX_CRYPTO_MD5_HMAC *)crypto_metadata;
    _nx_crypto_hmac_metadata_set(&hmac_metadata,
                                 &(ctx -> nx_md5_hmac_context),
                                 ctx -> nx_md5_hmac_k_ipad,
                                 ctx -> nx_md5_hmac_k_opad,
                                 method -> nx_crypto_algorithm,
                                 NX_CRYPTO_MD5_BLOCK_SIZE_IN_BYTES,
                                 NX_CRYPTO_HMAC_MD5_ICV_FULL_LEN_IN_BITS >> 3,
                                 (UINT (*)(VOID *, UINT))_nx_crypto_md5_initialize,
                                 (UINT (*)(VOID *, UCHAR *, UINT))_nx_crypto_md5_update,
                                 (UINT (*)(VOID *, UCHAR *, UINT))_nx_crypto_md5_digest_calculate);

    switch (op)
    {
    case NX_CRYPTO_HASH_INITIALIZE:
        if(key == NX_NULL)
        {
            return(NX_PTR_ERROR);
        }

        _nx_crypto_hmac_initialize(&hmac_metadata, key, key_size_in_bits >> 3);
        break;

    case NX_CRYPTO_HASH_UPDATE:
        _nx_crypto_hmac_update(&hmac_metadata, input, input_length_in_byte);
        break;

    case NX_CRYPTO_HASH_CALCULATE:
        if(output_length_in_byte == 0)
        {
            return(NX_CRYPTO_INVALID_BUFFER_SIZE);
        }

        _nx_crypto_hmac_digest_calculate(&hmac_metadata, output, 
                                         (output_length_in_byte > (ULONG)((method -> nx_crypto_ICV_size_in_bits) >> 3) ? 
                                         ((method -> nx_crypto_ICV_size_in_bits) >> 3) : output_length_in_byte));
        break;

    default:
        if(key == NX_NULL)
        {
            return(NX_PTR_ERROR);
        }

        if(output_length_in_byte == 0)
        {
            return(NX_CRYPTO_INVALID_BUFFER_SIZE);
        }

        _nx_crypto_hmac(&hmac_metadata, input, input_length_in_byte, key, (key_size_in_bits >> 3), output, 
                        (output_length_in_byte > (ULONG)((method -> nx_crypto_ICV_size_in_bits) >> 3) ? 
                        ((method -> nx_crypto_ICV_size_in_bits) >> 3) : output_length_in_byte));
        break;
    }

    return NX_CRYPTO_SUCCESS;
}

