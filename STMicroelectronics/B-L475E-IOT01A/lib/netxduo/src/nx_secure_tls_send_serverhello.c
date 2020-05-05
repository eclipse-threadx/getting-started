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
/*    _nx_secure_tls_send_serverhello                     PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function generates a ServerHello message, which is used to     */
/*    respond to an incoming ClientHello message and provide the remote   */
/*    TLS Client with the chosen ciphersuite and key generation data.     */
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
/*    _nx_secure_tls_send_serverhello_extensions                          */
/*                                          Send TLS ServerHello extension*/
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_server_handshake       TLS server state machine      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            supported renegotiation,    */
/*                                            optimized the logic,        */
/*                                            supported extensions,       */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s), and      */
/*                                            optimized internal logic,   */
/*                                            added flexibility of using  */
/*                                            macros instead of direct C  */
/*                                            library function calls,     */
/*                                            fix Session ID issue in     */
/*                                            initial ServerHello, fixed  */
/*                                            compiler warnings, improved */
/*                                            packet length verification, */
/*                                            stored cipher suite pointer,*/
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_send_serverhello(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET *send_packet)
{
ULONG  length;
UINT   gmt_time;
UINT   random_value;
UINT   i;
USHORT ciphersuite;
UCHAR *packet_buffer;
USHORT protocol_version;
UINT   status;


    /* Build up the ServerHello message.
     * Structure:
     * |     2       |          4 + 28          |    1       |   <SID len>  |      2      |      1      |    2     | <Ext. Len> |
     * | TLS version |  Random (time + random)  | SID length |  Session ID  | Ciphersuite | Compression | Ext. Len | Extensions |
     */

    if ((6u + sizeof(tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random)) >
        ((ULONG)(send_packet -> nx_packet_data_end) - (ULONG)(send_packet -> nx_packet_append_ptr)))
        
    {

        /* Packet buffer is too small to hold random. */
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    packet_buffer = send_packet -> nx_packet_append_ptr;

    /* Use our length as an index into the buffer. */
    length = 0;

    /* First two bytes of the hello following the header are the TLS major and minor version numbers. */
    protocol_version = tls_session -> nx_secure_tls_protocol_version;

    packet_buffer[length]     = (UCHAR)((protocol_version & 0xFF00) >> 8);
    packet_buffer[length + 1] = (UCHAR)(protocol_version & 0x00FF);
    length += 2;

    /* Set the Server random data, used in key generation. First 4 bytes is GMT time. */
    gmt_time = 0;
    if (tls_session -> nx_secure_tls_session_time_function != NX_NULL)
    {
        gmt_time = tls_session -> nx_secure_tls_session_time_function();
    }
    NX_CHANGE_ULONG_ENDIAN(gmt_time);

    NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random, (UCHAR *)&gmt_time, sizeof(gmt_time));

    /* Next 28 bytes is random data. */
    for (i = 4; i <= 28; i += (UINT)sizeof(random_value))
    {
        random_value = (UINT)NX_RAND();
        tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random[i]     = (UCHAR)(random_value);
        tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random[i + 1] = (UCHAR)(random_value >> 8);
        tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random[i + 2] = (UCHAR)(random_value >> 16);
        tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random[i + 3] = (UCHAR)(random_value >> 24);
    }

    /* Copy the random data into the packet. */
    NX_SECURE_MEMCPY(&packet_buffer[length], tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random,
                     sizeof(tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random));
    length += sizeof(tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random);

    /* Session ID length is one byte. Session ID data follows if we ever implement session resumption. */
    packet_buffer[length] = 0;
    length++;

    /* Finally, our chosen ciphersuite - this is selected when we receive the Client Hello. */
    ciphersuite = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_ciphersuite;
    packet_buffer[length]     = (UCHAR)(ciphersuite >> 8);
    packet_buffer[length + 1] = (UCHAR)ciphersuite;
    length += 2;

    /* Compression method - for now this is NULL. */
    packet_buffer[length] = 0x0;
    length++;

    /* ============ TLS ServerHello extensions. ============= */
    status = _nx_secure_tls_send_serverhello_extensions(tls_session, packet_buffer, &length,
                                                        ((ULONG)(send_packet -> nx_packet_data_end) -
                                                         (ULONG)packet_buffer));

    if (status)
    {
        return(status);
    }

    /* Finally, we have a complete length and can put it into the buffer. Before that,
       save off and return the number of bytes we wrote and need to send. */
    send_packet -> nx_packet_append_ptr = send_packet -> nx_packet_append_ptr + length;
    send_packet -> nx_packet_length = send_packet -> nx_packet_length + length;

    return(status);
}

