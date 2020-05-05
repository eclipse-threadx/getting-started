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
/*    _nx_secure_tls_process_serverhello                  PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an incoming ServerHello message, which is   */
/*    the response to a TLS ClientHello coming from this host. The        */
/*    ServerHello message contains the desired ciphersuite and data used  */
/*    in the key generation process later in the handshake.               */
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
/*    _nx_secure_tls_check_protocol_version Checking incoming TLS version */
/*    _nx_secure_tls_ciphersuite_lookup     Lookup current ciphersuite    */
/*    _nx_secure_tls_process_serverhello_extensions                       */
/*                                          Process ServerHello extensions*/
/*    [nx_secure_tls_session_client_callback                              */
/*                                          Client session callback       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_client_handshake      DTLS client state machine     */
/*    _nx_secure_tls_client_handshake       TLS client state machine      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            supported multiple          */
/*                                            extensions, added session   */
/*                                            callbacks,                  */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s), and      */
/*                                            added flexibility of using  */
/*                                            macros instead of direct C  */
/*                                            library function calls,     */
/*                                            improved packet length      */
/*                                            verification, stored        */
/*                                            cipher suite pointer,       */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_process_serverhello(NX_SECURE_TLS_SESSION *tls_session, UCHAR *packet_buffer,
                                        UINT message_length)
{
UINT                                  length;
UCHAR                                 compression_method;
USHORT                                version, total_extensions_length;
UINT                                  status;
USHORT                                ciphersuite;
NX_SECURE_TLS_HELLO_EXTENSION         extension_data[NX_SECURE_TLS_HELLO_EXTENSIONS_MAX];
UINT                                  num_extensions;

    /* Parse the ServerHello message.
     * Structure:
     * |     2       |          4 + 28          |    1       |   <SID len>  |      2      |      1      |    2     | <Ext. Len> |
     * | TLS version |  Random (time + random)  | SID length |  Session ID  | Ciphersuite | Compression | Ext. Len | Extensions |
     */

    if (message_length < 38)
    {
        /* Message was not the minimum required size for a ServerHello. */
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    /* Use our length as an index into the buffer. */
    length = 0;

    /* First two bytes of the server hello following the header are the TLS major and minor version numbers. */
    version = (USHORT)((packet_buffer[length] << 8) + packet_buffer[length + 1]);
    length += 2;
    /* Verify the version coming from the server. */
    status = _nx_secure_tls_check_protocol_version(tls_session, version, NX_SECURE_TLS);
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Set the protocol version to whatever the Server negotiated - we have checked that
       we support this version in the call above, so it's fine to continue. */
    tls_session -> nx_secure_tls_protocol_version = version;

    /* Set the Server random data, used in key generation. First 4 bytes is GMT time. */
    NX_SECURE_MEMCPY(&tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random[0], &packet_buffer[length], NX_SECURE_TLS_RANDOM_SIZE);
    length += NX_SECURE_TLS_RANDOM_SIZE;

    /* Session ID length is one byte. */
    tls_session -> nx_secure_tls_session_id_length = packet_buffer[length];
    length++;

    /* Session ID follows. */
    if (tls_session -> nx_secure_tls_session_id_length > 0)
    {
        NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_session_id, &packet_buffer[length], tls_session -> nx_secure_tls_session_id_length);
        length += tls_session -> nx_secure_tls_session_id_length;
    }

    /* Finally, the chosen ciphersuite - this is selected by the server from the list we provided in the ClientHello. */
    ciphersuite = (USHORT)((packet_buffer[length] << 8) + packet_buffer[length + 1]);
    length += 2;

    /* Find out the ciphersuite info of the chosen ciphersuite. */
    status = _nx_secure_tls_ciphersuite_lookup(tls_session, ciphersuite, &tls_session -> nx_secure_tls_session_ciphersuite);
    if (status != NX_SUCCESS)
    {
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    /* Compression method - for now this should be NULL. */
    compression_method = packet_buffer[length];

    /* There are no supported compression methods, so non-zero is an error. */
    if (compression_method != 0x00)
    {
        return(NX_SECURE_TLS_BAD_COMPRESSION_METHOD);
    }
    length++;

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
            status = _nx_secure_tls_process_serverhello_extensions(tls_session, &packet_buffer[length], total_extensions_length, extension_data, &num_extensions);

            /* Check for error. */
            if (status != NX_SUCCESS)
            {
                return(status);
            }

            /* If the server callback is set, invoke it now with the extensions that require application input. */
            if (tls_session -> nx_secure_tls_session_client_callback != NX_NULL)
            {
                status = tls_session -> nx_secure_tls_session_client_callback(tls_session, extension_data, num_extensions);

                /* Check for error. */
                if (status != NX_SUCCESS)
                {
                    return(status);
                }
            }
        }
    }

#ifdef NX_SECURE_TLS_CLIENT_DISABLED
    /* If TLS Client is disabled and we have processed a ServerHello, something is wrong... */
    tls_session -> nx_secure_tls_server_state = NX_SECURE_TLS_SERVER_STATE_ERROR;

    return(NX_SECURE_TLS_INVALID_STATE);
#else
    /* Set our state to indicate we sucessfully parsed the ServerHello. */
    tls_session -> nx_secure_tls_client_state = NX_SECURE_TLS_CLIENT_STATE_SERVERHELLO;

    return(NX_SUCCESS);
#endif
}

