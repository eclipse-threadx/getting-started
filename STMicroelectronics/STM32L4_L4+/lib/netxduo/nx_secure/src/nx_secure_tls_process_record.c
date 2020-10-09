/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
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

static VOID _nx_secure_tls_packet_trim(NX_PACKET *packet_ptr);

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_process_record                       PORTABLE C      */
/*                                                           6.0.2        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes a single TLS record, handling both          */
/*    handshake and application records. When multiple records are        */
/*    received in a single lower-level network packet (e.g. TCP), the     */
/*    "record_offset" parameter is used to offset into the packet buffer. */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           Pointer to TLS control block  */
/*    packet_ptr                            NX_PACKET containing a record */
/*    bytes_processed                       Return for size of packet     */
/*    wait_option                           Control timeout options       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_client_handshake       TLS Client state machine      */
/*    _nx_secure_tls_process_changecipherspec                             */
/*                                          Process ChangeCipherSpec      */
/*    _nx_secure_tls_process_header         Process record header         */
/*    _nx_secure_tls_record_payload_decrypt Decrypt record data           */
/*    _nx_secure_tls_server_handshake       TLS Server state machine      */
/*    _nx_secure_tls_verify_mac             Verify record MAC checksum    */
/*    nx_packet_allocate                    NetX Packet allocation call   */
/*    nx_packet_data_append                 Append data to packet         */
/*    nx_packet_data_extract_offset         Extract data from NX_PACKET   */
/*    _nx_secure_tls_packet_trim            Trim TLS packet               */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_session_receive_records                              */
/*                                          Receive TLS records           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  08-14-2020     Timothy Stapko           Modified comment(s),          */
/*                                            supported chained packet,   */
/*                                            fixed compiler warnings,    */
/*                                            resulting in version 6.0.2  */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_process_record(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET *packet_ptr,
                                   ULONG *bytes_processed, ULONG wait_option)
{
UINT       status;
UINT       error_status;
USHORT     header_length;
UCHAR      header_data[NX_SECURE_TLS_RECORD_HEADER_SIZE] = {0}; /* DTLS record header is larger than TLS. Allocate enough space for both. */
USHORT     message_type;
UINT       message_length;
ULONG      bytes_copied;
UCHAR     *packet_data = NX_NULL;
ULONG      record_offset = 0;
ULONG      record_offset_next = 0;
NX_PACKET *decrypted_packet;

    /* Basic state machine:
     * 1. Process header, which will set the state and return some data.
     * 2. Advance the packet pointer by the size of the header (returned by header processing)
     *    and process the record.
     * 3. Take any actions necessary based on state.
     *
     *
     * For reading TLS records from TCP packets we have two cases to worry about:
     * 1. Multiple records per packet
     *     - Multiple TLS records are packed into a single packet.
     *     - Loop below continues to process records until queue is emptied
     * 2. Multiple packets per record
     *     - Header or payload may span multiple chained packets
     *     - If we have less than a complete header, return NX_CONTINUE to read more bytes from TCP socket
     *     - If we have a complete header, process the header - this may happen multiple times
     *       if we returned NX_CONTINUE due to having an incomplete payload.
     *     - After processing the header, check to see if the entire payload (length from header) has
     *       been read from the socket into the queue. If incomplete, return NX_CONTINUE.
     *     - If the payload is complete, process it in the state machine (decrypt if necessary first)
     *       and update the record_offset and bytes_processed values accordingly.
     */

    status = NX_CONTINUE;
    *bytes_processed = 0;

    /* Process the packet. */
    if (packet_ptr != NX_NULL)
    {
        if (packet_ptr -> nx_packet_last == NX_NULL)
        {

            /* The logic below requires nx_packet_last is set which is always TRUE
               for chained packets coming from NetX. However, this packet is not chained
               so set the nx_packet_last pointer to itself. */
            packet_ptr -> nx_packet_last = packet_ptr;
        }

        /* Chain the packet. */
        if (tls_session -> nx_secure_record_queue_header == NX_NULL)
        {
            tls_session -> nx_secure_record_queue_header = packet_ptr;
        }
        else
        {

            /* Link current packet. */
            tls_session -> nx_secure_record_queue_header -> nx_packet_last -> nx_packet_next = packet_ptr;
            tls_session -> nx_secure_record_queue_header -> nx_packet_last = packet_ptr -> nx_packet_last;
            tls_session -> nx_secure_record_queue_header -> nx_packet_length += packet_ptr -> nx_packet_length;
        }
    }

    /* Process multiple records per packet. */
    while (status == NX_CONTINUE)
    {

        /* Make sure the decrypted packet is NULL. */
        if (tls_session -> nx_secure_record_decrypted_packet)
        {
            nx_secure_tls_packet_release(tls_session -> nx_secure_record_decrypted_packet);
            tls_session -> nx_secure_record_decrypted_packet = NX_NULL;
        }
        decrypted_packet = NX_NULL;

        /* Get packet from record queue. */
        packet_ptr = tls_session -> nx_secure_record_queue_header;

        /* Process the TLS record header, which will set the state. */
        status = _nx_secure_tls_process_header(tls_session, packet_ptr, record_offset, &message_type, &message_length, header_data, &header_length);

        if (status != NX_SECURE_TLS_SUCCESS)
        {
            return(status);
        }

        /* Ignore empty records. */
        if (message_length == 0)
        {

            /* Update the number of bytes we processed. */
            *bytes_processed += (ULONG)header_length;
            return(NX_SUCCESS);
        }

        /* Is the entire payload of the current record received? */
        if ((header_length + record_offset + message_length) > packet_ptr -> nx_packet_length)
        {

            /* Wait more TCP packets for this one record. */
            return(NX_CONTINUE);
        }

        /* Update the number of bytes we processed. */
        *bytes_processed += (ULONG)header_length + message_length;
        record_offset += (ULONG)header_length;
        record_offset_next = record_offset + message_length;

        /* Check for active encryption of incoming records. If encrypted, decrypt before further processing. */
        if (tls_session -> nx_secure_tls_remote_session_active
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
            && (!tls_session -> nx_secure_tls_1_3 || message_type == NX_SECURE_TLS_APPLICATION_DATA)
#endif
            )
        {

            /* Check the record length. */
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
            if (tls_session -> nx_secure_tls_1_3)
            {
                if (message_length > NX_SECURE_TLS_MAX_CIPHERTEXT_LENGTH_1_3)
                {
                    return(NX_SECURE_TLS_RECORD_OVERFLOW);
                }
            }
            else
#endif
            {
                if (message_length > NX_SECURE_TLS_MAX_CIPHERTEXT_LENGTH)
                {
                    return(NX_SECURE_TLS_RECORD_OVERFLOW);
                }
            }

            /* Decrypt the record data. */
            status = _nx_secure_tls_record_payload_decrypt(tls_session, packet_ptr, record_offset,
                                                           message_length, &decrypted_packet,
                                                           tls_session -> nx_secure_tls_remote_sequence_number,
                                                           (UCHAR)message_type, wait_option);

            /* Set the error status to something appropriate. */
            error_status = NX_SECURE_TLS_SUCCESS;

            /* Check the MAC hash if we were able to decrypt the record. */
            if (status != NX_SECURE_TLS_SUCCESS)
            {
                /* Save off the error status so we can return it after the mac check. */
                error_status = status;
            }
            else
            {

                /* Record it so it can be release in case it is not returned to user application. */
                tls_session -> nx_secure_record_decrypted_packet = decrypted_packet;
                message_length = decrypted_packet -> nx_packet_length;
            }

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
            /* TLS 1.3 uses AEAD for all authentication and encryption. Therefore
               the MAC verification is only needed for older TLS versions. */
            if(tls_session->nx_secure_tls_1_3)
            {

                if (status == NX_SECURE_TLS_SUCCESS)
                {

                    /* In TLS 1.3, encrypted records have a single byte at the
                    record that contains the message type (e.g. application data,
                    ect.), which is now the ACTUAL message type. */
                    status = nx_packet_data_extract_offset(decrypted_packet,
                                                           decrypted_packet -> nx_packet_length - 1,
                                                           &message_type, 1, &bytes_copied);
                    if (status || (bytes_copied != 1))
                    {
                        error_status = NX_SECURE_TLS_INVALID_PACKET;
                    }

                    /* Remove the content type byte from the data length to process. */
                    message_length = message_length - 1;

                    /* Adjust packet length. */
                    decrypted_packet -> nx_packet_length = message_length;

                    /* Increment the sequence number. This is done in the MAC verify
                    step for 1.2 and earlier, but AEAD includes the MAC so we don't
                    check the MAC and need to increment here. */
                    if ((tls_session -> nx_secure_tls_remote_sequence_number[0] + 1) == 0)
                    {
                        /* Check for overflow of the 32-bit unsigned number. */
                        tls_session -> nx_secure_tls_remote_sequence_number[1]++;
                    }
                    tls_session -> nx_secure_tls_remote_sequence_number[0]++;
                }
            }
            else
#endif
            {
                /* Verify the hash MAC in the decrypted record. */
                if (status)
                {

                    /* !!! NOTE - the MAC check MUST always be performed regardless of the error state of
                                  the payload decryption operation. Skipping the MAC check on padding failures
                                  could enable a timing-based attack allowing an attacker to determine whether
                                  padding was valid or not, causing an information leak. */
                    status = _nx_secure_tls_verify_mac(tls_session, header_data, header_length, packet_ptr, record_offset, &message_length);
                }
                else
                {
                    status = _nx_secure_tls_verify_mac(tls_session, header_data, header_length, decrypted_packet, 0, &message_length);

                    if (status == NX_SECURE_TLS_SUCCESS)
                    {

                        /* Adjust packet length. */
                        decrypted_packet -> nx_packet_length = message_length;
                    }
                }
            }

            /* Check to see if decryption or verification failed. */
            if(error_status != NX_SECURE_TLS_SUCCESS)
            {
                /* Decryption failed. */
                return(error_status);
            }

            if (status != NX_SECURE_TLS_SUCCESS)
            {
                /* MAC verification failed. */
                return(status);
            }

            if (message_length > NX_SECURE_TLS_MAX_PLAINTEXT_LENGTH)
            {
                return(NX_SECURE_TLS_RECORD_OVERFLOW);
            }

            /* Trim packet. */
            _nx_secure_tls_packet_trim(decrypted_packet);
        }

        if (message_type != NX_SECURE_TLS_APPLICATION_DATA)
        {

            /* For message other than application data, extract to packet buffer to make sure all data are in contiguous memory. */
            /* Check available area of buffer. */
            packet_data = tls_session -> nx_secure_tls_packet_buffer;
            if (message_length > tls_session -> nx_secure_tls_packet_buffer_size)
            {
                return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
            }

            if (decrypted_packet == NX_NULL)
            {
                status = nx_packet_data_extract_offset(packet_ptr, record_offset,
                                                       tls_session -> nx_secure_tls_packet_buffer,
                                                       message_length, &bytes_copied);
            }
            else
            {
                status = nx_packet_data_extract_offset(decrypted_packet, 0,
                                                       tls_session -> nx_secure_tls_packet_buffer,
                                                       message_length, &bytes_copied);
            }

            if (status || (bytes_copied != message_length))
            {
                return(NX_SECURE_TLS_INVALID_PACKET);
            }
        }

        switch (message_type)
        {
        case NX_SECURE_TLS_CHANGE_CIPHER_SPEC:
            /* Received a ChangeCipherSpec message - from now on all messages from remote host
               will be encrypted using the session keys. */
            _nx_secure_tls_process_changecipherspec(tls_session, packet_data, message_length);

            break;
        case NX_SECURE_TLS_ALERT:
            /* We have received an alert. Check what the alert was and take appropriate action. */
            /* The alert level is the first octet in the alert. The alert number is the second. */
            if(packet_data[0] == NX_SECURE_TLS_ALERT_LEVEL_FATAL)
            {
                /* If we receive a fatal alert, clear all session keys. */
                _nx_secure_tls_session_reset(tls_session);
            }

            /* Save off the alert level and value for the application to access. */
            tls_session->nx_secure_tls_received_alert_level = packet_data[0];
            tls_session->nx_secure_tls_received_alert_value = packet_data[1];
            /* We have received an alert. Check what the alert was and take appropriate action. */
            status = NX_SECURE_TLS_ALERT_RECEIVED;
            break;
        case NX_SECURE_TLS_HANDSHAKE:
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
            /* TLS 1.3 can send post-handshake messages with TLS HANDSHAKE record type. Process those separately. */
            if(tls_session->nx_secure_tls_1_3 && tls_session -> nx_secure_tls_client_state == NX_SECURE_TLS_CLIENT_STATE_HANDSHAKE_FINISHED)
            {
                /* Process post-handshake messages. */
                status = NX_SECURE_TLS_POST_HANDSHAKE_RECEIVED;
                break;
            }
#endif /* (NX_SECURE_TLS_TLS_1_3_ENABLED) */

#ifndef NX_SECURE_TLS_SERVER_DISABLED
            /* The socket is a TLS server, so process incoming handshake messages in that context. */
            if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_SERVER)
            {
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
                if(tls_session->nx_secure_tls_1_3)
                {
                    status = _nx_secure_tls_1_3_server_handshake(tls_session, packet_data, message_length, wait_option);

                    if ((status == NX_SUCCESS) && (tls_session -> nx_secure_tls_1_3 == NX_FALSE))
                    {

                        /* Negotiate a version of TLS prior to TLS 1.3. */
                        /* Handle the ClientHello packet by legacy routine. */
                        status = _nx_secure_tls_server_handshake(tls_session, packet_data, message_length, wait_option);
                    }
                }
                else
#endif
                {
                    status = _nx_secure_tls_server_handshake(tls_session, packet_data, message_length, wait_option);
                }
            }
#endif


#ifndef NX_SECURE_TLS_CLIENT_DISABLED
            /* The socket is a TLS client, so process incoming handshake messages in that context. */
            if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_CLIENT)
            {
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
                if(tls_session->nx_secure_tls_1_3)
                {
                    status = _nx_secure_tls_1_3_client_handshake(tls_session, packet_data, message_length, wait_option);
                    if ((status == NX_SUCCESS) && (tls_session -> nx_secure_tls_1_3 == NX_FALSE))
                    {

                        /* Server negotiates a version of TLS prior to TLS 1.3. */
#ifdef NX_SECURE_TLS_DISABLE_PROTOCOL_VERSION_DOWNGRADE
                        /* Protocol version downgrade is disabled. Return error status. */
                        status = NX_SECURE_TLS_UNSUPPORTED_TLS_VERSION;
#else
                        /* Handle the ServerHello packet by legacy routine. */
                        _nx_secure_tls_handshake_hash_init(tls_session);
                        status = _nx_secure_tls_client_handshake(tls_session, packet_data, message_length, wait_option);
#endif /* NX_SECURE_TLS_DISABLE_PROTOCOL_VERSION_DOWNGRADE */
                    }
                }
                else
#endif
                {
                    status = _nx_secure_tls_client_handshake(tls_session, packet_data, message_length, wait_option);
                }
            }
#endif
            break;
        case NX_SECURE_TLS_APPLICATION_DATA:
            /* The remote host is sending application data records now. Pass decrypted data back
               to the networking API for the application to process. */

            /* First, check for 0-length records. These can be sent but are internal to TLS so tell the
               caller to continue receiving. 0-length records are sent as part of the BEAST attack
               mitigation by some TLS implementations (notably OpenSSL). */
            if (message_length == 0)
            {
                record_offset = record_offset_next;
                status = NX_CONTINUE;
            }
            else
            {
                status = NX_SECURE_TLS_SUCCESS;
            }
            break;
        default:
            /* An unrecognized message was received, likely an error, possibly an attack. */
            status = NX_SECURE_TLS_UNRECOGNIZED_MESSAGE_TYPE;
            break;
        }

#ifdef NX_SECURE_KEY_CLEAR
        if (message_type != NX_SECURE_TLS_APPLICATION_DATA)
        {
            NX_SECURE_MEMSET(packet_data, 0, message_length);
        }
#endif /* NX_SECURE_KEY_CLEAR  */
    }

    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_packet_trim                          PORTABLE C      */
/*                                                           6.0.2        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function trims TLS packet after decryption and hash verify.    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    packet_ptr                            NX_PACKET containing a record */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_secure_tls_packet_release          Release packet                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_process_record         Process TLS records           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-14-2020     Timothy Stapko           Initial Version 6.0.2         */
/*                                                                        */
/**************************************************************************/
static VOID _nx_secure_tls_packet_trim(NX_PACKET *packet_ptr)
{
ULONG payload_length;
ULONG message_length = packet_ptr -> nx_packet_length;
NX_PACKET *current_ptr;

    if (message_length == 0)
    {

        /* Make sure the head packet is not released when the length is zero. */
        if (packet_ptr -> nx_packet_next)
        {

            /* Release packet if remaining data are in chained packet. */
            nx_secure_tls_packet_release(packet_ptr -> nx_packet_next);
            packet_ptr -> nx_packet_next = NX_NULL;
            packet_ptr -> nx_packet_last = packet_ptr;
        }
        packet_ptr -> nx_packet_prepend_ptr = packet_ptr -> nx_packet_data_start;
        packet_ptr -> nx_packet_append_ptr = packet_ptr -> nx_packet_data_start;
        return;
    }

    for (current_ptr = packet_ptr; current_ptr; current_ptr = current_ptr -> nx_packet_next)
    {
        payload_length = (ULONG)(current_ptr -> nx_packet_append_ptr - current_ptr -> nx_packet_prepend_ptr);
        if (message_length < payload_length)
        {

            /* Trim data from this packet. */
            current_ptr -> nx_packet_append_ptr = current_ptr -> nx_packet_prepend_ptr + message_length;
            if (current_ptr -> nx_packet_next)
            {

                /* Release packet if remaining data are in chained packet. */
                nx_secure_tls_packet_release(current_ptr -> nx_packet_next);
                current_ptr -> nx_packet_next = NX_NULL;
                packet_ptr -> nx_packet_last = current_ptr;
            }
            break;
        }
        message_length -= (ULONG)(current_ptr -> nx_packet_append_ptr - current_ptr -> nx_packet_prepend_ptr);
    }
}
