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

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
/* Defined in nx_secure_tls_send_serverhello.c */
extern const UCHAR _nx_secure_tls_hello_retry_request_random[32];
#endif

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_process_serverhello                  PORTABLE C      */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
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
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
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
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
USHORT                                tls_1_3 = tls_session -> nx_secure_tls_1_3;
NX_SECURE_TLS_SERVER_STATE            old_client_state = tls_session -> nx_secure_tls_client_state;

    tls_session -> nx_secure_tls_client_state = NX_SECURE_TLS_CLIENT_STATE_IDLE;
#endif

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

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    if (tls_session -> nx_secure_tls_1_3 &&
        (NX_SECURE_MEMCMP(_nx_secure_tls_hello_retry_request_random, &packet_buffer[length], NX_SECURE_TLS_RANDOM_SIZE) == 0))
    {

        /* A HelloRetryRequest is received. */
        if (old_client_state == NX_SECURE_TLS_CLIENT_STATE_HELLO_RETRY)
        {

            /* A second HelloRetryRequest is received. */
            return(NX_SECURE_TLS_UNRECOGNIZED_MESSAGE_TYPE);
        }
        else
        {
            tls_session -> nx_secure_tls_client_state = NX_SECURE_TLS_CLIENT_STATE_HELLO_RETRY;
        }
    }
    else
#endif
    {
            
        /* Set the Server random data, used in key generation. First 4 bytes is GMT time. */
        NX_SECURE_MEMCPY(&tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random[0], &packet_buffer[length], NX_SECURE_TLS_RANDOM_SIZE);
    }
    length += NX_SECURE_TLS_RANDOM_SIZE;

    /* Session ID length is one byte. */
    tls_session -> nx_secure_tls_session_id_length = packet_buffer[length];
    length++;

    if ((length + tls_session -> nx_secure_tls_session_id_length) > message_length)
    {
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

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
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
        if (tls_session -> nx_secure_tls_1_3)
        {
            return(NX_SECURE_TLS_1_3_UNKNOWN_CIPHERSUITE);
        }
#endif

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

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
            if (tls_session -> nx_secure_tls_1_3 != tls_1_3)
            {

                /* Server negotiates a version of TLS prior to TLS 1.3. */
                return(status);
            }
#endif

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

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    if ((tls_session -> nx_secure_tls_1_3) && (old_client_state == NX_SECURE_TLS_CLIENT_STATE_IDLE))
    {

        /* We have selected a ciphersuite so now we can initialize the handshake hash. */
        status = _nx_secure_tls_handshake_hash_init(tls_session);

        if(status != NX_SUCCESS)
        {
            return(status);
        }
    }

    if (tls_session -> nx_secure_tls_client_state != NX_SECURE_TLS_CLIENT_STATE_HELLO_RETRY)
#endif
    {
        
        /* Set our state to indicate we sucessfully parsed the ServerHello. */
        tls_session -> nx_secure_tls_client_state = NX_SECURE_TLS_CLIENT_STATE_SERVERHELLO;
    }

    return(NX_SUCCESS);
#endif
}

