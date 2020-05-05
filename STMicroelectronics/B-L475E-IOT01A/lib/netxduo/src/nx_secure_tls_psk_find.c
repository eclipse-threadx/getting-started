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
/*    _nx_secure_tls_psk_find                             PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function finds a pre-shared key (PSK) in a TLS session for use */
/*    with a PSK ciphersuite. The PSK is found using an "identity hint"   */
/*    that should match a field in the PSK structure in the TLS session.  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           Pointer to TLS Session        */
/*    psk_data                              Pointer to PSK data           */
/*    psk_length                            Length of PSK data            */
/*    psk_identity_hint                     PSK identity hint data        */
/*    identity_length                       Length of identity data       */
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
/*    _nx_secure_tls_generate_premaster_secret                            */
/*                                          Generate the shared secret    */
/*                                            used to generate keys later */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            supported ECJPAKE, returned */
/*                                            the first associated PSK    */
/*                                            when no hint is received,   */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s), and added*/
/*                                            added flexibility of using  */
/*                                            macros instead of direct C  */
/*                                            library function calls,     */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
#if defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES) || defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE)
UINT _nx_secure_tls_psk_find(NX_SECURE_TLS_SESSION *tls_session, UCHAR **psk_data, UINT *psk_length,
                             UCHAR *psk_identity_hint, UINT identity_length)
{
UINT psk_list_size;
UINT compare_val;
UINT i;

    /* Get the protection. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

    psk_list_size = tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_count;

    if ((psk_identity_hint[0] == 0) && (psk_list_size > 0))
    {

        /* No hint from server. Return the first associated PSK. */
        *psk_data = tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_store[0].nx_secure_tls_psk_data;
        *psk_length = tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_store[0].nx_secure_tls_psk_data_size;

        /* Release the protection. */
        tx_mutex_put(&_nx_secure_tls_protection);

        return(NX_SUCCESS);
    }

    /* Loop through all PSKs, looking for a matching identity string. */
    for (i = 0; i < psk_list_size; ++i)
    {
        /* Save off the PSK and its length. */
        compare_val = (UINT)NX_SECURE_MEMCMP(tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_store[i].nx_secure_tls_psk_id_hint, psk_identity_hint, identity_length);

        /* See if the identity matched, and the length is the same (without the length, we could have a
           matching prefix which could be a possible attack vector... */
        if (compare_val == 0 && identity_length == tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_store[i].nx_secure_tls_psk_id_hint_size)
        {
            /* Found a matching identity, return the associated PSK. */
            *psk_data = tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_store[i].nx_secure_tls_psk_data;
            *psk_length = tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_store[i].nx_secure_tls_psk_data_size;

            /* Release the protection. */
            tx_mutex_put(&_nx_secure_tls_protection);

            return(NX_SUCCESS);
        }
    }

    /* Release the protection. */
    tx_mutex_put(&_nx_secure_tls_protection);

    return(NX_SECURE_TLS_NO_MATCHING_PSK);
}
#endif

