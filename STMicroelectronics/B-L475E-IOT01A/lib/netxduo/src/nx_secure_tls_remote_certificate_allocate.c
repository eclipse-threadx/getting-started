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
/*    _nx_secure_tls_remote_certificate_allocate          PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This service adds an uninitialized NX_SECURE_TLS_CERTIFICATE        */
/*    structure instance to a TLS session for the purpose of allocating   */
/*    space for certificates provided by a remote host during a TLS       */
/*    session. The remote certificate data is parsed by NetX Secure TLS   */
/*    and that information is used to populate the certificate structure  */
/*    instance provided to this function. Certificates added in this      */
/*    manner are placed in a linked list.                                 */
/*                                                                        */
/*    The "raw_certificate_buffer" parameter is a pointer to a caller-    */
/*    supplied space which can be used to store the incoming certificate  */
/*    data for parsing. The size of the buffer should be large enough to  */
/*    accommodate the largest certificate your application expects to     */
/*    receive. Typical certificate lengths are 1-2KBytes when using       */
/*    2048-bit RSA. Larger key sizes will result in larger certificates.  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           Pointer to TLS Session        */
/*    certificate                           Pointer to certificate        */
/*    raw_certificate_buffer                Buffer for storing cert       */
/*    buffer_size                           Size of cert buffer           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*    _nx_secure_x509_store_certificate_add Add certificate to free store */
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
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s),          */
/*                                            add remote certificate      */
/*                                            memory optimization,        */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_remote_certificate_allocate(NX_SECURE_TLS_SESSION *tls_session,
                                                NX_SECURE_X509_CERT *certificate,
                                                UCHAR *raw_certificate_buffer, UINT buffer_size)
{
UINT status;

    /* Get the protection. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);


    certificate -> nx_secure_x509_certificate_raw_data = raw_certificate_buffer;
    certificate -> nx_secure_x509_certificate_raw_buffer_size = buffer_size;

    /* This certificate was allocated by the user application and not by TLS. */
    certificate->nx_secure_x509_user_allocated_cert = NX_TRUE;

    /* Add the certificate to the TLS session credentials X509 store. */
    status = _nx_secure_x509_store_certificate_add(certificate, &tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store,
                                                   NX_SECURE_X509_CERT_LOCATION_FREE);

    /* Release the protection. */
    tx_mutex_put(&_nx_secure_tls_protection);

    return(status);
}

