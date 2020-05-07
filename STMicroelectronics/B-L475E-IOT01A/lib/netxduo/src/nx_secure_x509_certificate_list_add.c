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
/*    _nx_secure_x509_certificate_list_add                PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function adds a NX_SECURE_X509_CERT instance to a              */
/*    certificate linked list. These lists are used to store certificates */
/*    for the local device, trusted store, and for allocating space for   */
/*    receiving certificates.                                             */
/*                                                                        */
/*    This function will reject certificates with duplicate distinguished */
/*    names unless "duplicates_ok" is set to non-zero. Some stores (e.g.  */
/*    the free store for incoming remote certificates) need to be able to */
/*    store duplicate certificates.                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    list_head                             Pointer to list head pointer  */
/*    certificate                           Pointer to certificate        */
/*    duplicates_ok                         If true, allow duplicates     */
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
/*    _nx_secure_x509_store_certificate_add Add certificate to free store */
/*    _nx_secure_tls_process_remote_certificate                           */
/*                                          Process server certificate    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s), fixed    */
/*                                            compiler warnings, fixed    */
/*                                            the logic to add certificate*/
/*                                            with duplicate ID, matched  */
/*                                            the certificate ID,         */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_x509_certificate_list_add(NX_SECURE_X509_CERT **list_head,
                                          NX_SECURE_X509_CERT *certificate, UINT duplicates_ok)
{
NX_SECURE_X509_CERT *current_cert;
NX_SECURE_X509_CERT *previous_cert;
INT                  compare_result;

    /* Check to see if the head of the certificates list is NULL. */
    if (*list_head == NULL)
    {
        /* Our certificate pointer was NULL, so just set it to this certificate. */
        *list_head = certificate;
        certificate -> nx_secure_x509_next_certificate = NULL;
    }
    else
    {
        /* There is already a certificate in the list. Walk the list
           until we find the end and add our new certificate. */
        current_cert = *list_head;
        previous_cert = current_cert;

        while (current_cert != NX_NULL)
        {
            if (current_cert == certificate)
            {
                /* Oops, tried to add the same certificate twice (would lead
                   to circular list)! */
                return(NX_INVALID_PARAMETERS);
            }

            /* If the certificate has a non-zero identifier, make sure the identifier wasn't added yet! */
            if (certificate -> nx_secure_x509_cert_identifier != 0 &&
                current_cert -> nx_secure_x509_cert_identifier == certificate -> nx_secure_x509_cert_identifier)
            {
                /* Duplicate ID found - don't add to the list! */
                return(NX_SECURE_TLS_CERT_ID_DUPLICATE);
            }

            /* We want to be able to add duplicate entries to some of the certificate stores (e.g. the
               free certificate list which contains uninitialized certs), so conditionally allow duplicates. */
            if (!duplicates_ok)
            {
                /* Make sure we don't try to add the same cert twice. */
                compare_result = _nx_secure_x509_distinguished_name_compare(&current_cert -> nx_secure_x509_distinguished_name,
                                                                            &certificate -> nx_secure_x509_distinguished_name, NX_SECURE_X509_NAME_COMMON_NAME);

                if (compare_result == 0)
                {
                    /* A certificate with the same distinguished name was already added to this list. */
                    return(NX_INVALID_PARAMETERS);
                }
            }

            /* Advance to the next certificate. */
            previous_cert = current_cert;
            current_cert =  current_cert -> nx_secure_x509_next_certificate;
        }

        /* Append the new certificate to the end of the list. */
        previous_cert -> nx_secure_x509_next_certificate = certificate;
        certificate -> nx_secure_x509_next_certificate = NULL;
    }

    return(NX_SECURE_TLS_SUCCESS);
}

