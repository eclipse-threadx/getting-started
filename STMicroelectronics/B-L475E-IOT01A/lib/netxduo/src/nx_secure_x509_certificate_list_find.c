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
/*    _nx_secure_x509_certificate_list_find               PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function finds a NX_SECURE_X509_CERT instance in a             */
/*    certificate linked list, based on the common name (CN) field.       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    list_head                             Pointer to list head pointer  */
/*    name                                  Distinguished name            */
/*    cert_id                               ID of certificate             */
/*    certificate                           Pointer to return certificate */
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
/*    _nx_secure_tls_local_certificate_find                               */
/*                                          Find local certificate        */
/*    _nx_secure_tls_remote_certificate_free                              */
/*                                          Free remote certificate       */
/*    _nx_secure_x509_local_device_certificate_get                        */
/*                                          Get the local certificate     */
/*    _nx_secure_x509_store_certificate_find                              */
/*                                          Find a cert in a store        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            fixed compiler warnings,    */
/*                                            matched the certificate ID, */
/*                                            optimized the logic,        */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_x509_certificate_list_find(NX_SECURE_X509_CERT **list_head,
                                           NX_SECURE_X509_DISTINGUISHED_NAME *name,
                                           UINT cert_id,
                                           NX_SECURE_X509_CERT **certificate)
{
NX_SECURE_X509_CERT *current_cert;
INT                  compare_result;


    /* Find out NULL pointers. */
    if (certificate == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Return NULL if certificate not found. */
    *certificate = NX_NULL;

    if (list_head == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Walk the list until we find a certificate with a matching CN.
       Use a two-level pointer so we can modify the cert easily. */
    current_cert = *list_head;

    while (current_cert != NX_NULL)
    {
        /* If name is passed as NX_NULL, use the cert_id to match. */
        if (name == NX_NULL)
        {
            /* Check the cert_id against the ID in the certificate. */
            compare_result = (current_cert -> nx_secure_x509_cert_identifier == cert_id) ? 0 : 1;
        }
        else
        {
            /* Check the common name passed in against the parsed certificate CN. */
            compare_result = _nx_secure_x509_distinguished_name_compare(name, &current_cert -> nx_secure_x509_distinguished_name, NX_SECURE_X509_NAME_COMMON_NAME);
        }

        /* We found a match, return it. */
        if (!compare_result)
        {
            *certificate = current_cert;
            return(NX_SUCCESS);
        }

        /* Advance our current certificate pointer. */
        current_cert = current_cert -> nx_secure_x509_next_certificate;
    }

    return(NX_SECURE_TLS_CERTIFICATE_NOT_FOUND);
}

