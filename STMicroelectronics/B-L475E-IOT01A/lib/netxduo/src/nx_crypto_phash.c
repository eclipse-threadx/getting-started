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
/**  Transport Layer Security (TLS) - PHASH function                      */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


#include "nx_crypto_phash.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_phash                                    PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function implements phash for the crypto module.               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    phash                                 phash control block           */
/*    output                                Pointer to output buffer      */
/*    desired_length                        Number of bytes to return     */
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
/*    Application                                                         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-15-2019     Timothy Stapko           Initial Version 5.12          */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT _nx_crypto_phash(NX_CRYPTO_PHASH *phash, UCHAR *output, UINT desired_length)
{
UINT    offset; /* point to the memory for the coming hmac data */
UINT    hash_size; /* the length of hmac output */
UINT    output_len; /* desired output length of hmac */
UINT    status;
/* A(i) */
UCHAR   *temp_A;
UINT    temp_A_size;
/* secret */
UCHAR   *secret;
UINT    secret_len;
/* seed */
UCHAR   *seed;
UINT    seed_len;
/* metadata */
UCHAR   *metadata;
UINT    metadata_size;
/* hmac output size */
UCHAR   *hmac_output;
UINT    hmac_output_size;
UINT    A_len; /* the length of current A(i) */
UINT    remaining_len, i; /* remaining length of data to be generated. */
VOID   *handler = NX_NULL;
NX_CRYPTO_METHOD *hash_method = phash -> nx_crypto_hmac_method;

    NX_CRYPTO_STATE_CHECK

    /* Validate pointers. */
    if (hash_method == NX_NULL
        || hash_method -> nx_crypto_operation == NX_NULL
        || hash_method -> nx_crypto_cleanup == NX_NULL
        || output == NX_NULL)
    {
        return(NX_INVALID_PARAMETERS);
    }

    /* Initialize temporary variables. */
    secret = phash -> nx_crypto_phash_secret;
    secret_len = phash -> nx_crypto_phash_secret_length;
    seed = phash -> nx_crypto_phash_seed;
    seed_len = phash -> nx_crypto_phash_seed_length;
    temp_A = phash -> nx_crypto_phash_temp_A;
    temp_A_size = phash -> nx_crypto_phash_temp_A_size;
    hash_size = hash_method -> nx_crypto_ICV_size_in_bits >> 3;
    metadata = phash -> nx_crypto_hmac_metadata;
    metadata_size = phash -> nx_crypto_hmac_metadata_size;
    hmac_output = phash -> nx_crypto_hmac_output;
    hmac_output_size = phash -> nx_crypto_hmac_output_size;

    if (hash_size > hmac_output_size)
    {
        return(NX_INVALID_PARAMETERS);
    }

    /* Assign the seed as A(0). */
    NX_CRYPTO_MEMSET(temp_A, 0, temp_A_size);
    NX_CRYPTO_MEMCPY(temp_A, seed, seed_len);
    A_len = phash -> nx_crypto_phash_seed_length;

    remaining_len = desired_length;
    for (offset = 0; offset < desired_length; offset += hash_size)
    {
        /* Calculate A(i) */
        if (hash_method -> nx_crypto_init)
        {
            status = hash_method -> nx_crypto_init(hash_method,
                                          secret,
                                          (NX_CRYPTO_KEY_SIZE)(secret_len << 3),
                                          &handler,
                                          metadata,
                                          metadata_size);

            if(status != NX_SUCCESS)
            {
                return(status);
            }                                                     
        }

        status = hash_method -> nx_crypto_operation(NX_CRYPTO_AUTHENTICATE,
                                           handler,
                                           hash_method,
                                           secret,
                                           (NX_CRYPTO_KEY_SIZE)(secret_len << 3),
                                           temp_A,
                                           A_len,
                                           NX_NULL,
                                           temp_A,
                                           temp_A_size,
                                           metadata,
                                           metadata_size,
                                           NX_NULL,
                                           NX_NULL);

        if(status != NX_SUCCESS)
        {
            return(status);
        }                                                     
        
        /* Updated the length of A(i) */
        A_len = hash_size;

        /* Concatenate A[i] and seed to feed into digest. */
        NX_CRYPTO_MEMCPY(&temp_A[A_len], seed, seed_len);

        /* Output block is the size of the digest unless the remaining
           desired length is smaller than the digest length. */
        if (remaining_len < hash_size)
        {
            output_len = remaining_len;
        }
        else
        {
            output_len = hash_size;
        }

        /* Calculate p-hash block, store in output. */
        status = hash_method -> nx_crypto_operation(NX_CRYPTO_AUTHENTICATE,
                                           handler,
                                           hash_method,
                                           secret,
                                           (NX_CRYPTO_KEY_SIZE)(secret_len << 3),
                                           temp_A,
                                           A_len + seed_len,
                                           NX_NULL,
                                           hmac_output,
                                           output_len,
                                           metadata,
                                           metadata_size,
                                           NX_NULL,
                                           NX_NULL);

        if(status != NX_SUCCESS)
        {
            return(status);
        }                                                     
        
        /* Append the hmac result to the output. */
        for (i = 0; i < output_len; i++)
        {
            output[offset + i] ^= hmac_output[i];
        }

        /* Adjust our remaining length by the number of bytes written. */
        remaining_len -= hash_size;

        status = hash_method -> nx_crypto_cleanup(metadata);

        if(status != NX_SUCCESS)
        {
            return(status);
        }                                                     
    }

    return(NX_CRYPTO_SUCCESS);
}
