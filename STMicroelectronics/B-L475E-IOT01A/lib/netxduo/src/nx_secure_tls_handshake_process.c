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
/*    _nx_secure_tls_handshake_process                    PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes a TLS handshake, whether at the beginning   */
/*    of a new TLS connection or during a session re-negotiation. The     */
/*    handshake state machine is implemented for each of TLS Client and   */
/*    Server in their own functions, this function is simply the entry    */
/*    point for handling the handshake messages.                          */
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
/*    _nx_secure_tls_session_receive_records                              */
/*                                          Receive TLS records           */
/*    nx_packet_release                     Release packet                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_session_start          Start TLS session             */
/*    _nx_secure_tls_session_receive        Receive TCP data              */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko          Initial Version 5.11           */
/*  08-15-2019     Timothy Stapko          Modified comment(s), added     */
/*                                           logic to clear encryption    */
/*                                           key and other secret data,   */
/*                                           resulting in version 5.12    */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_handshake_process(NX_SECURE_TLS_SESSION *tls_session, UINT wait_option)
{
UINT       status = NX_NOT_SUCCESSFUL;
NX_PACKET *incoming_packet;
#ifdef NX_SECURE_KEY_CLEAR
NX_PACKET *current_packet;
#endif /* NX_SECURE_KEY_CLEAR */

    /* Process the handshake depending on the TLS session type. */
#ifndef NX_SECURE_TLS_CLIENT_DISABLED
    if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_CLIENT)
    {

        /* Handle our incoming handshake messages. Continue processing until the handshake is complete
         * or an error/timeout occurs. */
        while (tls_session -> nx_secure_tls_client_state != NX_SECURE_TLS_CLIENT_STATE_HANDSHAKE_FINISHED)
        {
            status = _nx_secure_tls_session_receive_records(tls_session, &incoming_packet, wait_option);

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

            /* Release the incoming packet if we do receive it. */
            nx_packet_release(incoming_packet);
        }
    }
#endif

#ifndef NX_SECURE_TLS_SERVER_DISABLED
    if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_SERVER)
    {
        /* Session is a TLS Server type. */
        /* The client socket connection has already been accepted at this point, process the handshake.  */

        /* Now handle our incoming handshake messages. Continue processing until the handshake is complete
         * or an error/timeout occurs. */
        while (tls_session -> nx_secure_tls_server_state != NX_SECURE_TLS_SERVER_STATE_HANDSHAKE_FINISHED)
        {
            status = _nx_secure_tls_session_receive_records(tls_session, &incoming_packet, wait_option);

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

            /* Release the incoming packet if we do receive it. */
            nx_packet_release(incoming_packet);
        }
    }
#endif

    return(status);
}

