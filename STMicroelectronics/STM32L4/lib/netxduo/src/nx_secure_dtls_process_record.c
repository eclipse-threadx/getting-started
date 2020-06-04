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
/**    Datagram Transport Layer Security (DTLS)                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SECURE_SOURCE_CODE

#include "nx_secure_dtls.h"

#ifdef NX_SECURE_ENABLE_DTLS
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_dtls_process_record                      PORTABLE C      */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes a single DTLS record, handling both         */
/*    handshake and application records. When multiple records are        */
/*    received in a single lower-level network packet (e.g. UDP), the     */
/*    "record_offset" parameter is used to offset into the packet buffer. */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dtls_session                          Pointer to DTLS control block */
/*    packet_ptr                            NX_PACKET containing a record */
/*    record_offset                         Offset of record in packet    */
/*    bytes_processed                       Return for size of packet     */
/*    wait_option                           Control timeout options       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_dtls_client_handshake      DTLS Client state machine     */
/*    _nx_secure_dtls_process_header        Process record header         */
/*    _nx_secure_dtls_server_handshake      DTLS Server state machine     */
/*    _nx_secure_dtls_verify_mac            Verify record MAC checksum    */
/*    _nx_secure_tls_process_changecipherspec                             */
/*                                          Process ChangeCipherSpec      */
/*    _nx_secure_tls_record_payload_decrypt Decrypt record data           */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_session_receive       Receive DTLS data             */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_dtls_process_record(NX_SECURE_DTLS_SESSION *dtls_session, NX_PACKET *packet_ptr,
                                    ULONG record_offset, ULONG *bytes_processed, ULONG wait_option)
{
UINT                   status;
USHORT                 header_length;
UCHAR                  header_data[NX_SECURE_DTLS_RECORD_HEADER_SIZE]; /* DTLS record header is larger than TLS. Allocate enough space for both. */
USHORT                 message_type;
UINT                   message_length;
UCHAR                 *packet_data;
NX_SECURE_TLS_SESSION *tls_session;
UCHAR                  epoch_seq_num[8];

    /* Basic state machine:
     * 1. Process header, which will set the state and return some data.
     * 2. Advance the packet pointer by the size of the header (returned by header processing)
     *    and process the record.
     * 3. Take any actions necessary based on state.
     */

    /* Get a reference to basic TLS state. */
    tls_session = &dtls_session -> nx_secure_dtls_tls_session;

    header_length = NX_SECURE_DTLS_RECORD_HEADER_SIZE;


    /* Process the DTLS record header, which will set the state. */
    status = _nx_secure_dtls_process_header(dtls_session, packet_ptr, record_offset, &message_type, &message_length, header_data, &header_length);

    if (status == NX_SECURE_TLS_OUT_OF_ORDER_MESSAGE || status == NX_SECURE_TLS_INVALID_EPOCH || status == NX_SECURE_TLS_REPEAT_MESSAGE_RECEIVED)
    {
        /* Received an out-of-order message. Ignore it. */
        *bytes_processed = (ULONG)header_length + message_length;

        /* Pretend this record never happened. */
        return(NX_CONTINUE);
    }

    if (status != NX_SECURE_TLS_SUCCESS)
    {
        return(status);
    }

    /* Ignore empty records. */
    if (message_length == 0)
    {
        /* Update the number of bytes we processed. */
        *bytes_processed = (ULONG)header_length + message_length;
        return(NX_CONTINUE);
    }

    if (((ULONG)(packet_ptr -> nx_packet_append_ptr) - (ULONG)(packet_ptr -> nx_packet_prepend_ptr)) <
        ((ULONG)header_length + message_length))
    {

        /* Chained packet is not supported. */
        return(NX_SECURE_TLS_INVALID_PACKET);
    }

    /* Now extract the record. */
    packet_data = &packet_ptr -> nx_packet_prepend_ptr[header_length];

    /*status = nx_packet_data_extract_offset(packet_ptr, header_length + record_offset, packet_data, message_length, &bytes_copied);*/

    /* Update the number of bytes we processed. */
    *bytes_processed = (ULONG)header_length + message_length;

    /* Check for active encryption of incoming records. If encrypted, decrypt before further processing. */
    if (tls_session -> nx_secure_tls_remote_session_active)
    {
        epoch_seq_num[0] = header_data[10];
        epoch_seq_num[1] = header_data[9];
        epoch_seq_num[2] = header_data[8];
        epoch_seq_num[3] = header_data[7];
        epoch_seq_num[4] = header_data[6];
        epoch_seq_num[5] = header_data[5];
        epoch_seq_num[6] = header_data[4];
        epoch_seq_num[7] = header_data[3];

        /* Decrypt the record data. */
        status = _nx_secure_tls_record_payload_decrypt(tls_session, packet_data, &message_length, (ULONG *)epoch_seq_num, (UCHAR)message_type);

        /* Check the MAC hash if we were able to decrypt the record. */
        if (status == NX_SECURE_TLS_SUCCESS)
        {
            /* Verify the hash MAC in the decrypted record. */
            status = _nx_secure_dtls_verify_mac(dtls_session, header_data, header_length, packet_data, &message_length);
        }

        /* Check to see if decryption or verification failed. */
        if (status != NX_SECURE_TLS_SUCCESS)
        {
            return(status);
        }
    }

    switch (message_type)
    {
    case NX_SECURE_TLS_CHANGE_CIPHER_SPEC:
        /* Received a ChangeCipherSpec message - from now on all messages from remote host
           will be encrypted using the session keys. */
        _nx_secure_tls_process_changecipherspec(tls_session, packet_data, message_length);


        /* New epoch if we receive a CCS message. */
        dtls_session -> nx_secure_dtls_remote_epoch = (USHORT)(dtls_session -> nx_secure_dtls_remote_epoch + 1);

        break;
    case NX_SECURE_TLS_ALERT:
        /* We have received an alert. Check what the alert was and take appropriate action. */

        /* Save off the alert level and value for the application to access. */
        tls_session->nx_secure_tls_received_alert_level = packet_data[0];
        tls_session->nx_secure_tls_received_alert_value = packet_data[1];

        status = NX_SECURE_TLS_ALERT_RECEIVED;
        break;
    case NX_SECURE_TLS_HANDSHAKE:
#ifndef NX_SECURE_TLS_SERVER_DISABLED
        /* The socket is a TLS server, so process incoming handshake messages in that context. */
        if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_SERVER)
        {
            status = _nx_secure_dtls_server_handshake(dtls_session, packet_data, wait_option);
        }
#endif
#ifndef NX_SECURE_TLS_CLIENT_DISABLED
        /* The socket is a TLS client, so process incoming handshake messages in that context. */
        if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_CLIENT)
        {
            status = _nx_secure_dtls_client_handshake(dtls_session, packet_data, message_length, wait_option);
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
            status = NX_CONTINUE;
        }
        else
        {

            /* Release the protection before suspending on nx_packet_data_append. */
            tx_mutex_put(&_nx_secure_tls_protection);

            packet_ptr -> nx_packet_append_ptr = packet_ptr -> nx_packet_prepend_ptr;
            packet_ptr -> nx_packet_length = message_length;
            NX_SECURE_MEMMOVE(packet_ptr -> nx_packet_append_ptr, packet_data, message_length);
            packet_ptr -> nx_packet_append_ptr += message_length;
            /* Get the protection after nx_packet_data_append. */
            tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);
        }
        break;
    default:
        /* An unrecognized message was received, likely an error, possibly an attack. */
        status = NX_SECURE_TLS_UNRECOGNIZED_MESSAGE_TYPE;
        break;
    }

    return(status);
}
#endif /* NX_SECURE_ENABLE_DTLS */

