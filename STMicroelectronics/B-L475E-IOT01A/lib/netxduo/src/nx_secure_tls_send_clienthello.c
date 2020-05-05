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

#ifndef NX_SECURE_TLS_CLIENT_DISABLED
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_send_clienthello                     PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function populates an NX_PACKET with a ClientHello message,    */
/*    which kicks off a TLS handshake when sent to a remote TLS server.   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    send_packet                           Outgoing TLS packet           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_handshake_hash_init    Initialize the finished hash  */
/*    _nx_secure_tls_newest_supported_version                             */
/*                                          Get the version of TLS to use */
/*    _nx_secure_tls_send_clienthello_extensions                          */
/*                                          Send TLS ClientHello extension*/
/*    [nx_secure_tls_session_time_function] Get the current time for the  */
/*                                            TLS timestamp               */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_client_handshake       TLS client state machine      */
/*    _nx_secure_tls_session_renegotiate    Renegotiate TLS session       */
/*    _nx_secure_tls_session_start          Start TLS session             */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            supported extensions,       */
/*                                            optimized the logic,        */
/*                                            supported more ciphersuites,*/
/*                                            supported renegotiation,    */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s), and      */
/*                                            optimized internal logic,   */
/*                                            added flexibility of using  */
/*                                            macros instead of direct C  */
/*                                            library function calls,     */
/*                                            improved packet length      */
/*                                            verification, supported TLS */
/*                                            Fallback SCSV, improved     */
/*                                            packet length verification, */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_send_clienthello(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET *send_packet)
{
ULONG                       length;
UINT                        gmt_time;
UINT                        random_value;
UINT                        i;
USHORT                      ciphersuite;
USHORT                      ciphersuites_length;
UCHAR                      *packet_buffer;
USHORT                      protocol_version;
USHORT                      newest_version;
UINT                        status;
UINT                        fallback_enabled = NX_FALSE;
const NX_SECURE_TLS_CRYPTO *crypto_table;

    /* ClientHello structure:
     * |     2       |          4 + 28          |    1       |   <SID len>  |   2    | <CS Len>     |    1    | <Comp Len>  |    2    | <Ext. Len> |
     * | TLS version |  Random (time + random)  | SID length |  Session ID  | CS Len | Ciphersuites |Comp Len | Compression |Ext. Len | Extensions |
     */

    packet_buffer = send_packet -> nx_packet_append_ptr;

    /* Initialize the handshake hashes used for the Finished message. */
    _nx_secure_tls_handshake_hash_init(tls_session);

    /* Use our length as an index into the buffer. */
    length = 0;

    /* First two bytes of the hello following the header are the TLS major and minor version numbers. 
       Get the version we are going to use for this session. */
    _nx_secure_tls_protocol_version_get(tls_session, &protocol_version, NX_SECURE_TLS);

    /* Get the newest supported version to see if we have a version fallback. */
    _nx_secure_tls_newest_supported_version(tls_session, &newest_version, NX_SECURE_TLS);
    if(protocol_version != newest_version)
    {
        /* The version we are using is not the latest, indicate fallback with SCSV. */
        fallback_enabled = NX_TRUE;
    }

    /* Length of the ciphersuites list that follows. The client can send any number of suites for the
     * server to choose from, with a 2-byte length field. */
    crypto_table = tls_session -> nx_secure_tls_crypto_table;

    ciphersuites_length = (USHORT)((crypto_table -> nx_secure_tls_ciphersuite_lookup_table_size << 1));

#ifdef NX_SECURE_TLS_USE_SCSV_CIPHPERSUITE
    /* If this is an initial handshake (and not a response to a HelloRequest for session re-negotiation)
       then include the length of the SCSV pseudo-ciphersuite (if enabled). */
    if (!tls_session -> nx_secure_tls_renegotiation_handshake && tls_session -> nx_secure_tls_client_state != NX_SECURE_TLS_CLIENT_STATE_RENEGOTIATING)
    {
        ciphersuites_length = (USHORT)(ciphersuites_length + 2);
    }
#endif

    /* If we are falling back to an earlier TLS version, include the fallback notification SCSV. */
    if(fallback_enabled)
    {
        ciphersuites_length = (USHORT)(ciphersuites_length + 2);
    }

    if (((ULONG)(send_packet -> nx_packet_data_end) - (ULONG)(send_packet -> nx_packet_append_ptr)) <
        (7u + sizeof(tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_random) +
         tls_session -> nx_secure_tls_session_id_length + ciphersuites_length))
    {

        /* Packet buffer is too small to hold random and ID. */
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    
    packet_buffer[length]     = (UCHAR)((protocol_version & 0xFF00) >> 8);
    packet_buffer[length + 1] = (UCHAR)(protocol_version & 0x00FF);
    tls_session -> nx_secure_tls_protocol_version = protocol_version;
    length += 2;

    /* Set the Client random data, used in key generation. First 4 bytes is GMT time. */
    gmt_time = 0;
    if (tls_session -> nx_secure_tls_session_time_function != NX_NULL)
    {
        gmt_time = tls_session -> nx_secure_tls_session_time_function();
    }
    NX_CHANGE_ULONG_ENDIAN(gmt_time);
    NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_random, (UCHAR *)&gmt_time, sizeof(gmt_time));

    /* Next 28 bytes is random data. */
    for (i = 4; i <= 28; i += (UINT)sizeof(random_value))
    {
        random_value = (UINT)NX_RAND();
        tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_random[i] =     (UCHAR)(random_value);
        tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_random[i + 1] = (UCHAR)(random_value >> 8);
        tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_random[i + 2] = (UCHAR)(random_value >> 16);
        tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_random[i + 3] = (UCHAR)(random_value >> 24);
    }

    /* Copy the random data into the packet. */
    NX_SECURE_MEMCPY(&packet_buffer[length], tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_random,
                     sizeof(tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_random));
    length += sizeof(tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_random);

    /* Session ID length is one byte. */
    tls_session -> nx_secure_tls_session_id_length  = 0;
    packet_buffer[length] = tls_session -> nx_secure_tls_session_id_length;
    length++;

    /* Session ID follows. */
    if (tls_session -> nx_secure_tls_session_id_length > 0)
    {
        NX_SECURE_MEMCPY(&packet_buffer[length], tls_session -> nx_secure_tls_session_id, tls_session -> nx_secure_tls_session_id_length);
        length += tls_session -> nx_secure_tls_session_id_length;
    }

    packet_buffer[length] = (UCHAR)((ciphersuites_length & 0xFF00) >> 8);
    packet_buffer[length + 1] = (UCHAR)(ciphersuites_length & 0x00FF);
    length += 2;

    /* Our chosen ciphersuites. In sending a ClientHello, we need to include all the ciphersuites
     * we are interested in speaking. This will be a list of ciphersuites the user has chosen to support
     * and all of them will be presented to the remote server in this initial message. */

    for (i = 0; i < crypto_table -> nx_secure_tls_ciphersuite_lookup_table_size; i++)
    {
        ciphersuite = crypto_table -> nx_secure_tls_ciphersuite_lookup_table[i].nx_secure_tls_ciphersuite;
        packet_buffer[length] = (UCHAR)((ciphersuite & 0xFF00) >> 8);
        packet_buffer[length + 1] = (UCHAR)(ciphersuite & 0x00FF);
        length += 2;
    }

#ifdef NX_SECURE_TLS_USE_SCSV_CIPHPERSUITE
    /* Only send SCSV on initial ClientHello. The SCSV is used for compatibility with OLD TLS 1.0
       and SSLv3.0 servers and generally shouldn't be needed. */
    if (!tls_session -> nx_secure_tls_renegotiation_handshake && tls_session -> nx_secure_tls_client_state != NX_SECURE_TLS_CLIENT_STATE_RENEGOTIATING)
    {
        /* For the secure rengotiation indication extension, we need to send a
           Signalling Ciphersuite Value (SCSV) as detailled in RFC5746. */
        ciphersuite = TLS_EMPTY_RENEGOTIATION_INFO_SCSV;
        packet_buffer[length] = (UCHAR)((ciphersuite & 0xFF00) >> 8);
        packet_buffer[length + 1] = (UCHAR)(ciphersuite & 0x00FF);
        length += 2;
    }
#endif

    /* If we are falling back to an earlier TLS version, include the fallback notification SCSV. */
    if(fallback_enabled)
    {
        ciphersuite = TLS_FALLBACK_NOTIFY_SCSV;
        packet_buffer[length] = (UCHAR)((ciphersuite & 0xFF00) >> 8);
        packet_buffer[length + 1] = (UCHAR)(ciphersuite & 0x00FF);
        length += 2;
    }
    
    /* Compression methods length - one byte. For now we only have the NULL method, for a length of 1. */
    packet_buffer[length] = 0x1;
    length++;

    /* Compression methods - for now this is NULL. */
    packet_buffer[length] = 0x0;
    length++;

    /* ============ TLS ClientHello extensions. ============= */
    status = _nx_secure_tls_send_clienthello_extensions(tls_session, packet_buffer, &length,
                                                        ((ULONG)send_packet -> nx_packet_data_end -
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
#endif /* NX_SECURE_TLS_CLIENT_DISABLED */
