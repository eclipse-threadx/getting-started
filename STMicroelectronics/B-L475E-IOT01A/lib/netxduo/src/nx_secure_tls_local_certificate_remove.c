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
/*    _nx_secure_tls_local_certificate_remove             PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function removes an NX_SECURE_TLS_CERTIFICATE instance from    */
/*    the local certificates list, keyed on the Common Name field.        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           Pointer to TLS Session        */
/*    common_name                           String to match "CN" field    */
/*    common_name_length                    Length of name                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*    _nx_secure_x509_store_certificate_remove                            */
/*                                          Remove certificate from the   */
/*                                           local store                  */
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
/*                                            matched the certificate ID, */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s), and      */
/*                                            added flexibility of using  */
/*                                            macros instead of direct C  */
/*                                            library function calls,     */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_local_certificate_remove(NX_SECURE_TLS_SESSION *tls_session, UCHAR *common_name,
                                             UINT common_name_length)
{
UINT                              status;
NX_SECURE_X509_DISTINGUISHED_NAME name;

    /* Get the protection. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

    NX_SECURE_MEMSET(&name, 0, sizeof(NX_SECURE_X509_DISTINGUISHED_NAME));

    /* Setup our common name so we can delete from the certificate store. */
    name.nx_secure_x509_common_name = common_name;
    name.nx_secure_x509_common_name_length = (USHORT)common_name_length;

    /* Remove the certificate from the local store. */
    status = _nx_secure_x509_store_certificate_remove(&tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store,
                                                      &name, NX_SECURE_X509_CERT_LOCATION_LOCAL, 0);
    /* Release the protection. */
    tx_mutex_put(&_nx_secure_tls_protection);

    return(status);
}

