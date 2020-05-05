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
/*    _nx_secure_tls_handshake_hash_init                  PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the hash function states needed for the   */
/*    TLS Finished message handshake hash.                                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nx_crypto_operation]                 Hash initialization functions */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_send_clienthello      Send ClientHello              */
/*    _nx_secure_dtls_server_handshake      DTLS server state machine     */
/*    _nx_secure_tls_send_clienthello       Send ClientHello              */
/*    _nx_secure_tls_server_handshake       TLS server state machine      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017      Timothy Stapko         Initial Version 5.10           */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s), added    */
/*                                            logic to properly initialize*/
/*                                            the crypto control blcok,   */
/*                                            passed crypto handle into   */
/*                                            crypto internal functions,  */
/*                                            updated error return checks,*/
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT  _nx_secure_tls_handshake_hash_init(NX_SECURE_TLS_SESSION *tls_session)
{
NX_CRYPTO_METHOD *method_ptr;
UINT              status;
VOID             *handler = NX_NULL;
VOID             *metadata;
UINT              metadata_size;

    /* We need to hash all of the handshake messages that we receive and send. When sending a ClientHello,
       we need to initialize the hashes (TLS 1.1 uses both MD5 and SHA-1, TLS 1.2 uses SHA-256). The final
       hash is generated in the "Finished" message.  */

    /* Initialize both the handshake "finished" hashes - TLS 1.1 uses both SHA-1 and MD5, TLS 1.2 uses SHA-256 by default.
       At this point we don't yet know the version we will use, so initialize all of them. */
    /* Hash is determined by ciphersuite in TLS 1.2. Default is SHA-256. */
#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
    method_ptr = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_handshake_hash_sha256_method;
    metadata = tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata;
    metadata_size = tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata_size;

    if (method_ptr -> nx_crypto_init)
    {
        status = method_ptr -> nx_crypto_init(method_ptr,
                                     NX_NULL,
                                     0,
                                     &handler,
                                     metadata,
                                     metadata_size);

        if(status != NX_SUCCESS)
        {
            return(status);
        }                                                     
        tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_handler = handler;
    }

    if (method_ptr -> nx_crypto_operation != NX_NULL)
    {
        status = method_ptr -> nx_crypto_operation(NX_CRYPTO_HASH_INITIALIZE,
                                          handler,
                                          method_ptr,
                                          NX_NULL,
                                          0,
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
    }
#endif

#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
    /* TLS 1.0 and 1.1 use both MD5 and SHA-1. */
    method_ptr = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_handshake_hash_md5_method;
    metadata = tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_md5_metadata;
    metadata_size = tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_md5_metadata_size;

    if (method_ptr -> nx_crypto_init)
    {
        status = method_ptr -> nx_crypto_init(method_ptr,
                                     NX_NULL,
                                     0,
                                     &handler,
                                     metadata,
                                     metadata_size);

        if(status != NX_SUCCESS)
        {
            return(status);
        }                                                     

        tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_md5_handler = handler;
    }

    if (method_ptr -> nx_crypto_operation != NX_NULL)
    {
        status = method_ptr -> nx_crypto_operation(NX_CRYPTO_HASH_INITIALIZE,
                                          handler,
                                          method_ptr,
                                          NX_NULL,
                                          0,
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
    }

    method_ptr = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_handshake_hash_sha1_method;
    metadata = tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha1_metadata;
    metadata_size = tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha1_metadata_size;

    if (method_ptr -> nx_crypto_init)
    {
        status = method_ptr -> nx_crypto_init(method_ptr,
                                     NX_NULL,
                                     0,
                                     &handler,
                                     metadata,
                                     metadata_size);

        if(status != NX_SUCCESS)
        {
            return(status);
        }                                                     
        tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha1_handler = handler;
    }

    if (method_ptr -> nx_crypto_operation != NX_NULL)
    {
        status = method_ptr -> nx_crypto_operation(NX_CRYPTO_HASH_INITIALIZE,
                                          handler,
                                          method_ptr,
                                          NX_NULL,
                                          0,
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
    }
#endif

    return(NX_SUCCESS);
}

