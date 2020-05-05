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
/*    _nx_secure_tls_record_hash_calculate                PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function generates the Message Authentication Code (MAC) value */
/*    from previously added data. The MAC is placed at the end of all     */
/*    encrypted TLS messages.                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    record_hash                           Pointer to output hash buffer */
/*    hash_length                           Length of hash                */
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
/*    _nx_secure_tls_send_record            Send the TLS record           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*  08-15-2019     Timothy Stapko           Modified comment(s), added    */
/*                                            logic to clear encryption   */
/*                                            key and other secret data,  */
/*                                            removed cipher suite lookup,*/
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_record_hash_calculate(NX_SECURE_TLS_SESSION *tls_session, UCHAR *record_hash,
                                          UINT *hash_length)
{
UINT                                  hash_size;
UINT                                  status;
NX_CRYPTO_METHOD                     *authentication_method;

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

    /* Assert that we have an authentication mechanism. */
    NX_ASSERT(authentication_method -> nx_crypto_operation != NX_NULL);

    /* Get the hash size and MAC secret for our current session. */
    hash_size = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_hash_size;

    /* Calculate the final hash for the data in question. */
    status = authentication_method -> nx_crypto_operation(NX_CRYPTO_HASH_CALCULATE,
                                                          tls_session -> nx_secure_hash_mac_handler,
                                                          authentication_method,
                                                          NX_NULL,
                                                          0,
                                                          NX_NULL,
                                                          0,
                                                          NX_NULL,
                                                          record_hash,
                                                          NX_SECURE_TLS_MAX_HASH_SIZE,
                                                          tls_session -> nx_secure_hash_mac_metadata_area,
                                                          tls_session -> nx_secure_hash_mac_metadata_size,
                                                          NX_NULL,
                                                          NX_NULL);

    if (status)
    {
        return(status);
    }

    if (authentication_method -> nx_crypto_cleanup)
    {
        status = authentication_method -> nx_crypto_cleanup(tls_session -> nx_secure_hash_mac_metadata_area);
    }

    /* Return how many bytes our hash is since the caller doesn't necessarily know. */
    *hash_length = hash_size;

    return(status);
}

