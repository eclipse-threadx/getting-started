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
/*    _nx_secure_tls_psk_add                              PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function adds a pre-shared key (PSK) to a TLS session for use  */
/*    with a PSK ciphersuite. The second parameter is the PSK identity    */
/*    used during the TLS handshake to select the proper key.             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           Pointer to TLS Session        */
/*    pre_shared_key                        Pointer to PSK data           */
/*    psk_length                            Length of PSK data            */
/*    psk_identity                          PSK identity data             */
/*    identity_length                       Length of identity data       */
/*    hint                                  PSK hint data                 */
/*    hint_length                           Length of hint data           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            supported ECJPAKE,          */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s), and added*/
/*                                            added flexibility of using  */
/*                                            macros instead of direct C  */
/*                                            library function calls,     */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
#if defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES) || defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE)
UINT _nx_secure_tls_psk_add(NX_SECURE_TLS_SESSION *tls_session, UCHAR *pre_shared_key,
                            UINT psk_length, UCHAR *psk_identity, UINT identity_length, UCHAR *hint,
                            UINT hint_length)
{
UINT status;
UINT current_index;

    /* Get the protection. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

    current_index = tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_count;

    /* Make sure we have space to add the PSK and its identity data. */
    if ((current_index + 1) < NX_SECURE_TLS_MAX_PSK_KEYS &&
        psk_length < NX_SECURE_TLS_MAX_PSK_SIZE &&
        identity_length < NX_SECURE_TLS_MAX_PSK_ID_SIZE)
    {
        /* Save off the PSK and its length. */
        NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_store[current_index].nx_secure_tls_psk_data, pre_shared_key, psk_length);
        tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_store[current_index].nx_secure_tls_psk_data_size = psk_length;

        /* Save off the identity and its length. */
        NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_store[current_index].nx_secure_tls_psk_id, psk_identity, identity_length);
        tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_store[current_index].nx_secure_tls_psk_id_size = identity_length;

        /* Save off the identity and its length. */
        NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_store[current_index].nx_secure_tls_psk_id_hint, hint, hint_length);
        tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_store[current_index].nx_secure_tls_psk_id_hint_size = hint_length;

        /* Increment the session counter. */
        tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_count = current_index + 1;

        status = NX_SUCCESS;
    }
    else
    {
        /* Can't add any more PSKs. */
        status = NX_SECURE_TLS_NO_MORE_PSK_SPACE;
    }

    /* Release the protection. */
    tx_mutex_put(&_nx_secure_tls_protection);

    return(status);
}
#endif

