/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
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
/*    _nx_secure_tls_newest_supported_version             PORTABLE C      */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    Return the newest currently supported and enabled TLS/DTLS protocol */
/*    version. This enables the backward-compatible TLS/DTLS handshake    */
/*    and forces the upgrade to the latest supported TLS/DTLS version.    */
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
/*    _nx_secure_tls_protocol_version_get   Get current TLS version to use*/
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*                                                                        */
/**************************************************************************/
VOID _nx_secure_tls_newest_supported_version(NX_SECURE_TLS_SESSION *session_ptr,
                                             USHORT *protocol_version, UINT id)
{
INT i;

    NX_PARAMETER_NOT_USED(session_ptr); 

#ifndef NX_SECURE_TLS_SERVER_DISABLED    
    if(session_ptr -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_SERVER &&
       session_ptr -> nx_secure_tls_protocol_version_override != 0)
    {
        /* If this is a server and the user has overriden the protocol version,
           THAT version is now the higest supported. */
        (*protocol_version) = session_ptr -> nx_secure_tls_protocol_version_override;
        return;
    }
#endif    
    /* Table is in order of oldest to newest, so walk backward to get
     * the newest version we support. */
    for (i = (INT)(nx_secure_supported_versions_list[id].nx_secure_versions_list_count - 1); i >= 0; --i)
    {
        /* If the version is supported, return it. */
        if (nx_secure_supported_versions_list[id].nx_secure_versions_list[i].nx_secure_tls_is_supported)
        {
            (*protocol_version) = nx_secure_supported_versions_list[id].nx_secure_versions_list[i].nx_secure_tls_protocol_version;
            return;
        }
    }

    /* If we get here, no versions of TLS have been enabled. Set protocol to 0 to indicate failure. */
    (*protocol_version) = 0x0;
    return;
}

