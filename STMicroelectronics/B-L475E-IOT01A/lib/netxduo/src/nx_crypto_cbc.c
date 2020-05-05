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
/**   CBC Mode                                                            */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "nx_crypto_cbc.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_cbc_xor                                  PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs XOR operation on the output buffer.          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    plaintext                             Pointer to input plantext     */
/*    key                                   Value to be xor'ed            */
/*    ciphertext                            Output buffer                 */
/*    block_size                            Block size                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_cbc_encrypt                Perform CBC mode encryption   */
/*    _nx_crypto_cbc_decrypt                Perform CBC mode decryption   */
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
NX_CRYPTO_KEEP static VOID _nx_crypto_cbc_xor(UCHAR *plaintext, UCHAR *key, UCHAR *ciphertext, UCHAR block_size)
{
UINT i;

    /* FIXME: Operate 32 bits in each round. */
    for (i = 0; i < block_size; i++)
    {
        ciphertext[i] = plaintext[i] ^ key[i];
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_cbc_encrypt                              PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs CBC mode encryption.                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    crypto_metadata                       Pointer to crypto metadata    */
/*    crypto_function                       Pointer to crypto function    */
/*    key_set_function                      Pointer to key set function   */
/*    additional_data                       Pointer to the additional data*/
/*    additional_len                        Length of additional data     */
/*    input                                 Pointer to clear text input   */
/*    output                                Pointer to encrypted output   */
/*                                            The size of the output      */
/*                                            buffer must be at least     */
/*                                            the size of input message.  */
/*    length                                Length of the input message.  */
/*    iv                                    Nonce length + Nonce          */
/*    icv_len                               ICV length                    */
/*    block_size                            Block size                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_cbc_xor                    Perform CBC XOR operation     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*    _nx_crypto_method_aes_operation       Handle AES encrypt or decrypt */
/*    _nx_crypto_method_des_operation       Handle DES encrypt or decrypt */
/*    _nx_crypto_method_3des_operation      Handle 3DES encrypt or decrypt*/
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*  08-15-2019     Timothy Stapko           Modified comment(s),          */
/*                                            added logic so NetX Crypto  */
/*                                            is FIPS 140-2 compliant,    */
/*                                            static analysis fixes,      */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT _nx_crypto_cbc_encrypt(VOID *crypto_metadata,
                                           UINT (*crypto_function)(VOID *, UCHAR *, UCHAR *, UINT),
                                           UINT (*key_set_function)(VOID *, UCHAR *, UINT),
                                           VOID *additional_data, UINT additional_len,
                                           UCHAR *input, UCHAR *output, UINT length,
                                           UCHAR *iv, UCHAR icv_len, UCHAR block_size)
{
UCHAR *last_cipher;
UINT   i;

    NX_PARAMETER_NOT_USED(key_set_function);
    NX_PARAMETER_NOT_USED(additional_data);
    NX_PARAMETER_NOT_USED(additional_len);
    NX_PARAMETER_NOT_USED(icv_len);

    if (block_size == 0)
    {
        return(NX_INVALID_PARAMETERS);
    }

    /* Determine if data length is multiple of block size. */
    if (length % block_size)
    {
        return(NX_PTR_ERROR);
    }

    last_cipher = iv;

    for (i = 0; i < length; i += block_size)
    {

        /* XOR. */
        _nx_crypto_cbc_xor(&input[i], last_cipher, output, block_size);

        /* Encrypt the block. */
        crypto_function(crypto_metadata, output, output, block_size);

        /* Remember the previous encrypt block result. */
        last_cipher = output;

        output += block_size;
    }

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_cbc_decrypt                              PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs CBC mode decryption.                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    crypto_metadata                       Pointer to crypto metadata    */
/*    crypto_function                       Pointer to crypto function    */
/*    key_set_function                      Pointer to key set function   */
/*    additional_data                       Pointer to the additional data*/
/*    additional_len                        Length of additional data     */
/*    input                                 Pointer to clear text input   */
/*    output                                Pointer to encrypted output   */
/*                                            The size of the output      */
/*                                            buffer must be at least     */
/*                                            the size of input message.  */
/*    length                                Length of the input message.  */
/*    iv                                    Nonce length + Nonce          */
/*    icv_len                               ICV length                    */
/*    block_size                            Block size                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_cbc_xor                    Perform CBC XOR operation     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*    _nx_crypto_method_aes_operation       Handle AES encrypt or decrypt */
/*    _nx_crypto_method_des_operation       Handle DES encrypt or decrypt */
/*    _nx_crypto_method_3des_operation      Handle 3DES encrypt or decrypt*/
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*  08-15-2019     Timothy Stapko           Modified comment(s),          */
/*                                            added logic so NetX Crypto  */
/*                                            is FIPS 140-2 compliant,    */
/*                                            static analysis fixes,      */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT _nx_crypto_cbc_decrypt(VOID *crypto_metadata,
                                           UINT (*crypto_function)(VOID *, UCHAR *, UCHAR *, UINT),
                                           UINT (*key_set_function)(VOID *, UCHAR *, UINT),
                                           VOID *additional_data, UINT additional_len,
                                           UCHAR *input, UCHAR *output, UINT length,
                                           UCHAR *iv, UCHAR icv_len, UCHAR block_size)
{
UCHAR last_cipher[16];
UCHAR save_input[16];
UINT  i;

    NX_PARAMETER_NOT_USED(key_set_function);
    NX_PARAMETER_NOT_USED(additional_data);
    NX_PARAMETER_NOT_USED(additional_len);
    NX_PARAMETER_NOT_USED(icv_len);

    if (block_size == 0)
    {
        return(NX_INVALID_PARAMETERS);
    }

    /* Determine if data length is multiple of block size. */
    if (length % block_size)
    {
        return(NX_PTR_ERROR);
    }

    /* Determine if block size is larger than the size of save_input. */
    if (block_size > sizeof(save_input))
    {
        return(NX_PTR_ERROR);
    }

    NX_CRYPTO_MEMCPY(last_cipher, iv, block_size);

    for (i = 0; i < length; i += block_size)
    {
        /* If input == output, the xor clobbers the input buffer so we need to save off our last ciphertext
           before doing the xor. */
        NX_CRYPTO_MEMCPY(save_input, &input[i], block_size);

        /* Decrypt the block.  */
        crypto_function(crypto_metadata, &input[i], &output[i], block_size);

        /* XOR.  */
        _nx_crypto_cbc_xor(&output[i], last_cipher, &output[i], block_size);

        NX_CRYPTO_MEMCPY(last_cipher, save_input, block_size);
    }

#ifdef NX_SECURE_KEY_CLEAR
    NX_CRYPTO_MEMSET(last_cipher, 0, sizeof(last_cipher));
    NX_CRYPTO_MEMSET(save_input, 0, sizeof(save_input));
#endif /* NX_SECURE_KEY_CLEAR  */

    return(NX_CRYPTO_SUCCESS);
}

