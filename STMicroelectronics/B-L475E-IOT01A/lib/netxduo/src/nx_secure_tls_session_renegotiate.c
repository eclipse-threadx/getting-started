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

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_session_renegotiate                  PORTABLE C      */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function re-negotiates a previously established connection     */
/*    with a remote host. This functionality allows a TLS host (client or */
/*    server) to generate new session keys in response to an application  */
/*    need, usually due to a connection being open for a long time or in  */
/*    response to a potential security issue.                             */
/*                                                                        */
/*    If the session is still active (no CloseNotify messages have been   */
/*    sent) then a re-negotiation is done using the Secure Renegotiation  */
/*    Indication Extension (RFC 5746), if enabled. If the session has     */
/*    been closed, a new session is established using the existing TCP    */
/*    socket assigned in the call to nx_secure_tls_session start. If a    */
/*    new session is being established, session resumption will be used   */
/*    if available.                                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
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
/*    _nx_secure_tls_remote_certificate_free_all                          */
/*                                          Free all remote certificates  */
/*    _nx_secure_tls_send_clienthello       Send ClientHello              */
/*    _nx_secure_tls_send_handshake_record  Send TLS handshake record     */
/*    _nx_secure_tls_send_hellorequest      Send HelloRequest             */
/*    _nx_secure_tls_session_receive_records                              */
/*                                          Receive TLS records           */
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
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_session_renegotiate(NX_SECURE_TLS_SESSION *tls_session, UINT wait_option)
{
UINT       status = NX_NOT_SUCCESSFUL;
NX_PACKET *incoming_packet;
NX_PACKET *send_packet;
#ifdef NX_SECURE_KEY_CLEAR
NX_PACKET *current_packet;
#endif /* NX_SECURE_KEY_CLEAR */

    /* Get the protection. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

    /* Reset the record queue. */
    tls_session -> nx_secure_record_queue_header = NX_NULL;
    tls_session -> nx_secure_record_queue_tail = NX_NULL;
    tls_session -> nx_secure_record_decrypted_packet = NX_NULL;
    tls_session -> nx_secure_record_queue_length = 0;

    /* If the session isn't active, trying to renegotiate is an error! */
    if (tls_session -> nx_secure_tls_remote_session_active != NX_TRUE || tls_session -> nx_secure_tls_local_session_active != NX_TRUE)
    {
        /* Release the protection. */
        tx_mutex_put(&_nx_secure_tls_protection);
        return(NX_SECURE_TLS_RENEGOTIATION_SESSION_INACTIVE);
    }

    /* Make sure the remote host supports renegotiation. */
    if(!tls_session -> nx_secure_tls_secure_renegotiation)
    {
        tx_mutex_put(&_nx_secure_tls_protection);
        return(NX_SECURE_TLS_RENEGOTIATION_FAILURE);
    }

    /* Re-establish the TLS connection based on the session type. */
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

        /* This is a renegotiation handshake so indicate that to the stack. */
        tls_session -> nx_secure_tls_client_state = NX_SECURE_TLS_CLIENT_STATE_RENEGOTIATING;

        /* On a session resumption free all certificates for the new session.
         * SESSION RESUMPTION: if session resumption is enabled, don't free!!
         */
        status = _nx_secure_tls_remote_certificate_free_all(tls_session);

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

        /* Now handle our incoming handshake messages. Continue processing until the handshake is complete
         * or an error/timeout occurs. */
        while (tls_session -> nx_secure_tls_client_state != NX_SECURE_TLS_CLIENT_STATE_HANDSHAKE_FINISHED)
        {
            /* Release the protection. */
            tx_mutex_put(&_nx_secure_tls_protection);

            status = _nx_secure_tls_session_receive_records(tls_session, &incoming_packet, wait_option);

            /* Get the protection. */
            tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

            /* Make sure we didn't have an error during the receive. */
            if (status != NX_SUCCESS)
            {
                break;
            }

#ifdef NX_SECURE_KEY_CLEAR
            /* Clear all data in chained packet. */
            current_packet = incoming_packet;
            while (current_packet)
            {
                NX_SECURE_MEMSET(current_packet -> nx_packet_prepend_ptr, 0,
                       (ULONG)current_packet -> nx_packet_append_ptr -
                       (ULONG)current_packet -> nx_packet_prepend_ptr);
                current_packet = current_packet -> nx_packet_next;
            }
#endif /* NX_SECURE_KEY_CLEAR  */

            /* On any error, the handshake has failed so break out of our processing loop and return. */
            nx_packet_release(incoming_packet);
        }
    }
#endif

#ifndef NX_SECURE_TLS_SERVER_DISABLED
    if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_SERVER)
    {
        /* Session is a TLS Server type. */

        /* The session is active, so send a HelloRequest to re-establish the connection. */
        /* Allocate a handshake packet so we can send the HelloRequest message. */
        status = _nx_secure_tls_allocate_handshake_packet(tls_session, tls_session -> nx_secure_tls_packet_pool, &send_packet, wait_option);

        if (status != NX_SUCCESS)
        {

            /* Release the protection. */
            tx_mutex_put(&_nx_secure_tls_protection);
            return(status);
        }

        /* Populate our packet with HelloRequest data. */
        status = _nx_secure_tls_send_hellorequest(tls_session, send_packet);

        if (status == NX_SUCCESS)
        {

            /* Send the HelloRequest to kick things off. */
            status = _nx_secure_tls_send_handshake_record(tls_session, send_packet, NX_SECURE_TLS_HELLO_REQUEST, wait_option);
        }

        /* If anything after the allocate fails, we need to release our packet. */
        if (status != NX_SUCCESS)
        {

            /* Release the protection. */
            tx_mutex_put(&_nx_secure_tls_protection);
            nx_packet_release(send_packet);
            return(status);
        }

        /* The client socket connection has already been accepted at this point, process the handshake.  */

        /* Now handle our incoming handshake messages. Continue processing until the handshake is complete
         * or an error/timeout occurs. */
        while (tls_session -> nx_secure_tls_server_state != NX_SECURE_TLS_SERVER_STATE_HANDSHAKE_FINISHED)
        {
            /* Release the protection. */
            tx_mutex_put(&_nx_secure_tls_protection);

            status = _nx_secure_tls_session_receive_records(tls_session, &incoming_packet, wait_option);

            /* Get the protection. */
            tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

            /* Make sure we didn't have an error during the receive. */
            if (status != NX_SUCCESS)
            {
                break;
            }

#ifdef NX_SECURE_KEY_CLEAR
            /* Clear all data in chained packet. */
            current_packet = incoming_packet;
            while (current_packet)
            {
                NX_SECURE_MEMSET(current_packet -> nx_packet_prepend_ptr, 0,
                       (ULONG)current_packet -> nx_packet_append_ptr -
                       (ULONG)current_packet -> nx_packet_prepend_ptr);
                current_packet = current_packet -> nx_packet_next;
            }
#endif /* NX_SECURE_KEY_CLEAR  */

            /* On any error, the handshake has failed so break out of our processing loop and return. */
            nx_packet_release(incoming_packet);
        }
    }
#endif

    /* Release the protection. */
    tx_mutex_put(&_nx_secure_tls_protection);

    return(status);
}

