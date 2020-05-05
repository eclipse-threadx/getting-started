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
/*    _nx_secure_tls_record_hash_update                   PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function updates a previously-initialized MAC hash for a TLS   */
/*    message.                                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    data                                  Record payload                */
/*    length                                Length of record payload      */
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
/*  08-15-2019     Timothy Stapko           Modified comment(s), and      */
/*                                            passed crypto handle into   */
/*                                            crypto internal functions,  */
/*                                            removed cipher suite lookup,*/
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_record_hash_update(NX_SECURE_TLS_SESSION *tls_session, UCHAR *data, UINT length)
{
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

    /* Update the hash with the new data. */
    status = authentication_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                                          tls_session -> nx_secure_hash_mac_handler,
                                                          authentication_method,
                                                          NX_NULL,
                                                          0,
                                                          data,
                                                          length,
                                                          NX_NULL,
                                                          NX_NULL,
                                                          0,
                                                          tls_session -> nx_secure_hash_mac_metadata_area,
                                                          tls_session -> nx_secure_hash_mac_metadata_size,
                                                          NX_NULL,
                                                          NX_NULL);

    return(status);
}

