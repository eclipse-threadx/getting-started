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
static UINT _nx_secure_tls_proc_clienthello_sec_reneg_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                                UCHAR *packet_buffer,
                                                                UINT extension_length);
#endif

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_process_clienthello_extensions       PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes any extensions included in an incoming      */
/*    ClientHello message from a remote host.                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    packet_buffer                         Pointer to message data       */
/*    message_length                        Length of message data (bytes)*/
/*    extensions                            Extensions for output         */
/*    num_extensions                        Number of extensions          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_proc_clienthello_sec_reneg_extension                 */
/*                                          Process ClientHello           */
/*                                            Renegotiation extension     */
/*    _nx_secure_tls_proc_clienthello_ec_groups_extension                 */
/*                                          Process ClientHello           */
/*                                            EC groups extension         */
/*    _nx_secure_tls_proc_clienthello_ec_point_formats_extension          */
/*                                          Process ClientHello           */
/*                                            EC point formats extension  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_process_clienthello    Process ClientHello           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*  08-15-2019     Timothy Stapko           Modified comment(s), added    */
/*                                            elliptic curve cryptography */
/*                                            support, added extension    */
/*                                            hook, added buffer size     */
/*                                            checking, improved packet   */
/*                                            length verification,        */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_process_clienthello_extensions(NX_SECURE_TLS_SESSION *tls_session,
                                                   UCHAR *packet_buffer, UINT message_length,
                                                   NX_SECURE_TLS_HELLO_EXTENSION *extensions,
                                                   UINT *num_extensions)
{
UINT   status = NX_SUCCESS;
UINT   offset;
UINT   max_extensions;
USHORT extension_id;
UINT   extension_length;

#ifndef NX_SECURE_TLS_ENABLE_SECURE_RENEGOTIATION
    NX_PARAMETER_NOT_USED(tls_session);
#endif /* NX_SECURE_TLS_ENABLE_SECURE_RENEGOTIATION */

    max_extensions = *num_extensions;
    offset = 0;
    *num_extensions = 0;

    /* Process extensions until we run out. */
    while (offset < message_length && *num_extensions < max_extensions)
    {

        /* Make sure there are at least 4 bytes available so we can read extension_id and
           extension_length. */
        if((offset + 4) > message_length)
        {
            return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
        }

        /* See what the extension is. */
        extension_id = (USHORT)((packet_buffer[offset] << 8) + packet_buffer[offset + 1]);
        offset += 2;

        /* Get extension length. */
        extension_length = (UINT)((packet_buffer[offset] << 8) + packet_buffer[offset + 1]);
        offset += 2;

        /* Verify the message_length is at least "extension_length". */
        if((offset + extension_length) > message_length)
        {
            return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
        }            

        /* Parse the extension. */
        switch (extension_id)
        {
        case NX_SECURE_TLS_EXTENSION_SIGNATURE_ALGORITHMS:
            /* Ignore for now. */
            break;
#ifdef NX_SECURE_TLS_ENABLE_SECURE_RENEGOTIATION
        case NX_SECURE_TLS_EXTENSION_SECURE_RENEGOTIATION:
            status = _nx_secure_tls_proc_clienthello_sec_reneg_extension(tls_session,
                                                                         &packet_buffer[offset],
                                                                         extension_length);

            break;
#endif /* NX_SECURE_TLS_ENABLE_SECURE_RENEGOTIATION */

        case NX_SECURE_TLS_EXTENSION_SERVER_NAME_INDICATION:
        case NX_SECURE_TLS_EXTENSION_MAX_FRAGMENT_LENGTH:
        case NX_SECURE_TLS_EXTENSION_CLIENT_CERTIFICATE_URL:
        case NX_SECURE_TLS_EXTENSION_TRUSTED_CA_INDICATION:
        case NX_SECURE_TLS_EXTENSION_CERTIFICATE_STATUS_REQUEST:
        case NX_SECURE_TLS_EXTENSION_EC_GROUPS:
        case NX_SECURE_TLS_EXTENSION_EC_POINT_FORMATS:
        case NX_SECURE_TLS_EXTENSION_ECJPAKE_KEY_KP_PAIR:
            /* These extensions require information to be passed to the application. Save off
               the extension data in our extensions array to pass along in the hello callback. */
            extensions[*num_extensions].nx_secure_tls_extension_id = extension_id;
            extensions[*num_extensions].nx_secure_tls_extension_data = &packet_buffer[offset];
            extensions[*num_extensions].nx_secure_tls_extension_data_length = (USHORT)extension_length;

            /* Added another extension to the array. */
            *num_extensions = *num_extensions + 1;

            break;
        case NX_SECURE_TLS_EXTENSION_TRUNCATED_HMAC:
        default:
            /* Unknown or unsupported extension, just ignore - TLS supports multiple extensions and the default
               behavior is to ignore any extensions that we don't know. */
            break;
        }

        /* Adjust our offset with the length of the extension we just parsed. */
        offset += extension_length;
    }

    NX_SECURE_PROCESS_CLIENTHELLO_EXTENSIONS_EXTENSION

    return(status);
}



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_proc_clienthello_sec_reneg_extension PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses the Secure Renegotiation Indication extension  */
/*    from an incoming ClientHello record.See RFC 5746 for more           */
/*    information.                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    packet_buffer                         Outgoing TLS packet buffer    */
/*    extension_length                      Length of extension data      */
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
/*    _nx_secure_tls_process_clienthello_extensions                       */
/*                                          Process ClientHello extensions*/
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*  08-15-2019     Timothy Stapko           Modified comment(s), fixed    */
/*                                            issue with requiring both   */
/*                                            the SCSV and extension to be*/
/*                                            present for renegotation,   */
/*                                            changed the name of the     */
/*                                            SCSV renegotiation flag,    */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
#ifdef NX_SECURE_TLS_ENABLE_SECURE_RENEGOTIATION
static UINT _nx_secure_tls_proc_clienthello_sec_reneg_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                                UCHAR *packet_buffer,
                                                                UINT extension_length)
{
ULONG  offset = 0;
UCHAR  renegotiated_connection_length;
INT    compare_value;

    /* Secure Renegotiation Indication Extensions structure:
     * Initial ClientHello:
     * |     2      |     2     |        1         |
     * |  Ext Type  |  Ext Len  |  Reneg Info Len  |
     * |   0xff01   |   0x0001  |       0x00       |
     *
     * Renegotiating ClientHello:
     * |     2      |     2     |        1         |         12         |
     * |  Ext Type  |  Ext Len  |  Reneg Info Len  | client_verify_data |
     * |   0xff01   |   0x000d  |       0x0c       |                    |
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

    /* Get the "renegotiated_connection" field. */
    renegotiated_connection_length = packet_buffer[offset];
    offset++;

    /* See if the client is attempting to renegotiate an established connection. */
    if (renegotiated_connection_length)
    {
        /* The remote host is attempting a renegotiation - make sure our local session is active and renegotiation is OK. */
        if (!tls_session -> nx_secure_tls_local_session_active || !tls_session -> nx_secure_tls_remote_session_active)
        {
            /* Remote host is attempting a renegotiation but the server is not currently in a session! */
            return(NX_SECURE_TLS_RENEGOTIATION_SESSION_INACTIVE);
        }

        if(!tls_session -> nx_secure_tls_secure_renegotiation)
        {
            return(NX_SECURE_TLS_RENEGOTIATION_FAILURE);
        }

        if (renegotiated_connection_length != NX_SECURE_TLS_FINISHED_HASH_SIZE)
        {
            /* Do not have the right amount of data for comparison. */
            return(NX_SECURE_TLS_RENEGOTIATION_EXTENSION_ERROR);
        }

        /* Compare the received verify data to our locally-stored version. */
        compare_value = NX_SECURE_MEMCMP(&packet_buffer[offset], tls_session -> nx_secure_tls_remote_verify_data, NX_SECURE_TLS_FINISHED_HASH_SIZE);

        if (compare_value)
        {
            /* Session verify data did not match what we expect - error. */
            return(NX_SECURE_TLS_RENEGOTIATION_EXTENSION_ERROR);
        }
    }
    else
    {
        /* Verify that the extension contains only the initial handshake data - this is a new connection. */
        if ((extension_length != 1) || (tls_session -> nx_secure_tls_local_session_active))
        {
            /* Error - the provided extension length was not expected for an initial handshake. */
            return(NX_SECURE_TLS_RENEGOTIATION_EXTENSION_ERROR);
        }

        /* The remote host supports renegotiation. */
        tls_session -> nx_secure_tls_secure_renegotiation = NX_TRUE;
    }

    return(NX_SUCCESS);
}
#endif
