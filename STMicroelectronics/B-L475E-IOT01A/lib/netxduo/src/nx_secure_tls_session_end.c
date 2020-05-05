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
/*    _nx_secure_tls_session_end                          PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function ends an active TLS session by sending the TLS         */
/*    CloseNotify alert to the remote host, then waiting for the response */
/*    CloseNotify before returning.                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    wait_option                           Indicates how long the caller */
/*                                          should wait for the response  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_packet_allocate        Allocate internal TLS packet  */
/*    _nx_secure_tls_send_alert             Generate the CloseNotify      */
/*    _nx_secure_tls_send_record            Send the CloseNotify          */
/*    _nx_secure_tls_session_reset          Clear out the session         */
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
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            fixed compiler warnings,    */
/*                                            improved TLS session        */
/*                                            shutdown, released packet   */
/*                                            on send failed,             */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s),          */
/*                                            reset TLS session state if  */
/*                                            errors are encountered,     */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT  _nx_secure_tls_session_end(NX_SECURE_TLS_SESSION *tls_session, UINT wait_option)
{
UINT       status;
UINT       error_return;
UINT       send_close_notify;
NX_PACKET *send_packet;
NX_PACKET *tmp_ptr;
#ifdef NX_SECURE_KEY_CLEAR
NX_PACKET *current_packet;
#endif /* NX_SECURE_KEY_CLEAR */

    /* Get the protection. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

    /* Release packets in queue. */
    while (tls_session -> nx_secure_record_queue_header)
    {
        tmp_ptr = tls_session -> nx_secure_record_queue_header;
        tls_session -> nx_secure_record_queue_header = tmp_ptr -> nx_packet_queue_next;
        tmp_ptr -> nx_packet_queue_next = NX_NULL;
#ifdef NX_SECURE_KEY_CLEAR
        /* Clear all data in chained packet. */
        current_packet = tmp_ptr;
        while (current_packet)
        {
            NX_SECURE_MEMSET(current_packet -> nx_packet_prepend_ptr, 0,
                   (ULONG)current_packet -> nx_packet_append_ptr -
                   (ULONG)current_packet -> nx_packet_prepend_ptr);
            current_packet = current_packet -> nx_packet_next;
        }
#endif /* NX_SECURE_KEY_CLEAR  */
        nx_packet_release(tmp_ptr);
    }
    if (tls_session -> nx_secure_record_decrypted_packet)
    {
#ifdef NX_SECURE_KEY_CLEAR
        /* Clear all data in chained packet. */
        current_packet = tls_session -> nx_secure_record_decrypted_packet;
        while (current_packet)
        {
            NX_SECURE_MEMSET(current_packet -> nx_packet_prepend_ptr, 0,
                   (ULONG)current_packet -> nx_packet_append_ptr -
                   (ULONG)current_packet -> nx_packet_prepend_ptr);
            current_packet = current_packet -> nx_packet_next;
        }
#endif /* NX_SECURE_KEY_CLEAR  */
        nx_packet_release(tls_session -> nx_secure_record_decrypted_packet);
        tls_session -> nx_secure_record_decrypted_packet = NX_NULL;
    }

    /* See if we want to send a CloseNotify alert, or if there was an error, don't send
       a CloseNotify, just reset the TLS session. */
    send_close_notify = 0;

#ifndef NX_SECURE_TLS_SERVER_DISABLED
    /* Only send a CloseNotify if the handshake was finished. */
    if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_SERVER)
    {
        send_close_notify = tls_session -> nx_secure_tls_server_state == NX_SECURE_TLS_SERVER_STATE_HANDSHAKE_FINISHED;
    }
#endif

#ifndef NX_SECURE_TLS_CLIENT_DISABLED
    if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_CLIENT)
    {
        send_close_notify = tls_session -> nx_secure_tls_client_state == NX_SECURE_TLS_CLIENT_STATE_HANDSHAKE_FINISHED;
    }
#endif

    if (send_close_notify)
    {
        /* Release the protection before suspending on nx_packet_allocate. */
        tx_mutex_put(&_nx_secure_tls_protection);

        /* Allocate a packet for our close-notify alert. */
        status = _nx_secure_tls_packet_allocate(tls_session, tls_session -> nx_secure_tls_packet_pool, &send_packet, wait_option);

        /* Check for errors in allocating packet. */
        if (status != NX_SUCCESS)
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

        /* Get the protection after nx_packet_allocate. */
        tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

        /* A close-notify alert shuts down the TLS session cleanly. */
        _nx_secure_tls_send_alert(tls_session, send_packet, NX_SECURE_TLS_ALERT_CLOSE_NOTIFY, NX_SECURE_TLS_ALERT_LEVEL_WARNING);

        /* Finally, send the alert record to the remote host. */
        status = _nx_secure_tls_send_record(tls_session, send_packet, NX_SECURE_TLS_ALERT, wait_option);

        if (status)
        {
            /* Release the packet on send errors. */
            nx_packet_release(send_packet);

            /* Release the protection. */
            tx_mutex_put(&_nx_secure_tls_protection);

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
    }

    /* Release the protection. */
    tx_mutex_put(&_nx_secure_tls_protection);

    /* Reset the TLS state so this socket can be reused. */
    status = _nx_secure_tls_session_reset(tls_session);

    return(status);
}

