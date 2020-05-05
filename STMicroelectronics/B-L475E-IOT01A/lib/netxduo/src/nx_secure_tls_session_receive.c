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
/*    _nx_secure_tls_session_receive                      PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function receives data from an active TLS session, handling    */
/*    all decryption and verification before returning the data to the    */
/*    caller in the supplied NX_PACKET structure.                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    packet_ptr_ptr                        Pointer to return packet      */
/*    wait_option                           Indicates how long the caller */
/*                                          should wait for the response  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_handshake_process      Process TLS handshake         */
/*    _nx_secure_tls_session_receive_records                              */
/*                                          Receive TLS records           */
/*    nx_packet_release                     Release packet                */
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
/*                                            supported renegotiation,    */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s), added    */
/*                                            logic to clear encryption   */
/*                                            key and other secret data,  */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT  _nx_secure_tls_session_receive(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET **packet_ptr_ptr,
                                     ULONG wait_option)
{
UINT status;
UINT local_initiated_renegotiation = NX_FALSE;
#ifdef NX_SECURE_KEY_CLEAR
NX_PACKET *current_packet;
#endif /* NX_SECURE_KEY_CLEAR */

    /* Session receive logic:
     * 1. Receive incoming packets
     * 2. Process records and receive while full record is not yet received.
     * 3. If renegotiation inititated, process the renegotiation handshake.
     *    3a. Process entire handshake (receive TCP packets, process records)
     *    3b. Once handshake processed, receive any new packets, but only if
     *        the remote host initiated the renegotiation.
     */


#ifndef NX_SECURE_TLS_CLIENT_DISABLED
    if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_CLIENT &&
        tls_session -> nx_secure_tls_client_state == NX_SECURE_TLS_CLIENT_STATE_RENEGOTIATING)
    {
        local_initiated_renegotiation = NX_TRUE;
    }
#endif

#ifndef NX_SECURE_TLS_SERVER_DISABLED
    if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_SERVER &&
        tls_session -> nx_secure_tls_server_state == NX_SECURE_TLS_SERVER_STATE_HELLO_REQUEST)
    {
        local_initiated_renegotiation = NX_TRUE;
    }
#endif

    /* Try receiving records from the remote host. */
    status = _nx_secure_tls_session_receive_records(tls_session, packet_ptr_ptr, wait_option);

    /* See if we have a renegotiation handshake. Continue processing following the
       hello message that was received. */
    if (status == NX_SUCCESS && tls_session -> nx_secure_tls_renegotiation_handshake)
    {

#ifdef NX_SECURE_KEY_CLEAR
        /* Clear all data in chained packet. */
        current_packet = *packet_ptr_ptr;
        while (current_packet)
        {
            NX_SECURE_MEMSET(current_packet -> nx_packet_prepend_ptr, 0,
                   (ULONG)current_packet -> nx_packet_append_ptr -
                   (ULONG)current_packet -> nx_packet_prepend_ptr);
            current_packet = current_packet -> nx_packet_next;
        }
#endif /* NX_SECURE_KEY_CLEAR  */

        /* Release the incoming packet if we do receive it. */
        nx_packet_release(*packet_ptr_ptr);

        /* Clear flag to prevent infinite recursion. */
        tls_session -> nx_secure_tls_renegotiation_handshake = NX_FALSE;

        /* Process the handshake. */
        status = _nx_secure_tls_handshake_process(tls_session, wait_option);

        if (status != NX_SUCCESS)
        {
            return(status);
        }

        /* If this renegotiation was initiated by us, don't receive additional data as
           that will be up to the application. */
        if (!local_initiated_renegotiation)
        {
            /* Handle any data that followed the re-negotiation handshake. */
            status = _nx_secure_tls_session_receive_records(tls_session, packet_ptr_ptr, wait_option);
        }
    }


    return(status);
}

