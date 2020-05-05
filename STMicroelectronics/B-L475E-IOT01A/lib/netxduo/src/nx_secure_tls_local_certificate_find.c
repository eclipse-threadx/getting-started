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
/*    _nx_secure_tls_local_certificate_find               PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function finds a local certificate based upon its Common Name  */
/*    field in the certificate's Subject section.                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           Pointer to TLS Session        */
/*    certificate                           Return certificate pointer    */
/*    common_name                           Certificate Common Name       */
/*    name_length                           Length of Common Name         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_certificate_list_find                               */
/*                                          Find certificate in local     */
/*                                            store by name               */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
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
UINT  _nx_secure_tls_local_certificate_find(NX_SECURE_TLS_SESSION *tls_session,
                                            NX_SECURE_X509_CERT **certificate, UCHAR *common_name,
                                            UINT name_length)
{
UINT                              status;
NX_SECURE_X509_CERT              *list_head;
NX_SECURE_X509_CERTIFICATE_STORE *store;
NX_SECURE_X509_DISTINGUISHED_NAME name;

    /* Get the remote certificate store from our TLS session. */
    store = &tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store;

    /* Get the first certificate in the local store. */
    list_head = store -> nx_secure_x509_local_certificates;

    /* Set up the distinguished name - the find call below only uses the Common Name field to match
       certificates so it's ok if we only fill in the Common Name here. Also, the Distinguished Name
       is only used in this function's context, so it should be OK that we are assigning the pointer
       common_name instead of copying the string data. */
    name.nx_secure_x509_common_name = common_name;
    name.nx_secure_x509_common_name_length = (USHORT)name_length;

    /* Find the certificate using it's name. */
    status = _nx_secure_x509_certificate_list_find(&list_head, &name, 0, certificate);

    /* Return completion status.  */
    return(status);
}

