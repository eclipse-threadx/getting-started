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

/* We need to access the supported versions table located in nx_secure_tls_check_protocol_version.c. */
extern const NX_SECURE_VERSIONS_LIST nx_secure_supported_versions_list[];

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_protocol_version_get                 PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    Return the protocol version to use for the TLS connection. This may */
/*    be a user-supplied version using the API                            */
/*    nx_secure_tls_session_protocol_version_override. If no version      */
/*    override is supplied, the newest supported and enabled version is   */
/*    returned.                                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                           TLS session                   */
/*    protocol_version                      Pointer to version variable   */
/*    id                                    TLS or DTLS                   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_send_clienthello      Send ClientHello              */
/*    _nx_secure_tls_process_clienthello    Process ClientHello           */
/*    _nx_secure_tls_send_clienthello       Send ClientHello              */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-15-2019     Timothy Stapko           Initial Version 5.12          */
/*                                                                        */
/**************************************************************************/
VOID _nx_secure_tls_protocol_version_get(NX_SECURE_TLS_SESSION *session_ptr,
                                         USHORT *protocol_version, UINT id)
{

    /* First, check for protocol version override and return it if the user has selected to
       use a specific version of TLS (even if a newer version is enabled). */
    if (session_ptr -> nx_secure_tls_protocol_version_override != 0)
    {
        (*protocol_version) = session_ptr -> nx_secure_tls_protocol_version_override;
        return;
    }

    /* No user protocol version override, return newest supported version. */
    _nx_secure_tls_newest_supported_version(session_ptr, protocol_version, id);
    
    return;
}

