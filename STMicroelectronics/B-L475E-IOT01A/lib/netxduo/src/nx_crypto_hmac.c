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

#include "nx_crypto_hmac.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_hmac                                     PORTABLE C      */
/*                                                           5.12         */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function calculates the HMAC.                                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    hmac_metadata                         pointer to HMAC metadata      */
/*    input_ptr                             input byte stream             */
/*    input_length                          input byte stream length      */
/*    key_ptr                               key stream                    */
/*    key_length                            key stream length             */
/*    digest_ptr                            generated crypto digest       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_hmac_initialize            Perform HMAC initialization   */
/*    _nx_crypto_hmac_update                Perform HMAC update           */
/*    _nx_crypto_hmac_digest_calculate      Calculate HMAC digest         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*    _nx_crypto_method_hmac_md5_operation  Handle HMAC-MD5 operation     */
/*    _nx_crypto_method_hmac_sha1_operation Handle HMAC-SHA1 operation    */
/*    _nx_crypto_method_hmac_sha256_operation Handle HMAC-SHA256 operation*/
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
NX_CRYPTO_KEEP UINT _nx_crypto_hmac(NX_CRYPTO_HMAC *hmac_metadata,
                                    UCHAR *input_ptr, UINT input_length,
                                    UCHAR *key_ptr, UINT key_length,
                                    UCHAR *digest_ptr, UINT digest_length)
{

    /* Initialize, update and calculate.  */
    _nx_crypto_hmac_initialize(hmac_metadata, key_ptr, key_length);
    _nx_crypto_hmac_update(hmac_metadata, input_ptr, input_length);
    _nx_crypto_hmac_digest_calculate(hmac_metadata, digest_ptr, digest_length);

    /* Return success.  */
    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_hmac_initialize                          PORTABLE C      */
/*                                                           5.12         */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs HMAC initialization.                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    hmac_metadata                         pointer to HMAC metadata      */
/*    key_ptr                               key stream                    */
/*    key_length                            key stream length             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [crypto_digest_calculate]             Calculate crypto digest       */
/*    [crypto_initialize]                   Perform crypto initialization */
/*    [crypto_update]                       Perform crypto update         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*    _nx_crypto_hmac                       Calculate the HMAC            */
/*    _nx_crypto_method_hmac_md5_operation  Handle HMAC-MD5 operation     */
/*    _nx_crypto_method_hmac_sha1_operation Handle HMAC-SHA1 operation    */
/*    _nx_crypto_method_hmac_sha256_operation Handle HMAC-SHA256 operation*/
/*    _nx_crypto_method_hmac_sha512_operation Handle HMAC-SHA512 operation*/
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
NX_CRYPTO_KEEP UINT _nx_crypto_hmac_initialize(NX_CRYPTO_HMAC *hmac_metadata, UCHAR *key_ptr, UINT key_length)
{
/* FIXME: check the size of temp_key.  */
UCHAR temp_key[128];
UINT  i;

    /* If key is longer than block size, reset it to key=CRYPTO(key). */
    if (key_length > hmac_metadata -> block_size)
    {

        hmac_metadata -> crypto_initialize(hmac_metadata -> context, hmac_metadata -> algorithm);

        hmac_metadata -> crypto_update(hmac_metadata -> context, key_ptr, key_length);

        hmac_metadata -> crypto_digest_calculate(hmac_metadata -> context, temp_key, hmac_metadata -> algorithm);

        key_ptr = temp_key;

        key_length = hmac_metadata -> output_length;
    }

    hmac_metadata -> crypto_initialize(hmac_metadata -> context, hmac_metadata -> algorithm);

    /* The HMAC_CRYPTO transform looks like:

       CRYPTO(K XOR opad, CRYPTO(K XOR ipad, text))

       where K is an n byte key,
       ipad is the byte 0x36 repeated block_size times,
       opad is the byte 0x5c repeated block_size times,
       and text is the data being protected.      */

    NX_CRYPTO_MEMSET(hmac_metadata -> k_ipad, 0, hmac_metadata -> block_size);

    NX_CRYPTO_MEMSET(hmac_metadata -> k_opad, 0, hmac_metadata -> block_size);

    NX_CRYPTO_MEMCPY(hmac_metadata -> k_ipad, key_ptr, key_length);

    NX_CRYPTO_MEMCPY(hmac_metadata -> k_opad, key_ptr, key_length);


    /* XOR key with ipad and opad values. */
    for (i = 0; i < hmac_metadata -> block_size; i++)
    {
        hmac_metadata -> k_ipad[i] ^= 0x36;
        hmac_metadata -> k_opad[i] ^= 0x5c;
    }

    /* Kick off the inner hash with our padded key. */
    hmac_metadata -> crypto_update(hmac_metadata -> context, hmac_metadata -> k_ipad, hmac_metadata -> block_size);

#ifdef NX_SECURE_KEY_CLEAR
    NX_CRYPTO_MEMSET(temp_key, 0, sizeof(temp_key));
#endif /* NX_SECURE_KEY_CLEAR  */

    /* Return success.  */
    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_hmac_update                              PORTABLE C      */
/*                                                           5.12         */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs HMAC update.                                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    hmac_metadata                         pointer to HMAC metadata      */
/*    input_ptr                             input byte stream             */
/*    input_length                          input byte stream length      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [crypto_update]                       Perform crypto update         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*    _nx_crypto_hmac                       Calculate the HMAC            */
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
NX_CRYPTO_KEEP UINT _nx_crypto_hmac_update(NX_CRYPTO_HMAC *hmac_metadata, UCHAR *input_ptr, UINT input_length)
{

    /* Update inner CRYPTO. */
    hmac_metadata -> crypto_update(hmac_metadata -> context, input_ptr, input_length);

    /* Return success.  */
    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_hmac_digest_calculate                    PORTABLE C      */
/*                                                           5.12         */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs HMAC digest calculation.                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    hmac_metadata                         pointer to HMAC metadata      */
/*    digest_ptr                            generated crypto digest       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [crypto_digest_calculate]             Calculate crypto digest       */
/*    [crypto_initialize]                   Perform crypto initialization */
/*    [crypto_update]                       Perform crypto update         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*    _nx_crypto_hmac                       Calculate the HMAC            */
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
NX_CRYPTO_KEEP UINT _nx_crypto_hmac_digest_calculate(NX_CRYPTO_HMAC *hmac_metadata, UCHAR *digest_ptr, UINT digest_length)
{
/* FIXME: check the size of icv_ptr.  */
UCHAR icv_ptr[64];

    /* Perform outer CRYPTO. */

    hmac_metadata -> crypto_digest_calculate(hmac_metadata -> context, icv_ptr, hmac_metadata -> algorithm);

    hmac_metadata -> crypto_initialize(hmac_metadata -> context, hmac_metadata -> algorithm);

    hmac_metadata -> crypto_update(hmac_metadata -> context, hmac_metadata -> k_opad, hmac_metadata -> block_size);

    hmac_metadata -> crypto_update(hmac_metadata -> context, icv_ptr, hmac_metadata -> output_length);

    hmac_metadata -> crypto_digest_calculate(hmac_metadata -> context, icv_ptr, hmac_metadata -> algorithm);

    NX_CRYPTO_MEMCPY(digest_ptr, icv_ptr,  (digest_length > hmac_metadata -> output_length ? hmac_metadata -> output_length : digest_length));

#ifdef NX_SECURE_KEY_CLEAR
    NX_CRYPTO_MEMSET(icv_ptr, 0, sizeof(icv_ptr));
#endif /* NX_SECURE_KEY_CLEAR  */

    /* Return success.  */
    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_hmac_metadata_set                        PORTABLE C      */
/*                                                           5.12         */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets HMAC metadata.                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    hmac_metadata                         pointer to HMAC metadata      */
/*    context                               crypto context                */
/*    k_ipad                                ipad key                      */
/*    k_opad                                opad key                      */
/*    algorithm                             algorithm                     */
/*    block_size                            block size                    */
/*    output_length                         output length                 */
/*    crypto_intitialize                    initializtion function        */
/*    crypto_update                         update function               */
/*    crypto_digest_calculate               digest calculation function   */
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
/*    Application Code                                                    */
/*    _nx_crypto_method_hmac_md5_operation  Handle HMAC-MD5 operation     */
/*    _nx_crypto_method_hmac_sha1_operation Handle HMAC-SHA1 operation    */
/*    _nx_crypto_method_hmac_sha256_operation Handle HMAC-SHA256 operation*/
/*    _nx_crypto_method_hmac_sha512_operation Handle HMAC-SHA512 operation*/
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
NX_CRYPTO_KEEP VOID _nx_crypto_hmac_metadata_set(NX_CRYPTO_HMAC *hmac_metadata,
                                                 VOID *context,  UCHAR *k_ipad, UCHAR  *k_opad,
                                                 UINT algorithm, UINT block_size, UINT output_length,
                                                 UINT (*crypto_initialize)(VOID *, UINT),
                                                 UINT (*crypto_update)(VOID *, UCHAR *, UINT),
                                                 UINT (*crypto_digest_calculate)(VOID *, UCHAR *, UINT))
{

    hmac_metadata -> context = context;
    hmac_metadata -> k_ipad = k_ipad;
    hmac_metadata -> k_opad = k_opad;
    hmac_metadata -> algorithm = algorithm;
    hmac_metadata -> block_size = block_size;
    hmac_metadata -> output_length = output_length;
    hmac_metadata -> crypto_initialize = crypto_initialize;
    hmac_metadata -> crypto_update = crypto_update;
    hmac_metadata -> crypto_digest_calculate = crypto_digest_calculate;
}

