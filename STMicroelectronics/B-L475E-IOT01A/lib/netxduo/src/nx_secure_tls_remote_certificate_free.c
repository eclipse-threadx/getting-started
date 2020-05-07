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


/* Include necessary system files.  */

#include "nx_secure_tls.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_remote_certificate_free              PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function moves a remote certificate buffer back into the free  */
/*    store. It is used when the remote certificate is no longer needed,  */
/*    such as when a TLS session is ended.                                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           Pointer to TLS Session        */
/*    name                                  Certificate distinguished name*/
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_certificate_list_find                               */
/*                                          Find certificate by name      */
/*    _nx_secure_x509_store_certificate_remove                            */
/*                                          Remove certificate from store */
/*    _nx_secure_x509_store_certificate_add                               */
/*                                          Add certificate to store      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_remote_certificate_free_all                          */
/*                                          Free all remote certificates  */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*  08-15-2019     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_remote_certificate_free(NX_SECURE_TLS_SESSION *tls_session,
                                            NX_SECURE_X509_DISTINGUISHED_NAME *name)
{
UINT                              status;
NX_SECURE_X509_CERT              *list_head;
NX_SECURE_X509_CERTIFICATE_STORE *store;
NX_SECURE_X509_CERT              *certificate;

    /* Get the remote certificate store from our TLS session. */
    store = &tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store;

    /* Get the first certificate in the remote store. */
    list_head = store -> nx_secure_x509_remote_certificates;

    /* Find the certificate using it's name. */
    status = _nx_secure_x509_certificate_list_find(&list_head, name, 0, &certificate);

    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Remove the certificate from the remote store. */
    _nx_secure_x509_store_certificate_remove(store, name, NX_SECURE_X509_CERT_LOCATION_REMOTE, 0);

    /* Add the certificate back to the free store. */
    status = _nx_secure_x509_store_certificate_add(certificate, store, NX_SECURE_X509_CERT_LOCATION_FREE);

    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Return completion status.  */
    return(status);
}

