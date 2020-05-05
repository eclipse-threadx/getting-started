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

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_session_certificate_callback_set     PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets up a function pointer that TLS will invoke when  */
/*    a certificate is received from a remote host, allowing the          */
/*    application to perform validation checks such as DNS validation,    */
/*    certificate revocation, and certificate policy enforcement.         */
/*                                                                        */
/*    NetX Secure TLS will perform basic validation on the certificate    */
/*    before invoking the callback to assure that the certificate can be  */
/*    traced to a certificate in the TLS trusted certificate store, but   */
/*    all other validation will be handled by this callback.              */
/*                                                                        */
/*    The callback provides the TLS session pointer and a pointer to the  */
/*    remote host identity certificate (the leaf in the certificate       */
/*    chain). The callback should return NX_SUCCESS if all validation is  */
/*    successful, otherwise it should return an error code indicating the */
/*    validation failure. Any value other than NX_SUCCESS will cause the  */
/*    TLS handshake to immediately abort.                                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    func_ptr                              Pointer to callback function  */
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
UINT _nx_secure_tls_session_certificate_callback_set(NX_SECURE_TLS_SESSION *tls_session,
                                                     ULONG (*func_ptr)(NX_SECURE_TLS_SESSION *session,
                                                                       NX_SECURE_X509_CERT *certificate))
{
    /* Set the function pointer in the TLS session. */
    tls_session -> nx_secure_tls_session_certificate_callback = func_ptr;

    return(NX_SUCCESS);
}

