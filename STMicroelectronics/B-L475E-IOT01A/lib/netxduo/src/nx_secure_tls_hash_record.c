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
/** NetX Secure Component                                                 */
/**                                                                       */
/**    Transport Layer Security (TLS)                                     */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SECURE_SOURCE_CODE

#include "nx_secure_tls.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_hash_record                          PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function hashes an outgoing TLS record to generate the Message */
/*    Authentication Code (MAC) value that is placed at the end of all    */
/*    encrypted TLS messages.                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    sequence_num                          Record sequence number        */
/*    header                                Record header                 */
/*    header_length                         Length of record header       */
/*    data                                  Record payload                */
/*    length                                Length of record payload      */
/*    record_hash                           Pointer to output hash buffer */
/*    hash_length                           Length of hash                */
/*    mac_secret                            Key used for MAC generation   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nx_crypto_operation]                 Crypto operation              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_verify_mac             Verify record MAC checksum    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s), added    */
/*                                            logic to clear encryption   */
/*                                            key and other secret data,  */
/*                                            passed crypto handle into   */
/*                                            crypto internal functions,  */
/*                                            updated error return checks,*/
/*                                            removed cipher suite lookup,*/
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_hash_record(NX_SECURE_TLS_SESSION *tls_session,
                                ULONG sequence_num[NX_SECURE_TLS_SEQUENCE_NUMBER_SIZE],
                                UCHAR *header, UINT header_length, UCHAR *data, UINT length,
                                UCHAR *record_hash, UINT *hash_length, UCHAR *mac_secret)
{
UINT                                  hash_size;
UINT                                  status = NX_SECURE_TLS_MISSING_CRYPTO_ROUTINE;;
NX_CRYPTO_METHOD                     *authentication_method;
UCHAR                                 adjusted_sequence_num[8];
VOID                                 *metadata;
UINT                                  metadata_size;
VOID                                 *handler = NX_NULL;

    NX_PARAMETER_NOT_USED(header_length);

    /* We need to generate a Message Authentication Code (MAC) for each record during an "active" TLS session
       (following a ChangeCipherSpec message). The hash algorithm is determined by the ciphersuite, and HMAC
       is used with that hash algorithm to protect the TLS record contents from tampering.

       The MAC is generated as:

       HMAC_hash(MAC_write_secret, seq_num + type + version + length + data);

     */

    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {

        /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    /* Get our authentication method from the ciphersuite. */
    authentication_method = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_hash;
    metadata = tls_session -> nx_secure_hash_mac_metadata_area;
    metadata_size = tls_session -> nx_secure_hash_mac_metadata_size;

    /* Get the hash size and MAC secret for our current session. */
    hash_size = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_hash_size;

    /* Correct the endianness of our sequence number before hashing. */
    adjusted_sequence_num[0] = (UCHAR)(sequence_num[1] >> 24);
    adjusted_sequence_num[1] = (UCHAR)(sequence_num[1] >> 16);
    adjusted_sequence_num[2] = (UCHAR)(sequence_num[1] >> 8);
    adjusted_sequence_num[3] = (UCHAR)(sequence_num[1]);
    adjusted_sequence_num[4] = (UCHAR)(sequence_num[0] >> 24);
    adjusted_sequence_num[5] = (UCHAR)(sequence_num[0] >> 16);
    adjusted_sequence_num[6] = (UCHAR)(sequence_num[0] >> 8);
    adjusted_sequence_num[7] = (UCHAR)(sequence_num[0]);

    if (authentication_method -> nx_crypto_init)
    {
        status = authentication_method -> nx_crypto_init(authentication_method,
                                                mac_secret,
                                                (NX_CRYPTO_KEY_SIZE)(hash_size << 3),
                                                &handler,
                                                metadata,
                                                metadata_size);

        if(status != NX_SUCCESS)
        {
            return(status);
        }                                                     
    }

    /* TLS header type, version, and length are in the proper order. */
    if (authentication_method -> nx_crypto_operation != NX_NULL)
    {
        status = authentication_method -> nx_crypto_operation(NX_CRYPTO_HASH_INITIALIZE,
                                                     handler,
                                                     authentication_method,
                                                     mac_secret,
                                                     (NX_CRYPTO_KEY_SIZE)(hash_size << 3),
                                                     NX_NULL,
                                                     0,
                                                     NX_NULL,
                                                     NX_NULL,
                                                     0,
                                                     metadata,
                                                     metadata_size,
                                                     NX_NULL,
                                                     NX_NULL);

        if(status != NX_SUCCESS)
        {
            return(status);
        }  

        status = authentication_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                                     handler,
                                                     authentication_method,
                                                     NX_NULL,
                                                     0,
                                                     adjusted_sequence_num,
                                                     8,
                                                     NX_NULL,
                                                     NX_NULL,
                                                     0,
                                                     metadata,
                                                     metadata_size,
                                                     NX_NULL,
                                                     NX_NULL);

        if(status != NX_SUCCESS)
        {
            return(status);
        }  

        status = authentication_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                                     handler,
                                                     authentication_method,
                                                     NX_NULL,
                                                     0,
                                                     header,
                                                     5,
                                                     NX_NULL,
                                                     NX_NULL,
                                                     0,
                                                     metadata,
                                                     metadata_size,
                                                     NX_NULL,
                                                     NX_NULL);

        if(status != NX_SUCCESS)
        {
            return(status);
        }                                                     

        status = authentication_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                                     handler,
                                                     authentication_method,
                                                     NX_NULL,
                                                     0,
                                                     data,
                                                     length,
                                                     NX_NULL,
                                                     NX_NULL,
                                                     0,
                                                     metadata,
                                                     metadata_size,
                                                     NX_NULL,
                                                     NX_NULL);

        if(status != NX_SUCCESS)
        {
            return(status);
        }  

        status = authentication_method -> nx_crypto_operation(NX_CRYPTO_HASH_CALCULATE,
                                                     handler,
                                                     authentication_method,
                                                     NX_NULL,
                                                     0,
                                                     NX_NULL,
                                                     0,
                                                     NX_NULL,
                                                     record_hash,
                                                     NX_SECURE_TLS_MAX_HASH_SIZE,
                                                     metadata,
                                                     metadata_size,
                                                     NX_NULL,
                                                     NX_NULL);

        if(status != NX_SUCCESS)
        {
            return(status);
        }                                                     
#ifdef NX_SECURE_KEY_CLEAR
        NX_SECURE_MEMSET(adjusted_sequence_num, 0, 8);
#endif /* NX_SECURE_KEY_CLEAR  */
    }

    if (authentication_method -> nx_crypto_cleanup)
    {
        status = authentication_method -> nx_crypto_cleanup(metadata);

        if(status != NX_SUCCESS)
        {
            return(status);
        }                                                     
    }

    /* Return how many bytes our hash is since the caller doesn't necessarily know. */
    *hash_length = hash_size;

    return(status);
}

