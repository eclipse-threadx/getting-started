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
/*    _nx_secure_dtls_process_header                      PORTABLE C      */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an NX_PACKET data structure, extracting     */
/*    and parsing a DTLS header received from a remote host.              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dtls_session                          Pointer to DTLS control block */
/*    packet_ptr                            Pointer to incoming packet    */
/*    record_offset                         Offset of current record      */
/*    message_type                          Return message type value     */
/*    length                                Return message length value   */
/*    header_data                           Pointer to header to parse    */
/*    header_length                         Length of header data (bytes) */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_check_protocol_version Check incoming TLS version    */
/*    nx_packet_data_extract_offset         Extract data from NX_PACKET   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_process_record        Process DTLS record data      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_dtls_process_header(NX_SECURE_DTLS_SESSION *dtls_session, NX_PACKET *packet_ptr,
                                    ULONG record_offset, USHORT *message_type, UINT *length,
                                    UCHAR *header_data, USHORT *header_length)
{
ULONG                  bytes_copied;
UINT                   status;
USHORT                 protocol_version;
ULONG                  remaining_bytes = NX_SECURE_DTLS_RECORD_HEADER_SIZE;
ULONG                  remote_sequence_number[2];
USHORT                 remote_epoch;
NX_SECURE_TLS_SESSION *tls_session;


    /* Obtain a reference to the internal TLS state for ease of use. */
    tls_session = &dtls_session -> nx_secure_dtls_tls_session;

    *header_length = NX_SECURE_DTLS_RECORD_HEADER_SIZE;

    while (remaining_bytes)
    {

        /* Check the packet. */
        if (packet_ptr == NX_NULL)
        {

            /* There was an error in extracting the header from the supplied packet. */
            return(NX_SECURE_TLS_INVALID_PACKET);
        }

        /* Process the TLS record header, which will set the state. */
        status = nx_packet_data_extract_offset(packet_ptr, record_offset, &header_data[NX_SECURE_DTLS_RECORD_HEADER_SIZE - remaining_bytes],
                                               remaining_bytes, &bytes_copied);

        /* Make sure we actually got a header. */
        if (status != NX_SUCCESS)
        {

            /* There was an error in extracting the header from the supplied packet. */
            return(NX_SECURE_TLS_INVALID_PACKET);
        }

        record_offset = 0;
        remaining_bytes -= bytes_copied;

        packet_ptr = packet_ptr -> nx_packet_queue_next;
    }

    /* Extract message type from packet/record. */
    *message_type = header_data[0];

    /* Extract the protocol version. */
    protocol_version = (USHORT)(((USHORT)header_data[1] << 8) | header_data[2]);

    /* Get the length of the DTLS data. */
    *length = (UINT)(((UINT)header_data[11] << 8) + header_data[12]);

    /* Epoch and sequence number handling.
     * |    2     |         6         |
     * |  Epoch   |  Sequence number  |
     * --------------------------------
     * |    seq[0]    |    seq[1]     |
     * --------------------------------
     * The epoch is at the beginning of the sequence number (first 2 bytes) for
     * network byte ordering. This means that the epoch is in the bottom 2 bytes
     * of the first ULONG of the sequence number. In DTLS, the sequence number
     * is tied to the epoch - epoch and sequence start at 0, but when the epoch
     * is advanced (when the ChangeCipherSpec message is sent), the sequence is
     * reset to 0.
     *
     * To handle the message ordering properly, the epochs must match (the CCS
     * message changes the epoch after being sent or received so new messages
     * should all have the new epoch). When the epochs match, the sequence numbers
     * can be compared. We have 3 cases to handle:
     *  1) The incoming sequence number is less than or equal to the local count.
     *        - This is a retransmission of an earlier message and should be ignored.
     *  2) The incoming sequence is equal to the local count + 1
     *        - This is the valid next message in the sequence
     *  3) The incoming sequence is greater than the local count + 1
     *        - A message was dropped. We need to handle the out-of-order
     *          message, either by ignoring it (and waiting for the retransmission)
     *          or by buffering the message until we get to the proper
     *          sequence number.
     */


    /* Get epoch and sequence number from header. */
    NX_SECURE_MEMCPY((UCHAR *)&remote_sequence_number[0], &header_data[3], 4);
    NX_SECURE_MEMCPY((UCHAR *)&remote_sequence_number[1], &header_data[7], 4);

    /* Swap endianness for comparisons. */
    NX_CHANGE_ULONG_ENDIAN(remote_sequence_number[0]);
    NX_CHANGE_ULONG_ENDIAN(remote_sequence_number[1]);
    NX_CHANGE_ULONG_ENDIAN(tls_session -> nx_secure_tls_remote_sequence_number[0]);
    NX_CHANGE_ULONG_ENDIAN(tls_session -> nx_secure_tls_remote_sequence_number[1]);

    /* The remote epoch is the top 2 bytes of the incoming 8-byte sequence number. */
    remote_epoch = (USHORT)(remote_sequence_number[0] >> 16);

    /* Clear out epoch bytes in sequence number before comparing. */
    remote_sequence_number[0] = (remote_sequence_number[0] & 0x0000FFFF);

    /* If the epochs do not match, then ignore. */
    if (remote_epoch != dtls_session -> nx_secure_dtls_remote_epoch)
    {
        NX_CHANGE_ULONG_ENDIAN(tls_session -> nx_secure_tls_remote_sequence_number[0]);
        NX_CHANGE_ULONG_ENDIAN(tls_session -> nx_secure_tls_remote_sequence_number[1]);
        return(NX_SECURE_TLS_INVALID_EPOCH);
    }

    /* Now check the sequence number against what we already received to see if it is a new message
       or a retransmission from the remote host. */
    if (remote_sequence_number[0] > 0 || remote_sequence_number[1] > 0)
    {
        if (remote_sequence_number[0] < tls_session -> nx_secure_tls_remote_sequence_number[0] ||
            remote_sequence_number[1] < tls_session -> nx_secure_tls_remote_sequence_number[1])
        {
            /* We already saw this message! */
            NX_CHANGE_ULONG_ENDIAN(tls_session -> nx_secure_tls_remote_sequence_number[0]);
            NX_CHANGE_ULONG_ENDIAN(tls_session -> nx_secure_tls_remote_sequence_number[1]);
            return(NX_SECURE_TLS_REPEAT_MESSAGE_RECEIVED);
        }
        else if (remote_sequence_number[1] != (tls_session -> nx_secure_tls_remote_sequence_number[1] + 1))
        {
            /* Out-of-order message - we missed a datagram! */
            NX_CHANGE_ULONG_ENDIAN(tls_session -> nx_secure_tls_remote_sequence_number[0]);
            NX_CHANGE_ULONG_ENDIAN(tls_session -> nx_secure_tls_remote_sequence_number[1]);
            return(NX_SECURE_TLS_OUT_OF_ORDER_MESSAGE);
        }
        else
        {
            /* Update the current sequence number to match what we just received. */
            tls_session -> nx_secure_tls_remote_sequence_number[0] = remote_sequence_number[0];
            tls_session -> nx_secure_tls_remote_sequence_number[1] = remote_sequence_number[1];

            /* Swap back now that comparisons are done. */
            NX_CHANGE_ULONG_ENDIAN(tls_session -> nx_secure_tls_remote_sequence_number[0]);
            NX_CHANGE_ULONG_ENDIAN(tls_session -> nx_secure_tls_remote_sequence_number[1]);
        }
    }
    else if (remote_epoch == 0)
    {

#ifndef NX_SECURE_TLS_CLIENT_DISABLED
        if ((tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_CLIENT) &&
            (tls_session -> nx_secure_tls_client_state != NX_SECURE_TLS_CLIENT_STATE_IDLE))
        {

            /* Invalid sequence number. */
            NX_CHANGE_ULONG_ENDIAN(tls_session -> nx_secure_tls_remote_sequence_number[0]);
            NX_CHANGE_ULONG_ENDIAN(tls_session -> nx_secure_tls_remote_sequence_number[1]);
            return(NX_SECURE_TLS_OUT_OF_ORDER_MESSAGE);
        }
#endif /* NX_SECURE_TLS_CLIENT_DISABLED */

#ifndef NX_SECURE_TLS_SERVER_DISABLED
        if ((tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_SERVER) &&
            (tls_session -> nx_secure_tls_server_state != NX_SECURE_TLS_SERVER_STATE_IDLE))
        {

            /* Invalid sequence number. */
            NX_CHANGE_ULONG_ENDIAN(tls_session -> nx_secure_tls_remote_sequence_number[0]);
            NX_CHANGE_ULONG_ENDIAN(tls_session -> nx_secure_tls_remote_sequence_number[1]);
            return(NX_SECURE_TLS_OUT_OF_ORDER_MESSAGE);
        }
#endif /* NX_SECURE_TLS_SERVER_DISABLED */
    }

    /* Check the protocol version, except when we haven't established a version yet */
    if (tls_session -> nx_secure_tls_protocol_version != 0)
    {
        /* Check the record's protocol version against the current session. */
        status = _nx_secure_tls_check_protocol_version(tls_session, protocol_version, NX_SECURE_DTLS);
        return(status);
    }

    return(NX_SUCCESS);
}
#endif /* NX_SECURE_ENABLE_DTLS */

