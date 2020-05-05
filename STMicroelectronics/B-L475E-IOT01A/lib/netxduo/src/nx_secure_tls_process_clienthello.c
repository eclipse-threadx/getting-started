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
/*    _nx_secure_tls_process_clienthello                  PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an incoming ClientHello message from a      */
/*    remote host, kicking off a TLS handshake.                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    packet_buffer                         Pointer to message data       */
/*    message_length                        Length of message data (bytes)*/
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_ciphersuite_lookup     Lookup current ciphersuite    */
/*    _nx_secure_tls_check_protocol_version Check incoming TLS version    */
/*    _nx_secure_tls_newest_supported_version                             */
/*                                          Get newest TLS version        */
/*    _nx_secure_tls_protocol_version_get   Get TLS version to use        */
/*    _nx_secure_tls_process_clienthello_extensions                       */
/*                                          Process ClientHello extensions*/
/*    _nx_secure_tls_remote_certificate_free_all                          */
/*                                          Free all remote certificates  */
/*    [_nx_secure_tls_session_renegotiate_callback_set]                   */
/*                                          Renegotiation callback        */
/*    [_nx_secure_tls_session_server_callback_set]                        */
/*                                          Server session callback       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_server_handshake       TLS Server state machine      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s), fixed    */
/*                                            compiler warnings, supported*/
/*                                            multiple extensions, more   */
/*                                            ciphersuites, renegotiation,*/
/*                                            and added session callbacks,*/
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s), and      */
/*                                            added flexibility of using  */
/*                                            macros instead of direct C  */
/*                                            library function calls,     */
/*                                            added extension hook,       */
/*                                            corrected the index of      */
/*                                            compress method, supported  */
/*                                            TLS Fallback SCSV, stored   */
/*                                            cipher suite pointer,       */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_process_clienthello(NX_SECURE_TLS_SESSION *tls_session, UCHAR *packet_buffer,
                                        UINT message_length)
{
UINT                                  length;
USHORT                                ciphersuite_list_length;
UCHAR                                 compression_methods_length;
USHORT                                cipher_entry;
UCHAR                                 session_id_length;
UINT                                  i;
UINT                                  status;
USHORT                                protocol_version;
USHORT                                newest_version;
UINT                                  total_extensions_length;
const NX_SECURE_TLS_CIPHERSUITE_INFO *ciphersuite_info;
NX_SECURE_TLS_HELLO_EXTENSION         extension_data[NX_SECURE_TLS_HELLO_EXTENSIONS_MAX];
UINT                                  num_extensions = NX_SECURE_TLS_HELLO_EXTENSIONS_MAX;

    NX_SECURE_PROCESS_CLIENTHELLO_EXTENSION

    /* Structure of ClientHello:
     * |     2       |          4 + 28          |    1       |   <SID len>  |   2    | <CS Len>     |    1    | <Comp Len>  |    2    | <Ext. Len> |
     * | TLS version |  Random (time + random)  | SID length |  Session ID  | CS Len | Ciphersuites |Comp Len | Compression |Ext. Len | Extensions |
     */

    if (message_length < 38)
    {
        /* Message was not the minimum required size for a ClientHello. */
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    /* Use our length as an index into the buffer. */
    length = 0;

    /* If we are currently in a session, we have a renegotiation handshake. */
    if (tls_session -> nx_secure_tls_local_session_active)
    {
        if (tls_session -> nx_secure_tls_renegotation_enabled)
        {
            tls_session -> nx_secure_tls_renegotiation_handshake = NX_TRUE;

            /* On a session resumption free all certificates for the new session.
             * SESSION RESUMPTION: if session resumption is enabled, don't free!!
             */
            status = _nx_secure_tls_remote_certificate_free_all(tls_session);

            if (status != NX_SUCCESS)
            {
                return(status);
            }

            /* Invoke user callback to notify application of renegotiation request. */
            if (tls_session -> nx_secure_tls_session_renegotiation_callback != NX_NULL)
            {
                status = tls_session -> nx_secure_tls_session_renegotiation_callback(tls_session);

                if (status != NX_SUCCESS)
                {
                    return(status);
                }
            }
        }
        else
        {
            /* Session renegotiation is disabled, so this is an error! */
            return(NX_SECURE_TLS_NO_RENEGOTIATION_ERROR);
        }
    }

    /* Client is establishing a TLS session with our server. */
    /* Extract the protocol version - only part of the ClientHello message. */
    protocol_version = (USHORT)((packet_buffer[length] << 8) | packet_buffer[length + 1]);
    length += 2;

    /* Check protocol version provided by client. */
    status = _nx_secure_tls_check_protocol_version(tls_session, protocol_version, NX_SECURE_TLS);

    if (status != NX_SECURE_TLS_SUCCESS)
    {
        /* If we have an active session, this is a renegotiation attempt, treat the protocol error as
           if we are starting a new session. */
        if (status == NX_SECURE_TLS_UNSUPPORTED_TLS_VERSION || tls_session -> nx_secure_tls_local_session_active)
        {
            /* If the version isn't supported, it's not an issue - TLS is backward-compatible,
             * so pick the highest version we do support. If the version isn't recognized,
             * flag an error. */
            _nx_secure_tls_protocol_version_get(tls_session, &protocol_version, NX_SECURE_TLS);

            if (protocol_version == 0x0)
            {
                /* Error, no versions enabled. */
                return(NX_SECURE_TLS_UNSUPPORTED_TLS_VERSION);
            }
        }
        else
        {
            /* Protocol version unknown (not TLS or SSL!), return status. */
            return(status);
        }
    }

    /* Assign our protocol version to our socket. This is used for all further communications
     * in this session. */
    tls_session -> nx_secure_tls_protocol_version = protocol_version;

    /* Save off the random value for key generation later. */
    NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_random, &packet_buffer[length], NX_SECURE_TLS_RANDOM_SIZE);
    length += NX_SECURE_TLS_RANDOM_SIZE;

    /* Extract the session ID if there is one. */
    session_id_length = packet_buffer[length];
    length++;

    /* If there is a session ID, copy it into our TLS socket structure. */
    tls_session -> nx_secure_tls_session_id_length = session_id_length;
    if (session_id_length > 0)
    {
        NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_session_id, &packet_buffer[length], session_id_length);
        length += session_id_length;
    }

    /* Negotiate the ciphersuite we want to use. */
    ciphersuite_list_length = (USHORT)((packet_buffer[length] << 8) + packet_buffer[length + 1]);
    length += 2;

    /* Make sure the list length makes sense. */
    if ((length + ciphersuite_list_length) > message_length)
    {
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    for (i = 0; i < ciphersuite_list_length; i += 2)
    {
        /* Loop through list of acceptable ciphersuites. */
        cipher_entry = (USHORT)((packet_buffer[length + i] << 8) + packet_buffer[length + i + 1]);

        status = _nx_secure_tls_ciphersuite_lookup(tls_session, cipher_entry, &ciphersuite_info);

        /* Save the first ciphersuite we find - assume cipher table is in priority order. */
        if (status == NX_SUCCESS && tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
        {
            /* Save the ciphersuite but continue processing the entire list. */
            tls_session -> nx_secure_tls_session_ciphersuite = ciphersuite_info;
        }

#ifdef NX_SECURE_TLS_ENABLE_SECURE_RENEGOTIATION
        if (cipher_entry == TLS_EMPTY_RENEGOTIATION_INFO_SCSV)
        {
            /* Secure Renegotiation signalling ciphersuite value was encountered.
               This indicates that the Client supports secure renegotiation. */
            tls_session -> nx_secure_tls_secure_renegotiation = NX_TRUE;
        }
#endif /* NX_SECURE_TLS_ENABLE_SECURE_RENEGOTIATION */
        
        /* Check for the fallback notification SCSV. */
        if(cipher_entry == TLS_FALLBACK_NOTIFY_SCSV)
        {
            /* A fallback is indicated by the Client, check the TLS version. */
            _nx_secure_tls_newest_supported_version(tls_session, &newest_version, NX_SECURE_TLS);
            
            if(protocol_version != newest_version)
            {
                return(NX_SECURE_TLS_INAPPROPRIATE_FALLBACK);
            }
        }
        
    }

    /* See if we found an acceptable ciphersuite. */
    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {

        /* No supported ciphersuites found. */
        return(NX_SECURE_TLS_NO_SUPPORTED_CIPHERS);
    }

    length += ciphersuite_list_length;

    /* Compression methods length - one byte. For now we only support the NULL method. */
    compression_methods_length = packet_buffer[length];
    length++;

    /* Make sure NULL compression method is supported. */
    status = NX_SECURE_TLS_BAD_COMPRESSION_METHOD;
    for (i = 0; i < compression_methods_length; ++i)
    {
        if (packet_buffer[length + i] == 0x0)
        {
            status = NX_SUCCESS;
            break;
        }
    }

    /* Check for error. */
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    length += compression_methods_length;

    /* Padding data? */
    if (message_length >= (length + 2))
    {

        /* TLS Extensions come next. Get the total length of all extensions first. */
        total_extensions_length = (USHORT)((packet_buffer[length] << 8) + packet_buffer[length + 1]);
        length += 2;

        /* Message length overflow. */
        if ((length + total_extensions_length) > message_length)
        {
            return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
        }

        if (total_extensions_length > 0)
        {
            /* Process serverhello extensions. */
            status = _nx_secure_tls_process_clienthello_extensions(tls_session, &packet_buffer[length], total_extensions_length, extension_data, &num_extensions);

            /* Check for error. */
            if (status != NX_SUCCESS)
            {
                return(status);
            }

            /* If the server callback is set, invoke it now with the extensions that require application input. */
            if (tls_session -> nx_secure_tls_session_server_callback != NX_NULL)
            {
                status = tls_session -> nx_secure_tls_session_server_callback(tls_session, extension_data, num_extensions);

                /* Check for error. */
                if (status != NX_SUCCESS)
                {
                    return(status);
                }
            }
        }
    }

#ifdef NX_SECURE_TLS_SERVER_DISABLED
    /* If TLS Server is disabled and we have processed a ClientHello, something is wrong... */
    tls_session -> nx_secure_tls_client_state = NX_SECURE_TLS_CLIENT_STATE_ERROR;
    return(NX_SECURE_TLS_INVALID_STATE);
#else
    return(NX_SUCCESS);
#endif
}


