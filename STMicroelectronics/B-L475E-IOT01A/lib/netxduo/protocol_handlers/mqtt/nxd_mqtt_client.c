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
/** NetX Component                                                        */
/**                                                                       */
/**   MQTT (MQTT)                                                         */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NXD_MQTT_CLIENT_SOURCE_CODE


/* Force error checking to be disabled in this module */

#ifndef NX_DISABLE_ERROR_CHECKING
#define NX_DISABLE_ERROR_CHECKING
#endif


/* Include necessary system files.  */

#include    "tx_api.h"
#include    "nx_api.h"
#include    "nx_ip.h"
#include    "nxd_mqtt_client.h"

/* Bring in externals for caller checking code.  */

#define MQTT_ALL_EVENTS               ((ULONG)0xFFFFFFFF)
#define MQTT_TIMEOUT_EVENT            ((ULONG)0x00000001)
#define MQTT_PACKET_RECEIVE_EVENT     ((ULONG)0x00000002)
#define MQTT_DELETE_EVENT             ((ULONG)0x00000008)
#define MQTT_PING_TIMEOUT_EVENT       ((ULONG)0x00000010)
#define MQTT_NETWORK_DISCONNECT_EVENT ((ULONG)0x00000020)

static UINT _nxd_mqtt_packet_allocate(NXD_MQTT_CLIENT *client_ptr, NX_PACKET **packet_ptr);
static UINT append_message(NX_PACKET *packet_ptr, CHAR *message, UINT length);

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_set_remaining_length                      PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function writes the remaining length filed in the outgoing     */
/*    MQTT packet.                                                        */
/*                                                                        */
/*    This function follows the logic outlined in 2.2.3 in MQTT           */
/*    specification                                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    packet_ptr                            Outgoing MQTT packet.         */
/*    length                                remaining length in bytes,    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    length                                Number of bytes written       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_process_publish                                           */
/*    _nxd_mqtt_client_message_get                                        */
/*    _nxd_mqtt_process_sub_unsub_ack                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nxd_mqtt_set_remaining_length(NX_PACKET *packet_ptr, UINT length)
{
UCHAR *byte = packet_ptr -> nx_packet_append_ptr;
UINT   count = 0;

    do
    {
        if (length & 0xFFFFFF80)
        {
            *(byte + count) = (UCHAR)((length & 0x7F) | 0x80);
        }
        else
        {
            *(byte + count) = length & 0x7F;
        }
        length = length >> 7;

        count++;
    } while (length != 0);

    packet_ptr -> nx_packet_append_ptr = packet_ptr -> nx_packet_append_ptr + count;
    packet_ptr -> nx_packet_length += count;

    return(count);
}



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_read_remaining_length                     PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses the remaining length filed in the incoming     */
/*    MQTT packet.                                                        */
/*                                                                        */
/*    This function follows the logic outlined in 2.2.3 in MQTT           */
/*    specification                                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    packet_ptr                            Incoming MQTT packet.         */
/*    remaining_length                      remaining length in bytes,    */
/*                                            this is the return value.   */
/*    variable_header                       Start position of the         */
/*                                            variable header.            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_process_publish                                           */
/*    _nxd_mqtt_client_message_get                                        */
/*    _nxd_mqtt_process_sub_unsub_ack                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nxd_mqtt_read_remaining_length(NX_PACKET *packet_ptr, UINT *remaining_length, UCHAR **variable_header)
{
UINT   value = 0;
UCHAR *byte = packet_ptr -> nx_packet_prepend_ptr;
UINT   multiplier = 1;
UINT   byte_count = 0;

    do
    {
        if (byte_count >= 4)
        {
            return(NXD_MQTT_INTERNAL_ERROR);
        }
        byte++;
        value += (((*byte) & 0x7F) * multiplier);
        multiplier = multiplier << 7;
        byte_count++;
    } while ((*byte) & 0x80);

    if (((ULONG)packet_ptr -> nx_packet_append_ptr - ((ULONG)byte + 1)) < value)
    {

        /* Remaining length is larger than packet size. */
        return(NXD_MQTT_INTERNAL_ERROR);
    }

    if (remaining_length)
    {
        *remaining_length = value;
    }

    if (variable_header)
    {
        *variable_header = (byte + 1);
    }

    return(NXD_MQTT_SUCCESS);
}



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_sub_unsub                          PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends a subscribe or unsubscribe message to the       */
/*    broker.                                                             */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    op                                    Subscribe or Unsubscribe      */
/*    topic_name                            Pointer to the topic string   */
/*                                            to subscribe to             */
/*    topic_name_length                     Length of the topic string    */
/*                                            in bytes                    */
/*    QoS                                   Expected QoS level            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                                                        */
/*    _nxd_mqtt_packet_allocate                                           */
/*    append_message                                                      */
/*    tx_mutex_put                                                        */
/*    nx_tcp_socket_send                                                  */
/*    nx_secure_tls_session_send                                          */
/*    nx_packet_release                                                   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_client_subscribe                                          */
/*    _nxd_mqtt_client_unsubscribe                                        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comments, added      */
/*                                            logic to prevent packet id  */
/*                                            value from being zero, fixed*/
/*                                            a potential packet leak     */
/*                                            under error conditions,     */
/*                                            supported user defined      */
/*                                            memory functions, supported */
/*                                            user defined memory         */
/*                                            functions, checked topic    */
/*                                            name length in runtime,     */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nxd_mqtt_client_sub_unsub(NXD_MQTT_CLIENT *client_ptr, UINT op,
                                CHAR *topic_name, UINT topic_name_length, UINT QoS)
{


NX_PACKET          *packet_ptr;
UINT                status;
UINT                length = 0;
UINT                ret = NXD_MQTT_SUCCESS;
MQTT_MESSAGE_BLOCK *message_block;
UCHAR              *byte;

    /* Validate topic_name */
    if (topic_name_length > NXD_MQTT_MAX_TOPIC_NAME_LENGTH)
    {
        return(NXD_MQTT_INVALID_PARAMETER);
    }

    /* Obtain the mutex. */
    status = tx_mutex_get(&client_ptr -> nxd_mqtt_protection, NX_WAIT_FOREVER);

    if (status != TX_SUCCESS)
    {
        return(NXD_MQTT_MUTEX_FAILURE);
    }

    /* Do nothing if the client is already connected. */
    if (client_ptr -> nxd_mqtt_client_state != NXD_MQTT_CLIENT_STATE_CONNECTED)
    {
        tx_mutex_put(&client_ptr -> nxd_mqtt_protection);
        return(NXD_MQTT_NOT_CONNECTED);
    }

    status = _nxd_mqtt_packet_allocate(client_ptr, &packet_ptr);
    if (status)
    {
        tx_mutex_put(&client_ptr -> nxd_mqtt_protection);
        return(status);
    }

    byte = packet_ptr -> nx_packet_prepend_ptr;

    *byte = (UCHAR)op;
    byte++;
    packet_ptr -> nx_packet_append_ptr = packet_ptr -> nx_packet_prepend_ptr + 1;
    packet_ptr -> nx_packet_length = 1;

    /* Compute the remaining length field, starting with 2 bytes of packet ID */
    length = 2;

    /* Count the topic. */
    length += (2 + topic_name_length);

    if (op == ((MQTT_CONTROL_PACKET_TYPE_SUBSCRIBE << 4) | 0x02))
    {
        /* Count one byte for QoS */
        length++;
    }

    if (length > ((ULONG)(packet_ptr -> nx_packet_data_end) - (ULONG)(packet_ptr -> nx_packet_append_ptr)))
    {
        nx_packet_release(packet_ptr);
        tx_mutex_put(&client_ptr -> nxd_mqtt_protection);

        /* Packet buffer is too small to hold the message. */
        return(NX_SIZE_ERROR);
    }

    /* Write out the remaining length field. */
    _nxd_mqtt_set_remaining_length(packet_ptr, length);

    byte = packet_ptr -> nx_packet_append_ptr;

    *byte = (UCHAR)(client_ptr -> nxd_mqtt_client_packet_identifier >> 8);
    *(byte + 1) = (client_ptr -> nxd_mqtt_client_packet_identifier &  0xFF);

    packet_ptr -> nx_packet_append_ptr += 2;
    packet_ptr -> nx_packet_length += 2;


    /* Append topic name */
    append_message(packet_ptr, topic_name, topic_name_length);

    byte = packet_ptr -> nx_packet_append_ptr;
    if (op == ((MQTT_CONTROL_PACKET_TYPE_SUBSCRIBE << 4) | 0x02))
    {
        /* Fill in QoS value. */
        *byte = QoS & 0x3;
        packet_ptr -> nx_packet_append_ptr++;
        packet_ptr -> nx_packet_length++;
    }

    if (client_ptr -> message_block_free_list == NX_NULL)
    {
        /* Release the mutex. */
        tx_mutex_put(&client_ptr -> nxd_mqtt_protection);

        /* Release the packet. */
        nx_packet_release(packet_ptr);

        return(NXD_MQTT_INTERNAL_ERROR);
    }

    message_block = client_ptr -> message_block_free_list;

    client_ptr -> message_block_free_list = message_block -> next;

    message_block -> fixed_header = (UCHAR)op;
    message_block -> remaining_length = length;
    message_block -> packet_id = (USHORT)(client_ptr -> nxd_mqtt_client_packet_identifier);
    NXD_MQTT_SECURE_MEMCPY(message_block -> topic_name, topic_name, topic_name_length);
    message_block -> topic_name_length = (USHORT)topic_name_length;
    message_block -> message_length = 0;
    message_block -> next = NX_NULL;
    if (client_ptr -> message_transmit_queue_head == NX_NULL)
    {
        client_ptr -> message_transmit_queue_head = message_block;
    }
    else
    {
        client_ptr -> message_transmit_queue_tail -> next = message_block;
    }
    client_ptr -> message_transmit_queue_tail = message_block;

    client_ptr -> nxd_mqtt_client_packet_identifier = (client_ptr -> nxd_mqtt_client_packet_identifier + 1) & 0xFFFF;

    /* Prevent packet identifier from being zero. MQTT-2.3.1-1 */
    if(client_ptr -> nxd_mqtt_client_packet_identifier == 0)
        client_ptr -> nxd_mqtt_client_packet_identifier = 1;

    /* Update the timeout value. */
    client_ptr -> nxd_mqtt_timeout = tx_time_get() + client_ptr -> nxd_mqtt_keepalive;

    /* Release the mutex. */
    tx_mutex_put(&client_ptr -> nxd_mqtt_protection);

    /* Ready to send the connect message to the server. */
#ifdef NX_SECURE_ENABLE
    if (client_ptr -> nxd_mqtt_client_use_tls)
    {
        status = nx_secure_tls_session_send(&(client_ptr -> nxd_mqtt_tls_session), packet_ptr, NX_WAIT_FOREVER);
    }
    else
    {
        status = nx_tcp_socket_send(&client_ptr -> nxd_mqtt_client_socket, packet_ptr, NX_WAIT_FOREVER);
    }
#else
    status = nx_tcp_socket_send(&client_ptr -> nxd_mqtt_client_socket, packet_ptr, NX_WAIT_FOREVER);
#endif /* NX_SECURE_ENABLE */


    if (status)
    {
        /* Release the packet. */
        nx_packet_release(packet_ptr);

        ret = NXD_MQTT_COMMUNICATION_FAILURE;
    }

    return(ret);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_packet_allocate                           PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function allocates a packet for trnasmtting MQTT message.      */
/*    Special care has to be taken for accomondating IPv4/IPv6 header,    */
/*    and possibly TLS record if TLS is being used. On failure, the       */
/*    TLS mutex is released and the caller can simply return.             */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    packet_ptr                            Allocated packet to be        */
/*                                            returned to the caller.     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_secure_tls_packet_allocate         Allocate packet for MQTT      */
/*                                            over TLS socket             */
/*    nx_packet_allocate                    Allocate a pakcet for MQTT    */
/*                                            over regular TCP socket     */
/*    tx_mutex_put                          Release a mutex               */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_process_publish                                           */
/*    _nxd_mqtt_client_connect                                            */
/*    _nxd_mqtt_client_publish                                            */
/*    _nxd_mqtt_client_subscribe                                          */
/*    _nxd_mqtt_client_unsubscribe                                        */
/*    _nxd_mqtt_client_send_simple_message                                */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comments, and        */
/*                                            used TLS API to allocate    */
/*                                            packet for TLS session,     */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
static UINT _nxd_mqtt_packet_allocate(NXD_MQTT_CLIENT *client_ptr, NX_PACKET **packet_ptr)
{
UINT status = NXD_MQTT_SUCCESS;

#ifdef NX_SECURE_ENABLE
    if (client_ptr -> nxd_mqtt_client_use_tls)
    {
        /* Use TLS packet allocate.  The TLS packet allocate is able to count for 
           TLS-related header space including crypto initial vector area. */
        status = nx_secure_tls_packet_allocate(&client_ptr -> nxd_mqtt_tls_session, client_ptr -> nxd_mqtt_client_packet_pool_ptr,
                                               packet_ptr, TX_WAIT_FOREVER);
    }
    /* Allocate a packet  */
    else
    {
#endif
        if (client_ptr -> nxd_mqtt_client_socket.nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V4)
        {
            status = nx_packet_allocate(client_ptr -> nxd_mqtt_client_packet_pool_ptr, packet_ptr, NX_IPv4_TCP_PACKET,
                                        TX_WAIT_FOREVER);
        }
        else
        {
            status = nx_packet_allocate(client_ptr -> nxd_mqtt_client_packet_pool_ptr, packet_ptr, NX_IPv6_TCP_PACKET,
                                        TX_WAIT_FOREVER);
        }
#ifdef NX_SECURE_ENABLE
    }
#endif

    if (status != NX_SUCCESS)
    {

        tx_mutex_put(&client_ptr -> nxd_mqtt_protection);
        return(NXD_MQTT_PACKET_POOL_FAILURE);
    }
    return(NXD_MQTT_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_receive_callback                          PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function is installed as TCP receive callback         */
/*    function.  On receiving a TCP message, the callback function        */
/*    sets an evant flag to trigger MQTT client to process received       */
/*    message.                                                            */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            The socket that receives      */
/*                                           the message.                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_event_flags_set                                                  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_thread_entry                                              */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            removed the usage of global */
/*                                            variable for MQTT client,   */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
static VOID _nxd_mqtt_receive_callback(NX_TCP_SOCKET *socket_ptr)
{
NXD_MQTT_CLIENT *client_ptr;

    client_ptr = (NXD_MQTT_CLIENT *)socket_ptr -> nx_tcp_socket_reserved_ptr;

    if (&(client_ptr -> nxd_mqtt_client_socket) == socket_ptr)
    {
        /* Set the event flag. */
        tx_event_flags_set(&client_ptr -> nxd_mqtt_events, MQTT_PACKET_RECEIVE_EVENT, TX_OR);
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_release_transmit_message_block            PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function releases a transmit message block.           */
/*    A messasge block is allocated to store QoS 1 and 2 messages.        */
/*    Upon a message being properly acknowledged, the message block can   */
/*    be released.                                                        */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    message_block                         Pointer to the MQTT message   */
/*                                            block to be removed         */
/*    previous_block                        Pointer to the previous block */
/*                                            or NULL if none exists      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_thread_entry                                              */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
static void _nxd_mqtt_release_transmit_message_block(NXD_MQTT_CLIENT *client_ptr, MQTT_MESSAGE_BLOCK *message_block, MQTT_MESSAGE_BLOCK *previous_block)
{

    if (previous_block)
    {
        previous_block -> next = message_block -> next;
    }
    else
    {
        client_ptr -> message_transmit_queue_head = message_block -> next;
    }

    if (message_block == client_ptr -> message_transmit_queue_tail)
    {
        client_ptr -> message_transmit_queue_tail = previous_block;
    }

    message_block -> next = client_ptr -> message_block_free_list;
    client_ptr -> message_block_free_list = message_block;

    return;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_release_receive_message_block             PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function releases a receive message block.            */
/*    A messasge block is allocated to store QoS 1 and 2 messages.        */
/*    Upon a message being properly acknowledged, the message block can   */
/*    be released.                                                        */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    message_block                         Pointer to the MQTT message   */
/*                                            block to be removed         */
/*    previous_block                        Pointer to the previous block */
/*                                            or NULL if none exists      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_thread_entry                                              */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
static VOID _nxd_mqtt_release_receive_message_block(NXD_MQTT_CLIENT *client_ptr, MQTT_MESSAGE_BLOCK *message_block, MQTT_MESSAGE_BLOCK *previous_block)
{

    if (previous_block)
    {
        previous_block -> next = message_block -> next;
    }
    else
    {
        client_ptr -> message_receive_queue_head = message_block -> next;
    }

    if (message_block == client_ptr -> message_receive_queue_tail)
    {
        client_ptr -> message_receive_queue_tail = previous_block;
    }

    client_ptr -> message_receive_queue_depth--;

    message_block -> next = client_ptr -> message_block_free_list;
    client_ptr -> message_block_free_list = message_block;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_process_publish                           PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function process a publish message from the broker.   */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    packet_ptr                            Pointer to the packet         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Status                                                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [receive_notify]                      User supplied receive         */
/*                                            callback function           */
/*    _nxd_mqtt_packet_allocate                                           */
/*    nx_tcp_socket_send                                                  */
/*    nx_packet_release                                                   */
/*    nx_secure_tls_session_send                                          */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_packet_receive_process                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            supported user defined      */
/*                                            memory functions,           */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s), and      */
/*                                            improved internal logic,    */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
static VOID _nxd_mqtt_process_publish(NXD_MQTT_CLIENT *client_ptr, NX_PACKET *packet_ptr)
{
MQTT_PACKET_PUBLISH          *publish_ptr;
MQTT_PACKET_PUBLISH_RESPONSE *pubresp_ptr;
UCHAR                        *data = NX_NULL;
UINT                          status;
USHORT                        packet_id = 0;
UCHAR                         QoS;
UINT                          enqueue_message = 0;
MQTT_MESSAGE_BLOCK           *message_block;
MQTT_MESSAGE_BLOCK           *previous_block = NX_NULL;
MQTT_MESSAGE_BLOCK           *message_ptr;
UINT                          remaining_length = 0;
UINT                          qos_value;
UCHAR                        *data_start;
UCHAR                        *topic_start = NX_NULL;
UINT                          topic_length;
UINT                          copy_length;


    publish_ptr = (MQTT_PACKET_PUBLISH *)(packet_ptr -> nx_packet_prepend_ptr);
    QoS = (UCHAR)((publish_ptr -> mqtt_publish_packet_header & MQTT_PUBLISH_QOS_LEVEL_FIELD) >> 1);

    status = _nxd_mqtt_read_remaining_length(packet_ptr, &remaining_length, &data);

    if (status)
    {
        return;
    }

    /* Skip topic.  The 1st 2 bytes are length of the topic. */
    data = data + 2 + (((*data) << 8) | (*(data + 1)));


    if (QoS == 0)
    {
        enqueue_message = 1;
    }
    else
    {
        /* QoS 1 or QoS 2 messages. */

        packet_id = (USHORT)(((*data) << 8) | (*(data + 1)));

        /* Look for an existing message block with the same packet id */
        message_block = client_ptr -> message_transmit_queue_head;

        while (message_block)
        {
            if ((message_block -> packet_id == packet_id) &&
                ((message_block -> fixed_header & (MQTT_CONTROL_PACKET_TYPE_PUBACK << 4)) || /* Response to PUBLISH with QoS 1 */
                 (message_block -> fixed_header & (MQTT_CONTROL_PACKET_TYPE_PUBREC << 4)) || /* Response to PUBLISH with QoS 2 */
                 (message_block -> fixed_header & (MQTT_CONTROL_PACKET_TYPE_PUBCOMP << 4))))
            {
                /* Found a message block containing the packet_id */
                break;
            }
            message_block = message_block -> next;
        }

        if (message_block)
        {
            /* This published data is already in our system.  No need to deliver this message to the application. */
            enqueue_message = 0;
        }
        else
        {
            enqueue_message = 1;
        }
    }

    if (enqueue_message)
    {
        if (client_ptr -> message_block_free_list == NX_NULL)
        {
            return;
        }

        /* Increment the queue depth counter. */
        client_ptr -> message_receive_queue_depth++;

        message_ptr = client_ptr -> message_block_free_list;
        client_ptr -> message_block_free_list = message_ptr -> next;

        if (client_ptr -> message_receive_queue_head == NX_NULL)
        {
            client_ptr -> message_receive_queue_head = message_ptr;
        }
        else
        {
            client_ptr -> message_receive_queue_tail -> next = message_ptr;
        }
        client_ptr -> message_receive_queue_tail = message_ptr;

        qos_value = ((*packet_ptr -> nx_packet_prepend_ptr) >> 1) & 0x3;

        if (_nxd_mqtt_read_remaining_length(packet_ptr, &remaining_length, &topic_start))
        {
            return;
        }

        if (remaining_length < 2)
        {
            return;
        }

        topic_length = (UINT)(*(topic_start) << 8) | (*(topic_start + 1));

        if (topic_length > remaining_length - 2u)
        {
            return;
        }

        if (topic_length < NXD_MQTT_MAX_TOPIC_NAME_LENGTH)
        {
            copy_length = topic_length;
        }
        else
        {
            copy_length = NXD_MQTT_MAX_TOPIC_NAME_LENGTH;
        }
        NXD_MQTT_SECURE_MEMCPY((VOID *)message_ptr -> topic_name, (VOID *)(topic_start + 2), copy_length);
        message_ptr -> topic_name_length = (USHORT)copy_length;

        remaining_length = remaining_length - topic_length - 2;
        if ((qos_value == 1) || (qos_value == 2))
        {
            data_start = topic_start + 2 + 2 + topic_length;

            if (remaining_length < 2)
            {
                return;
            }
            remaining_length = remaining_length - 2;
        }
        else
        {
            data_start = topic_start + 2 + topic_length;
        }

        if (remaining_length < NXD_MQTT_MAX_MESSAGE_LENGTH)
        {
            copy_length = remaining_length;
        }
        else
        {
            copy_length = NXD_MQTT_MAX_MESSAGE_LENGTH;
        }
        NXD_MQTT_SECURE_MEMCPY(message_ptr -> message, data_start, copy_length);
        message_ptr -> message_length = (USHORT)copy_length;
        message_ptr -> next = NX_NULL;

        /* Invoke the user-defined receive notify function if it is set. */
        if (client_ptr -> nxd_mqtt_client_receive_notify)
        {
            (*(client_ptr -> nxd_mqtt_client_receive_notify))(client_ptr, client_ptr -> message_receive_queue_depth);
        }
    }

    /* If the message QoS level is 0, we are done. */
    if (QoS == 0)
    {
        /* Return */
        return;
    }

    /* Send out proper ACKs for QoS 1 and 2 messages. */
    /* Allocate a new packet so we can send out a response. */
    status = _nxd_mqtt_packet_allocate(client_ptr, &packet_ptr);
    if (status)
    {
        /* Packet allocation fails. */
        return;
    }

    /* Fill in the packet ID */
    pubresp_ptr = (MQTT_PACKET_PUBLISH_RESPONSE *)(packet_ptr -> nx_packet_prepend_ptr);
    pubresp_ptr -> mqtt_publish_response_packet_remaining_length = 2;
    pubresp_ptr -> mqtt_publish_response_packet_packet_identifier_msb = (UCHAR)(packet_id >> 8);
    pubresp_ptr -> mqtt_publish_response_packet_packet_identifier_lsb = (UCHAR)(packet_id & 0xFF);

    if ((publish_ptr -> mqtt_publish_packet_header & MQTT_PUBLISH_QOS_LEVEL_FIELD) == MQTT_PUBLISH_QOS_LEVEL_1)
    {

        pubresp_ptr -> mqtt_publish_response_packet_header = MQTT_CONTROL_PACKET_TYPE_PUBACK << 4;
    }
    else
    {
        pubresp_ptr -> mqtt_publish_response_packet_header = MQTT_CONTROL_PACKET_TYPE_PUBREC << 4;
    }

    packet_ptr -> nx_packet_append_ptr = packet_ptr -> nx_packet_prepend_ptr + sizeof(MQTT_PACKET_PUBLISH_RESPONSE);
    packet_ptr -> nx_packet_length = sizeof(MQTT_PACKET_PUBLISH_RESPONSE);

    message_block = client_ptr -> message_block_free_list;
    if (message_block)
    {
        client_ptr -> message_block_free_list = message_block -> next;

        NXD_MQTT_SECURE_MEMSET(message_block, 0, sizeof(MQTT_MESSAGE_BLOCK));

        message_block -> fixed_header =  pubresp_ptr -> mqtt_publish_response_packet_header;
        message_block -> packet_id = packet_id;

        if (client_ptr -> message_transmit_queue_head == NX_NULL)
        {
            client_ptr -> message_transmit_queue_head = message_block;
        }
        else
        {
            previous_block = client_ptr -> message_transmit_queue_tail;
            client_ptr -> message_transmit_queue_tail -> next = message_block;
        }

        client_ptr -> message_transmit_queue_tail = message_block;
    }

    tx_mutex_put(&client_ptr -> nxd_mqtt_protection);
#ifdef NX_SECURE_ENABLE
    if (client_ptr -> nxd_mqtt_client_use_tls)
    {
        status = nx_secure_tls_session_send(&(client_ptr -> nxd_mqtt_tls_session), packet_ptr, NX_WAIT_FOREVER);
    }
    else
    {
        status = nx_tcp_socket_send(&client_ptr -> nxd_mqtt_client_socket, packet_ptr, NX_WAIT_FOREVER);
    }
#else
    status = nx_tcp_socket_send(&client_ptr -> nxd_mqtt_client_socket, packet_ptr, NX_WAIT_FOREVER);

#endif /* NX_SECURE_ENABLE */

    tx_mutex_get(&client_ptr -> nxd_mqtt_protection, TX_WAIT_FOREVER);
    if (status)
    {

        /* Release the puback packet. */
        nx_packet_release(packet_ptr);

        /* Nothing to be done.  The QoS 1/2 response message is not transmitted, which
           will be retransmitted. */
    }
    else
    {
        /* Update the timeout value. */
        client_ptr -> nxd_mqtt_timeout = tx_time_get() + client_ptr -> nxd_mqtt_keepalive;

        if (QoS == 1 && message_block)
        {
            /* Now the response is sent, QoS level 1 message information can be removed. */
            if (message_block == client_ptr -> message_transmit_queue_head)
            {
                client_ptr -> message_transmit_queue_head = message_block -> next;

                if (client_ptr -> message_transmit_queue_tail == message_block)
                {
                    client_ptr -> message_transmit_queue_tail = client_ptr -> message_transmit_queue_head;
                }
            }
            else
            {

                previous_block = client_ptr -> message_transmit_queue_head;

                while (previous_block)
                {
                    if (previous_block -> next == message_block)
                    {
                        previous_block -> next = message_block -> next;
                        if (client_ptr -> message_transmit_queue_tail == message_block)
                        {
                            client_ptr -> message_transmit_queue_tail = previous_block;
                        }
                        break;
                    }
                    else
                    {
                        previous_block = previous_block -> next;
                    }
                }
            }

            message_block -> next = client_ptr -> message_block_free_list;
            client_ptr -> message_block_free_list = message_block;
        }
    }

    /* Return */
    return;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_process_publish_response                  PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function process a publish response messages.         */
/*    Publish Response messages are: PUBACK, PUBREC, PUBREL               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    packet_ptr                            Pointer to the packet         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Status                                                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nxd_mqtt_client_receive_notify]      User supplied publish         */
/*                                            callback function           */
/*    _nxd_mqtt_release_transmit_message_block                            */
/*    nx_tcp_socket_send                                                  */
/*    nx_secure_tls_session_send                                          */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_packet_receive_process                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
static UINT _nxd_mqtt_process_publish_response(NXD_MQTT_CLIENT *client_ptr, NX_PACKET *packet_ptr)
{
MQTT_PACKET_PUBLISH_RESPONSE *response_ptr;
UINT                          ret = 1; /* By default notify the caller to release the packet. */
USHORT                        packet_id;
MQTT_MESSAGE_BLOCK           *message_block, *previous_block;
NX_PACKET                    *response_packet;

    response_ptr = (MQTT_PACKET_PUBLISH_RESPONSE *)(packet_ptr -> nx_packet_prepend_ptr);

    /* Validate the packet. */
    if (response_ptr -> mqtt_publish_response_packet_remaining_length != 2)
    {
        /* Invalid remaining_length value. Return 1 so the caller can release
           the packet. */

        return(1);
    }

    packet_id = (USHORT)((response_ptr -> mqtt_publish_response_packet_packet_identifier_msb << 8) |
                         (response_ptr -> mqtt_publish_response_packet_packet_identifier_lsb));

    /* Search all the outstanding transmitted message blocks for a match. */
    previous_block = NX_NULL;
    message_block = client_ptr -> message_transmit_queue_head;
    while (message_block)
    {
        if (message_block -> packet_id == packet_id)
        {
            /* Found the matching packet id */
            if (((response_ptr -> mqtt_publish_response_packet_header) >> 4) == MQTT_CONTROL_PACKET_TYPE_PUBACK)
            {

                /* PUBACK is the response to a PUBLISH packet with QoS Levle 1*/
                /* Therefore we verify that message block contains PUBLISH packet with QoS level 1*/
                if ((message_block -> fixed_header & 0xF6) == ((MQTT_CONTROL_PACKET_TYPE_PUBLISH << 4) | MQTT_PUBLISH_QOS_LEVEL_1))
                {
                    /* QoS Level1 message receives an ACK. */
                    /* This message can be released. */
                    _nxd_mqtt_release_transmit_message_block(client_ptr, message_block, previous_block);
                }
                /* Return with value 1, so the caller will release packet_ptr */
                return(1);
            }
            else if (((response_ptr -> mqtt_publish_response_packet_header) >> 4) == MQTT_CONTROL_PACKET_TYPE_PUBREC)
            {
                /* QoS 2 publish received, part 1. */
                /* Therefore we verify that message block contains PUBLISH packet with QoS level 2*/
                if ((message_block -> fixed_header & (MQTT_CONTROL_PACKET_TYPE_FIELD | MQTT_PUBLISH_QOS_LEVEL_FIELD)) ==
                    (((MQTT_CONTROL_PACKET_TYPE_PUBLISH << 4) | MQTT_PUBLISH_QOS_LEVEL_2)))
                {
                    /* QoS Level1 message receives an ACK. */
                    /* This message can be released. */
                    /* Send PUBREL */
                    message_block -> fixed_header = (UCHAR)((message_block -> fixed_header & 0x0F) | (MQTT_CONTROL_PACKET_TYPE_PUBREC << 4));

                    /* Allocate a packet to send the response. */
                    ret = _nxd_mqtt_packet_allocate(client_ptr, &response_packet);
                    if (ret)
                    {
                        return(1);
                    }

                    if (4u > ((ULONG)(response_packet -> nx_packet_data_end) - (ULONG)(response_packet -> nx_packet_append_ptr)))
                    {
                        nx_packet_release(response_packet);

                        /* Packet buffer is too small to hold the message. */
                        return(NX_SIZE_ERROR);
                    }

                    response_ptr = (MQTT_PACKET_PUBLISH_RESPONSE *)response_packet -> nx_packet_prepend_ptr;

                    response_ptr ->  mqtt_publish_response_packet_header = (MQTT_CONTROL_PACKET_TYPE_PUBREL << 4);
                    response_ptr ->  mqtt_publish_response_packet_remaining_length = 2;

                    /* Fill in packet ID */
                    response_packet -> nx_packet_prepend_ptr[3] = packet_ptr -> nx_packet_prepend_ptr[3];
                    response_packet -> nx_packet_prepend_ptr[4] = packet_ptr -> nx_packet_prepend_ptr[4];
                    response_packet -> nx_packet_append_ptr = response_packet -> nx_packet_prepend_ptr + 4;
                    response_packet -> nx_packet_length = 4;

                    tx_mutex_put(&client_ptr -> nxd_mqtt_protection);

#ifdef NX_SECURE_ENABLE
                    if (client_ptr -> nxd_mqtt_client_use_tls)
                    {
                        ret = nx_secure_tls_session_send(&(client_ptr -> nxd_mqtt_tls_session), response_packet, NX_WAIT_FOREVER);
                    }
                    else
                    {
                        ret = nx_tcp_socket_send(&client_ptr -> nxd_mqtt_client_socket, response_packet, NX_WAIT_FOREVER);
                    }
#else
                    ret = nx_tcp_socket_send(&client_ptr -> nxd_mqtt_client_socket, response_packet, NX_WAIT_FOREVER);

#endif /* NX_SECURE_ENABLE */

                    tx_mutex_get(&client_ptr -> nxd_mqtt_protection, TX_WAIT_FOREVER);

                    /* Update the timeout value. */
                    client_ptr -> nxd_mqtt_timeout = tx_time_get() + client_ptr -> nxd_mqtt_keepalive;

                    if (ret != NX_SUCCESS)
                    {
                        nx_packet_release(response_packet);
                    }
                }

                /* Return with value 1, so the caller will release packet_ptr */
                return(1);
            }
            else if (((response_ptr -> mqtt_publish_response_packet_header) >> 4) == MQTT_CONTROL_PACKET_TYPE_PUBREL)
            {
                /* QoS 2 publish Release received, part 2. */
                /* Therefore we verify that message block contains PUBLISH packet with QoS level 2*/
                if ((message_block -> fixed_header & 0xF6) == (MQTT_CONTROL_PACKET_TYPE_PUBREC << 4))
                {
                    /* QoS Level1 message receives an ACK. */
                    /* This message can be released. */
                    /* Send PUBREL */
                    message_block -> fixed_header = (UCHAR)((message_block -> fixed_header & 0x0F) | (MQTT_CONTROL_PACKET_TYPE_PUBREL << 4));

                    /* Allocate a packet to send the response. */
                    ret = _nxd_mqtt_packet_allocate(client_ptr, &response_packet);
                    if (ret)
                    {
                        return(1);
                    }

                    if (4u > ((ULONG)(response_packet -> nx_packet_data_end) - (ULONG)(response_packet -> nx_packet_append_ptr)))
                    {
                        nx_packet_release(response_packet);

                        /* Packet buffer is too small to hold the message. */
                        return(NX_SIZE_ERROR);
                    }

                    response_ptr = (MQTT_PACKET_PUBLISH_RESPONSE *)response_packet -> nx_packet_prepend_ptr;

                    response_ptr ->  mqtt_publish_response_packet_header = MQTT_CONTROL_PACKET_TYPE_PUBCOMP << 4;
                    response_ptr ->  mqtt_publish_response_packet_remaining_length = 2;

                    /* Fill in packet ID */
                    response_packet -> nx_packet_prepend_ptr[3] = packet_ptr -> nx_packet_prepend_ptr[3];
                    response_packet -> nx_packet_prepend_ptr[4] = packet_ptr -> nx_packet_prepend_ptr[4];
                    response_packet -> nx_packet_append_ptr = response_packet -> nx_packet_prepend_ptr + 4;
                    response_packet -> nx_packet_length = 4;

                    tx_mutex_put(&client_ptr -> nxd_mqtt_protection);

#ifdef NX_SECURE_ENABLE
                    if (client_ptr -> nxd_mqtt_client_use_tls)
                    {
                        ret = nx_secure_tls_session_send(&(client_ptr -> nxd_mqtt_tls_session), response_packet, NX_WAIT_FOREVER);
                    }
                    else
                    {
                        ret = nx_tcp_socket_send(&client_ptr -> nxd_mqtt_client_socket, response_packet, NX_WAIT_FOREVER);
                    }
#else
                    ret = nx_tcp_socket_send(&client_ptr -> nxd_mqtt_client_socket, response_packet, NX_WAIT_FOREVER);

#endif /* NX_SECURE_ENABLE */

                    tx_mutex_get(&client_ptr -> nxd_mqtt_protection, TX_WAIT_FOREVER);

                    /* Update the timeout value. */
                    client_ptr -> nxd_mqtt_timeout = tx_time_get() + client_ptr -> nxd_mqtt_keepalive;

                    if (ret)
                    {
                        nx_packet_release(response_packet);
                    }
                }
                /* The PUBREL packet doesn't match. Return and release the packet. */
                return(1);
            }
            else if (((response_ptr -> mqtt_publish_response_packet_header) >> 4 == MQTT_CONTROL_PACKET_TYPE_PUBCOMP))
            {
                /* QoS 2 publish Complete received part 3. */
                /* Therefore we verify that message block contains PUBLISH packet with QoS level 2*/
                if ((message_block -> fixed_header & 0xF6) == (((MQTT_CONTROL_PACKET_TYPE_PUBREC << 4) | MQTT_PUBLISH_QOS_LEVEL_2)))
                {

                    /* This message can be released. */
                    _nxd_mqtt_release_transmit_message_block(client_ptr, message_block, previous_block);
                }
                /* Return and release the packet. */
                return(1);
            }
        }
        else
        {
            /* Move on to the next block */
            previous_block = message_block;
            message_block = message_block -> next;
        }
    }

    /* nothing is found.  Return 1 to release the packet.*/
    return(1);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_process_sub_unsub_ack                     PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function process an ACK message for subscribe         */
/*    or unsubscribe request.                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    packet_ptr                            Pointer to the packet         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Status                                                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nxd_mqtt_client_receive_notify]      User supplied publish         */
/*                                            callback function           */
/*    _nxd_mqtt_release_transmit_message_block                            */
/*                                          Release the memory block      */
/*    _nxd_mqtt_read_remaining_length       Skip the remaining length     */
/*                                            field                       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_packet_receive_process                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
static UINT _nxd_mqtt_process_sub_unsub_ack(NXD_MQTT_CLIENT *client_ptr, NX_PACKET *packet_ptr)
{

MQTT_PACKET_SUBSCRIBE *response_ptr;
USHORT                 packet_id;
MQTT_MESSAGE_BLOCK    *message_block, *previous_block;
UCHAR                 *data;
UINT                   status;

    response_ptr = (MQTT_PACKET_SUBSCRIBE *)(packet_ptr -> nx_packet_prepend_ptr);

    /* Skip the remaining length field. */
    status = _nxd_mqtt_read_remaining_length(packet_ptr, NX_NULL, &data);
    if (status)
    {
        /* Unable to process the sub/unsub ack.  Simply return and release the packet. */
        return(1);
    }

    packet_id = (USHORT)(((*data) << 8) | (*(data + 1)));

    /* Search all the outstanding transmitted message blocks for a match. */
    previous_block = NX_NULL;
    message_block = client_ptr -> message_transmit_queue_head;
    while (message_block)
    {
        if (message_block -> packet_id == packet_id)
        {
            /* Found the matching packet id */
            if ((((response_ptr -> mqtt_subscribe_packet_header) >> 4) == MQTT_CONTROL_PACKET_TYPE_SUBACK) &&
                ((message_block -> fixed_header >> 4) == MQTT_CONTROL_PACKET_TYPE_SUBSCRIBE))
            {
                /* Validate the packet. */
                if (response_ptr -> mqtt_subscribe_packet_remaining_length != 3)
                {
                    /* Invalid remaining_length value. */
                    return(1);
                }

                /* Release the block. */
                _nxd_mqtt_release_transmit_message_block(client_ptr, message_block, previous_block);

                return(1);
            }
            else if ((((response_ptr -> mqtt_subscribe_packet_header) >> 4) == MQTT_CONTROL_PACKET_TYPE_UNSUBACK) &&
                     ((message_block -> fixed_header >> 4) == MQTT_CONTROL_PACKET_TYPE_UNSUBSCRIBE))
            {
                /* Validate the packet. */
                if (response_ptr -> mqtt_subscribe_packet_remaining_length != 2)
                {
                    /* Invalid remaining_length value. */
                    return(1);
                }

                /* Unsubscribe succeeded. */
                /* Release the block. */
                _nxd_mqtt_release_transmit_message_block(client_ptr, message_block, previous_block);

                return(1);
            }
        }
        else
        {
            /* Move on to the next block. */
            previous_block = message_block;
            message_block = message_block -> next;
        }
    }
    return(1);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_process_pingresp                          PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function process a PINGRESP message.                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    packet_ptr                            Pointer to the packet         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Status                                                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                            callback function           */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_packet_receive_process                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
static VOID _nxd_mqtt_process_pingresp(NXD_MQTT_CLIENT *client_ptr)
{


    /* If there is an outstanding ping, mark it as responded. */
    if (client_ptr -> nxd_mqtt_ping_not_responded == NX_TRUE)
    {
        client_ptr -> nxd_mqtt_ping_not_responded = NX_FALSE;

        client_ptr -> nxd_mqtt_ping_sent_time = 0;
    }

    return;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_process_disconnect                        PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function process a DISCONNECT message.                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    packet_ptr                            Pointer to the packet         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*   nx_secure_tls_session_send                                           */
/*   nx_tcp_socket_disconnect                                             */
/*   nx_tcp_client_socket_unbind                                          */
/*   _nxd_mqtt_release_transmit_message_block                             */
/*   _nxd_mqtt_release_receive_message_block                              */
/*   tx_timer_delete                                                      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   _nxd_mqtt_packet_receive_process                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            set wait option on socket   */
/*                                            disconnect to               */
/*                                            NXD_MQTT_SOCKET_TIMEOUT,    */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s),          */
/*                                            deleted TLS session when    */
/*                                            disconnecting MQTT,corrected*/
/*                                            timeout for TLS session,    */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
static VOID _nxd_mqtt_process_disconnect(NXD_MQTT_CLIENT *client_ptr)
{
MQTT_MESSAGE_BLOCK *previous = NX_NULL;
MQTT_MESSAGE_BLOCK *current;
MQTT_MESSAGE_BLOCK *next;
UINT                disconnect_callback = NX_FALSE;
UINT                status;

    if (client_ptr -> nxd_mqtt_client_state == NXD_MQTT_CLIENT_STATE_CONNECTED)
    {
        /* State changes from CONNECTED TO IDLE.  Call disconnect notify callback
           if the function is set. */
        disconnect_callback = NX_TRUE;
    }

    /* Mark the session as terminated. */
    client_ptr -> nxd_mqtt_client_state = NXD_MQTT_CLIENT_STATE_IDLE;

    tx_mutex_put(&client_ptr -> nxd_mqtt_protection);

#ifdef NX_SECURE_ENABLE
    if (client_ptr -> nxd_mqtt_client_use_tls)
    {
        nx_secure_tls_session_end(&(client_ptr -> nxd_mqtt_tls_session), NXD_MQTT_SOCKET_TIMEOUT);
        nx_secure_tls_session_delete(&(client_ptr -> nxd_mqtt_tls_session));
    }
#endif

    nx_tcp_socket_disconnect(&(client_ptr -> nxd_mqtt_client_socket), NXD_MQTT_SOCKET_TIMEOUT);
    nx_tcp_client_socket_unbind(&(client_ptr -> nxd_mqtt_client_socket));

    status = tx_mutex_get(&client_ptr -> nxd_mqtt_protection, TX_WAIT_FOREVER);

    /* Free up sub/unsub message blocks on the transmit queue. */
    current = client_ptr -> message_transmit_queue_head;

    while (current)
    {
        next = current -> next;

        if (((current -> fixed_header & 0xF0) == (MQTT_CONTROL_PACKET_TYPE_SUBSCRIBE << 4)) ||
            ((current -> fixed_header & 0xF0) == (MQTT_CONTROL_PACKET_TYPE_UNSUBSCRIBE << 4)))
        {
            _nxd_mqtt_release_transmit_message_block(client_ptr, current, previous);
        }
        else
        {
            previous = current;
        }
        current = next;
    }

    /* If a callback notification is defined, call it now. */
    if ((disconnect_callback == NX_TRUE) && (client_ptr -> nxd_mqtt_disconnect_notify))
    {
        client_ptr -> nxd_mqtt_disconnect_notify(client_ptr);
    }

    if (status == TX_SUCCESS)
    {
        /* Remove all the packets in the receive queue. */
        while (client_ptr -> message_receive_queue_head)
        {
            _nxd_mqtt_release_receive_message_block(client_ptr, client_ptr -> message_receive_queue_head, NX_NULL);
        }
        client_ptr -> message_receive_queue_depth = 0;

        /* Clear the MQTT_PACKET_RECEIVE_EVENT */
        tx_event_flags_set(&client_ptr -> nxd_mqtt_events, ~MQTT_PACKET_RECEIVE_EVENT, TX_AND);
    }

    /* Delete the timer if keep alive is enabled. */
    if (client_ptr -> nxd_mqtt_keepalive)
    {
        tx_timer_delete(&(client_ptr -> nxd_mqtt_timer));
    }

    return;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_packet_receive_process                    PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function process MQTT message.                        */
/*    NOTE: MQTT Mutex is NOT obtained on entering this function.         */
/*    Therefore it shouldn't hold the mutex when it exists this function. */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_secure_tls_session_receive                                       */
/*    nx_tcp_socket_receive                                               */
/*    _nxd_mqtt_process_publish                                           */
/*    _nxd_mqtt_process_publish_response                                  */
/*    _nxd_mqtt_process_sub_unsub_ack                                     */
/*    _nxd_mqtt_process_pingresp                                          */
/*    _nxd_mqtt_process_disconnect                                        */
/*    nx_packet_release                                                   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_packet_receive_process                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            fixed a bug that packet may */
/*                                            be queued by TLS and can not*/
/*                                            be received, improved       */
/*                                            internal logic,             */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comments, added      */
/*                                            packet receive notify,      */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
static VOID _nxd_mqtt_packet_receive_process(NXD_MQTT_CLIENT *client_ptr)
{
NX_PACKET *packet_ptr;
UINT       status;
UCHAR      packet_type;
UINT       release_packet = 1;
UINT       remaining_length;
UCHAR     *variable_header;

    for (;;)
    {

        /* Release the mutex. */
        tx_mutex_put(&client_ptr -> nxd_mqtt_protection);

        /* Make a receive call. */
#ifdef NX_SECURE_ENABLE
        if (client_ptr -> nxd_mqtt_client_use_tls)
        {
            status = nx_secure_tls_session_receive(&(client_ptr -> nxd_mqtt_tls_session), &packet_ptr, NX_NO_WAIT);
        }
        else
        {
            status = nx_tcp_socket_receive(&client_ptr -> nxd_mqtt_client_socket, &packet_ptr, NX_NO_WAIT);
        }
#else
        status = nx_tcp_socket_receive(&client_ptr -> nxd_mqtt_client_socket, &packet_ptr, NX_NO_WAIT);
#endif /* NX_SECURE_ENABLE */

        if (status != NX_SUCCESS)
        {
            /* Nothing needs to be done. */
            break;
        }

        /* Check notify function.  */
        if (client_ptr -> nxd_mqtt_packet_receive_notify)
        {

            /* Call notify funciton. Return NX_TRUE if the packet has been consumed.  */
            if (client_ptr -> nxd_mqtt_packet_receive_notify(client_ptr, packet_ptr, client_ptr -> nxd_mqtt_packet_receive_context) == NX_TRUE)
            {
                tx_mutex_get(&client_ptr -> nxd_mqtt_protection, NX_WAIT_FOREVER);
                continue;
            }
        }

        /* Obtain the mutex. */
        status = tx_mutex_get(&client_ptr -> nxd_mqtt_protection, NX_WAIT_FOREVER);
        while (packet_ptr -> nx_packet_prepend_ptr < packet_ptr -> nx_packet_append_ptr)
        {
            /* Parse the incoming packet. */
            packet_type = *(packet_ptr -> nx_packet_prepend_ptr);

            /* Right shift 4 bits to get the packet type. */
            packet_type = packet_type >> 4;

            /* Process based on packet type. */
            switch (packet_type)
            {
            case MQTT_CONTROL_PACKET_TYPE_CONNECT:
            case MQTT_CONTROL_PACKET_TYPE_CONNACK:
                /* Client does not accept connections, and shouldn't be receiving CONNACK
                   at this point.  Nothing needs to be done. */
                break;

            case MQTT_CONTROL_PACKET_TYPE_PUBLISH:
                _nxd_mqtt_process_publish(client_ptr, packet_ptr);
                break;

            case MQTT_CONTROL_PACKET_TYPE_PUBACK:
            case MQTT_CONTROL_PACKET_TYPE_PUBREC:
            case MQTT_CONTROL_PACKET_TYPE_PUBCOMP:
            case MQTT_CONTROL_PACKET_TYPE_PUBREL:
                if (_nxd_mqtt_process_publish_response(client_ptr, packet_ptr) == 0)
                {
                    release_packet = 0;
                }
                break;

            case MQTT_CONTROL_PACKET_TYPE_SUBSCRIBE:
            case MQTT_CONTROL_PACKET_TYPE_UNSUBSCRIBE:
                /* Client should not process subscribe or unsubscribe message. */
                break;

            case MQTT_CONTROL_PACKET_TYPE_SUBACK:
            case MQTT_CONTROL_PACKET_TYPE_UNSUBACK:
                if (_nxd_mqtt_process_sub_unsub_ack(client_ptr, packet_ptr) == 0)
                {
                    release_packet = 0;
                }
                break;


            case MQTT_CONTROL_PACKET_TYPE_PINGREQ:
                /* Client is not supposed to receive ping req.  Ignore it. */
                break;

            case MQTT_CONTROL_PACKET_TYPE_PINGRESP:
                _nxd_mqtt_process_pingresp(client_ptr);
                release_packet = 1;
                break;

            case MQTT_CONTROL_PACKET_TYPE_DISCONNECT:
                _nxd_mqtt_process_disconnect(client_ptr);
                break;

            default:
                /* Unknown type. */
                break;
            }
            status = _nxd_mqtt_read_remaining_length(packet_ptr, &remaining_length, &variable_header);
            if (status == NXD_MQTT_SUCCESS)
            {
                packet_ptr -> nx_packet_length = (UINT)((INT)packet_ptr -> nx_packet_length - (1 + (INT)remaining_length + (variable_header - packet_ptr -> nx_packet_prepend_ptr)));
                packet_ptr -> nx_packet_prepend_ptr = variable_header + remaining_length;
            }
            else
            {
                return;
            }
        }
        if (release_packet)
        {
            nx_packet_release(packet_ptr);
        }
    }

    /* No more data in the receive queue.  Return. */

    return;
}


static UINT append_message(NX_PACKET *packet_ptr, CHAR *message, UINT length)
{
UINT ret = 0;

    if (length + 2u > ((ULONG)(packet_ptr -> nx_packet_data_end) - (ULONG)(packet_ptr -> nx_packet_append_ptr)))
    {
        /* Packet buffer is too small to hold the message. */
        return(ret);
    }

    /* Write the Client Identifier filed, starting with length MSB, followed by length LSB */

    packet_ptr -> nx_packet_append_ptr[0] = (length >> 8) & 0xFF;
    packet_ptr -> nx_packet_append_ptr[1] = length  & 0xFF;

    packet_ptr -> nx_packet_append_ptr += 2;
    ret = 2;
    packet_ptr -> nx_packet_length += 2;

    if (length)
    {
        /* Copy the string into the packet. */
        NXD_MQTT_SECURE_MEMCPY(packet_ptr -> nx_packet_append_ptr, message, length);

        packet_ptr -> nx_packet_append_ptr += length;
        packet_ptr -> nx_packet_length += length;
        ret += length;
    }

    return(ret);
}


static UINT _nxd_mqtt_send_simple_message(NXD_MQTT_CLIENT *client_ptr, UCHAR header_value);


/* MQTT internal function */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _mqtt_client_disconenct_callback                    PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function is passed to MQTT client socket create call. */
/*    This callback function notifies MQTT client thread when the TCP     */
/*    connection is lost.                                                 */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to TCP socket that    */
/*                                            disconnected.               */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    TCP socket disconnect callback                                      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
static VOID _nxd_mqtt_periodic_timer_entry(ULONG client)
{
/* Check if it is time to send out a ping message. */
NXD_MQTT_CLIENT *client_ptr = (NXD_MQTT_CLIENT *)client;

    /* If an outstanding ping response has not been received, and the client exceeds the time waiting for ping response,
       the client shall disconnect from the server. */
    if (client_ptr -> nxd_mqtt_ping_not_responded)
    {
        /* If current time is greater than the ping timeout */
        if ((tx_time_get() - client_ptr -> nxd_mqtt_ping_sent_time) >= client_ptr -> nxd_mqtt_ping_timeout)
        {
            /* Ping timed out.  Need to termiante the connection. */
            tx_event_flags_set(&client_ptr -> nxd_mqtt_events, MQTT_PING_TIMEOUT_EVENT, TX_OR);

            return;
        }
    }

    /* About to timeout? */
    if ((client_ptr -> nxd_mqtt_timeout - tx_time_get()) <= client_ptr -> nxd_mqtt_timer_value)
    {
        /* Set the flag so the MQTT thread can send the ping. */
        tx_event_flags_set(&client_ptr -> nxd_mqtt_events, MQTT_TIMEOUT_EVENT, TX_OR);
    }

    /* If keepalive is not enabled, just reutrn. */
    return;
}



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_thread_entry                              PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function serves as the entry point for the MQTT       */
/*    client thread.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    mqtt_client                           Pointer to MQTT Client        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_release_transmit_message_block                            */
/*    _nxd_mqtt_release_receive_message_block                             */
/*    tx_event_flags_set                                                  */
/*    tx_event_flags_get                                                  */
/*    _nxd_mqtt_send_simple_message                                       */
/*    _nxd_mqtt_process_disconnect                                        */
/*    _nxd_mqtt_packet_receive_process                                    */
/*    tx_timer_delete                                                     */
/*    tx_event_flags_delete                                               */
/*    nx_tcp_socket_delete                                                */
/*    tx_timer_delete                                                     */
/*    tx_mutex_delete                                                     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   _nxd_mqtt_client_create                                              */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            added logic to delete the   */
/*                                            socket here, removed the    */
/*                                            usage of global variable for*/
/*                                            MQTT client,                */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
static VOID _nxd_mqtt_thread_entry(ULONG mqtt_client)
{
NXD_MQTT_CLIENT *client_ptr;
ULONG            events;

    client_ptr = (NXD_MQTT_CLIENT *)mqtt_client;

    /* Obtain the mutex. */
    tx_mutex_get(&(client_ptr -> nxd_mqtt_protection), TX_WAIT_FOREVER);

    /* Move MQTT Client state to STARTED */
    client_ptr -> nxd_mqtt_client_state = NXD_MQTT_CLIENT_STATE_IDLE;

    /* Loop to process events on the MQTT client */
    for (;;)
    {

        /* Release the MQTT client mutex */
        tx_mutex_put(&client_ptr -> nxd_mqtt_protection);

        tx_event_flags_get(&client_ptr -> nxd_mqtt_events, MQTT_ALL_EVENTS, TX_OR_CLEAR, &events, TX_WAIT_FOREVER);

        /* Obtain the mutex. */
        tx_mutex_get(&(client_ptr -> nxd_mqtt_protection), TX_WAIT_FOREVER);


        if (events & MQTT_TIMEOUT_EVENT)
        {
            /* Send out PING only if the client is connected. */
            if (client_ptr -> nxd_mqtt_client_state == NXD_MQTT_CLIENT_STATE_CONNECTED)
            {
                _nxd_mqtt_send_simple_message(client_ptr, MQTT_CONTROL_PACKET_TYPE_PINGREQ);
            }
        }
        if (events & MQTT_PACKET_RECEIVE_EVENT)
        {
            _nxd_mqtt_packet_receive_process(client_ptr);
        }
        if (events & MQTT_DELETE_EVENT)
        {
            break;
        }
        if (events & MQTT_PING_TIMEOUT_EVENT)
        {
            /* The server/broker didn't respond to our ping request message. Disconnect from the server. */
            _nxd_mqtt_process_disconnect(client_ptr);
        }
        if (events & MQTT_NETWORK_DISCONNECT_EVENT)
        {
            /* The server closed TCP socket. We shall go through the disconnect code path. */
            _nxd_mqtt_process_disconnect(client_ptr);
        }
    }

    /* Stop the client and disconnect from the server. */
    if (client_ptr -> nxd_mqtt_client_state == NXD_MQTT_CLIENT_STATE_CONNECTED)
    {
        _nxd_mqtt_process_disconnect(client_ptr);
    }

    /* Delete the timer. Check first if it is already deleted. */
    if ((client_ptr -> nxd_mqtt_timer).tx_timer_id != 0)
        tx_timer_delete(&(client_ptr -> nxd_mqtt_timer));

    /* Delete the event flag. Check first if it is already deleted. */
    if ((client_ptr -> nxd_mqtt_events).tx_event_flags_group_id != 0)
        tx_event_flags_delete(&client_ptr -> nxd_mqtt_events);

    /* Release all the messages on the receive queue. */
    while (client_ptr -> message_receive_queue_head)
    {
        _nxd_mqtt_release_receive_message_block(client_ptr, client_ptr -> message_receive_queue_head, NX_NULL);
    }
    client_ptr -> message_receive_queue_depth = 0;

    /* Delete all the messages sitting in the receive and transmit queue. */
    while (client_ptr -> message_transmit_queue_head)
    {
        _nxd_mqtt_release_transmit_message_block(client_ptr, client_ptr -> message_transmit_queue_head, NX_NULL);
    }

    /* Release mutex */
    tx_mutex_put(&client_ptr -> nxd_mqtt_protection);

    /* Delete the mutex. */
    tx_mutex_delete(&client_ptr -> nxd_mqtt_protection);

    /* Deleting the socket, (the socket ID is cleared); this signals it is ok to delete this thread. */
    nx_tcp_socket_delete(&client_ptr -> nxd_mqtt_client_socket);

    return;
}



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _mqtt_client_disconenct_callback                    PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function is passed to MQTT client socket create call. */
/*    This callback function notifies MQTT client thread when the TCP     */
/*    connection is lost.                                                 */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to TCP socket that    */
/*                                            disconnected.               */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    TCP socket disconnect callback                                      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
static VOID _mqtt_client_disconnect_callback(NX_TCP_SOCKET *socket_ptr)
{
NXD_MQTT_CLIENT *client_ptr = (NXD_MQTT_CLIENT *)(socket_ptr -> nx_tcp_socket_reserved_ptr);

    /* Set the MQTT_NETWORK_DISCONNECT  event.  This event indicates
       that the disconnect is initiated from the network. */
    tx_event_flags_set(&client_ptr -> nxd_mqtt_events, MQTT_NETWORK_DISCONNECT_EVENT, TX_OR);

    return;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_create                             PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates an instance for MQTT Client.  It initializes  */
/*    MQTT Client control block, creates a thread, mutex and event queue  */
/*    for MQTT Client, and creates the TCP socket for MQTT messaging.     */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    client_id                             Client ID for this instance   */
/*    client_id_length                      Length of Client ID, in bytes */
/*    ip_ptr                                Pointer to IP instance        */
/*    pool_ptr                              Pointer to packet pool        */
/*    stack_ptr                             Client thread's stack pointer */
/*    stack_size                            Client thread's stack size    */
/*    mqtt_thread_priority                  Priority for MQTT thread      */
/*    memory_ptr                            Pointer to memory block the   */
/*                                            client can use              */
/*    memory_size                           Size of the memory area       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_thread_create                      Create a thread               */
/*    tx_mutex_create                       Create mutex                  */
/*    tx_mutex_get                                                        */
/*    tx_mutex_put                                                        */
/*    tx_event_flag_create                  Create event flag             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            added logic to create the   */
/*                                            socket here, added logic to */
/*                                            clear resources on error,   */
/*                                            supported user defined      */
/*                                            memory functions,           */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nxd_mqtt_client_create(NXD_MQTT_CLIENT *client_ptr, CHAR *client_name,
                             CHAR *client_id, UINT client_id_length,
                             NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr,
                             VOID *stack_ptr, ULONG stack_size, UINT mqtt_thread_priority,
                             VOID *memory_ptr, ULONG memory_size)
{
UINT                status;
MQTT_MESSAGE_BLOCK *block_ptr;
MQTT_MESSAGE_BLOCK *prev_block;
UINT                end_of_memory;

    /* Clear the MQTT Client control block. */
    NXD_MQTT_SECURE_MEMSET((void *)client_ptr, 0, sizeof(NXD_MQTT_CLIENT));

    status = tx_mutex_create(&client_ptr -> nxd_mqtt_protection, client_name, TX_NO_INHERIT);

    /* Determine if an error occured. */
    if (status != TX_SUCCESS)
    {

        return(NXD_MQTT_INTERNAL_ERROR);
    }
    /* Now create MQTT client thread */
    status = tx_thread_create(&(client_ptr -> nxd_mqtt_thread), client_name, _nxd_mqtt_thread_entry,
                              (ULONG)client_ptr, stack_ptr, stack_size, mqtt_thread_priority, mqtt_thread_priority,
                              NXD_MQTT_CLIENT_THREAD_TIME_SLICE, TX_DONT_START);

    /* Determine if an error occured. */
    if (status != TX_SUCCESS)
    {
        /* Delete the mutex. */
        tx_mutex_delete(&client_ptr -> nxd_mqtt_protection);

        /* Return error code. */
        return(NXD_MQTT_INTERNAL_ERROR);
    }

    status = tx_event_flags_create(&(client_ptr -> nxd_mqtt_events), client_name);

    if (status != TX_SUCCESS)
    {
        /* Delete the mutex. */
        tx_mutex_delete(&client_ptr -> nxd_mqtt_protection);

        /* Delete the thread. */
        tx_thread_delete(&(client_ptr -> nxd_mqtt_thread));
        
        /* Return error code. */
        return(NXD_MQTT_INTERNAL_ERROR);
    }

    /* Record the client ID information. */
    client_ptr -> nxd_mqtt_client_id = client_id;
    client_ptr -> nxd_mqtt_client_id_length = client_id_length;
    client_ptr -> nxd_mqtt_client_ip_ptr = ip_ptr;
    client_ptr -> nxd_mqtt_client_packet_pool_ptr = pool_ptr;
    client_ptr -> nxd_mqtt_client_name = client_name;

    /* Create client message blocks using memory_ptr. */
    NXD_MQTT_SECURE_MEMSET(memory_ptr, 0, memory_size);
    if (memory_ptr)
    {
        /* Make sure the message block starts with 4-byte alignment. */
        block_ptr = (MQTT_MESSAGE_BLOCK *)(((ULONG)memory_ptr + 3) & 0xFFFFFFFC);

        end_of_memory = (ULONG)(memory_ptr) + memory_size;

        client_ptr -> message_block_free_list = block_ptr;
        prev_block = block_ptr;

        while (((UINT)block_ptr + sizeof(MQTT_MESSAGE_BLOCK)) < end_of_memory)
        {
            prev_block = block_ptr;
            block_ptr++;
            prev_block -> next = block_ptr;
        }

        prev_block -> next = NX_NULL;
    }

    /* Create the socket. */
    status = nx_tcp_socket_create(client_ptr -> nxd_mqtt_client_ip_ptr, &(client_ptr -> nxd_mqtt_client_socket), client_ptr -> nxd_mqtt_client_name,
                                  NX_IP_NORMAL, NX_DONT_FRAGMENT, 0x80, NXD_MQTT_CLIENT_SOCKET_WINDOW_SIZE,
                                  NX_NULL, _mqtt_client_disconnect_callback);

    /* Determine if an error occurred. */
    if (status != NX_SUCCESS)
    {
        /* Delete the mutex. */
        tx_mutex_delete(&client_ptr -> nxd_mqtt_protection);

        /* Delete the event flags. */
        tx_event_flags_delete(&(client_ptr -> nxd_mqtt_events));

        /* Delete the thread. */
        tx_thread_delete(&(client_ptr -> nxd_mqtt_thread));

        return(NXD_MQTT_INTERNAL_ERROR);
    }

    client_ptr -> nxd_mqtt_client_state = NXD_MQTT_CLIENT_STATE_INITIALIZE;

    /* Start MQTT thread. */
    tx_thread_resume(&(client_ptr -> nxd_mqtt_thread));

    return(NXD_MQTT_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_login_set                          PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets optional MQTT username and password.  Note       */
/*    if the broker requires username and password, this information      */
/*    must be set prior to calling nxd_mqtt_client_connect or             */
/*    nxd_mqtt_client_secure_connect.                                     */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    username                              User name, or NULL if user    */
/*                                            name is not required        */
/*    username_length                       Length of the user name, or   */
/*                                            0 if user name is NULL      */
/*    password                              Password string, or NULL if   */
/*                                            password is not required    */
/*    password_length                       Length of the password, or    */
/*                                            0 if password is NULL       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nxd_mqtt_client_login_set(NXD_MQTT_CLIENT *client_ptr,
                                CHAR *username, UINT username_length, CHAR *password, UINT password_length)
{
UINT status;

    /* Obtain the mutex. */
    status = tx_mutex_get(&client_ptr -> nxd_mqtt_protection, NX_WAIT_FOREVER);

    if (status != TX_SUCCESS)
    {
        return(NXD_MQTT_MUTEX_FAILURE);
    }
    client_ptr -> nxd_mqtt_client_username = username;
    client_ptr -> nxd_mqtt_client_username_length = (USHORT)username_length;
    client_ptr -> nxd_mqtt_client_password = password;
    client_ptr -> nxd_mqtt_client_password_length = (USHORT)password_length;

    tx_mutex_put(&client_ptr -> nxd_mqtt_protection);

    return(NX_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_will_message_set                   PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets optional MQTT will topic and will message.       */
/*    Note that if will message is needed, application must set will      */
/*    message prior to calling nxd_mqtt_client_connect or                 */
/*    nxd_mqtt_client_secure_connect.                                     */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    will_topic                            Will topic string.            */
/*    will_topic_length                     Length of the will topic.     */
/*    will_message                          Will message string.          */
/*    will_message_length                   Length of the will message.   */
/*    will_retain_flag                      Whether or not the will       */
/*                                            message is to be retained   */
/*                                            when it is published.       */
/*                                            Valid values are NX_TRUE    */
/*                                            NX_FALSE                    */
/*    will_QoS                              QoS level to be used when     */
/*                                            publishing will message.    */
/*                                            Valid values are 0, 1, 2    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nxd_mqtt_client_will_message_set(NXD_MQTT_CLIENT *client_ptr,
                                       const UCHAR *will_topic, UINT will_topic_length, const UCHAR *will_message,
                                       UINT will_message_length, UINT will_retain_flag, UINT will_QoS)
{
UINT status;

    if (will_QoS == 2)
    {
        return(NXD_MQTT_QOS2_NOT_SUPPORTED);
    }

    /* Obtain the mutex. */
    status = tx_mutex_get(&client_ptr -> nxd_mqtt_protection, NX_WAIT_FOREVER);

    if (status != TX_SUCCESS)
    {
        return(NXD_MQTT_MUTEX_FAILURE);
    }

    client_ptr -> nxd_mqtt_client_will_topic = will_topic;
    client_ptr -> nxd_mqtt_client_will_topic_length = will_topic_length;
    client_ptr -> nxd_mqtt_client_will_message = will_message;
    client_ptr -> nxd_mqtt_client_will_message_length = will_message_length;

    if (will_retain_flag == NX_TRUE)
    {
        client_ptr -> nxd_mqtt_client_will_qos_retain = 0x80;
    }
    client_ptr -> nxd_mqtt_client_will_qos_retain = (UCHAR)(client_ptr -> nxd_mqtt_client_will_qos_retain | will_QoS);

    tx_mutex_put(&client_ptr -> nxd_mqtt_protection);

    return(NX_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_mqtt_client_will_message_set                  PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the                              */
/*    nxd_mqtt_client_will_message_set call.                              */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    will_topic                            Will topic string.            */
/*    will_topic_length                     Length of the will topic.     */
/*    will_message                          Will message string.          */
/*    will_message_length                   Length of the will message.   */
/*    will_retain_flag                      Whether or not the will       */
/*                                            message is to be retained   */
/*                                            when it is published.       */
/*                                            Valid values are NX_TRUE    */
/*                                            NX_FALSE                    */
/*    will_QoS                              QoS level to be used when     */
/*                                            publishing will message.    */
/*                                            Valid values are 0, 1, 2    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_client_will_message_set                                   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comments,            */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nxde_mqtt_client_will_message_set(NXD_MQTT_CLIENT *client_ptr,
                                        const UCHAR *will_topic, UINT will_topic_length, const UCHAR *will_message,
                                        UINT will_message_length, UINT will_retain_flag, UINT will_QoS)
{

    if (client_ptr == NX_NULL)
    {
        return(NXD_MQTT_INVALID_PARAMETER);
    }

    /* Valid will_topic string.  The will topic string cannot be NULL. */
    if ((will_topic == NX_NULL) || (will_topic_length  == 0))
    {
        return(NXD_MQTT_INVALID_PARAMETER);
    }

    if ((will_retain_flag != NX_TRUE) && (will_retain_flag != NX_FALSE))
    {
        return(NXD_MQTT_INVALID_PARAMETER);
    }

    if (will_QoS > 2)
    {
        return(NXD_MQTT_INVALID_PARAMETER);
    }

    return(_nxd_mqtt_client_will_message_set(client_ptr, will_topic, will_topic_length, will_message,
                                             will_message_length, will_retain_flag, will_QoS));
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_mqtt_client_login_set                         PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the nxd_mqtt_client_login call.  */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    username                              User name, or NULL if user    */
/*                                            name is not required        */
/*    username_length                       Length of the user name, or   */
/*                                            0 if user name is NULL      */
/*    password                              Password string, or NULL if   */
/*                                            password is not required    */
/*    password_length                       Length of the password, or    */
/*                                            0 if password is NULL       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_client_login_set                                          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s), allowed  */
/*                                            zero length of username and */
/*                                            password,                   */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nxde_mqtt_client_login_set(NXD_MQTT_CLIENT *client_ptr,
                                 CHAR *username, UINT username_length, CHAR *password, UINT password_length)
{
    if (client_ptr == NX_NULL)
    {
        return(NXD_MQTT_INVALID_PARAMETER);
    }

    /* Username and username length don't match,
       or password and password length don't match. */
    if (((username == NX_NULL) && (username_length != 0)) ||
        ((password == NX_NULL) && (password_length != 0)))
    {
        return(NXD_MQTT_INVALID_PARAMETER);
    }

    return(_nxd_mqtt_client_login_set(client_ptr, username, username_length, password, password_length));
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_retransmit_message                 PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retransmit QoS1 messages upon reconnection, if the    */
/*    connection is not set CLEAN_SESSION.                                */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_packet_allocate                                           */
/*    append_message                                                      */
/*    _nxd_mqtt_set_remaining_length                                      */
/*    tx_mutex_get                                                        */
/*    tx_mutex_put                                                        */
/*    nx_secure_tls_session_send                                          */
/*    nx_tcp_socket_send                                                  */
/*    nx_packet_release                                                   */
/*    tx_time_get                                                         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            supported user defined      */
/*                                            memory functions,           */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
static UINT _nxd_mqtt_client_retransmit_message(NXD_MQTT_CLIENT *client_ptr)
{
MQTT_MESSAGE_BLOCK *message_block;
UINT                status = NXD_MQTT_SUCCESS;
UINT                mutex_status;
NX_PACKET          *packet_ptr;
UCHAR              *byte;

    message_block = client_ptr -> message_transmit_queue_head;

    while (message_block)
    {

        /* Obtain a NetX Packet. */
        status = _nxd_mqtt_packet_allocate(client_ptr, &packet_ptr);

        if (status != NXD_MQTT_SUCCESS)
        {
            return(NXD_MQTT_PACKET_POOL_FAILURE);
        }

        byte = packet_ptr -> nx_packet_prepend_ptr;
        *byte = message_block -> fixed_header;
        packet_ptr -> nx_packet_append_ptr = packet_ptr -> nx_packet_prepend_ptr + 1;
        packet_ptr -> nx_packet_length = 1;

        /* Write the reminaing Length. */
        _nxd_mqtt_set_remaining_length(packet_ptr, message_block -> remaining_length);

        if ((message_block -> fixed_header & 0xF0) == (MQTT_CONTROL_PACKET_TYPE_PUBLISH << 4))
        {
            /* Fill in Topic Name, Packet ID, and Payload(message). */
            append_message(packet_ptr, message_block -> topic_name, message_block -> topic_name_length);

            *(packet_ptr -> nx_packet_append_ptr) = (UCHAR)(message_block -> packet_id >> 8);
            *(packet_ptr -> nx_packet_append_ptr + 1) = (UCHAR)(message_block -> packet_id & 0xFF);
            packet_ptr -> nx_packet_length += 2;
            packet_ptr -> nx_packet_append_ptr += 2;

            if (message_block -> message &&
                message_block -> message_length <= ((ULONG)(packet_ptr -> nx_packet_data_end) - (ULONG)(packet_ptr -> nx_packet_append_ptr)))
            {
                NXD_MQTT_SECURE_MEMCPY(packet_ptr -> nx_packet_append_ptr, message_block -> message,
                       message_block -> message_length);
                packet_ptr -> nx_packet_append_ptr += message_block -> message_length;
                packet_ptr -> nx_packet_length += message_block -> message_length;
            }
        }
        else
        {
            *(packet_ptr -> nx_packet_append_ptr) = (UCHAR)(message_block -> packet_id >> 8);
            *(packet_ptr -> nx_packet_append_ptr + 1) = (UCHAR)(message_block -> packet_id & 0xFF);
            packet_ptr -> nx_packet_length += 2;
            packet_ptr -> nx_packet_append_ptr += 2;
        }

        /* Release the mutex. */
        tx_mutex_put(&client_ptr -> nxd_mqtt_protection);

        /* Send the packet. */
#ifdef NX_SECURE_ENABLE
        if (client_ptr -> nxd_mqtt_client_use_tls)
        {
            status = nx_secure_tls_session_send(&(client_ptr -> nxd_mqtt_tls_session), packet_ptr, NX_WAIT_FOREVER);
        }
        else
        {
            status = nx_tcp_socket_send(&client_ptr -> nxd_mqtt_client_socket, packet_ptr, NX_WAIT_FOREVER);
        }
#else
        status = nx_tcp_socket_send(&client_ptr -> nxd_mqtt_client_socket, packet_ptr, NX_WAIT_FOREVER);

#endif /* NX_SECURE_ENABLE */

        if (status)
        {
            /* Release the packet. */
            nx_packet_release(packet_ptr);

            status = NXD_MQTT_COMMUNICATION_FAILURE;
        }
        /* Obtain the mutex. */
        mutex_status = tx_mutex_get(&client_ptr -> nxd_mqtt_protection, NX_WAIT_FOREVER);

        if (mutex_status != TX_SUCCESS)
        {
            return(NXD_MQTT_MUTEX_FAILURE);
        }
        if (status)
        {
            return(status);
        }
        message_block = message_block -> next;
    }

    /* Update the timeout value. */
    client_ptr -> nxd_mqtt_timeout = tx_time_get() + client_ptr -> nxd_mqtt_keepalive;

    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_connect                            PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function makes an initial connection to the MQTT server.       */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    server_ip                             Server IP address structure   */
/*    server_port                           Server port number, in host   */
/*                                            byte order                  */
/*    keepalive                             Keepalive flag                */
/*    clean_session                         Clean session flag            */
/*    wait_option                           Timeout value                 */
/*                                                                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                                                        */
/*    nx_tcp_socket_create                  Create TCP socket             */
/*    nx_tcp_socket_receive_notify                                        */
/*    nx_tcp_client_socket_bind                                           */
/*    nxd_tcp_client_socket_connect                                       */
/*    nx_tcp_client_socket_unbind                                         */
/*    tx_mutex_put                                                        */
/*    nx_secure_tls_session_start                                         */
/*    nx_secure_tls_session_send                                          */
/*    nx_secure_tls_sesion_receive                                        */
/*    _nxd_mqtt_packet_allocate                                           */
/*    append_message                                                      */
/*    nx_tcp_socket_send                                                  */
/*    nx_packet_release                                                   */
/*    nx_tcp_socket_receive                                               */
/*    tx_event_flag_set                                                   */
/*    _nxd_mqtt_release_transmit_message_block                            */
/*    tx_timer_create                                                     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comments, added a    */
/*                                            a symbol for user to set    */
/*                                            initial packet ID value,    */
/*                                            improved internal logic,    */
/*                                            moved the socket create call*/
/*                                            to the MQTT client create   */
/*                                            function, removed socket    */
/*                                            delete calls, added symbol  */
/*                                            NXD_MQTT_REQUIRE_TLS to     */
/*                                            enforce TLS connections,    */
/*                                            allowed zero length of      */
/*                                            username and password,      */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s), corrected*/
/*                                            the mutex processing for    */
/*                                            receiving MQTT CONNACK,     */
/*                                            deleted TLS session when    */
/*                                            MQTT connect failed,        */
/*                                            improved packet length      */
/*                                            verification, corrected     */
/*                                            timeout for TLS session,    */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nxd_mqtt_client_connect(NXD_MQTT_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                              UINT keepalive, UINT clean_session, ULONG wait_option)
{
NX_PACKET           *packet_ptr;
UINT                 status;
UINT                 length = 0;
UCHAR                connection_flags = 0;
UINT                 ret = NXD_MQTT_SUCCESS;
MQTT_PACKET_CONNACK *connack_packet_ptr;
UCHAR               *byte;



    /* Obtain the mutex. */
    status = tx_mutex_get(&client_ptr -> nxd_mqtt_protection, NX_WAIT_FOREVER);

    if (status != TX_SUCCESS)
    {
#ifdef NX_SECURE_ENABLE
        if (client_ptr -> nxd_mqtt_client_use_tls)
        {
            nx_secure_tls_session_end(&(client_ptr -> nxd_mqtt_tls_session), NX_NO_WAIT);
            nx_secure_tls_session_delete(&(client_ptr -> nxd_mqtt_tls_session));
        }
#endif
        return(NXD_MQTT_MUTEX_FAILURE);
    }

    /* Do nothing if the client is already connected. */
    if (client_ptr -> nxd_mqtt_client_state == NXD_MQTT_CLIENT_STATE_CONNECTED)
    {
#ifdef NX_SECURE_ENABLE
        if (client_ptr -> nxd_mqtt_client_use_tls)
        {
            nx_secure_tls_session_end(&(client_ptr -> nxd_mqtt_tls_session), NX_NO_WAIT);
            nx_secure_tls_session_delete(&(client_ptr -> nxd_mqtt_tls_session));
        }
#endif
        tx_mutex_put(&client_ptr -> nxd_mqtt_protection);
        return(NXD_MQTT_ALREADY_CONNECTED);
    }

    /* Set the receive callback. */
    nx_tcp_socket_receive_notify(&client_ptr -> nxd_mqtt_client_socket, _nxd_mqtt_receive_callback);

    /* Record the client_ptr in the socket structure. */
    client_ptr -> nxd_mqtt_client_socket.nx_tcp_socket_reserved_ptr = (VOID *)client_ptr;

    /* First attempt to bind the client socket. */
    status = nx_tcp_client_socket_bind(&(client_ptr -> nxd_mqtt_client_socket), NX_ANY_PORT, wait_option);

    if (status != NX_SUCCESS)
    {

#ifdef NX_SECURE_ENABLE
        if (client_ptr -> nxd_mqtt_client_use_tls)
        {
            nx_secure_tls_session_end(&(client_ptr -> nxd_mqtt_tls_session), NX_NO_WAIT);
            nx_secure_tls_session_delete(&(client_ptr -> nxd_mqtt_tls_session));
        }
#endif
        tx_mutex_put(&client_ptr -> nxd_mqtt_protection);
        return(NXD_MQTT_INTERNAL_ERROR);
    }

    /* Connect to the MQTT server */
    status = nxd_tcp_client_socket_connect(&(client_ptr -> nxd_mqtt_client_socket), server_ip, server_port, wait_option);
    if (status != NX_SUCCESS)
    {
#ifdef NX_SECURE_ENABLE
        if (client_ptr -> nxd_mqtt_client_use_tls)
        {
            nx_secure_tls_session_end(&(client_ptr -> nxd_mqtt_tls_session), NX_NO_WAIT);
            nx_secure_tls_session_delete(&(client_ptr -> nxd_mqtt_tls_session));
        }
#endif
        nx_tcp_client_socket_unbind(&(client_ptr -> nxd_mqtt_client_socket));
        tx_mutex_put(&client_ptr -> nxd_mqtt_protection);
        return(NXD_MQTT_CONNECT_FAILURE);
    }

    /* Record the keepalive value, converted to TX timer ticks. */
    client_ptr -> nxd_mqtt_keepalive = keepalive * NX_IP_PERIODIC_RATE;
    if (keepalive)
    {
        client_ptr -> nxd_mqtt_timer_value = NXD_MQTT_KEEPALIVE_TIMER_RATE;
        client_ptr -> nxd_mqtt_ping_timeout = NXD_MQTT_PING_TIMEOUT_DELAY;

        /* Create timer */
        status = tx_timer_create(&(client_ptr -> nxd_mqtt_timer), "MQTT Timer", _nxd_mqtt_periodic_timer_entry, (ULONG)client_ptr,
                                 client_ptr -> nxd_mqtt_timer_value, client_ptr -> nxd_mqtt_timer_value, TX_AUTO_ACTIVATE);
        if (status)
        {
#ifdef NX_SECURE_ENABLE
            if (client_ptr -> nxd_mqtt_client_use_tls)
            {
                nx_secure_tls_session_end(&(client_ptr -> nxd_mqtt_tls_session), NX_NO_WAIT);
                nx_secure_tls_session_delete(&(client_ptr -> nxd_mqtt_tls_session));
            }
#endif
            nx_tcp_socket_disconnect(&(client_ptr -> nxd_mqtt_client_socket), 0);
            nx_tcp_client_socket_unbind(&(client_ptr -> nxd_mqtt_client_socket));
            tx_mutex_put(&client_ptr -> nxd_mqtt_protection);
            return(NXD_MQTT_INTERNAL_ERROR);
        }
    }
    else
    {
        client_ptr -> nxd_mqtt_timer_value = 0;
        client_ptr -> nxd_mqtt_ping_timeout = 0;
    }

    /* If TLS is enabled, start TLS */
#ifdef NX_SECURE_ENABLE

    if (client_ptr -> nxd_mqtt_client_use_tls)
    {

        status = nx_secure_tls_session_start(&(client_ptr -> nxd_mqtt_tls_session), &(client_ptr -> nxd_mqtt_client_socket), wait_option);

        if (status != NX_SUCCESS)
        {
#ifdef NX_SECURE_ENABLE
            if (client_ptr -> nxd_mqtt_client_use_tls)
            {
                nx_secure_tls_session_end(&(client_ptr -> nxd_mqtt_tls_session), NX_NO_WAIT);
                nx_secure_tls_session_delete(&(client_ptr -> nxd_mqtt_tls_session));
            }
#endif
            nx_tcp_socket_disconnect(&(client_ptr -> nxd_mqtt_client_socket), 0);
            nx_tcp_client_socket_unbind(&(client_ptr -> nxd_mqtt_client_socket));
            tx_mutex_put(&client_ptr -> nxd_mqtt_protection);

            /* Disable timer if timer has been started. */
            if (client_ptr -> nxd_mqtt_keepalive)
            {
                 tx_timer_delete(&(client_ptr -> nxd_mqtt_timer));
            }
            return(NXD_MQTT_CONNECT_FAILURE);
        }
    }
#ifdef NXD_MQTT_REQUIRE_TLS
    else 
    {
        /* NXD_MQTT_REQUIRE_TLS is defined but the application does not use TLS.
           This is security violation.  Return with failure code. */
#ifdef NX_SECURE_ENABLE
        if (client_ptr -> nxd_mqtt_client_use_tls)
        {
            nx_secure_tls_session_end(&(client_ptr -> nxd_mqtt_tls_session), NX_NO_WAIT);
            nx_secure_tls_session_delete(&(client_ptr -> nxd_mqtt_tls_session));
        }
#endif
        nx_tcp_socket_disconnect(&(client_ptr -> nxd_mqtt_client_socket), 0);
        nx_tcp_client_socket_unbind(&(client_ptr -> nxd_mqtt_client_socket));
        tx_mutex_put(&client_ptr -> nxd_mqtt_protection);
        if (client_ptr -> nxd_mqtt_keepalive)
        {
            tx_timer_delete(&(client_ptr -> nxd_mqtt_timer));
        }
        return(NXD_MQTT_CONNECT_FAILURE);
    }
           
#endif /* NXD_MQTT_REQUIRE_TLS */
#endif /* NX_SECURE_ENABLE */

    /* Construct connect flags by taking the connect flag user supplies, or'ing the username and
       password bits, if they are supplied. */
    if (client_ptr -> nxd_mqtt_client_username)
    {
        connection_flags |= MQTT_CONNECT_FLAGS_USERNAME;

        /* Add the password flag only if username is supplied. */
        if (client_ptr -> nxd_mqtt_client_password)
        {
            connection_flags |= MQTT_CONNECT_FLAGS_PASSWORD;
        }
    }

    if (client_ptr -> nxd_mqtt_client_will_topic)
    {
        connection_flags = connection_flags | MQTT_CONNECT_FLAGS_WILL_FLAG;


        if (client_ptr -> nxd_mqtt_client_will_qos_retain & 0x80)
        {
            connection_flags = connection_flags | MQTT_CONNECT_FLAGS_WILL_RETAIN;
        }

        connection_flags = (UCHAR)(connection_flags | ((client_ptr -> nxd_mqtt_client_will_qos_retain & 0x3) << 3));
    }

    if (clean_session == NX_TRUE)
    {
        connection_flags = connection_flags | MQTT_CONNECT_FLAGS_CLEAN_SESSION;

        /* Clear any transmit blocks from the previous session. */
        while (client_ptr -> message_transmit_queue_head)
        {
            _nxd_mqtt_release_transmit_message_block(client_ptr, client_ptr -> message_transmit_queue_head, NX_NULL);
        }
    }

    /* Set the length of the packet. */
    length = 10;

    /* Add the size of the client Identifier. */
    length += (client_ptr -> nxd_mqtt_client_id_length + 2);

    /* Add the will topic, if present. */
    if (connection_flags & MQTT_CONNECT_FLAGS_WILL_FLAG)
    {
        length += (client_ptr -> nxd_mqtt_client_will_topic_length + 2);
        length += (client_ptr -> nxd_mqtt_client_will_message_length + 2);
    }
    if (connection_flags & MQTT_CONNECT_FLAGS_USERNAME)
    {
        length += (UINT)(client_ptr -> nxd_mqtt_client_username_length + 2);
    }
    if (connection_flags & MQTT_CONNECT_FLAGS_PASSWORD)
    {
        length += (UINT)(client_ptr -> nxd_mqtt_client_password_length + 2);
    }

    /* Check for invalid length. */
    if (length > (127 * 127 * 127 * 127))
    {

#ifdef NX_SECURE_ENABLE
        if (client_ptr -> nxd_mqtt_client_use_tls)
        {
            nx_secure_tls_session_end(&(client_ptr -> nxd_mqtt_tls_session), NX_NO_WAIT);
            nx_secure_tls_session_delete(&(client_ptr -> nxd_mqtt_tls_session));
        }
#endif
        nx_tcp_socket_disconnect(&(client_ptr -> nxd_mqtt_client_socket), 0);
        nx_tcp_client_socket_unbind(&(client_ptr -> nxd_mqtt_client_socket));

        /* Disable timer if timer has been started. */
        if (client_ptr -> nxd_mqtt_keepalive)
        {
             tx_timer_delete(&(client_ptr -> nxd_mqtt_timer));
        }
        tx_mutex_put(&client_ptr -> nxd_mqtt_protection);

        return(NXD_MQTT_INTERNAL_ERROR);
    }

    status = _nxd_mqtt_packet_allocate(client_ptr, &packet_ptr);
    if (status)
    {
#ifdef NX_SECURE_ENABLE
        if (client_ptr -> nxd_mqtt_client_use_tls)
        {
            nx_secure_tls_session_end(&(client_ptr -> nxd_mqtt_tls_session), NX_NO_WAIT);
            nx_secure_tls_session_delete(&(client_ptr -> nxd_mqtt_tls_session));
        }
#endif
        nx_tcp_socket_disconnect(&(client_ptr -> nxd_mqtt_client_socket), 0);
        nx_tcp_client_socket_unbind(&(client_ptr -> nxd_mqtt_client_socket));

        /* Disable timer if timer has been started. */
        if (client_ptr -> nxd_mqtt_keepalive)
        {
             tx_timer_delete(&(client_ptr -> nxd_mqtt_timer));
        }
        tx_mutex_put(&client_ptr -> nxd_mqtt_protection);

        return(status);
    }

    if (length > ((ULONG)(packet_ptr -> nx_packet_data_end) - (ULONG)(packet_ptr -> nx_packet_append_ptr)))
    {
        nx_packet_release(packet_ptr);
        tx_mutex_put(&client_ptr -> nxd_mqtt_protection);

        /* Packet buffer is too small to hold the message. */
        return(NX_SIZE_ERROR);
    }

    /* Construct MQTT CONNECT message. */
    byte = packet_ptr -> nx_packet_prepend_ptr;

    *byte = ((MQTT_CONTROL_PACKET_TYPE_CONNECT << 4) & 0xF0);

    packet_ptr -> nx_packet_append_ptr++;
    packet_ptr -> nx_packet_length++;

    _nxd_mqtt_set_remaining_length(packet_ptr, length);

    append_message(packet_ptr, "MQTT", 4);

    /* Fill in protocol level, */
    byte = packet_ptr -> nx_packet_append_ptr;
    *byte = MQTT_PROTOCOL_LEVEL;

    /* Fill in byte 8: connect flags */
    *(byte + 1) = connection_flags;

    /* Fill in byte 9 and 10: keep alive */
    *(byte + 2) = (keepalive >> 8) & 0xFF;
    *(byte + 3) = (keepalive & 0xFF);
    packet_ptr -> nx_packet_append_ptr += 4;
    packet_ptr -> nx_packet_length += 4;

    /* Fill in payload area, in the order of: client identifier, will topic, will message,
       user name, and password. */
    append_message(packet_ptr, client_ptr -> nxd_mqtt_client_id, client_ptr -> nxd_mqtt_client_id_length);

    /* Next fill will topic and will message if the will flag is set. */
    if (connection_flags & MQTT_CONNECT_FLAGS_WILL_FLAG)
    {
        append_message(packet_ptr, (CHAR *)client_ptr -> nxd_mqtt_client_will_topic,
                       client_ptr -> nxd_mqtt_client_will_topic_length);
        append_message(packet_ptr, (CHAR *)client_ptr -> nxd_mqtt_client_will_message,
                       client_ptr -> nxd_mqtt_client_will_message_length);
    }

    /* Fill username if username flag is set */
    if (connection_flags & MQTT_CONNECT_FLAGS_USERNAME)
    {
        append_message(packet_ptr, client_ptr -> nxd_mqtt_client_username, client_ptr -> nxd_mqtt_client_username_length);
    }

    /* Fill password if password flag is set */
    if (connection_flags & MQTT_CONNECT_FLAGS_PASSWORD)
    {
        append_message(packet_ptr, client_ptr -> nxd_mqtt_client_password, client_ptr -> nxd_mqtt_client_password_length);
    }

    /* Ready to send the connect message to the server. */
#ifdef NX_SECURE_ENABLE
    if (client_ptr -> nxd_mqtt_client_use_tls)
    {
        status = nx_secure_tls_session_send(&(client_ptr -> nxd_mqtt_tls_session), packet_ptr, wait_option);
    }
    else
    {
        status = nx_tcp_socket_send(&client_ptr -> nxd_mqtt_client_socket, packet_ptr, wait_option);
    }
#else
    status = nx_tcp_socket_send(&client_ptr -> nxd_mqtt_client_socket, packet_ptr, wait_option);

#endif /* NX_SECURE_ENABLE */


    if (status)
    {

        nx_packet_release(packet_ptr);

#ifdef NX_SECURE_ENABLE
        if (client_ptr -> nxd_mqtt_client_use_tls)
        {
            nx_secure_tls_session_end(&(client_ptr -> nxd_mqtt_tls_session), NX_NO_WAIT);
            nx_secure_tls_session_delete(&(client_ptr -> nxd_mqtt_tls_session));
        }
#endif
        nx_tcp_socket_disconnect(&(client_ptr -> nxd_mqtt_client_socket), 0);
        nx_tcp_client_socket_unbind(&(client_ptr -> nxd_mqtt_client_socket));
        
        /* Disable timer if timer has been started. */
        if (client_ptr -> nxd_mqtt_keepalive)
        {
             tx_timer_delete(&(client_ptr -> nxd_mqtt_timer));
        }

        /* Release mutex */
        tx_mutex_put(&client_ptr -> nxd_mqtt_protection);
        return(NXD_MQTT_COMMUNICATION_FAILURE);
    }

    /* Update the timeout value. */
    client_ptr -> nxd_mqtt_timeout = tx_time_get() + client_ptr -> nxd_mqtt_keepalive;

    ret = NXD_MQTT_COMMUNICATION_FAILURE;

    /* Wait for response. */
    packet_ptr = NX_NULL;

#ifdef NX_SECURE_ENABLE
    if (client_ptr -> nxd_mqtt_client_use_tls)
    {
        status = nx_secure_tls_session_receive(&(client_ptr -> nxd_mqtt_tls_session), &packet_ptr, wait_option);
    }
    else
    {
        status = nx_tcp_socket_receive(&client_ptr -> nxd_mqtt_client_socket, &packet_ptr, wait_option);
    }
#else
    status = nx_tcp_socket_receive(&client_ptr -> nxd_mqtt_client_socket, &packet_ptr, wait_option);
#endif /* NX_SECURE_ENABLE */

    /* Release mutex */
    tx_mutex_put(&client_ptr -> nxd_mqtt_protection);

    if (status)
    {
        ret = NXD_MQTT_COMMUNICATION_FAILURE;
    }
    else if (packet_ptr -> nx_packet_length != sizeof(MQTT_PACKET_CONNACK))
    {
        /* Invalid packet length.  Free the packet and process error. */
        ret = NXD_MQTT_SERVER_MESSAGE_FAILURE;
    }
    else
    {
        connack_packet_ptr = (MQTT_PACKET_CONNACK *)(packet_ptr -> nx_packet_prepend_ptr);
        if (connack_packet_ptr -> mqtt_connack_packet_remaining_length != 2)
        {
            ret = NXD_MQTT_SERVER_MESSAGE_FAILURE;
        }
        /* Follow MQTT-3.2.2-1 rule.  */
        else if ((clean_session) && (connack_packet_ptr -> mqtt_connack_packet_ack_flags & MQTT_CONNACK_CONNECT_FLAGS_SP))
        {
            /* Client requested clean session, and server responded with Session Present.  This is a violation. */
            ret = NXD_MQTT_SERVER_MESSAGE_FAILURE;
        }
        else if (connack_packet_ptr -> mqtt_connack_packet_return_code >  MQTT_CONNACK_CONNECT_RETURN_CODE_NOT_AUTHORIZED)
        {
            ret = NXD_MQTT_SERVER_MESSAGE_FAILURE;
        }
        else if (connack_packet_ptr -> mqtt_connack_packet_return_code > 0)
        {
            /* Pass the server return code to the application. */
            ret = (UINT)(NXD_MQTT_ERROR_CONNECT_RETURN_CODE + connack_packet_ptr -> mqtt_connack_packet_return_code);
        }
        else
        {
            ret = NXD_MQTT_SUCCESS;

            /* Obtain mutex before we modify client control block. */
            tx_mutex_get(&client_ptr -> nxd_mqtt_protection, NX_WAIT_FOREVER);

            client_ptr -> nxd_mqtt_client_state = NXD_MQTT_CLIENT_STATE_CONNECTED;

            /* Initialize the packet identificaiton field. */
            client_ptr -> nxd_mqtt_client_packet_identifier = NXD_MQTT_INITIAL_PACKET_ID_VALUE;
            
            /* Prevent packet identifier from being zero. MQTT-2.3.1-1 */
            if(client_ptr -> nxd_mqtt_client_packet_identifier == 0)
                client_ptr -> nxd_mqtt_client_packet_identifier = 1;

            /* Release mutex */
            tx_mutex_put(&client_ptr -> nxd_mqtt_protection);
        }
    }

    /* Record the keepalive timer. Conver the keepalive value (in seconds) into ThreadX tick value. */
    client_ptr -> nxd_mqtt_keepalive = keepalive * NX_IP_PERIODIC_RATE;

    /* All done.  Release packet. */
    if (packet_ptr)
    {
        nx_packet_release(packet_ptr);
    }

    if (ret == NXD_MQTT_SUCCESS)
    {

        /* If client doesn't start with Clean Session, and there are un-acked PUBLISH messages,
           we shall re-publish these messages. */
        if ((clean_session != NX_TRUE) && (client_ptr -> message_transmit_queue_head))
        {

            tx_mutex_get(&client_ptr -> nxd_mqtt_protection, NX_WAIT_FOREVER);

            /* There are messages from the previous session that has not been acknowledged. */
            ret = _nxd_mqtt_client_retransmit_message(client_ptr);

            /* Release mutex */
            tx_mutex_put(&client_ptr -> nxd_mqtt_protection);
        }
    }
    if (ret != NXD_MQTT_SUCCESS)
    {
#ifdef NX_SECURE_ENABLE
        if (client_ptr -> nxd_mqtt_client_use_tls)
        {
            nx_secure_tls_session_end(&(client_ptr -> nxd_mqtt_tls_session), NX_NO_WAIT);
            nx_secure_tls_session_delete(&(client_ptr -> nxd_mqtt_tls_session));
        }
#endif

        /* Close the tcp connection. */
        nx_tcp_socket_disconnect(&(client_ptr -> nxd_mqtt_client_socket), 1);

        /* Unbind the socket so the application can call MQTT Connect again. */
        nx_tcp_client_socket_unbind(&client_ptr -> nxd_mqtt_client_socket);

        /* Disable timer if timer has been started. */
        if (client_ptr -> nxd_mqtt_keepalive)
        {
            tx_timer_delete(&(client_ptr -> nxd_mqtt_timer));
        }        
    }

    return(ret);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_secure_connect                     PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function makes an initial secure (TLS) connection to           */
/*    the MQTT server.                                                    */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    server_ip                             Server IP address structure   */
/*    server_port                           Server port number, in host   */
/*                                            byte order                  */
/*    tls_setup                             User-supplied callback        */
/*                                            function to set up TLS      */
/*                                            parameters.                 */
/*    username                              User name, or NULL if user    */
/*                                            name is not required        */
/*    username_length                       Length of the user name, or   */
/*                                            0 if user name is NULL      */
/*    password                              Password string, or NULL if   */
/*                                            password is not required    */
/*    password_length                       Length of the password, or    */
/*                                            0 if password is NULL       */
/*    connection_flag                       Flag psassed to the server    */
/*    timeout                               Timeout value                 */
/*                                                                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_client_connect              Actual MQTT Client connect    */
/*                                            call                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s), checked  */
/*                                            the return code of TLS setup*/
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
#ifdef NX_SECURE_ENABLE
UINT _nxd_mqtt_client_secure_connect(NXD_MQTT_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                     UINT (*tls_setup)(NXD_MQTT_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *,
                                                       NX_SECURE_X509_CERT *, NX_SECURE_X509_CERT *),
                                     UINT keepalive, UINT clean_session, ULONG wait_option)
{
UINT ret;

    /* Set up TLS session information. */
    ret = (*tls_setup)(client_ptr, &client_ptr -> nxd_mqtt_tls_session,
                       &client_ptr -> nxd_mqtt_tls_certificate,
                       &client_ptr -> nxd_mqtt_tls_trusted_certificate);

    if (ret)
    {
        return(ret);
    }

    /* Mark the connection as seucre. */
    client_ptr -> nxd_mqtt_client_use_tls = 1;

    ret = _nxd_mqtt_client_connect(client_ptr, server_ip, server_port, keepalive, clean_session, wait_option);

    return(ret);
}

#endif /* NX_SECURE_ENABLE */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_delete                             PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function deletes a previously created MQTT client instance.    */
/*    If the NXD_MQTT_SOCKET_TIMEOUT is set to NX_WAIT_FOREVER, this may  */
/*    suspend indefinately. This is because the MQTT Client must          */
/*    disconnect with the server, and if the network link is disabled or  */
/*    the server is not responding, this will blocks this function from   */
/*    completing.                                                         */ 
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_event_flags_set                                                  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            added logic to delete MQTT  */
/*                                            client thread,              */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nxd_mqtt_client_delete(NXD_MQTT_CLIENT *client_ptr)
{


    /* Set the event flag for DELETE. Next time when the MQTT client thread
       wakes up, it will perform the deletion process. */
    tx_event_flags_set(&client_ptr -> nxd_mqtt_events, MQTT_DELETE_EVENT, TX_OR);

    /* Check if the MQTT client thread has completed. */
    while((client_ptr -> nxd_mqtt_client_socket).nx_tcp_socket_id != 0) 
    {
        tx_thread_sleep(NX_IP_PERIODIC_RATE);
    }

    /* Now we can delete the Client instance. */
    tx_thread_delete(&(client_ptr -> nxd_mqtt_thread));

    return(NXD_MQTT_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_publish                            PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function publishes a message to the connected broker.          */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    topic_name                            Name of the topic             */
/*    topic_name_length                     Length of the topic name      */
/*    message                               Message string                */
/*    message_length                        Length of the message,        */
/*                                            in bytes                    */
/*    retain                                The retain flag, whether      */
/*                                            or not the broker should    */
/*                                            store this message          */
/*    QoS                                   Expected QoS level            */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                                                        */
/*    _nxd_mqtt_packet_allocate                                           */
/*    append_message                                                      */
/*    tx_mutex_put                                                        */
/*    nx_tcp_socket_send                                                  */
/*    nx_secure_tls_session_send                                          */
/*    nx_packet_release                                                   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comments, added      */
/*                                            logic to prevent packet id  */
/*                                            value from being zero, fixed*/
/*                                            a packet leak issue, used   */
/*                                            packet append API instead   */
/*                                            of memcpy to prevent data   */
/*                                            overflow the NX_PACKET,     */
/*                                            supported user defined      */
/*                                            memory functions, checked   */
/*                                            topic name and message      */
/*                                            lengths in runtime,         */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nxd_mqtt_client_publish(NXD_MQTT_CLIENT *client_ptr, CHAR *topic_name, UINT topic_name_length,
                              CHAR *message, UINT message_length, UINT retain, UINT QoS, ULONG wait_option)
{

NX_PACKET *packet_ptr;
UINT       status;
UINT       length = 0;
UCHAR      flags;
UCHAR     *byte;
UINT       ret = NXD_MQTT_SUCCESS;

    /* Validate message length. */
    if (message_length > NXD_MQTT_MAX_MESSAGE_LENGTH)
    {
        return(NXD_MQTT_INVALID_PARAMETER);
    }

    /* Validate topic_name */
    if (topic_name_length > NXD_MQTT_MAX_TOPIC_NAME_LENGTH)
    {
        return(NXD_MQTT_INVALID_PARAMETER);
    }

    if (QoS == 2)
    {
        return(NXD_MQTT_QOS2_NOT_SUPPORTED);
    }


    /* Obtain the mutex. */
    status = tx_mutex_get(&client_ptr -> nxd_mqtt_protection, NX_WAIT_FOREVER);

    if (status != TX_SUCCESS)
    {
        return(NXD_MQTT_MUTEX_FAILURE);
    }

    /* Do nothing if the client is already connected. */
    if (client_ptr -> nxd_mqtt_client_state != NXD_MQTT_CLIENT_STATE_CONNECTED)
    {
        tx_mutex_put(&client_ptr -> nxd_mqtt_protection);
        return(NXD_MQTT_NOT_CONNECTED);
    }

    status = _nxd_mqtt_packet_allocate(client_ptr, &packet_ptr);

    if (status != NXD_MQTT_SUCCESS)
    {
        tx_mutex_put(&client_ptr -> nxd_mqtt_protection);
        return(NXD_MQTT_PACKET_POOL_FAILURE);
    }

    byte = packet_ptr -> nx_packet_prepend_ptr;

    flags = (UCHAR)((MQTT_CONTROL_PACKET_TYPE_PUBLISH << 4) | (QoS << 1));

    if (retain)
    {
        flags = flags | MQTT_PUBLISH_RETAIN;
    }


    *byte = flags;

    packet_ptr -> nx_packet_append_ptr = packet_ptr -> nx_packet_prepend_ptr + 1;
    packet_ptr -> nx_packet_length = 1;


    /* Compute the remaining length. */

    /* Topic Name. */
    /* Compute Topic Name length. */
    length = topic_name_length + 2;

    /* Count packet ID for QoS 1 or QoS 2 message. */
    if ((QoS == 1) || (QoS == 2))
    {
        length += 2;
    }

    /* Count message. */
    if ((message != NX_NULL) && (message_length != 0))
    {
        length += message_length;
    }

    if (length > ((ULONG)(packet_ptr -> nx_packet_data_end) - (ULONG)(packet_ptr -> nx_packet_append_ptr)))
    {
        nx_packet_release(packet_ptr);
        tx_mutex_put(&client_ptr -> nxd_mqtt_protection);

        /* Packet buffer is too small to hold the message. */
        return(NX_SIZE_ERROR);
    }

    /* Write out the remaining length field. */
    _nxd_mqtt_set_remaining_length(packet_ptr, length);

    /* Write out topic */
    append_message(packet_ptr, topic_name, topic_name_length);

    /* Append Packet Identifier for QoS level 1 or 2  MQTT 3.3.2.2 */
    if ((QoS == 1) || (QoS == 2))
    {
        *(packet_ptr -> nx_packet_append_ptr) = (UCHAR)(client_ptr -> nxd_mqtt_client_packet_identifier >> 8);
        *(packet_ptr -> nx_packet_append_ptr + 1) = (client_ptr -> nxd_mqtt_client_packet_identifier & 0xFF);
        packet_ptr -> nx_packet_length += 2;
        packet_ptr -> nx_packet_append_ptr += 2;
    }

    /* Append message. */
    if ((message != NX_NULL) && (message_length) != 0)
    {
        
        /* Use nx_packet_data_append to move user-supplied message data into the packet.
           nx_packet_data_append uses chained packet if the additional storage space is 
           needed. */
        ret = nx_packet_data_append(packet_ptr, message, message_length, 
                                       client_ptr -> nxd_mqtt_client_packet_pool_ptr, wait_option);
        if(ret)
        {
            /* Unable to obtain a new packet to store the message. */

            /* Release the mutex. */
            tx_mutex_put(&client_ptr -> nxd_mqtt_protection);

            /* Release the packet. */
            nx_packet_release(packet_ptr);

            return(NXD_MQTT_INTERNAL_ERROR);
        }
    }

    if (QoS != 0)
    {
    /* This message needs to be stored locally for possible retransmission. */
    MQTT_MESSAGE_BLOCK *message_block;
        /* Obtain a free MQTT_MESSAGE_BLOCK. */
        if (client_ptr -> message_block_free_list == NX_NULL)
        {
            /* Release the mutex. */
            tx_mutex_put(&client_ptr -> nxd_mqtt_protection);

            /* Release the packet. */
            nx_packet_release(packet_ptr);

            return(NXD_MQTT_INTERNAL_ERROR);
        }

        message_block = client_ptr -> message_block_free_list;

        client_ptr -> message_block_free_list = message_block -> next;

        /* Record the fix header data.  Also or'ing in the DUP flag, as the re-published message
           will have the DUP flag set. */
        message_block -> fixed_header = (*(packet_ptr -> nx_packet_prepend_ptr)) | MQTT_PUBLISH_DUP_FLAG;
        message_block -> remaining_length = length;
        message_block -> packet_id = (USHORT)client_ptr -> nxd_mqtt_client_packet_identifier;
        NXD_MQTT_SECURE_MEMCPY(message_block -> topic_name, topic_name, topic_name_length);
        NXD_MQTT_SECURE_MEMCPY(message_block -> message, message, message_length);
        message_block -> topic_name_length = (USHORT)topic_name_length;
        message_block -> message_length = (USHORT)message_length;
        message_block -> next = NX_NULL;

        if (client_ptr -> message_transmit_queue_head == NX_NULL)
        {
            client_ptr -> message_transmit_queue_head = message_block;
        }
        else
        {
            client_ptr -> message_transmit_queue_tail -> next = message_block;
        }
        client_ptr -> message_transmit_queue_tail = message_block;

        client_ptr -> nxd_mqtt_client_packet_identifier = (client_ptr -> nxd_mqtt_client_packet_identifier + 1) & 0xFFFF;

        /* Prevent packet identifier from being zero. MQTT-2.3.1-1 */
        if(client_ptr -> nxd_mqtt_client_packet_identifier == 0)
            client_ptr -> nxd_mqtt_client_packet_identifier = 1;
    }

    /* Update the timeout value. */
    client_ptr -> nxd_mqtt_timeout = tx_time_get() + client_ptr -> nxd_mqtt_keepalive;


    /* Release the mutex. */
    tx_mutex_put(&client_ptr -> nxd_mqtt_protection);

    /* Ready to send the connect message to the server. */
#ifdef NX_SECURE_ENABLE
    if (client_ptr -> nxd_mqtt_client_use_tls)
    {
        status = nx_secure_tls_session_send(&(client_ptr -> nxd_mqtt_tls_session), packet_ptr, wait_option);
    }
    else
    {
        status = nx_tcp_socket_send(&client_ptr -> nxd_mqtt_client_socket, packet_ptr, wait_option);
    }
#else
    status = nx_tcp_socket_send(&client_ptr -> nxd_mqtt_client_socket, packet_ptr, wait_option);

#endif /* NX_SECURE_ENABLE */

    if (status)
    {
        /* Release the packet. */
        nx_packet_release(packet_ptr);

        ret = NXD_MQTT_COMMUNICATION_FAILURE;
    }


    return(ret);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_subscribe                          PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends a subscribe message to the broker.              */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    topic_name                            Pointer to the topic string   */
/*                                            to subscribe to             */
/*    topic_name_length                     Length of the topic string    */
/*                                            in bytes                    */
/*    QoS                                   Expected QoS level            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_client_sub_unsub            The actual routine that       */
/*                                            performs the sub/unsub      */
/*                                            acktion.                    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nxd_mqtt_client_subscribe(NXD_MQTT_CLIENT *client_ptr, CHAR *topic_name, UINT topic_name_length, UINT QoS)
{

    if (QoS == 2)
    {
        return(NXD_MQTT_QOS2_NOT_SUPPORTED);
    }

    return(_nxd_mqtt_client_sub_unsub(client_ptr, (MQTT_CONTROL_PACKET_TYPE_SUBSCRIBE << 4) | 0x02,
                                      topic_name, topic_name_length, QoS));
}




/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_unsubscribe                        PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function unsubscribes a topic from the broker.                 */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    topic_name                            Pointer to the topic string   */
/*                                            to subscribe to             */
/*    topic_name_length                     Length of the topic string    */
/*                                            in bytes                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                                                        */
/*    _nxd_mqtt_packet_allocate                                           */
/*    append_message                                                      */
/*    tx_mutex_put                                                        */
/*    nx_tcp_socket_send                                                  */
/*    nx_secure_tls_session_send                                          */
/*    nx_packet_release                                                   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nxd_mqtt_client_unsubscribe(NXD_MQTT_CLIENT *client_ptr, CHAR *topic_name, UINT topic_name_length)
{
    return(_nxd_mqtt_client_sub_unsub(client_ptr, (MQTT_CONTROL_PACKET_TYPE_UNSUBSCRIBE << 4) | 0x02,
                                      topic_name, topic_name_length, 0));
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_send_simple_message                       PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function handles the transmission of PINGREQ or       */
/*    DISCONNECT message.                                                 */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    header_value                          Value to be programmed into   */
/*                                            MQTT header.                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                                                        */
/*    _nxd_mqtt_packet_allocate                                           */
/*    tx_mutex_put                                                        */
/*    nx_tcp_socket_send                                                  */
/*    nx_secure_tls_session_send                                          */
/*    nx_packet_release                                                   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_client_ping                                               */
/*    _nxd_mqtt_client_disconnect                                         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
static UINT _nxd_mqtt_send_simple_message(NXD_MQTT_CLIENT *client_ptr, UCHAR header_value)
{

NX_PACKET *packet_ptr;
UINT       status;
UINT       status_mutex;
UCHAR     *byte;

    status = _nxd_mqtt_packet_allocate(client_ptr, &packet_ptr);
    if (status)
    {
        return(NXD_MQTT_INTERNAL_ERROR);
    }

    if (2u > ((ULONG)(packet_ptr -> nx_packet_data_end) - (ULONG)(packet_ptr -> nx_packet_append_ptr)))
    {
        nx_packet_release(packet_ptr);

        /* Packet buffer is too small to hold the message. */
        return(NX_SIZE_ERROR);
    }

    byte = packet_ptr -> nx_packet_prepend_ptr;

    *byte = (UCHAR)(header_value << 4);
    *(byte + 1) = 0;

    packet_ptr -> nx_packet_append_ptr = packet_ptr -> nx_packet_prepend_ptr + 2;
    packet_ptr -> nx_packet_length = 2;


    /* Release MQTT protection before making NetX/TLS calls. */
    tx_mutex_put(&client_ptr -> nxd_mqtt_protection);
#ifdef NX_SECURE_ENABLE
    if (client_ptr -> nxd_mqtt_client_use_tls)
    {
        status = nx_secure_tls_session_send(&(client_ptr -> nxd_mqtt_tls_session), packet_ptr, NX_WAIT_FOREVER);
    }
    else
    {
        status = nx_tcp_socket_send(&client_ptr -> nxd_mqtt_client_socket, packet_ptr, NX_WAIT_FOREVER);
    }
#else

    status = nx_tcp_socket_send(&client_ptr -> nxd_mqtt_client_socket, packet_ptr, NX_WAIT_FOREVER);

#endif /* NX_SECURE_ENABLE */

    status_mutex = tx_mutex_get(&client_ptr -> nxd_mqtt_protection, NX_WAIT_FOREVER);
    if (status)
    {

        /* Release the packet. */
        nx_packet_release(packet_ptr);

        status = NXD_MQTT_COMMUNICATION_FAILURE;
    }
    if (status_mutex)
    {
        return(NXD_MQTT_MUTEX_FAILURE);
    }

    if (header_value == MQTT_CONTROL_PACKET_TYPE_PINGREQ)
    {
        /* Do not update the ping sent time if the outstanding ping has not been responded yet */
        if (client_ptr -> nxd_mqtt_ping_not_responded != 1)
        {
            /* Record the time we send out the PINGREG */
            client_ptr -> nxd_mqtt_ping_sent_time = tx_time_get();
            client_ptr -> nxd_mqtt_ping_not_responded = 1;
        }
    }

    /* Update the timeout value. */
    client_ptr -> nxd_mqtt_timeout = tx_time_get() + client_ptr -> nxd_mqtt_keepalive;

    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_disconnect                         PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function disconnects the MQTT client from a server.            */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_send_simple_message                                       */
/*    _nxd_mqtt_process_disconnect                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            added logic to ensure the   */
/*                                            client timer is deleted,    */
/*                                            do not abort if the send    */
/*                                            message fails so that client*/
/*                                            resources can be cleared,   */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nxd_mqtt_client_disconnect(NXD_MQTT_CLIENT *client_ptr)
{
UINT status;

    /* Obtain the mutex. */
    status = tx_mutex_get(&(client_ptr -> nxd_mqtt_protection), TX_WAIT_FOREVER);
    if (status != TX_SUCCESS)
    {
        /* Disable timer if timer has been started. */
        if (client_ptr -> nxd_mqtt_keepalive)
        {
            tx_timer_delete(&(client_ptr -> nxd_mqtt_timer));
        }

        return(NXD_MQTT_MUTEX_FAILURE);
    }

    /* Let the server know we are ending the MQTT session. */
    _nxd_mqtt_send_simple_message(client_ptr, MQTT_CONTROL_PACKET_TYPE_DISCONNECT);

    /* Call the disconnect routine to disconnect the socket,
       release transmit message blocks, release receied packets,
       and delete the client timer. */
    _nxd_mqtt_process_disconnect(client_ptr);

    /* Ensure the Client timer is deleted. */
    if (client_ptr -> nxd_mqtt_keepalive)
    {
        tx_timer_delete(&(client_ptr -> nxd_mqtt_timer));
    }

    /* Release the mutex. */
    tx_mutex_put(&(client_ptr -> nxd_mqtt_protection));

    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_receive_notify_set                 PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function installs the MQTT client publish notify callback      */
/*    function.                                                           */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    mqtt_client_receive_notify            User-supplied callback        */
/*                                            function, which is invoked  */
/*                                            upon receiving a publish    */
/*                                            message.                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                                                        */
/*    tx_mutex_put                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nxd_mqtt_client_receive_notify_set(NXD_MQTT_CLIENT *client_ptr,
                                         VOID (*receive_notify)(NXD_MQTT_CLIENT *client_ptr, UINT message_count))
{

    tx_mutex_get(&client_ptr -> nxd_mqtt_protection, NX_WAIT_FOREVER);

    client_ptr -> nxd_mqtt_client_receive_notify = receive_notify;

    tx_mutex_put(&client_ptr -> nxd_mqtt_protection);

    return(NXD_MQTT_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_message_get                        PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves a published MQTT message.                   */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    topic_buffer                          Pointer to the topic buffer   */
/*                                            where topic is copied to    */
/*    topic_buffer_size                     Size of the topic buffer.     */
/*    actual_topic_length                   Number of bytes copied into   */
/*                                            topic_buffer                */
/*    message_buffer                        Pointer to the buffer where   */
/*                                            message is copied to        */
/*    message_buffer_size                   Size of the message_buffer    */
/*    actual_message_length                 Number of bytes copied into   */
/*                                            the message buffer.         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_client_disconnect           Actual MQTT Client disconnect */
/*                                            call                        */
/*    _nxd_mqtt_read_remaining_length       Skip the remaining length     */
/*                                            field                       */
/*    tx_mutex_get                                                        */
/*    tx_mutex_put                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            supported user defined      */
/*                                            memory functions,           */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nxd_mqtt_client_message_get(NXD_MQTT_CLIENT *client_ptr, UCHAR *topic_buffer, UINT topic_buffer_size, UINT *actual_topic_length,
                                  UCHAR *message_buffer, UINT message_buffer_size, UINT *actual_message_length)
{

UINT                ret = NXD_MQTT_SUCCESS;
MQTT_MESSAGE_BLOCK *message_block;

    tx_mutex_get(&client_ptr -> nxd_mqtt_protection, NX_WAIT_FOREVER);
    if (client_ptr -> message_receive_queue_depth == 0)
    {

        tx_mutex_put(&client_ptr -> nxd_mqtt_protection);
        return(NXD_MQTT_NO_MESSAGE);
    }

    message_block = client_ptr -> message_receive_queue_head;

    if ((topic_buffer_size >= message_block -> topic_name_length) &&
        (message_buffer_size >= message_block -> message_length))
    {

        client_ptr -> message_receive_queue_head = message_block -> next;

        if (client_ptr -> message_receive_queue_tail == message_block)
        {
            client_ptr -> message_receive_queue_tail = NX_NULL;
        }

        NXD_MQTT_SECURE_MEMCPY(topic_buffer, message_block -> topic_name, message_block -> topic_name_length);
        *actual_topic_length = message_block -> topic_name_length;
        NXD_MQTT_SECURE_MEMCPY(message_buffer, message_block -> message, message_block -> message_length);
        *actual_message_length = message_block -> message_length;


        message_block -> next = client_ptr -> message_block_free_list;
        client_ptr -> message_block_free_list = message_block;
        client_ptr -> message_receive_queue_depth--;
        ret = NXD_MQTT_SUCCESS;
    }
    else
    {
        ret = NXD_MQTT_INSUFFICIENT_BUFFER_SPACE;
    }
    tx_mutex_put(&client_ptr -> nxd_mqtt_protection);

    return(ret);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_mqtt_client_create                            PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in nxd_mqt_client_create call.      */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    client_name                           Name string used in by the    */
/*                                            client                      */
/*    client_id                             Client ID used by the client  */
/*    client_id_length                      Length of Client ID, in bytes */
/*    ip_ptr                                Pointer to IP instance        */
/*    pool_ptr                              Pointer to packet pool        */
/*    stack_ptr                             Client thread's stack pointer */
/*    stack_size                            Client thread's stack size    */
/*    mqtt_thread_priority                  Priority for MQTT thread      */
/*    memory_ptr                            Pointer to memory block the   */
/*                                            client can use              */
/*    memory_size                           Size of the memory area       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_client_create               Actual client create call     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nxde_mqtt_client_create(NXD_MQTT_CLIENT *client_ptr, CHAR *client_name, CHAR *client_id, UINT client_id_length,
                              NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr,
                              VOID *stack_ptr, ULONG stack_size, UINT mqtt_thread_priority,
                              VOID *memory_ptr, ULONG memory_size)
{


    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID) ||
        (stack_ptr == NX_NULL) || (stack_size == 0) || (pool_ptr == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    if ((memory_ptr == NX_NULL) || (memory_size < sizeof(MQTT_MESSAGE_BLOCK)))
    {
        return(NXD_MQTT_INVALID_PARAMETER);
    }

    return(_nxd_mqtt_client_create(client_ptr, client_name, client_id, client_id_length, ip_ptr,
                                   pool_ptr, stack_ptr, stack_size, mqtt_thread_priority,
                                   memory_ptr, memory_size));
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_mqtt_client_connect                           PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in MQTT client stop call.           */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    server_ip                             Server IP address structure   */
/*    server_port                           Server port number, in host   */
/*                                            byte order                  */
/*    keepalive                             The MQTT keepalive timer      */
/*    clean_session                         Clean session flag            */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_client_connect              Actual MQTT Client connect    */
/*                                            call                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nxde_mqtt_client_connect(NXD_MQTT_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                               UINT keepalive, UINT clean_session, ULONG wait_option)
{

UINT status;

    if (client_ptr == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    if (server_ip == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Test for IP version flag. */
    if ((server_ip -> nxd_ip_version != 4) && (server_ip -> nxd_ip_version != 6))
    {
        return(NXD_MQTT_INVALID_PARAMETER);
    }

    if (server_port == 0)
    {
        return(NX_INVALID_PORT);
    }


    if (client_ptr -> nxd_mqtt_client_state == NXD_MQTT_CLIENT_STATE_INITIALIZE)
    {
        return(NXD_MQTT_CLIENT_NOT_RUNNING);
    }

    status = _nxd_mqtt_client_connect(client_ptr, server_ip, server_port, keepalive, clean_session, wait_option);

    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_mqtt_client_secure_connect                    PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in MQTT client TLS secure connect.  */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    server_ip                             Server IP address structure   */
/*    server_port                           Server port number, in host   */
/*                                            byte order                  */
/*    tls_setup                             User-supplied callback        */
/*                                            function to set up TLS      */
/*                                            parameters.                 */
/*    keepalive                             The MQTT keepalive timer      */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_client_connect              Actual MQTT Client connect    */
/*                                            call                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
#ifdef NX_SECURE_ENABLE

UINT _nxde_mqtt_client_secure_connect(NXD_MQTT_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                      UINT (*tls_setup)(NXD_MQTT_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *,
                                                        NX_SECURE_X509_CERT *, NX_SECURE_X509_CERT *),
                                      UINT keepalive, UINT clean_session, ULONG wait_option)
{

UINT status;

    if (client_ptr == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    if (server_ip == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    if (server_port == 0)
    {
        return(NX_INVALID_PORT);
    }

    if (client_ptr -> nxd_mqtt_client_state == NXD_MQTT_CLIENT_STATE_INITIALIZE)
    {
        return(NXD_MQTT_CLIENT_NOT_RUNNING);
    }

    status = _nxd_mqtt_client_secure_connect(client_ptr, server_ip, server_port, tls_setup,
                                             keepalive, clean_session, wait_option);

    return(status);
}
#endif /* NX_SECURE_ENABLE */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_mqtt_client_delete                            PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in MQTT client delete call.         */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_client_delete               Actual MQTT Client delete     */
/*                                            call.                       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nxde_mqtt_client_delete(NXD_MQTT_CLIENT *client_ptr)
{

UINT status;

    if (client_ptr == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    status = _nxd_mqtt_client_delete(client_ptr);

    return(status);
}



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_mqtt_client_publish                           PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs error checking to the publish service.       */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    topic_name                            Name of the topic             */
/*    topic_name_length                     Length of the topic name      */
/*    message                               Message string                */
/*    message_length                        Length of the message,        */
/*                                            in bytes                    */
/*    retain                                The retain flag, whether      */
/*                                            or not the broker should    */
/*                                            store this message          */
/*    QoS                                   Expected QoS level            */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            checked topic name and      */
/*                                            message lengths in runtime, */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nxde_mqtt_client_publish(NXD_MQTT_CLIENT *client_ptr, CHAR *topic_name, UINT topic_name_length,
                               CHAR *message, UINT message_length, UINT retain, UINT QoS, ULONG wait_option)
{
    /* Validate client_ptr */
    if (client_ptr == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Validate topic_name */
    if ((topic_name == NX_NULL) || (topic_name_length == 0))
    {
        return(NXD_MQTT_INVALID_PARAMETER);
    }

    /* Validate message length. */
    if (message && (message_length == 0))
    {
        return(NXD_MQTT_INVALID_PARAMETER);
    }

    /* Validate QoS value. */
    if (QoS > 3)
    {
        return(NXD_MQTT_INVALID_PARAMETER);
    }

    return(_nxd_mqtt_client_publish(client_ptr, topic_name, topic_name_length, message, message_length, retain, QoS, wait_option));
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_mqtt_client_subscribe                         PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs error checking to the subscribe service.     */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    topic_name                            Pointer to the topic string   */
/*                                            to subscribe to             */
/*    topic_name_length                     Length of the topic string    */
/*                                            in bytes                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    topic_name                            Pointer to the topic string   */
/*                                            to subscribe to             */
/*    topic_name_length                     Length of the topic string    */
/*                                            in bytes                    */
/*    QoS                                   Expected QoS level            */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_client_subscribe                                          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nxde_mqtt_client_subscribe(NXD_MQTT_CLIENT *client_ptr, CHAR *topic_name, UINT topic_name_length, UINT QoS)
{


    /* Validate client_ptr */
    if (client_ptr == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Validate topic_name */
    if ((topic_name == NX_NULL) || (topic_name_length == 0))
    {
        return(NXD_MQTT_INVALID_PARAMETER);
    }

    /* Validate QoS value. */
    if (QoS > 2)
    {
        return(NXD_MQTT_INVALID_PARAMETER);
    }

    return(_nxd_mqtt_client_subscribe(client_ptr, topic_name, topic_name_length, QoS));
}




/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_mqtt_client_unsubscribe                       PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs error checking to the unsubscribe service.   */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    topic_name                            Pointer to the topic string   */
/*                                            to unsubscribe              */
/*    topic_name_length                     Length of the topic string    */
/*                                            in bytes                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nxde_mqtt_client_unsubscribe(NXD_MQTT_CLIENT *client_ptr, CHAR *topic_name, UINT topic_name_length)
{
    /* Validate client_ptr */
    if (client_ptr == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Validate topic_name */
    if ((topic_name == NX_NULL) || (topic_name_length == 0))
    {
        return(NXD_MQTT_INVALID_PARAMETER);
    }

    return(_nxd_mqtt_client_unsubscribe(client_ptr, topic_name, topic_name_length));
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_mqtt_client_disconnect                        PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in MQTT client disconnect call.     */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_client_disconnect           Actual MQTT Client disconnect */
/*                                            call                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nxde_mqtt_client_disconnect(NXD_MQTT_CLIENT *client_ptr)
{
    /* Validate client_ptr */
    if (client_ptr == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    return(_nxd_mqtt_client_disconnect(client_ptr));
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_mqtt_client_message_get                       PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in MQTT client message get call.    */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    topic_buffer                          Pointer to the topic buffer   */
/*                                            where topic is copied to    */
/*    topic_buffer_size                     Size of the topic buffer.     */
/*    actual_topic_length                   Number of bytes copied into   */
/*                                            topic_buffer                */
/*    message_buffer                        Pointer to the buffer where   */
/*                                            message is copied to        */
/*    message_buffer_size                   Size of the message_buffer    */
/*    actual_message_length                 Number of bytes copied into   */
/*                                            the message buffer.         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_client_message_get                                        */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nxde_mqtt_client_message_get(NXD_MQTT_CLIENT *client_ptr, UCHAR *topic_buffer, UINT topic_buffer_size, UINT *actual_topic_length,
                                   UCHAR *message_buffer, UINT message_buffer_size, UINT *actual_message_length)
{

    /* Validate client_ptr */
    if (client_ptr == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Topic and topic_length can be NULL if caller does not care the topic string. */

    /* Validate message.  Message_length can be NULL if caller does not care message length. */
    if ((message_buffer == NX_NULL) || (topic_buffer == NX_NULL))
    {
        return(NXD_MQTT_INVALID_PARAMETER);
    }


    return(_nxd_mqtt_client_message_get(client_ptr, topic_buffer, topic_buffer_size, actual_topic_length,
                                        message_buffer, message_buffer_size, actual_message_length));
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_mqtt_client_receive_notify_set                PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in MQTT client publish notify call. */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    receive_notify                        User-supplied callback        */
/*                                            function, which is invoked  */
/*                                            upon receiving a publish    */
/*                                            message.                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_client_receive_notify_set                                 */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nxde_mqtt_client_receive_notify_set(NXD_MQTT_CLIENT *client_ptr,
                                          VOID (*receive_notify)(NXD_MQTT_CLIENT *client_ptr, UINT message_count))
{
    /* Validate cient_ptr */
    if (client_ptr == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    if (receive_notify == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    return(_nxd_mqtt_client_receive_notify_set(client_ptr, receive_notify));
}



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_disconnect_notify_set              PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets the notify function for the disconnect event.    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    disconnect_notify                     The notify function to be     */
/*                                            used when the client is     */
/*                                            disconnected from the       */
/*                                            server.                     */
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
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nxd_mqtt_client_disconnect_notify_set(NXD_MQTT_CLIENT *client_ptr, VOID (*disconnect_notify)(NXD_MQTT_CLIENT *))
{

    client_ptr -> nxd_mqtt_disconnect_notify = disconnect_notify;

    return(NXD_MQTT_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_mqtt_client_disconnect_notify_set             PORTABLE C      */
/*                                                           5.12         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Express Logic, Inc.                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in setting MQTT client disconnect   */
/*    callback fucntion.                                                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    disconnect_callback                   The callback function to be   */
/*                                            used when an on-going       */
/*                                            connection is disconnected. */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_client_disconnect_notify_set                              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Yuxin Zhou               Initial Version 5.10          */
/*  07-15-2018     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.11   */
/*  08-15-2019     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 5.12   */
/*                                                                        */
/**************************************************************************/
UINT _nxde_mqtt_client_disconnect_notify_set(NXD_MQTT_CLIENT *client_ptr, VOID (*disconnect_notify)(NXD_MQTT_CLIENT *))
{

    /* Validate cient_ptr */
    if (client_ptr == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }
    return(_nxd_mqtt_client_disconnect_notify_set(client_ptr, disconnect_notify));
}

