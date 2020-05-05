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
#ifdef NX_SECURE_ENABLE_DTLS
#include "nx_secure_dtls.h"
#endif /* NX_SECURE_ENABLE_DTLS */

/* This table is used to determine the TLS versions currently enabled and supported. */
static const NX_SECURE_TLS_VERSIONS nx_secure_tls_supported_versions[] =
{
    {NX_SECURE_TLS_VERSION_SSL_3_0, NX_SECURE_TLS_SSL_3_0_ENABLED},   /* SSLv3   */
    {NX_SECURE_TLS_VERSION_TLS_1_0, NX_SECURE_TLS_TLS_1_0_ENABLED},   /* TLS 1.0 */
    {NX_SECURE_TLS_VERSION_TLS_1_1, NX_SECURE_TLS_TLS_1_1_ENABLED},   /* TLS 1.1 */
    {NX_SECURE_TLS_VERSION_TLS_1_2, NX_SECURE_TLS_TLS_1_2_ENABLED}    /* TLS 1.2 */
};

#ifdef NX_SECURE_ENABLE_DTLS
/* This table is used to determine the DTLS versions currently enabled and supported. */
static const NX_SECURE_TLS_VERSIONS nx_secure_dtls_supported_versions[] =
{
    {NX_SECURE_DTLS_VERSION_1_0, NX_SECURE_TLS_TLS_1_1_ENABLED},      /* DTLS 1.0 */
    {NX_SECURE_DTLS_VERSION_1_2, NX_SECURE_TLS_TLS_1_2_ENABLED},      /* DTLS 1.2 */
};
#endif /* NX_SECURE_ENABLE_DTLS */

/* Supported version list for TLS and DTLS. */
NX_SECURE_VERSIONS_LIST nx_secure_supported_versions_list[] =
{
    {nx_secure_tls_supported_versions, sizeof(nx_secure_tls_supported_versions) / sizeof(NX_SECURE_TLS_VERSIONS)},
#ifdef NX_SECURE_ENABLE_DTLS
    {nx_secure_dtls_supported_versions, sizeof(nx_secure_dtls_supported_versions) / sizeof(NX_SECURE_TLS_VERSIONS)},
#endif /* NX_SECURE_ENABLE_DTLS */
};


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_check_protocol_version               PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks the protocol version in received TLS/DTLS      */
/*    headers against the established TLS/DTLS session version.           */
/*                                                                        */
/*    The function also uses a compile-time constant table to see if the  */
/*    version provided in the initial Hello messages is currently         */
/*    supported and enabled in the TLS/DTLS stack. If the version is not  */
/*    recognized or not supported, an error is returned.                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS session                   */
/*    protocol_version                      Received TLS version          */
/*    id                                    TLS or DTLS                   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Enabled status of version     */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_process_header        Process record header         */
/*    _nx_secure_tls_process_clienthello    Process ClientHello           */
/*    _nx_secure_tls_process_header         Process record header         */
/*    _nx_secure_tls_process_serverhello    Process ServerHello           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            supported TLS protocol      */
/*                                            version override, supported */
/*                                            DTLS,                       */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_check_protocol_version(NX_SECURE_TLS_SESSION *tls_session,
                                           USHORT protocol_version, UINT id)
{
UINT i;

    /* See if this is an established TLS session with a negotiated TLS version. */
    if (tls_session -> nx_secure_tls_protocol_version != 0)
    {
        /* If the session already has a protocol version, it means the version was already
         * determined, and if we get a different version in a received record, we have a problem. */
        if (tls_session -> nx_secure_tls_protocol_version == protocol_version)
        {
            /* Version is good, return success. */
            return(NX_SUCCESS);
        }

        /* We have an established session, but the incoming version does not match! This is
           a fatal error. */
        return(NX_SECURE_TLS_PROTOCOL_VERSION_CHANGED);
    }
    else
    {
        /* See if the application has overridden the protocol version. */
        if (tls_session -> nx_secure_tls_protocol_version_override != 0)
        {
            /* If the override is euqal to the supplied version, we're all good. */
            if (tls_session -> nx_secure_tls_protocol_version_override == protocol_version)
            {
                return(NX_SUCCESS);
            }

            /* Supplied version doesn't match the override, allow the stack to re-negotiate
               to the selected version by returning "unsupported version" even if we do support
               the supplied version. */
            return(NX_SECURE_TLS_UNSUPPORTED_TLS_VERSION);
        }

        /* Loop through the supported versions to see if the passed-in version is one that is enabled. */
        for (i = 0; i < nx_secure_supported_versions_list[id].nx_secure_versions_list_count; ++i)
        {
            if (protocol_version == nx_secure_supported_versions_list[id].nx_secure_versions_list[i].nx_secure_tls_protocol_version)
            {
                /* We have a match. See if it is supported. */
                if (nx_secure_supported_versions_list[id].nx_secure_versions_list[i].nx_secure_tls_is_supported)
                {
                    /* Supported version. Return success. */
                    return(NX_SUCCESS);
                }
                else
                {
                    /* Recognized the version, but it is not enabled or supported. */
                    return(NX_SECURE_TLS_UNSUPPORTED_TLS_VERSION);
                }
            }
        }
    }

    /* This error indicates an incorrect TLS record or possible attack. */
    return(NX_SECURE_TLS_UNKNOWN_TLS_VERSION);
}

