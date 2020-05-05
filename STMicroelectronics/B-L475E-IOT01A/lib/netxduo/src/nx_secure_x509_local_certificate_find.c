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
/*    _nx_secure_x509_local_certificate_find              PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function searches a given certificate store for a specific     */
/*    certificate. This is decided by searching the "local" certificate   */
/*    list in the given store for a certificate based on a specific       */
/*    unique ID in case multiple certificates share the same name.        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    store                                 Pointer to certificate store  */
/*    certificate                           Pointer to cert pointer       */
/*    cert_id                               Unique certificate identifier */
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
/*    _nx_secure_tls_server_certificate_find                              */
/*                                          Find server certificate       */
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
UINT _nx_secure_x509_local_certificate_find(NX_SECURE_X509_CERTIFICATE_STORE *store,
                                            NX_SECURE_X509_CERT **certificate, UINT cert_id)
{
NX_SECURE_X509_CERT *list_head;
NX_SECURE_X509_CERT *current_cert;

    /* Get the first certificate in the local store. */
    list_head = store -> nx_secure_x509_local_certificates;

    if (list_head == NX_NULL)
    {
        /* No certificates in this store! */
        return(NX_SECURE_TLS_CERTIFICATE_NOT_FOUND);
    }

    /* Walk the list until we find a certificate that has a matching ID. */
    current_cert = list_head;

    while (current_cert != NX_NULL)
    {
        if (current_cert -> nx_secure_x509_cert_identifier == cert_id)
        {
            /* We found a match, return it. */
            if (certificate != NX_NULL)
            {
                /* If certificate is NULL, just return that one was found, but nothing to return. */
                *certificate = current_cert;
            }

            /* We are OK to quit now, we found the certificate. */
            return(NX_SUCCESS);
        }

        /* Advance our current certificate pointer. */
        current_cert = current_cert -> nx_secure_x509_next_certificate;
    }

    /* No matching certificates in this store! */
    return(NX_SECURE_TLS_CERTIFICATE_NOT_FOUND);
}

