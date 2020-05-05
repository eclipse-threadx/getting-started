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
/*    _nx_secure_tls_ciphersuite_lookup                   PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns data about a selected ciphersuite for use     */
/*    in various TLS internal functions, such as the ciphers used and     */
/*    associated key sizes.                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS session control block     */
/*    ciphersuite                           Ciphersuite value             */
/*    info                                  Pointer to ciphersuite info   */
/*                                            structure (output)          */
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
/*    _nx_secure_dtls_process_clienthello   Process ClientHello           */
/*    _nx_secure_tls_process_clienthello    Process ClientHello           */
/*    _nx_secure_tls_process_serverhello    Process ServerHello           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            optimized the logic,        */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_ciphersuite_lookup(NX_SECURE_TLS_SESSION *tls_session, UINT ciphersuite,
                                       NX_SECURE_TLS_CIPHERSUITE_INFO const **info)
{
USHORT                          index;
NX_SECURE_TLS_CIPHERSUITE_INFO *cipher_table;
USHORT                          cipher_table_size;

    cipher_table = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_ciphersuite_lookup_table;
    cipher_table_size = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_ciphersuite_lookup_table_size;

    /* The number of ciphersuites is very small so a linear search should be fine. */
    for (index = 0; index < cipher_table_size; ++index)
    {
        /* See if the ciphersuite is supported. */
        if (cipher_table[index].nx_secure_tls_ciphersuite == ciphersuite)
        {
            *info = &cipher_table[index];
            return(NX_SUCCESS);
        }
    }

    /* No ciphersuite found, suite unknown. */
    return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
}

