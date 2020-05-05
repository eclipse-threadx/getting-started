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
/*    _nx_secure_tls_session_start                        PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function starts a TLS session given a TCP socket. The TCP      */
/*    connection must be established before calling this function,        */
/*    or the TLS handshake will fail.                                     */
/*                                                                        */
/*    The type of TLS session is derived automatically from the TCP       */
/*    socket, which must have gone through a successful call to           */
/*    either nx_tcp_client_socket_connect or nx_tcp_server_socket_accept. */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    tcp_socket                            TCP socket pointer            */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_allocate_handshake_packet                            */
/*                                          Allocate TLS packet           */
/*    _nx_secure_tls_handshake_process      Process TLS handshake         */
/*    _nx_secure_tls_send_clienthello       Send ClientHello              */
/*    _nx_secure_tls_send_handshake_record  Send TLS handshake record     */
/*    nx_packet_release                     Release packet                */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017      Timothy Stapko          Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            optimized the logic,        */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s), reset    */
/*                                            TLS session state if errors */
/*                                            are encountered, reset      */
/*                                            alert tracking fields,      */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_session_start(NX_SECURE_TLS_SESSION *tls_session, NX_TCP_SOCKET *tcp_socket,
                                  UINT wait_option)
{
UINT       status = NX_NOT_SUCCESSFUL;
UINT       error_return;
#ifndef NX_SECURE_TLS_CLIENT_DISABLED
NX_PACKET *send_packet;
#endif

    /* Get the protection. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

    /* Assign the packet pool from which TLS will allocate internal message packets. */
    tls_session -> nx_secure_tls_packet_pool = tcp_socket -> nx_tcp_socket_ip_ptr -> nx_ip_default_packet_pool;

    /* Assign the TCP socket to the TLS session. */
    tls_session -> nx_secure_tls_tcp_socket = tcp_socket;

    /* Reset the record queue. */
    tls_session -> nx_secure_record_queue_header = NX_NULL;
    tls_session -> nx_secure_record_queue_tail = NX_NULL;
    tls_session -> nx_secure_record_decrypted_packet = NX_NULL;
    tls_session -> nx_secure_record_queue_length = 0;

    /* Make sure we are starting with a fresh session. */
    tls_session -> nx_secure_tls_local_session_active = 0;
    tls_session -> nx_secure_tls_remote_session_active = 0;
    tls_session -> nx_secure_tls_received_remote_credentials = NX_FALSE;

    /* Reset alert tracking. */
    tls_session -> nx_secure_tls_received_alert_level = 0;
    tls_session -> nx_secure_tls_received_alert_value = 0;


    /* See if this is a TCP server started with listen/accept, or a TCP client started with connect. */
    if (tcp_socket -> nx_tcp_socket_client_type)
    {
        /* The TCP socket is a client, so our TLS session is a TLS Client. */
        tls_session -> nx_secure_tls_socket_type = NX_SECURE_TLS_SESSION_TYPE_CLIENT;
    }
    else
    {
        /* This session is now being treated as a server - indicate that fact to the TLS stack. */
        tls_session -> nx_secure_tls_socket_type = NX_SECURE_TLS_SESSION_TYPE_SERVER;
    }

    /* Now process the handshake depending on the TLS session type. */
#ifndef NX_SECURE_TLS_CLIENT_DISABLED
    if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_CLIENT)
    {

        /* Allocate a handshake packet so we can send the ClientHello. */
        status = _nx_secure_tls_allocate_handshake_packet(tls_session, tls_session -> nx_secure_tls_packet_pool, &send_packet, wait_option);

        if (status != NX_SUCCESS)
        {

            /* Release the protection. */
            tx_mutex_put(&_nx_secure_tls_protection);
            return(status);
        }

        /* Populate our packet with clienthello data. */
        status = _nx_secure_tls_send_clienthello(tls_session, send_packet);

        if (status == NX_SUCCESS)
        {

            /* Send the ClientHello to kick things off. */
            status = _nx_secure_tls_send_handshake_record(tls_session, send_packet, NX_SECURE_TLS_CLIENT_HELLO, wait_option);
        }

        /* If anything after the allocate fails, we need to release our packet. */
        if (status != NX_SUCCESS)
        {

            /* Release the protection. */
            tx_mutex_put(&_nx_secure_tls_protection);
            nx_packet_release(send_packet);
            return(status);
        }
    }
#endif

    /* Release the protection. */
    tx_mutex_put(&_nx_secure_tls_protection);

    /* Now handle our incoming handshake messages. Continue processing until the handshake is complete
       or an error/timeout occurs. */
    status = _nx_secure_tls_handshake_process(tls_session, wait_option);

    if(status != NX_SUCCESS)
    {
        /* Save the return status before resetting the TLS session. */
        error_return = status;

        /* Reset the TLS state so this socket can be reused. */
        status = _nx_secure_tls_session_reset(tls_session);

        if(status != NX_SUCCESS)
        {
            return(status);
        }

        return(error_return);
    }

    return(status);
}

