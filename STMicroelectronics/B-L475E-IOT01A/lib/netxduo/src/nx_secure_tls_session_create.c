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
/*    _nx_secure_tls_session_create                       PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes a TLS session control block for later     */
/*    use in establishing a secure TLS session over a TCP socket or       */
/*    other lower-level networking protocol.                              */
/*                                                                        */
/*    To calculate the necessary metadata size, the API                   */
/*    nx_secure_tls_metadata_size_calculate may be used.                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                           TLS session control block     */
/*    crypto_table                          crypto method table           */
/*    metadata_buffer                       Encryption metadata area      */
/*    metadata_size                         Encryption metadata size      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_session_reset          Clear out the session         */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*    _nx_secure_dtls_session_create        Create DTLS session           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            optimized the logic,        */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s), and      */
/*                                            optimized internal logic,   */
/*                                            added flexibility of using  */
/*                                            macros instead of direct C  */
/*                                            library function calls,     */
/*                                            fixed the usage of crypto   */
/*                                            metadata for hash method,   */
/*                                            fixed the usage of crypto   */
/*                                            metadata for hash method,   */
/*                                            verify TLS is initalized    */
/*                                            before TLS operations,      */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_session_create(NX_SECURE_TLS_SESSION *session_ptr,
                                   const NX_SECURE_TLS_CRYPTO *crypto_table,
                                   VOID *metadata_buffer,
                                   ULONG metadata_size)
{
NX_SECURE_TLS_SESSION          *tail_ptr;
UINT                            i;
UINT                            max_public_cipher_metadata_size  = 0;
UINT                            max_public_auth_metadata_size    = 0;
UINT                            max_session_cipher_metadata_size = 0;
UINT                            max_hash_mac_metadata_size       = 0;
UINT                            max_tls_prf_metadata_size        = 0;
UINT                            max_handshake_hash_metadata_size = 0;
UINT                            max_handshake_hash_scratch_size  = 0;
ULONG                           max_total_metadata_size;
ULONG                           offset;
CHAR                           *metadata_area;

NX_SECURE_TLS_CIPHERSUITE_INFO *ciphersuite_table;
USHORT                          ciphersuite_table_size;
NX_SECURE_X509_CRYPTO          *cert_crypto;
USHORT                          cert_crypto_size;

#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
NX_CRYPTO_METHOD *crypto_method_md5;
NX_CRYPTO_METHOD *crypto_method_sha1;
#endif

#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
NX_CRYPTO_METHOD *crypto_method_sha256;
#endif

    NX_SECURE_MEMSET(session_ptr, 0, sizeof(NX_SECURE_TLS_SESSION));

    metadata_area = (CHAR *)metadata_buffer;

    /* Assign the table to the session. */
    session_ptr -> nx_secure_tls_crypto_table = crypto_table;

    /* Get working pointers to our crypto methods. */
    ciphersuite_table = crypto_table -> nx_secure_tls_ciphersuite_lookup_table;
    ciphersuite_table_size = crypto_table -> nx_secure_tls_ciphersuite_lookup_table_size;
    cert_crypto = crypto_table -> nx_secure_tls_x509_cipher_table;
    cert_crypto_size = crypto_table -> nx_secure_tls_x509_cipher_table_size;

#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
    crypto_method_md5 = crypto_table -> nx_secure_tls_handshake_hash_md5_method;
    crypto_method_sha1 = crypto_table -> nx_secure_tls_handshake_hash_sha1_method;
#endif
#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
    crypto_method_sha256 = crypto_table -> nx_secure_tls_handshake_hash_sha256_method;
#endif

    /* Loop through the ciphersuite table and find the largest metadata for each type of cipher. */
    for (i = 0; i < ciphersuite_table_size; ++i)
    {
        if (max_public_cipher_metadata_size < ciphersuite_table[i].nx_secure_tls_public_cipher -> nx_crypto_metadata_area_size)
        {
            max_public_cipher_metadata_size = ciphersuite_table[i].nx_secure_tls_public_cipher -> nx_crypto_metadata_area_size;
        }

        if (max_public_auth_metadata_size < ciphersuite_table[i].nx_secure_tls_public_auth -> nx_crypto_metadata_area_size)
        {
            max_public_auth_metadata_size = ciphersuite_table[i].nx_secure_tls_public_auth -> nx_crypto_metadata_area_size;
        }

        if (max_session_cipher_metadata_size < ciphersuite_table[i].nx_secure_tls_session_cipher -> nx_crypto_metadata_area_size)
        {
            max_session_cipher_metadata_size = ciphersuite_table[i].nx_secure_tls_session_cipher -> nx_crypto_metadata_area_size;
        }

        if (max_tls_prf_metadata_size < ciphersuite_table[i].nx_secure_tls_prf -> nx_crypto_metadata_area_size)
        {
            max_tls_prf_metadata_size = ciphersuite_table[i].nx_secure_tls_prf -> nx_crypto_metadata_area_size;
        }

        if (max_hash_mac_metadata_size < ciphersuite_table[i].nx_secure_tls_hash -> nx_crypto_metadata_area_size)
        {
            max_hash_mac_metadata_size = ciphersuite_table[i].nx_secure_tls_hash -> nx_crypto_metadata_area_size;
        }
    }

    /* Loop through the certificate cipher table as well. */
    for (i = 0; i < cert_crypto_size; ++i)
    {
        if (max_public_auth_metadata_size < cert_crypto[i].nx_secure_x509_public_cipher_method -> nx_crypto_metadata_area_size)
        {
            max_public_auth_metadata_size = cert_crypto[i].nx_secure_x509_public_cipher_method -> nx_crypto_metadata_area_size;
        }

        if (max_hash_mac_metadata_size < cert_crypto[i].nx_secure_x509_hash_method -> nx_crypto_metadata_area_size)
        {
            max_hash_mac_metadata_size = cert_crypto[i].nx_secure_x509_hash_method -> nx_crypto_metadata_area_size;
        }

        if (max_handshake_hash_scratch_size < cert_crypto[i].nx_secure_x509_hash_method -> nx_crypto_metadata_area_size)
        {
            max_handshake_hash_scratch_size = cert_crypto[i].nx_secure_x509_hash_method -> nx_crypto_metadata_area_size;
        }
    }

    /* We also need metadata space for the TLS handshake hash, so add that into the total.
       We need some scratch space to copy the handshake hash metadata during final hash generation
       so figure out the largest metadata between SHA-1+MD5 (TLSv1.0, 1.1) and SHA256 (TLSv1.2). */
#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
    max_handshake_hash_metadata_size += (crypto_method_md5 -> nx_crypto_metadata_area_size +
                                         crypto_method_sha1 -> nx_crypto_metadata_area_size);
    if (max_handshake_hash_scratch_size < crypto_method_md5 -> nx_crypto_metadata_area_size + crypto_method_sha1 -> nx_crypto_metadata_area_size)
    {
        max_handshake_hash_scratch_size = crypto_method_md5 -> nx_crypto_metadata_area_size + crypto_method_sha1 -> nx_crypto_metadata_area_size;
    }

    if (max_tls_prf_metadata_size < crypto_table -> nx_secure_tls_prf_1_method -> nx_crypto_metadata_area_size)
    {
        max_tls_prf_metadata_size = crypto_table -> nx_secure_tls_prf_1_method -> nx_crypto_metadata_area_size;
    }
#endif
#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
    max_handshake_hash_metadata_size += crypto_method_sha256 -> nx_crypto_metadata_area_size;

    /* See if the scratch size from above is bigger. */
    if (max_handshake_hash_scratch_size < crypto_method_sha256 -> nx_crypto_metadata_area_size)
    {
        max_handshake_hash_scratch_size = crypto_method_sha256 -> nx_crypto_metadata_area_size;
    }

    if (max_tls_prf_metadata_size < crypto_table -> nx_secure_tls_prf_sha256_method -> nx_crypto_metadata_area_size)
    {
        max_tls_prf_metadata_size = crypto_table -> nx_secure_tls_prf_sha256_method -> nx_crypto_metadata_area_size;
    }
#endif

    /* The public cipher and authentication should never be used simultaneously, so it should be OK
         to share their metadata areas. */
    if (max_public_cipher_metadata_size < max_public_auth_metadata_size)
    {
        max_public_cipher_metadata_size = max_public_auth_metadata_size;
    }

    /* The Total metadata size needed is the sum of all the maximums calculated above.
       We need to keep track of two separate session cipher states, one for the server and one for the client,
       so account for that extra space. */
    max_total_metadata_size = max_public_cipher_metadata_size +
                              (2 * max_session_cipher_metadata_size) +
                              max_hash_mac_metadata_size +
                              max_tls_prf_metadata_size +
                              max_handshake_hash_metadata_size +
                              max_handshake_hash_scratch_size;

    /* Check if the caller provided enough metadata space. */
    if (max_total_metadata_size > metadata_size)
    {
        return(NX_INVALID_PARAMETERS);
    }

    /* Get the protection. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);


    /* Clear out the X509 certificate stores when we create a new TLS Session. */
    session_ptr -> nx_secure_tls_credentials.nx_secure_tls_certificate_store.nx_secure_x509_remote_certificates = NX_NULL;
    session_ptr -> nx_secure_tls_credentials.nx_secure_tls_certificate_store.nx_secure_x509_local_certificates = NX_NULL;
    session_ptr -> nx_secure_tls_credentials.nx_secure_tls_certificate_store.nx_secure_x509_trusted_certificates = NX_NULL;
    session_ptr -> nx_secure_tls_credentials.nx_secure_tls_active_certificate = NX_NULL;

    /* Release the protection. */
    tx_mutex_put(&_nx_secure_tls_protection);

    /* Initialize/reset all the other TLS state. */
    _nx_secure_tls_session_reset(session_ptr);

    /* Get the protection. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

    /* Now allocate cipher metadata space from our calculated numbers above. */
    offset = 0;

    /* Handshake hash metadata. */
#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
    session_ptr -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_md5_metadata = &metadata_area[offset];
    session_ptr -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_md5_metadata_size = crypto_method_md5 -> nx_crypto_metadata_area_size;
    offset += crypto_method_md5 -> nx_crypto_metadata_area_size;

    session_ptr -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha1_metadata = &metadata_area[offset];
    session_ptr -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha1_metadata_size = crypto_method_sha1 -> nx_crypto_metadata_area_size;
    offset += crypto_method_sha1 -> nx_crypto_metadata_area_size;
#endif

#if NX_SECURE_TLS_TLS_1_2_ENABLED
    session_ptr -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata = &metadata_area[offset];
    session_ptr -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata_size = crypto_method_sha256 -> nx_crypto_metadata_area_size;
    offset += crypto_method_sha256 -> nx_crypto_metadata_area_size;
#endif

    session_ptr -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch = &metadata_area[offset];
    session_ptr -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch_size = max_handshake_hash_scratch_size;
    offset += max_handshake_hash_scratch_size;

    /* Client and server session cipher metadata. */
    session_ptr -> nx_secure_session_cipher_metadata_size = max_session_cipher_metadata_size;

    session_ptr -> nx_secure_session_cipher_metadata_area_client = &metadata_area[offset];
    offset += max_session_cipher_metadata_size;

    session_ptr -> nx_secure_session_cipher_metadata_area_server = &metadata_area[offset];
    offset += max_session_cipher_metadata_size;

    /* Public cipher metadata. */
    session_ptr -> nx_secure_public_cipher_metadata_area = &metadata_area[offset];
    session_ptr -> nx_secure_public_cipher_metadata_size = max_public_cipher_metadata_size;
    offset += max_public_cipher_metadata_size;

    /* Public authentication metadata. For now it shares space with the public cipher. */
    session_ptr -> nx_secure_public_auth_metadata_area = session_ptr -> nx_secure_public_cipher_metadata_area;
    session_ptr -> nx_secure_public_auth_metadata_size = max_public_cipher_metadata_size;

    /* Hash MAC metadata. */
    session_ptr -> nx_secure_hash_mac_metadata_area = &metadata_area[offset];
    session_ptr -> nx_secure_hash_mac_metadata_size = max_hash_mac_metadata_size;
    offset += max_hash_mac_metadata_size;

    /* TLS PRF metadata. */
    session_ptr -> nx_secure_tls_prf_metadata_area = &metadata_area[offset];
    session_ptr -> nx_secure_tls_prf_metadata_size = max_tls_prf_metadata_size;
    offset += max_tls_prf_metadata_size;

    /* Place the new TLS control block on the list of created TLS. */
    if (_nx_secure_tls_created_ptr)
    {

        /* Pickup tail pointer. */
        tail_ptr = _nx_secure_tls_created_ptr -> nx_secure_tls_created_previous;

        /* Place the new TLS control block in the list. */
        _nx_secure_tls_created_ptr -> nx_secure_tls_created_previous = session_ptr;
        tail_ptr -> nx_secure_tls_created_next = session_ptr;

        /* Setup this TLS's created links. */
        session_ptr -> nx_secure_tls_created_previous = tail_ptr;
        session_ptr -> nx_secure_tls_created_next = _nx_secure_tls_created_ptr;
    }
    else
    {

        /* The created TLS list is empty. Add TLS control block to empty list. */
        _nx_secure_tls_created_ptr = session_ptr;
        session_ptr -> nx_secure_tls_created_previous = session_ptr;
        session_ptr -> nx_secure_tls_created_next = session_ptr;
    }
    _nx_secure_tls_created_count++;

    /* Flag to indicate when a session renegotiation is enabled. Enabled by default. */
    session_ptr -> nx_secure_tls_renegotation_enabled = NX_TRUE;

    /* Set ID to check initialization status. */
    session_ptr -> nx_secure_tls_id = NX_SECURE_TLS_ID;

    /* Release the protection. */
    tx_mutex_put(&_nx_secure_tls_protection);

    return(NX_SUCCESS);
}

