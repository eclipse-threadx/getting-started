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
#ifdef NX_SECURE_ENABLE_DTLS
#include "nx_secure_dtls.h"
#endif /* NX_SECURE_ENABLE_DTLS */

/* Bring in externs for caller checking code.  */

NX_SECURE_CALLER_CHECKING_EXTERNS

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_secure_tls_session_protocol_version_override   PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the TLS protocol version         */
/*    override call.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           Pointer to TLS Session        */
/*    protocol_version                      Version of TLS to use         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_session_protocol_version_override                    */
/*                                          Actual protocol override call */
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
/*  08-15-2019     Timothy Stapko           Modified comment(s), check TLS*/
/*                                            session initialization,     */
/*                                            added caller checking,      */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT  _nxe_secure_tls_session_protocol_version_override(NX_SECURE_TLS_SESSION *tls_session,
                                                        USHORT protocol_version)
{
UINT status;

    if (tls_session == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Make sure the version supplied is known and supported. */
    switch (protocol_version)
    {
    /* Supported TLS versions. */
    case NX_SECURE_TLS_VERSION_TLS_1_0:
    case NX_SECURE_TLS_VERSION_TLS_1_1:
    case NX_SECURE_TLS_VERSION_TLS_1_2:
        break;
        /* DTLS versions are known, but this is a TLS session!
           SSLv3 is not supported. */
#ifdef NX_SECURE_ENABLE_DTLS
    case NX_SECURE_DTLS_VERSION_1_0:
    case NX_SECURE_DTLS_VERSION_1_2:
#endif /* NX_SECURE_ENABLE_DTLS */
    case NX_SECURE_TLS_VERSION_SSL_3_0:
        return(NX_SECURE_TLS_UNSUPPORTED_TLS_VERSION);
    default:
        return(NX_SECURE_TLS_UNKNOWN_TLS_VERSION);
    }

    /* Make sure the session is initialized. */
    if(tls_session -> nx_secure_tls_id != NX_SECURE_TLS_ID)
    {
        return(NX_SECURE_TLS_SESSION_UNINITIALIZED);
    }
    
    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    status =  _nx_secure_tls_session_protocol_version_override(tls_session, protocol_version);

    /* Return completion status.  */
    return(status);
}

