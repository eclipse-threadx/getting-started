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
/*    _nx_secure_x509_remote_endpoint_certificate_get     PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function searches a given certificate store for an endpoint    */
/*    certificate. This is decided by searching the "remote" certificate  */
/*    list in the given store for a certificate that is not the issuer    */
/*    for another certificate in the store.                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    store                                 Pointer to certificate store  */
/*    certificate                           Pointer to cert pointer       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_distinguished_name_compare                          */
/*                                          Compare distinguished name    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_process_certificate_verify                           */
/*                                          Process CertificateVerify     */
/*    _nx_secure_tls_remote_certificate_verify                            */
/*                                          Verify the server certificate */
/*    _nx_secure_tls_send_client_key_exchange                             */
/*                                          Send ClientKeyExchange        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            fixed compiler warnings,    */
/*                                            added compare bit fields,   */
/*                                            optimized the logic,        */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_x509_remote_endpoint_certificate_get(NX_SECURE_X509_CERTIFICATE_STORE *store,
                                                     NX_SECURE_X509_CERT **certificate)
{
NX_SECURE_X509_CERT *compare_cert;
NX_SECURE_X509_CERT *candidate;
NX_SECURE_X509_CERT *list_head;
INT                  compare_value;


    /* Get the first certificate in the remote store. */
    list_head = store -> nx_secure_x509_remote_certificates;
    candidate = list_head;

    if (candidate == NX_NULL)
    {
        /* No remote certificates in this store! */
        return(NX_SECURE_X509_CERTIFICATE_NOT_FOUND);
    }

    /* At this point, we have multiple certificates in the remote store. We need to loop
       to find the one that isn't an issuer for the others. The list should almost always be
       short (< 5 entries) so optimization isn't critical. */
    while (candidate -> nx_secure_x509_next_certificate != NX_NULL)
    {
        compare_cert = list_head;

        while (compare_cert != NX_NULL)
        {
            /* Search the entire list for this certificate's distinguished name in the issuer fields. */
            compare_value = _nx_secure_x509_distinguished_name_compare(&candidate -> nx_secure_x509_distinguished_name,
                                                                       &compare_cert -> nx_secure_x509_issuer, NX_SECURE_X509_NAME_ALL_FIELDS);

            /* If we matched, break out of the loop. */
            if (compare_value == 0)
            {
                break;
            }

            /* Advance the comparison pointer. */
            compare_cert = compare_cert -> nx_secure_x509_next_certificate;
        }

        if (compare_cert != NX_NULL)
        {
            /* Advance the pointer to the next entry in the list. */
            candidate = candidate -> nx_secure_x509_next_certificate;
        }
        else
        {
            /* We got through the compare loop without matching an issuer field, so break out of the loop
               and return the candidate. */
            break;
        }
    }

    /* Return the candidate. */
    *certificate = candidate;

    /* No matter what we found, it is a certificate so return success - if the certificate
       is invalid for any reason, that will be caught during certificate verification. */
    return(NX_SUCCESS);
}

