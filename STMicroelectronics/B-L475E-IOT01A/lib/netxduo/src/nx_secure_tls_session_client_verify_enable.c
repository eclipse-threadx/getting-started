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
/*    _nx_secure_tls_session_client_verify_enable         PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function enables Client Certificate Verification for TLS       */
/*    Server instances. If enabled, the TLS Server will request and       */
/*    verify a remote TLS Client Certificate using all available crypto   */
/*    signature routines. The certificate must have space allocated using */
/*    nx_secure_tls_remote_certificate_allocate and will be checked       */
/*    against the trusted certificate store built using                   */
/*    nx_secure_tls_trusted_certificate_add.                              */
/*                                                                        */
/*    Note that this will only happen for TLS Server sessions. Enabling   */
/*    Client Certificate Verification for TLS Client sessions will have   */
/*    no effect.                                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           Pointer to TLS Session        */
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
UINT  _nx_secure_tls_session_client_verify_enable(NX_SECURE_TLS_SESSION *tls_session)
{

    /* Signal the TLS stack to request and verify remote Client certificates. */
    tls_session -> nx_secure_tls_verify_client_certificate = NX_TRUE;

    /* Return completion status.  */
    return(NX_SUCCESS);
}

