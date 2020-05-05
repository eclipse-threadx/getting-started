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
/*    _nx_secure_tls_send_finished                        PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function generates the Finished message to send to the remote  */
/*    host. The Finished message contains a hash of all handshake         */
/*    messages received up to this point which is used to verify that     */
/*    none of the messages have been corrupted.                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    send_packet                           Packet used to send message   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_finished_hash_generate Generate Finished hash        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_client_handshake      DTLS client state machine     */
/*    _nx_secure_dtls_server_handshake      DTLS server state machine     */
/*    _nx_secure_tls_client_handshake       TLS client state machine      */
/*    _nx_secure_tls_server_handshake       TLS server state machine      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            supported renegotiation,    */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s), and      */
/*                                            added flexibility of using  */
/*                                            macros instead of direct C  */
/*                                            library function calls,     */
/*                                            improved packet length      */
/*                                            verification, cleared       */
/*                                            remote certificate space,   */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_send_finished(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET *send_packet)
{
UCHAR            *finished_label;
UINT              status;

    /* Select our label for generating the finished hash expansion. */
    if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_SERVER)
    {
        finished_label = (UCHAR *)"server finished";
    }
    else
    {
        finished_label = (UCHAR *)"client finished";
    }

    if (NX_SECURE_TLS_FINISHED_HASH_SIZE > ((ULONG)(send_packet -> nx_packet_data_end) - (ULONG)(send_packet -> nx_packet_append_ptr)))
    {
        
        /* Packet buffer too small. */
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    /* Finally, generate the verification data required by TLS - 12 bytes using the PRF and the data
       we have collected. Place the result directly into the packet buffer. */
    _nx_secure_tls_finished_hash_generate(tls_session, finished_label, send_packet -> nx_packet_append_ptr);

#ifdef NX_SECURE_TLS_ENABLE_SECURE_RENEGOTIATION
    /* If we are doing secure renegotiation as per RFC 5746, we need to save off the generated
       verify data now. For TLS 1.0-1.2 this is 12 bytes. If SSLv3 is ever used, it will be 36 bytes. */
    NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_local_verify_data, send_packet -> nx_packet_append_ptr, NX_SECURE_TLS_FINISHED_HASH_SIZE);
#endif

    /* The finished verify data is always 12 bytes. */
    send_packet -> nx_packet_append_ptr = send_packet -> nx_packet_append_ptr + NX_SECURE_TLS_FINISHED_HASH_SIZE;
    send_packet -> nx_packet_length = send_packet -> nx_packet_length + NX_SECURE_TLS_FINISHED_HASH_SIZE;

    /* Finished with the handshake - we can free certificates now. */
    status = _nx_secure_tls_remote_certificate_free_all(tls_session);

    if (status != NX_SUCCESS)
    {
        return(status);
    }

    return(NX_SECURE_TLS_SUCCESS);
}

