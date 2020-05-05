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

#ifdef NX_SECURE_TLS_ENABLE_SECURE_RENEGOTIATION
static UINT _nx_secure_tls_send_serverhello_sec_reneg_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                                UCHAR *packet_buffer,
                                                                ULONG *packet_offset,
                                                                USHORT *extension_length,
                                                                ULONG available_size);
#endif


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_send_serverhello_extensions          PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function generates a ServerHello extensions.                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    packet_buffer                         Outgoing TLS packet buffer    */
/*    packet_offset                         Offset into packet buffer     */
/*    available_size                        Available size of buffer      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_send_serverhello_sec_reneg_extension                 */
/*                                          Send ServerHello Renegotiation*/
/*                                            extension                   */
/*    _nx_secure_tls_send_serverhello_ec_extension                        */
/*                                          Send ClientHello EC extension */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_send_serverhello       Send TLS ServerHello          */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-15-2019     Timothy Stapko           Initial Version 5.12          */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_send_serverhello_extensions(NX_SECURE_TLS_SESSION *tls_session,
                                                UCHAR *packet_buffer, ULONG *packet_offset,
                                                ULONG available_size)
{
ULONG  length = *packet_offset;
UCHAR *extension_offset;
#ifdef NX_SECURE_TLS_ENABLE_SECURE_RENEGOTIATION
USHORT extension_length = 0;
#endif /* NX_SECURE_TLS_ENABLE_SECURE_RENEGOTIATION */
USHORT total_extensions_length;

    /* Save an offset to the beginning of the extensions so we can fill in the length
       once all the extensions are added. */
    extension_offset = &packet_buffer[length];

    /* The extensions length field is two bytes. */
    length += 2;
    total_extensions_length = 0;

    NX_SECURE_SEND_SERVERHELLO_EXTENSIONS_EXTENSION

#ifdef NX_SECURE_TLS_ENABLE_SECURE_RENEGOTIATION
    /* The renegotiation extension is not supported in TLS 1.0 and TLS 1.1. */
    if(tls_session -> nx_secure_tls_protocol_version != NX_SECURE_TLS_VERSION_TLS_1_0 &&
       tls_session -> nx_secure_tls_protocol_version != NX_SECURE_TLS_VERSION_TLS_1_1)
    {
        /* We have to add renegotiation extensions in both initial sessions and renegotiating sessions. */
        _nx_secure_tls_send_serverhello_sec_reneg_extension(tls_session, packet_buffer, &length,
                                                            &extension_length, available_size);
        total_extensions_length = (USHORT)(total_extensions_length + extension_length);
    }
#endif /* NX_SECURE_TLS_ENABLE_SECURE_RENEGOTIATION */

    /* Make sure we actually wrote some extension data. If none, back up the packet pointer and length. */
    if (total_extensions_length == 0)
    {
        /* No extensions written, back up our pointer and length. */
        length -= 2;
        packet_buffer = extension_offset;
    }
    else
    {
        /* Put the extensions length into the packet at our original offset and add
           the total to our packet length. */
        extension_offset[0] = (UCHAR)((total_extensions_length & 0xFF00) >> 8);
        extension_offset[1] = (UCHAR)(total_extensions_length & 0x00FF);
    }

    *packet_offset = length;

    return(NX_SECURE_TLS_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_send_serverhello_sec_reneg_extension PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function adds the Secure Renegotiation Indication extension    */
/*    to an outgoing ServerHello record if the ServerHello is part of a   */
/*    renegotiation handshake. See RFC 5746 for more information.         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    packet_buffer                         Outgoing TLS packet buffer    */
/*    packet_offset                         Offset into packet buffer     */
/*    extension_length                      Return length of data         */
/*    available_size                        Available size of buffer      */
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
/*    _nx_secure_tls_send_serverhello_extensions                          */
/*                                          Send TLS ServerHello extension*/
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-15-2019     Timothy Stapko           Initial Version 5.12          */
/*                                                                        */
/**************************************************************************/
#ifdef NX_SECURE_TLS_ENABLE_SECURE_RENEGOTIATION
static UINT _nx_secure_tls_send_serverhello_sec_reneg_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                                UCHAR *packet_buffer,
                                                                ULONG *packet_offset,
                                                                USHORT *extension_length,
                                                                ULONG available_size)
{
ULONG  offset;
USHORT ext;
UINT   data_length;

    /* Secure Renegotiation Indication Extensions structure (for serverhello):
     * |     2      |           12        |         12           |
     * |  Ext Type  |  client_verify_data |  server_verify_data  |
     */
    /*  From RFC 5746:
        struct {
             opaque renegotiated_connection<0..255>;
         } RenegotiationInfo;

          The contents of this extension are specified as follows.

      -  If this is the initial handshake for a connection, then the
         "renegotiated_connection" field is of zero length in both the
         ClientHello and the ServerHello.  Thus, the entire encoding of the
         extension is ff 01 00 01 00.  The first two octets represent the
         extension type, the third and fourth octets the length of the
         extension itself, and the final octet the zero length byte for the
         "renegotiated_connection" field.

      -  For ClientHellos that are renegotiating, this field contains the
         "client_verify_data" specified in Section 3.1.

      -  For ServerHellos that are renegotiating, this field contains the
         concatenation of client_verify_data and server_verify_data.  For
         current versions of TLS, this will be a 24-byte value (for SSLv3,
         it will be a 72-byte value).
     */

    if (((!tls_session -> nx_secure_tls_local_session_active) &&
         (available_size < (*packet_offset + 5))) ||
        ((tls_session -> nx_secure_tls_local_session_active) &&
         (available_size < (*packet_offset + 5 + (NX_SECURE_TLS_FINISHED_HASH_SIZE << 1)))))
    {

        /* Packet buffer too small. */
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    /* Start with our passed-in packet offset. */
    offset = *packet_offset;

    /* The extension identifier. */
    ext = NX_SECURE_TLS_EXTENSION_SECURE_RENEGOTIATION;

    /* Put the extension ID into the packet. */
    packet_buffer[offset] = (UCHAR)((ext & 0xFF00) >> 8);
    packet_buffer[offset + 1] = (UCHAR)(ext & 0x00FF);
    offset += 2;

    /* See if this is the initial handshake or not. */
    if (!tls_session -> nx_secure_tls_local_session_active)
    {
        /* The extension has zero data because this is an initial handshake. Send
           the encoded extension as documented in the RFC. */

        /* Fill in the length of current extension. */
        packet_buffer[offset] = 0x00;
        packet_buffer[offset + 1] = 0x01;

        /* Fill in the length of renegotiated connection field. */
        packet_buffer[offset + 2] = 0x00;

        offset += 3;
    }
    else
    {
        /* Fill in the length of current extension. */
        data_length = (NX_SECURE_TLS_FINISHED_HASH_SIZE * 2) + 1;
        packet_buffer[offset] = (UCHAR)((data_length & 0xFF00) >> 8);
        packet_buffer[offset + 1] = (UCHAR)(data_length & 0x00FF);
        offset += 2;

        /* The extension actually has a second length field of 1 byte that needs to be populated. */
        /* Fill in the length of renegotiated connection field. */
        packet_buffer[offset] = (UCHAR)((NX_SECURE_TLS_FINISHED_HASH_SIZE * 2) & 0x00FF);
        offset++;

        /* Copy the verify data into the packet. */
        NX_SECURE_MEMCPY(&packet_buffer[offset], tls_session -> nx_secure_tls_remote_verify_data, NX_SECURE_TLS_FINISHED_HASH_SIZE);
        offset += NX_SECURE_TLS_FINISHED_HASH_SIZE;
        NX_SECURE_MEMCPY(&packet_buffer[offset], tls_session -> nx_secure_tls_local_verify_data, NX_SECURE_TLS_FINISHED_HASH_SIZE);
        offset += NX_SECURE_TLS_FINISHED_HASH_SIZE;
    }

    /* Return the amount of data we wrote. */
    *extension_length = (USHORT)(offset - *packet_offset);

    /* Return our updated packet offset. */
    *packet_offset = offset;


    return(NX_SUCCESS);
}
#endif
