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
/*    _nx_secure_tls_send_changecipherspec                PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function populates an NX_PACKET with the TLS ChangeCipherSpec  */
/*    message, which indicates that all further TLS records will be       */
/*    encrypted ussing the generated session keys.                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    send_packet                           Packet to be filled           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
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
/*                                            optimized the logic,        */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_send_changecipherspec(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET *send_packet)
{

    NX_PARAMETER_NOT_USED(tls_session);

    if (((ULONG)(send_packet -> nx_packet_data_end) - (ULONG)(send_packet -> nx_packet_append_ptr)) < 1u)
    {

        /* Packet buffer is too small to hold random and ID. */
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    send_packet -> nx_packet_append_ptr[0] = 0x1;
    send_packet -> nx_packet_append_ptr = send_packet -> nx_packet_append_ptr + 1;
    send_packet -> nx_packet_length = 1;

    return(NX_SUCCESS);
}

