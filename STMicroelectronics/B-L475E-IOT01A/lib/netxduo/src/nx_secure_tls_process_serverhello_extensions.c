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
static UINT _nx_secure_tls_proc_serverhello_sec_reneg_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                                UCHAR *packet_buffer,
                                                                USHORT *extension_length);
#endif

#ifdef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
static UINT _nx_secure_tls_proc_serverhello_ecc_point_formats(NX_SECURE_TLS_SESSION *tls_session,
                                                              UCHAR *packet_buffer,
                                                              USHORT *extension_length);
static UINT _nx_secure_tls_proc_serverhello_ecjpake_key_kp_pair(NX_SECURE_TLS_SESSION *tls_session,
                                                                UCHAR *packet_buffer,
                                                                USHORT *extension_length);
#endif

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_process_serverhello_extensions       PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes any extensions included in an incoming      */
/*    ServerHello message from a remote host.                             */
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
/*    _nx_secure_tls_proc_serverhello_ecc_point_formats                   */
/*                                          Process ServerHello ECC       */
/*                                            point formats extension     */
/*    _nx_secure_tls_proc_serverhello_ecjpake_key_kp_pair                 */
/*                                          Process ServerHello ECJPAKE   */
/*                                            key kp pair extension       */
/*    _nx_secure_tls_proc_serverhello_sec_reneg_extension                 */
/*                                          Process ServerHello           */
/*                                            Renegotiation extension     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_server_handshake       Process ServerHello           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*  08-15-2019     Timothy Stapko           Modified comment(s), added    */
/*                                            extension hook, updated     */
/*                                            error return codes,         */
/*                                            removed cipher suite lookup,*/
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_process_serverhello_extensions(NX_SECURE_TLS_SESSION *tls_session,
                                                   UCHAR *packet_buffer, UINT message_length,
                                                   NX_SECURE_TLS_HELLO_EXTENSION *extensions,
                                                   UINT *num_extensions)
{
UINT status;

#ifdef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
USHORT                                ec_point_formats_match, zkp_verified;
#endif
UINT                                  offset;
USHORT                                extension_id;
USHORT                                extension_length;


#ifndef NX_SECURE_TLS_ENABLE_SECURE_RENEGOTIATION
#ifndef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
    NX_PARAMETER_NOT_USED(tls_session);
#endif
#endif

#ifdef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    ec_point_formats_match = NX_FALSE;
    zkp_verified = NX_FALSE;
#endif

    offset = 0;
    status = NX_SUCCESS;
    *num_extensions = 0;

    /* Process extensions until we run out. */
    while (offset < message_length)
    {
        /* See what the extension is. */
        extension_id = (USHORT)((packet_buffer[offset] << 8) + packet_buffer[offset + 1]);

        /* Skip type id of extentions. */
        offset += 2;

        /* Parse the extension. */
        switch (extension_id)
        {
#ifdef NX_SECURE_TLS_ENABLE_SECURE_RENEGOTIATION
        case NX_SECURE_TLS_EXTENSION_SECURE_RENEGOTIATION:
            status = _nx_secure_tls_proc_serverhello_sec_reneg_extension(tls_session, &packet_buffer[offset], &extension_length);

            break;
#endif /* NX_SECURE_TLS_ENABLE_SECURE_RENEGOTIATION */
#ifdef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
        /* ECJPAKE ciphersuite extensions. */
        case NX_SECURE_TLS_EXTENSION_EC_POINT_FORMATS:
            status = _nx_secure_tls_proc_serverhello_ecc_point_formats(tls_session, &packet_buffer[offset], &extension_length);
            if (status == NX_SUCCESS)
            {
                ec_point_formats_match = NX_TRUE;
            }
            break;
        case NX_SECURE_TLS_EXTENSION_ECJPAKE_KEY_KP_PAIR:
            status = _nx_secure_tls_proc_serverhello_ecjpake_key_kp_pair(tls_session, &packet_buffer[offset], &extension_length);
            if (status == NX_SUCCESS)
            {
                zkp_verified = NX_TRUE;
            }
            break;
#else
        case NX_SECURE_TLS_EXTENSION_EC_GROUPS:
        case NX_SECURE_TLS_EXTENSION_EC_POINT_FORMATS:
        case NX_SECURE_TLS_EXTENSION_ECJPAKE_KEY_KP_PAIR:
#endif /* NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE */

        case NX_SECURE_TLS_EXTENSION_SERVER_NAME_INDICATION:
        case NX_SECURE_TLS_EXTENSION_MAX_FRAGMENT_LENGTH:
        case NX_SECURE_TLS_EXTENSION_CLIENT_CERTIFICATE_URL:
        case NX_SECURE_TLS_EXTENSION_TRUSTED_CA_INDICATION:
        case NX_SECURE_TLS_EXTENSION_CERTIFICATE_STATUS_REQUEST:
            /* These extensions require information to be passed to the application. Save off
               the extension data in our extensions array to pass along in the hello callback. */
            extension_length = (USHORT)((packet_buffer[offset] << 8) + packet_buffer[offset + 1]);
            offset += 2;

            extensions[*num_extensions].nx_secure_tls_extension_id = extension_id;
            extensions[*num_extensions].nx_secure_tls_extension_data = &packet_buffer[offset];
            extensions[*num_extensions].nx_secure_tls_extension_data_length = extension_length;

            /* Added another extension to the array. */
            *num_extensions = *num_extensions + 1;

            break;
        case NX_SECURE_TLS_EXTENSION_SIGNATURE_ALGORITHMS:
        case NX_SECURE_TLS_EXTENSION_TRUNCATED_HMAC:
        default:
            /* Unknown extension, just ignore - TLS supports multiple extensions and the default
               behavior is to ignore any extensions that we don't know. Assume the next two
               octets are the length field so we can continue processing. */
            extension_length = (USHORT)((packet_buffer[offset] << 8) + packet_buffer[offset + 1]);
            offset += 2;

            break;
        }

        /* Adjust our offset with the length of the extension we just parsed. */
        offset += extension_length;
    }

#ifdef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
    /* Make sure no ECJPAKE extensions are missing.  */
    if ((tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECJPAKE) &&
        (ec_point_formats_match == 0 || zkp_verified == 0))
    {
        return(NX_SECURE_TLS_UNSUPPORTED_ECC_FORMAT);
    }
#endif /* NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE */

    NX_SECURE_PROCESS_SERVERHELLO_EXTENSIONS_EXTENSION

    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_proc_serverhello_ecc_point_formats   PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses the ec_point_formats extension when ECC        */
/*    ciphersuites are being used.                                        */
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
/*    _nx_secure_tls_process_serverhello_extensions                       */
/*                                          Process ServerHello extensions*/
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*  08-15-2019     Timothy Stapko           Modified comment(s),          */
/*                                            added operation method for  */
/*                                            elliptic curve cryptography,*/
/*                                            fixed compiler warnings,    */
/*                                            removed cipher suite lookup,*/
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
#ifdef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
static UINT _nx_secure_tls_proc_serverhello_ecc_point_formats(NX_SECURE_TLS_SESSION *tls_session,
                                                              UCHAR *packet_buffer,
                                                              USHORT *extension_length)
{
UINT                                  status;
UINT                                  i;
UCHAR                                 ec_point_formats_length;
UINT                                  offset;

    *extension_length = (USHORT)((*packet_buffer << 8) + packet_buffer[1] + 2);

    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    /* Skip the length field of this extension. */
    offset = 2;

    /* ec_point_formats Extension.  */
    ec_point_formats_length = packet_buffer[offset];
    offset += 1;

    /* Ignore the extension if we are not using ECJPAKE. */
    if (tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECJPAKE)
    {
        /* Make sure uncompressed (0) format is supported. */
        status = NX_SECURE_TLS_UNSUPPORTED_CIPHER;
        for (i = 0; i < ec_point_formats_length; ++i)
        {
            if (packet_buffer[offset + i] == 0x0)
            {
                status = NX_SUCCESS;
                break;
            }
        }

        /* Check for error. */
        if (status != NX_SUCCESS)
        {
            return(status);
        }
    }

    /* Return the extension length - add 1 for length itself. */
    *extension_length = (USHORT)(offset + ec_point_formats_length);

    return(NX_SUCCESS);
}
#endif


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_proc_serverhello_ecjpake_key_kp_pair PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses the ecjpake_key_kp_pair extension when ECC     */
/*    JPAKE ciphersuites are being used.                                  */
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
/*    [nx_crypto_operation]                 Crypto operation              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_process_serverhello_extensions                       */
/*                                          Process ServerHello extensions*/
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*  08-15-2019     Timothy Stapko           Modified comment(s),          */
/*                                            added operation method for  */
/*                                            elliptic curve cryptography,*/
/*                                            passed crypto handle into   */
/*                                            crypto internal functions,  */
/*                                            updated error return codes, */
/*                                            removed cipher suite lookup,*/
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
#ifdef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
static UINT _nx_secure_tls_proc_serverhello_ecjpake_key_kp_pair(NX_SECURE_TLS_SESSION *tls_session,
                                                                UCHAR *packet_buffer,
                                                                USHORT *extension_length)
{
UINT                                  status;
NX_CRYPTO_METHOD                     *crypto_method;

    *extension_length = (USHORT)((*packet_buffer << 8) + packet_buffer[1] + 2);

    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    /* Ignore the extension if we are not using ECJPAKE. */
    if (tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECJPAKE)
    {

        crypto_method = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_auth;

        /* ecjpake_key_kp_pair Extension.  */
        status = crypto_method -> nx_crypto_operation(NX_CRYPTO_ECJPAKE_SERVER_HELLO_PROCESS,
                                                      tls_session -> nx_secure_public_auth_handler,
                                                      crypto_method,
                                                      NX_NULL, 0,
                                                      &packet_buffer[2], /* Skip extension length. */
                                                      (ULONG)(*extension_length - 2),
                                                      NX_NULL, NX_NULL, 0,
                                                      tls_session -> nx_secure_public_auth_metadata_area,
                                                      tls_session -> nx_secure_public_auth_metadata_size,
                                                      NX_NULL, NX_NULL);
        if (status)
        {
            return(status);
        }
    }

    return(NX_SUCCESS);
}
#endif

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_proc_serverhello_sec_reneg_extension PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses the Secure Renegotiation Indication extension  */
/*    from an incoming ServerHello record.See RFC 5746 for more           */
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
/*    _nx_secure_tls_process_serverhello_extensions                       */
/*                                          Process ServerHello extensions*/
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*  08-15-2019     Timothy Stapko           Modified comment(s),          */
/*                                            added flexibility of using  */
/*                                            macros instead of direct C  */
/*                                            library function calls,     */
/*                                            fixed issue with requiring  */
/*                                            both the SCSV and extension */
/*                                            to be present for           */
/*                                            renegotation, changed the   */
/*                                            name of the SCSV            */
/*                                            renegotiation flag,         */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
#ifdef NX_SECURE_TLS_ENABLE_SECURE_RENEGOTIATION
static UINT _nx_secure_tls_proc_serverhello_sec_reneg_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                                UCHAR *packet_buffer,
                                                                USHORT *extension_length)
{
ULONG  offset;
UCHAR  renegotiated_connection_length;
USHORT parsed_length;
INT    compare_value;

    /* Secure Renegotiation Indication Extensions structure (for serverhello):
     * Initial ServerHello:
     * |     2      |     2     |        1         |
     * |  Ext Type  |  Ext Len  |  Reneg Info Len  |
     * |   0xff01   |   0x0001  |       0x00       |
     *
     * Renegotiating ServerHello:
     * |     2      |     2     |        1         |         12         |         12           |
     * |  Ext Type  |  Ext Len  |  Reneg Info Len  | client_verify_data |  server_verify_data  |
     * |   0xff01   |   0x0019  |       0x18       |                    |                      |
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

    /* Get the extension length. */
    parsed_length = (USHORT)((packet_buffer[0] << 8) + packet_buffer[1]);
    *extension_length = (USHORT)(2 + parsed_length);
    offset = 2;

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
            return(NX_SECURE_TLS_RENEGOTIATION_EXTENSION_ERROR);
        }

        if(!tls_session -> nx_secure_tls_secure_renegotiation)
        {
            return(NX_SECURE_TLS_RENEGOTIATION_EXTENSION_ERROR);
        }


        /* Check that the received verification data is the expected size. For ServerHello, the size is twice the
           finished verification hash size because it includes both client and server hashes. */
        if (renegotiated_connection_length != (2 * NX_SECURE_TLS_FINISHED_HASH_SIZE))
        {
            /* Do not have the right amount of data for comparison. */
            return(NX_SECURE_TLS_RENEGOTIATION_EXTENSION_ERROR);
        }

        /* Compare the received verify data to our locally-stored version - start with client (local) verify data. */
        compare_value = NX_SECURE_MEMCMP(&packet_buffer[offset], tls_session -> nx_secure_tls_local_verify_data, NX_SECURE_TLS_FINISHED_HASH_SIZE);

        if (compare_value)
        {
            /* Session verify data did not match what we expect - error. */
            return(NX_SECURE_TLS_RENEGOTIATION_EXTENSION_ERROR);
        }

        /* Now compare the remote verify data with what we just received. */
        offset += NX_SECURE_TLS_FINISHED_HASH_SIZE;
        compare_value = NX_SECURE_MEMCMP(&packet_buffer[offset], tls_session -> nx_secure_tls_remote_verify_data, NX_SECURE_TLS_FINISHED_HASH_SIZE);

        if (compare_value)
        {
            /* Session verify data did not match what we expect - error. */
            return(NX_SECURE_TLS_RENEGOTIATION_EXTENSION_ERROR);
        }

        /* If we get here, the verification data is good! */
    }
    else
    {
        /* Verify that the extension contains only the initial handshake data - this is a new connection. */
        if ((parsed_length != 1) || (tls_session -> nx_secure_tls_local_session_active))
        {
            /* Error - the provided extension length was not expected for an initial handshake. */
            return(NX_SECURE_TLS_RENEGOTIATION_EXTENSION_ERROR);
        }

        /* The remote host supports secure renegotiation. */
        tls_session -> nx_secure_tls_secure_renegotiation = NX_TRUE;
    }

    return(NX_SUCCESS);
}
#endif

