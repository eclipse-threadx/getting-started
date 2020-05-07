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
static UINT _nx_secure_tls_send_clienthello_sig_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                          UCHAR *packet_buffer, ULONG *packet_offset,
                                                          USHORT *extension_length,
                                                          ULONG available_size);
static UINT _nx_secure_tls_send_clienthello_sni_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                          UCHAR *packet_buffer, ULONG *packet_offset,
                                                          USHORT *extension_length,
                                                          ULONG available_size);

#ifdef NX_SECURE_TLS_ENABLE_SECURE_RENEGOTIATION
static UINT _nx_secure_tls_send_clienthello_sec_reneg_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                                UCHAR *packet_buffer,
                                                                ULONG *packet_offset,
                                                                USHORT *extension_length,
                                                                ULONG available_size);
#endif


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
/*    _nx_secure_tls_send_clienthello_sec_reneg_extension                 */
/*                                          Send ClientHello Renegotiation*/
/*                                            extension                   */
/*    _nx_secure_tls_send_clienthello_sig_extension                       */
/*                                          Send ClientHello Signature    */
/*                                            extension                   */
/*    _nx_secure_tls_send_clienthello_sni_extension                       */
/*                                          Send ClientHello SNI extension*/
/*    _nx_secure_tls_send_clienthello_ec_extension                        */
/*                                          Send ClientHello EC extension */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_send_clienthello       Send TLS ClientHello          */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-15-2019     Timothy Stapko           Initial Version 5.12          */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_send_clienthello_extensions(NX_SECURE_TLS_SESSION *tls_session,
                                                UCHAR *packet_buffer, ULONG *packet_offset,
                                                ULONG available_size)
{
ULONG  length = *packet_offset;
UCHAR *extension_offset;
USHORT extension_length = 0, total_extensions_length;

    if (available_size < (*packet_offset + 2))
    {

        /* Packet buffer too small. */
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    /* Save an offset to the beginning of the extensions so we can fill in the length
       once all the extensions are added. */
    extension_offset = &packet_buffer[length];

    /* The extensions length field is two bytes. */
    length += 2;
    total_extensions_length = 0;

    NX_SECURE_SEND_CLIENTHELLO_EXTENSIONS_EXTENSION

#ifdef NX_SECURE_TLS_ENABLE_SECURE_RENEGOTIATION
    /* We have to add renegotiation extensions in both initial sessions and renegotiating sessions. */
    if (tls_session -> nx_secure_tls_renegotation_enabled == NX_TRUE)
    {
        _nx_secure_tls_send_clienthello_sec_reneg_extension(tls_session, packet_buffer, &length, &extension_length, available_size);
        total_extensions_length = (USHORT)(total_extensions_length + extension_length);
    }
#endif

    /* Send the available signature algorithms extension. */
    _nx_secure_tls_send_clienthello_sig_extension(tls_session, packet_buffer, &length, &extension_length, available_size);
    total_extensions_length = (USHORT)(total_extensions_length + extension_length);

#ifndef NX_SECURE_TLS_SNI_EXTENSION_DISABLED
    /* Send the server name indication extension. */
    _nx_secure_tls_send_clienthello_sni_extension(tls_session, packet_buffer, &length, &extension_length, available_size);
    total_extensions_length = (USHORT)(total_extensions_length + extension_length);
#endif

    /* Put the extensions length into the packet at our original offset and add
       the total to our packet length. */
    extension_offset[0] = (UCHAR)((total_extensions_length & 0xFF00) >> 8);
    extension_offset[1] = (UCHAR)(total_extensions_length & 0x00FF);

    *packet_offset = length;

    return(NX_SUCCESS);
}



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_send_clienthello_sig_extension       PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function adds the Signature Algorithms extension to an         */
/*    outgoing ClientHello record. See RFC 5246 section 7.4.1.4.1.        */
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
/*    _nx_secure_tls_send_clienthello_extensions                          */
/*                                          Send TLS ClientHello extension*/
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*  08-15-2019     Timothy Stapko           Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
static UINT _nx_secure_tls_send_clienthello_sig_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                          UCHAR *packet_buffer, ULONG *packet_offset,
                                                          USHORT *extension_length,
                                                          ULONG available_size)
{
ULONG  offset;
USHORT ext_len, sig_len, sighash_len, ext, algo[2];

    /* Signature Extensions structure:
     * |     2     |     2   |       2      | <SigHash Len> |
     * |  Ext Type | Sig Len |  SigHash Len | SigHash Algos |
     *
     * Each algorithm pair has a hash ID and a public key operation ID represented
     * by a single octet. Therefore each entry in the list is 2 bytes long.
     */

#define TLS_SIG_EXTENSION_SHA1_RSA   0x0201
#define TLS_SIG_EXTENSION_SHA256_RSA 0x0401

    /* Once we add more signature methods, use the TLS session to determine which methods are available. */
    NX_PARAMETER_NOT_USED(tls_session);

    if (available_size < (*packet_offset + 10))
    {

        /* Packet buffer too small. */
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    /* Start with our passed-in packet offset. */
    offset = *packet_offset;

    ext_len = sighash_len = 0;

    ext = NX_SECURE_TLS_EXTENSION_SIGNATURE_ALGORITHMS;  /* Signature algorithms */
    ext_len = (USHORT)(ext_len + 2);

    sig_len = 2;
    ext_len = (USHORT)(ext_len + 2);

    algo[0] = TLS_SIG_EXTENSION_SHA1_RSA;
    sighash_len = (USHORT)(sighash_len + 2);

    algo[1] = TLS_SIG_EXTENSION_SHA256_RSA;
    sighash_len = (USHORT)(sighash_len + 2);

    sig_len = (USHORT)(sig_len + sighash_len);
    ext_len = (USHORT)(ext_len + sig_len);

    packet_buffer[offset] = (UCHAR)((ext & 0xFF00) >> 8);
    packet_buffer[offset + 1] = (UCHAR)(ext & 0x00FF);
    offset += 2;

    packet_buffer[offset] = (UCHAR)((sig_len & 0xFF00) >> 8);
    packet_buffer[offset + 1] = (UCHAR)(sig_len & 0x00FF);
    offset += 2;

    packet_buffer[offset] = (UCHAR)((sighash_len & 0xFF00) >> 8);
    packet_buffer[offset + 1] = (UCHAR)(sighash_len & 0x00FF);
    offset += 2;

    packet_buffer[offset] = (UCHAR)((algo[0] & 0xFF00) >> 8);
    packet_buffer[offset + 1] = (UCHAR)(algo[0] & 0x00FF);
    offset += 2;

    packet_buffer[offset] = (UCHAR)((algo[1] & 0xFF00) >> 8);
    packet_buffer[offset + 1] = (UCHAR)(algo[1] & 0x00FF);
    offset += 2;

    /* Return our updated packet offset. */
    *extension_length = ext_len;
    *packet_offset = offset;

    return(NX_SUCCESS);
}



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_send_clienthello_sni_extension       PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function adds the Server Name Indication extension to an       */
/*    outgoing ClientHello record if a server name has been set by the    */
/*    application.                                                        */
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
/*    _nx_secure_tls_send_clienthello_extensions                          */
/*                                          Send TLS ClientHello extension*/
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*  08-15-2019     Timothy Stapko           Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
#ifndef NX_SECURE_TLS_SNI_EXTENSION_DISABLED
static UINT _nx_secure_tls_send_clienthello_sni_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                          UCHAR *packet_buffer, ULONG *packet_offset,
                                                          USHORT *extension_length,
                                                          ULONG available_size)
{
ULONG  offset;
USHORT ext;
UINT   data_length;

    /* Server Name Indication Extension structure:
     * |     2      |      2        |      1     |     2       |   <name length>   |
     * |  Ext Type  |  list length  |  name type | name length |  Host name string |
     */
    /*  From RFC 6066:

          struct {
              NameType name_type;
              select (name_type) {
                  case host_name: HostName;
              } name;
          } ServerName;

          enum {
              host_name(0), (255)
          } NameType;

          opaque HostName<1..2^16-1>;

          struct {
              ServerName server_name_list<1..2^16-1>
          } ServerNameList;

          The contents of this extension are specified as follows.

      -   The ServerNameList MUST NOT contain more than one name of the same
          name_type.

      -   Currently, the only server names supported are DNS hostnames.
     */

    /* Start with our passed-in packet offset. */
    offset = *packet_offset;

    /* The extension identifier. */
    ext = NX_SECURE_TLS_EXTENSION_SERVER_NAME_INDICATION;

    /* If there is no SNI server name, just return. */
    if (tls_session -> nx_secure_tls_sni_extension_server_name == NX_NULL)
    {
        *extension_length = 0;
        return(NX_SUCCESS);
    }

    /* Get the length of the entire extension. In this case it is a single name entry. Add 2 for the list length, plus 1 for
       the name_type field and 2 for the name length field (name type and length would be duplicated for all entries). */
    data_length = (UINT)(tls_session -> nx_secure_tls_sni_extension_server_name -> nx_secure_x509_dns_name_length + 5);

    if (available_size < (*packet_offset + data_length + 4))
    {

        /* Packet buffer too small. */
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    /* Put the extension ID into the packet. */
    packet_buffer[offset] = (UCHAR)((ext & 0xFF00) >> 8);
    packet_buffer[offset + 1] = (UCHAR)(ext & 0x00FF);
    offset += 2;

    /* Set the total extension length. */
    packet_buffer[offset] = (UCHAR)((data_length & 0xFF00) >> 8);
    packet_buffer[offset + 1] = (UCHAR)(data_length & 0x00FF);
    offset += 2;
    data_length -= 2; /* Remove list length. */

    /* Set the name list length. */
    packet_buffer[offset] = (UCHAR)((data_length & 0xFF00) >> 8);
    packet_buffer[offset + 1] = (UCHAR)(data_length & 0x00FF);
    offset += 2;
    data_length -= 3; /* Remove name type and name length. */

    /* Set the name type. */
    packet_buffer[offset] = NX_SECURE_TLS_SNI_NAME_TYPE_DNS;
    offset++;

    /* Set the name length. */
    packet_buffer[offset] = (UCHAR)((data_length & 0xFF00) >> 8);
    packet_buffer[offset + 1] = (UCHAR)(data_length & 0x00FF);
    offset += 2;

    /* Write the name into the packet. */
    NX_SECURE_MEMCPY(&packet_buffer[offset], tls_session -> nx_secure_tls_sni_extension_server_name -> nx_secure_x509_dns_name, data_length);
    offset += data_length;

    /* Return the amount of data we wrote. */
    *extension_length = (USHORT)(offset - *packet_offset);

    /* Return our updated packet offset. */
    *packet_offset = offset;


    return(NX_SUCCESS);
}
#endif

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_send_clienthello_sec_reneg_extension PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function adds the Secure Renegotiation Indication extension    */
/*    to an outgoing ClientHello record if the ClientHello is part of a   */
/*    renegotiation handshake (the extension should be empty for the      */
/*    initial handshake. See RFC 5746 for more information.               */
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
/*    _nx_secure_tls_send_clienthello_extensions                          */
/*                                          Send TLS ClientHello extension*/
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*  08-15-2019     Timothy Stapko           Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
#ifdef NX_SECURE_TLS_ENABLE_SECURE_RENEGOTIATION
static UINT _nx_secure_tls_send_clienthello_sec_reneg_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                                UCHAR *packet_buffer,
                                                                ULONG *packet_offset,
                                                                USHORT *extension_length,
                                                                ULONG available_size)
{
ULONG  offset;
USHORT ext;
UINT   data_length;

    /* Secure Renegotiation Indication Extensions structure:
     * |     2      |           12        |
     * |  Ext Type  |  client_verify_data |
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

#ifdef NX_SECURE_TLS_USE_SCSV_CIPHPERSUITE
    /* If we are using the SCSV ciphersuite for TLS 1.0 compatibility
       and the session is not yet active (first handshake, not a renegotiation),
       then don't send the empty extension below, just return with no
       offset adjustments. */
    if (!tls_session -> nx_secure_tls_local_session_active)
    {
        *extension_length = 0;
        return(NX_SUCCESS);
    }
#endif

    if (((!tls_session -> nx_secure_tls_local_session_active) &&
         (available_size < (*packet_offset + 5))) ||
        ((tls_session -> nx_secure_tls_local_session_active) &&
         (available_size < (*packet_offset + 5 + NX_SECURE_TLS_FINISHED_HASH_SIZE))))
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
        data_length = NX_SECURE_TLS_FINISHED_HASH_SIZE + 1;
        packet_buffer[offset] = (UCHAR)((data_length & 0xFF00) >> 8);
        packet_buffer[offset + 1] = (UCHAR)(data_length & 0x00FF);
        offset += 2;

        /* The extension actually has a second length field of 1 byte that needs to be populated. */
        /* Fill in the length of renegotiated connection field. */
        packet_buffer[offset] = (UCHAR)(NX_SECURE_TLS_FINISHED_HASH_SIZE & 0x00FF);
        offset++;

        /* Copy the verify data into the packet. */
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
#endif /* NX_SECURE_TLS_CLIENT_DISABLED */

