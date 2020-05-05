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
/**    X509 Digital Certificates                                          */
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
/*    _nx_secure_x509_store_certificate_remove            PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function removes a certificate from an X509 certificate store  */
/*    in a caller-specified position (local device certificates, remote   */
/*    certs, or the trusted store).                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    store                                 Pointer to certificate store  */
/*    name                                  Name for cert matching        */
/*    location                              Location of certificate       */
/*    cert_id                               ID for cert match.            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_certificate_list_remove                             */
/*                                          Remove certificate from list  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_local_certificate_remove                             */
/*                                          Remove certificate from TLS   */
/*                                           local store                  */
/*    _nx_secure_tls_remote_certificate_free                              */
/*                                          Free remote certificate       */
/*    _nx_secure_tls_server_certificate_remove                            */
/*                                          Remove certificate from TLS   */
/*                                           server store                 */
/*    _nx_secure_tls_trusted_certificate_remove                           */
/*                                          Remove certificate from TLS   */
/*                                           trusted store                */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            matched the certificate ID, */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_x509_store_certificate_remove(NX_SECURE_X509_CERTIFICATE_STORE *store,
                                              NX_SECURE_X509_DISTINGUISHED_NAME *name,
                                              UINT location, UINT cert_id)
{
UINT                  status;
NX_SECURE_X509_CERT **store_ptr = NX_NULL;

    /* Store must be non-NULL. */
    if (store == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    status = NX_SUCCESS;

    /* Pick our store based on location. */
    switch (location)
    {
    case NX_SECURE_X509_CERT_LOCATION_LOCAL:
        store_ptr = &store -> nx_secure_x509_local_certificates;
        break;
    case NX_SECURE_X509_CERT_LOCATION_REMOTE:
        store_ptr = &store -> nx_secure_x509_remote_certificates;
        break;
    case NX_SECURE_X509_CERT_LOCATION_TRUSTED:
        store_ptr = &store -> nx_secure_x509_trusted_certificates;
        break;
    case NX_SECURE_X509_CERT_LOCATION_EXCEPTIONS:
        store_ptr = &store -> nx_secure_x509_certificate_exceptions;
        break;
    case NX_SECURE_X509_CERT_LOCATION_NONE:     /* Deliberate fall-through. */
    default:
        status = NX_INVALID_PARAMETERS;
        break;
    }

    /* Invalid certificate location or other issue. */
    if (status)
    {
        return(status);
    }

    /* Remove the certificate from the selected store. */
    status = _nx_secure_x509_certificate_list_remove(store_ptr, name, cert_id);

    return(status);
}

