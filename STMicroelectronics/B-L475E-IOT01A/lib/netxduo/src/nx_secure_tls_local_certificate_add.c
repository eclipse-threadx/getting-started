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
#include "nx_secure_x509.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_local_certificate_add                PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function adds an initialized NX_SECURE_TLS_CERTIFICATE to a    */
/*    TLS session for use as a local identification certificate - the     */
/*    TLS Server certificate for TLS servers, and the Client certificate  */
/*    for TLS clients. The function may be called repeatedly to add       */
/*    multiple certificates to the internal linked-list.                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           Pointer to TLS Session        */
/*    certificate                           Pointer to certificate        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*    _nx_secure_x509_store_certificate_add Add the certificate to the    */
/*                                           local store                  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*    _nx_secure_tls_server_certificate_add Add server certificate        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            fixed the usage of crypto   */
/*                                            metadata for hash method,   */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_local_certificate_add(NX_SECURE_TLS_SESSION *tls_session,
                                          NX_SECURE_X509_CERT *certificate)
{
UINT status;

    /* Get the protection. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

    /* Assign the TLS Session metadata areas to the certificate for later use. */
    certificate -> nx_secure_x509_public_cipher_metadata_area = tls_session -> nx_secure_public_cipher_metadata_area;
    certificate -> nx_secure_x509_public_cipher_metadata_size = tls_session -> nx_secure_public_cipher_metadata_size;

    certificate -> nx_secure_x509_hash_metadata_area = tls_session -> nx_secure_hash_mac_metadata_area;
    certificate -> nx_secure_x509_hash_metadata_size = tls_session -> nx_secure_hash_mac_metadata_size;

    /* Assign the cipher table from the parent TLS session. */
    certificate -> nx_secure_x509_cipher_table = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_x509_cipher_table;
    certificate -> nx_secure_x509_cipher_table_size = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_x509_cipher_table_size;


    /* Add the certificate to the TLS session credentials X509 store. */
    status = _nx_secure_x509_store_certificate_add(certificate, &tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store,
                                                   NX_SECURE_X509_CERT_LOCATION_LOCAL);

    /* Release the protection. */
    tx_mutex_put(&_nx_secure_tls_protection);

    return(status);
}

