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
/*    _nx_secure_tls_send_record                          PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function encapsulates the TLS record layer send                */
/*    functionality. The incoming packet data is wrapped in               */
/*    a TLS record, which includes a header and footer (for               */
/*    encrypted data). Also, all encryption of application                */
/*    data is handled here.                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           Pointer to TLS session        */
/*    send_packet                           Packet data to send           */
/*    record_type                           TLS record type               */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Record send status            */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_record_hash_calculate  Calculate hash of record      */
/*    _nx_secure_tls_record_hash_initialize Initialize hash of record     */
/*    _nx_secure_tls_record_hash_update     Update hash of record         */
/*    _nx_secure_tls_record_payload_encrypt Encrypt payload               */
/*    _nx_secure_tls_session_iv_size_get    Get IV size for this session. */
/*    nx_packet_data_append                 Append data to packet         */
/*    nx_tcp_socket_send                    Send packet                   */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_client_handshake       TLS client state machine      */
/*    _nx_secure_tls_send_handshake_record  Send TLS handshake record     */
/*    _nx_secure_tls_server_handshake       TLS server state machine      */
/*    _nx_secure_tls_session_end            End of a session              */
/*    _nx_secure_tls_session_receive_records                              */
/*                                          Receive TLS records           */
/*    _nx_secure_tls_session_send           Send session packet           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-09-2017     Timothy Stapko           Initial Version 5.10          */
/*  12-15-2017     Timothy Stapko           Modified comment(s),          */
/*                                            let the caller release      */
/*                                            packet when TCP send failed,*/
/*                                            optimized the logic,        */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Timothy Stapko           Modified comment(s), and skip */
/*                                            hash when it is not required*/
/*                                            logic to clear encryption   */
/*                                            key and other secret data,  */
/*                                            improved packet length      */
/*                                            verification, removed       */
/*                                            cipher suite lookup,        */
/*                                            resulting in version 5.12    */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_send_record(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET *send_packet,
                                UCHAR record_type, ULONG wait_option)
{
UINT       status;
UINT       message_length;
UCHAR     *mac_secret;
UCHAR     *record_header;
UCHAR      record_hash[NX_SECURE_TLS_MAX_HASH_SIZE];
UINT       hash_length;
UCHAR     *hash_data;
ULONG      hash_data_length;
ULONG      length;
USHORT     iv_size = 0;
NX_PACKET *current_packet;

    /* Length of the data in the packet. */
    length = send_packet -> nx_packet_length;

    /* See if this is an active session, we need to account for the IV if the session cipher
       uses one. */
    if (tls_session -> nx_secure_tls_local_session_active)
    {

        /* Get the size of the IV used by the session cipher. */
        status = _nx_secure_tls_session_iv_size_get(tls_session, &iv_size);

        if (status != NX_SUCCESS)
        {
            return(status);
        }

        /* Back off the pointer to the point before the IV data allocation
           (can be 0). Increases length since we are moving the prepend pointer. */
        send_packet -> nx_packet_prepend_ptr -= iv_size;
        send_packet -> nx_packet_length += iv_size;
    }

    /* Back off the prepend_ptr for TLS Record header. Note the packet_length field is adjusted
       prior to nx_tcp_socket_send() below. */
    //send_packet -> nx_packet_prepend_ptr -= NX_SECURE_TLS_RECORD_HEADER_SIZE;

    /* Get a pointer to our record header which is now right after the prepend pointer. */
    record_header = send_packet -> nx_packet_prepend_ptr - NX_SECURE_TLS_RECORD_HEADER_SIZE;

    /* Build the TLS record header. */
    record_header[0] = record_type;

    /* Set the version number. */
    record_header[1] = (UCHAR)((tls_session -> nx_secure_tls_protocol_version & 0xFF00) >> 8);
    record_header[2] = (UCHAR)(tls_session -> nx_secure_tls_protocol_version & 0x00FF);

    /* Set the length of the record prior to hashing and encryption - this is because
       the hashing is done on the record as if it were not encrypted so any additional
       padding or IVs, etc. added to the length would invalidate the hash. We update
       the length following the encryption below. */
    message_length = length;
    record_header[3] = (UCHAR)((length & 0xFF00) >> 8);
    record_header[4] = (UCHAR)(length & 0x00FF);

    /* If the session is active, hash and encrypt the record payload using
       the session keys and chosen ciphersuite. */
    if (tls_session -> nx_secure_tls_local_session_active)
    {
        /*************************************************************************************************************/

        if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
        {

            /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
            return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
        }

        if (tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_hash -> nx_crypto_operation)
        {

            /***** HASHING *****/
            /* Select our proper MAC secret for hashing. */
            if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_SERVER)
            {
                /* If we are a server, we need to use the client's MAC secret. */
                mac_secret = tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_write_mac_secret;
            }
            else
            {
                /* We are a client, so use the server's MAC secret. */
                mac_secret = tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_write_mac_secret;
            }

            /* Account for large records that exceed the packet size and are chained in multiple packets
               such as large certificate messages with multiple certificates.
               tls_session->nx_secure_hash_mac_metadata_area is persistent across the following calls, so it's important
               to not do anything that might change the contents of that buffer until the hash is calculated after the loop! */
            current_packet = send_packet;

            /* Initialize the hash routine with our MAC secret, sequence number, and header. */
            status = _nx_secure_tls_record_hash_initialize(tls_session, tls_session -> nx_secure_tls_local_sequence_number,
                                                           record_header, 5, &hash_length, mac_secret);

            /* Check return from hash routine initialization. */
            if (status != NX_SUCCESS)
            {
                return(status);
            }

            /* Start the hash data after the header and IV. */
            hash_data = current_packet -> nx_packet_prepend_ptr + iv_size;
            hash_data_length = (ULONG)(current_packet -> nx_packet_append_ptr - current_packet -> nx_packet_prepend_ptr) - iv_size;

            /* Walk packet chain. */
            do
            {
                /* Update the handshake hash with the data. */
                status = _nx_secure_tls_record_hash_update(tls_session, hash_data,
                                                           (UINT)hash_data_length);

                /* Advance the packet pointer to the next packet in the chain. */
                current_packet = current_packet -> nx_packet_next;
                if (current_packet != NX_NULL)
                {
                    hash_data = current_packet -> nx_packet_prepend_ptr;
                    hash_data_length = (ULONG)(current_packet -> nx_packet_append_ptr - current_packet -> nx_packet_prepend_ptr);
                }
            } while (current_packet != NX_NULL);

            /* Generate the hash on the plaintext data. */
            _nx_secure_tls_record_hash_calculate(tls_session, record_hash, &hash_length);

            /* Release the protection before suspending on nx_packet_data_append. */
            tx_mutex_put(&_nx_secure_tls_protection);

            /* Append the hash to the plaintext data in the last packet before encryption. */
            status = nx_packet_data_append(send_packet, record_hash, hash_length,
                                           tls_session -> nx_secure_tls_packet_pool, NX_WAIT_FOREVER);

#ifdef NX_SECURE_KEY_CLEAR
            NX_SECURE_MEMSET(record_hash, 0, sizeof(record_hash));
#endif /* NX_SECURE_KEY_CLEAR  */

            /* Get the protection after nx_packet_data_append. */
            tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);
        }

        /*************************************************************************************************************/
        /***** ENCRYPTION *****/

        status = _nx_secure_tls_record_payload_encrypt(tls_session, send_packet, tls_session -> nx_secure_tls_local_sequence_number, record_type);

        if (status != NX_SUCCESS)
        {
            return(status);
        }

        /*************************************************************************************************************/

        /* Increment the sequence number. */
        if ((tls_session -> nx_secure_tls_local_sequence_number[0] + 1) == 0)
        {
            /* Check for overflow of the 32-bit number. */
            tls_session -> nx_secure_tls_local_sequence_number[1]++;
        }
        tls_session -> nx_secure_tls_local_sequence_number[0]++;
    }

    /* The encryption above may have changed the payload length, so get the length from
       the packet and use it to update the record header. */
    message_length = send_packet -> nx_packet_length;

    /* Set the length of the record. */
    record_header[3] = (UCHAR)((message_length & 0xFF00) >> 8);
    record_header[4] = (UCHAR)(message_length & 0x00FF);

    /* Adjust packet length */
    send_packet -> nx_packet_prepend_ptr -= NX_SECURE_TLS_RECORD_HEADER_SIZE;
    send_packet -> nx_packet_length += NX_SECURE_TLS_RECORD_HEADER_SIZE;

    /* Release the protection before suspending on nx_tcp_socket_send. */
    tx_mutex_put(&_nx_secure_tls_protection);

    /* Send the TCP packet(s) containing our record. */
    status = nx_tcp_socket_send(tls_session -> nx_secure_tls_tcp_socket, send_packet, wait_option);

#ifdef NX_SECURE_KEY_CLEAR
    if (tls_session -> nx_secure_tls_local_session_active)
    {

        /* Clear all data in chained packet. */
        current_packet = send_packet;
        while (current_packet)
        {
            NX_SECURE_MEMSET(current_packet -> nx_packet_prepend_ptr, 0,
                   (ULONG)current_packet -> nx_packet_append_ptr -
                   (ULONG)current_packet -> nx_packet_prepend_ptr);
            current_packet = current_packet -> nx_packet_next;
        }
    }
#endif /* NX_SECURE_KEY_CLEAR  */

    /* Get the protection after nx_tcp_socket_send. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

    return(status);
}

