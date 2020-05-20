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

#include "nx_api.h"
#include "nx_ip.h"
#include "nx_ipv4.h"
#include "nx_ipv6.h"
#include "nx_tcp.h"
#include "nx_udp.h"
#include "tx_api.h"

#include "asc_security_core/collectors/connection_create.h"
#include "asc_security_core/logger.h"
#include "asc_security_core/message_schema_consts.h"
#include "asc_security_core/model/event.h"
#include "asc_security_core/utils/itime.h"
#include "asc_security_core/utils/notifier.h"
#include "asc_security_azurertos/collectors/connection_create_port.h"

#include "asc_security_azurertos/azurertos_configuration.h"

/**
 * @brief   Switch between the 2 hashtables.
 */
static void _collector_connection_create_switch_hashtables();


/**
 * @brief   A hashset_connection_create_t_for_each function. Appends the schema payload to the event,
 *          pointed by context and then deinitializes the schema struct.
 *
 * @param   data_ptr The payload struct
 * @param   context     The event handle
 */
static void _collector_connection_create_append_payload_to_list(connection_create_t* data_ptr, void* context);

/**
 * @brief   Parse an IPv4 packet to an event payload struct.
 *
 * @param   ip_packet   Pointer to the IPv4 packet data, in network byte-order.
 * @param   direction   The direction of the packet (NX_IP_PACKET_IN / NX_IP_PACKET_OUT)
 *
 * @return  connection_create_t*
 */
static connection_create_t* _collector_connection_create_ipv4_callback(VOID* ip_packet, UINT direction);


/**
 * @brief   Parse an IPv6 packet to an event payload struct.
 *
 * @param   ip_packet   Pointer to the IPv6 packet data, in network byte-order.
 * @param   direction   The direction of the packet (NX_IP_PACKET_IN / NX_IP_PACKET_OUT)
 *
 * @return  connection_create_t*
 */
static connection_create_t* _collector_connection_create_ipv6_callback(VOID* ip_packet, UINT direction);

static void _collector_connection_create_deinit(collector_internal_t* collector_internal_ptr);
static IOTSECURITY_RESULT _collector_connection_create_get_events(collector_internal_t* collector_internal_ptr, linked_list_event_t_handle events);

static void _connection_create_schema_deinit_with_context(connection_create_t* data_ptr, void* context) {
    schema_connection_create_deinit(data_ptr);
}

static collector_internal_t* _collector_internal_ptr = NULL;

static connection_create_t* _hashtables[2][CONNECTION_CREATE_HASHSET_SIZE] = { { NULL } };
static connection_create_t** _current_hashtable = _hashtables[0];
static int _current_hashtable_index = 0;


IOTSECURITY_RESULT collector_connection_create_init(collector_internal_t* collector_internal_ptr) {
    if (_collector_internal_ptr != NULL) {
        return IOTSECURITY_RESULT_OK;
    }

    if (collector_internal_ptr == NULL) {
        log_error("Could not initialize collector_connection_create, bad argument");
        return IOTSECURITY_RESULT_BAD_ARGUMENT;
    }

    hashset_connection_create_t_init(_hashtables[0]);
    hashset_connection_create_t_init(_hashtables[1]);
    _current_hashtable_index = 0;
    _current_hashtable = _hashtables[_current_hashtable_index];

    _collector_internal_ptr = collector_internal_ptr;

    _collector_internal_ptr->collect_function = _collector_connection_create_get_events;
    _collector_internal_ptr->deinit_function = _collector_connection_create_deinit;
    _collector_internal_ptr->priority = COLLECTOR_PRIORITY_HIGH;
    _collector_internal_ptr->state = NULL;
    _collector_internal_ptr->type = COLLECTOR_TYPE_CONNECTION_CREATE;

    if (collector_connection_create_port_init() != IOTSECURITY_RESULT_OK) {
        _collector_connection_create_deinit(_collector_internal_ptr);
    } else {
        notifier_notify(NOTIFY_TOPIC_SYSTEM, NOTIFY_MESSAGE_SYSTEM_CONFIGURATION, _collector_internal_ptr);
    }

    return IOTSECURITY_RESULT_OK;
}

static void _collector_connection_create_deinit(collector_internal_t* collector_internal_ptr) {
    if (_collector_internal_ptr == NULL) {
        return;
    }

    collector_connection_create_port_deinit();

    _current_hashtable = NULL;
    hashset_connection_create_t_clear(_hashtables[0], _connection_create_schema_deinit_with_context, NULL);
    hashset_connection_create_t_clear(_hashtables[1], _connection_create_schema_deinit_with_context, NULL);
    _collector_internal_ptr = NULL;
}

static IOTSECURITY_RESULT _collector_connection_create_get_events(collector_internal_t* collector_internal_ptr, linked_list_event_t_handle events) {
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;
    event_t* event_ptr = NULL;
    linked_list_connection_create_t payload_list;

    if (_collector_internal_ptr == NULL) {
        log_error("Collector%s uninitialized, cannot collect", CONNECTION_CREATE_NAME);
        result = IOTSECURITY_RESULT_UNINITIALIZED;
        goto cleanup;
    }

    if (events == NULL) {
        log_error("Collector%s bad argument, events=[NULL]", CONNECTION_CREATE_NAME);
        result = IOTSECURITY_RESULT_BAD_ARGUMENT;
        goto cleanup;
    }

    time_t current_time = itime_time(NULL);

    connection_create_t** previous_hashtable = _current_hashtable;
    _collector_connection_create_switch_hashtables();

    linked_list_connection_create_t_init(&payload_list, schema_connection_create_deinit);
    hashset_connection_create_t_clear(previous_hashtable, _collector_connection_create_append_payload_to_list, &payload_list);

    event_ptr = event_init(CONNECTION_CREATE_PAYLOAD_SCHEMA_VERSION, CONNECTION_CREATE_NAME, EVENT_TRIGGERED_CATEGORY, EVENT_TYPE_SECURITY_VALUE, current_time);
    if (event_ptr == NULL) {
        log_warn("Collector%s failed to initialize an Event, event data dropped", CONNECTION_CREATE_NAME);
        result = IOTSECURITY_RESULT_EXCEPTION;
        goto cleanup;
    }

    while (linked_list_connection_create_t_get_size(&payload_list) > 0) {
        connection_create_t* data_handle = linked_list_connection_create_t_get_first(&payload_list);
        if (event_append_connection_create(event_ptr, data_handle) == IOTSECURITY_RESULT_OK) {
            schema_connection_create_deinit(linked_list_connection_create_t_remove_first(&payload_list));
        } else {
            result = event_build(event_ptr);
            if (result != IOTSECURITY_RESULT_OK) {
                log_error("Collector%s failed to build event, event dropped", CONNECTION_CREATE_NAME);
                goto cleanup;
            }

            if (linked_list_event_t_add_last(events, event_ptr) == NULL) {
                log_error("Collector%s failed to add event to list, event dropped", CONNECTION_CREATE_NAME);
                goto cleanup;
            }

            event_ptr = event_init(CONNECTION_CREATE_PAYLOAD_SCHEMA_VERSION, CONNECTION_CREATE_NAME, EVENT_TRIGGERED_CATEGORY, EVENT_TYPE_SECURITY_VALUE, current_time);
            if (event_ptr == NULL) {
                log_warn("Collector%s failed to initialize an Event, event data dropped", CONNECTION_CREATE_NAME);
                result = IOTSECURITY_RESULT_EXCEPTION;
                goto cleanup;
            }
        }
    }

    result = event_build(event_ptr);
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Collector%s failed to build event, event dropped", CONNECTION_CREATE_NAME);
        goto cleanup;
    }

    if (linked_list_event_t_add_last(events, event_ptr) == NULL) {
        log_error("Collector%s failed to add event to list, event dropped", CONNECTION_CREATE_NAME);
        goto cleanup;
    }

cleanup:
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Collector%s failed to collect events", CONNECTION_CREATE_NAME);
        event_deinit(event_ptr);
        event_ptr = NULL;
    }

    linked_list_connection_create_t_deinit(&payload_list);

    return result;
}

IOTSECURITY_RESULT collector_connection_create_port_init() {
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;

    NX_IP *nx_ip_ptr = _nx_ip_created_ptr;
    ULONG created_count = _nx_ip_created_count;

    if (nx_ip_ptr == NULL) {
        log_error("Collector%s cannot initialize internal port. _nx_ip_created_ptr=[NULL]", CONNECTION_CREATE_NAME);
        result = IOTSECURITY_RESULT_EXCEPTION;
        goto cleanup;
    }

    while (created_count--) {
        nx_ip_ptr->nx_ip_packet_filter = collector_connection_create_port_ip_callback;
        nx_ip_ptr = nx_ip_ptr->nx_ip_created_next;
    }

cleanup:
    return result;
}


UINT collector_connection_create_port_ip_callback(VOID* ip_packet, UINT direction) {
    uint8_t version = (*(uint8_t*)ip_packet) >> 4;
    connection_create_t* schema_handle = NULL;

    if (version == NX_IP_VERSION_V4) {
        schema_handle = _collector_connection_create_ipv4_callback(ip_packet, direction);
    } else if (version == NX_IP_VERSION_V6) {
        schema_handle = _collector_connection_create_ipv6_callback(ip_packet, direction);
    } else {
        log_error("Collector%s illegal IP version, event dropped", CONNECTION_CREATE_NAME);
        goto cleanup;
    }

    if (schema_handle == NULL) {
        log_error("Collector%s could not initialize schema struct, event dropped", CONNECTION_CREATE_NAME);
        goto cleanup;
    }

    hashset_connection_create_t_add_or_update(_current_hashtable, schema_handle);

cleanup:
    // We must return NX_SUCCESS here, otherwise NetX will drop the packet
    return NX_SUCCESS;
}


void collector_connection_create_port_deinit() {
    NX_IP *nx_ip_ptr = _nx_ip_created_ptr;
    ULONG created_count = _nx_ip_created_count;

    if (nx_ip_ptr == NULL) {
        log_error("Collector%s cannot Deinitialize internal port. _nx_ip_created_ptr=[NULL]", CONNECTION_CREATE_NAME);
    }

    while (created_count--) {
        nx_ip_ptr->nx_ip_packet_filter = NULL;
        nx_ip_ptr = nx_ip_ptr->nx_ip_created_next;
    }
}


static void _collector_connection_create_switch_hashtables() {
    _current_hashtable_index = (_current_hashtable_index + 1) % 2;
    _current_hashtable = _hashtables[_current_hashtable_index];
}


static void _collector_connection_create_append_payload_to_list(connection_create_t* data_ptr, void* context) {
    linked_list_connection_create_t* payload_list_ptr = (linked_list_connection_create_t*)context;
    linked_list_connection_create_t_add_last(payload_list_ptr, data_ptr);
}


static connection_create_t* _collector_connection_create_ipv4_callback(VOID* ip_packet, UINT direction) {
    uint32_t* ip_header_ptr = (uint32_t*)ip_packet;
    uint16_t ip_header_length = (uint16_t)((*(uint8_t*)ip_packet & 0x0f) * 4);

    TRANSPORT_PROTOCOL transport_protocol;

    uint32_t protocol_word = ntohl(ip_header_ptr[2]);
    uint8_t protocol = (protocol_word >> 16) & 0xff;

    switch (protocol) {
#ifdef CONFIG_CONNECTION_CREATE_TCP_ENABLED
        case NX_PROTOCOL_TCP:
            transport_protocol = TRANSPORT_PROTOCOL_TCP;
            break;
#endif
#ifdef CONFIG_CONNECTION_CREATE_UDP_ENABLED
        case NX_PROTOCOL_UDP:
            transport_protocol = TRANSPORT_PROTOCOL_UDP;
            break;
#endif
#ifdef CONFIG_CONNECTION_CREATE_ICMP_ENABLED
        case NX_PROTOCOL_ICMP:
            transport_protocol = TRANSPORT_PROTOCOL_ICMP;
            break;
#endif
        default:
            return NULL;
    }

    // For ICMP, there are no ports, thus they will get parsed incorrectly, but we don't set them.
    uint32_t* transport_header_ptr = (uint32_t*) (((uint8_t*)ip_packet) + ip_header_length);
    uint32_t ports_word = ntohl(transport_header_ptr[0]);
    uint16_t source_port = (uint16_t)(ports_word >> 16);
    uint16_t destination_port = (uint16_t)(ports_word & 0xffff);

    uint32_t ip_header_word_0 = ntohl(ip_header_ptr[0]);
    uint16_t total_bytes = (uint16_t)((ip_header_word_0 & 0xffff) - ip_header_length);

    // For the addresses we keep the network byte order. These will get parsed using inet_ntop.
    if (direction == NX_IP_PACKET_IN) {
        return schema_connection_create_create_ipv4(
            total_bytes,        // Bytes In
            0,                  // Bytes Out
            ip_header_ptr[4],   // Local Address
            ip_header_ptr[3],   // Remote Address
            destination_port,   // Local Port
            source_port,        // Remote Port
            transport_protocol  // Transport Protocol
        );
    } else { // NX_IP_PACKET_OUT
        return schema_connection_create_create_ipv4(
            0,                  // Bytes In
            total_bytes,        // Bytes Out
            ip_header_ptr[3],   // Local Address
            ip_header_ptr[4],   // Remote Address
            source_port,        // Local Port
            destination_port,   // Remote Port
            transport_protocol  // Transport Protocol
        );
    }
}


static connection_create_t* _collector_connection_create_ipv6_callback(VOID* ip_packet, UINT direction) {
    uint32_t* ip_header_ptr = (uint32_t*)ip_packet;
    uint8_t* packet_ptr = (uint8_t*)ip_packet;

    uint32_t ip_header_word_1 = ntohl(ip_header_ptr[1]);
    uint8_t next_header_type = (uint8_t)((ip_header_word_1 >> 8) & 0xff);

    uint16_t payload_length = (uint16_t)(ip_header_word_1 >> 16);
    uint16_t header_length = 40;
    packet_ptr += header_length;

    bool found_transport_header = false;

    // Traverse the IPv6 extenstion headers chain
    // See following RFCs for header length calculations:
    //  - RFC-8200 (https://tools.ietf.org/html/rfc8200)
    //  - RFC-4302 (https://tools.ietf.org/html/rfc4302)
    do {
        switch (next_header_type) {
            case NX_PROTOCOL_NEXT_HEADER_HOP_BY_HOP:
            case NX_PROTOCOL_NEXT_HEADER_DESTINATION:
            case NX_PROTOCOL_NEXT_HEADER_ROUTING:
                header_length = (uint16_t)((*(packet_ptr + 1) + 1) * 8);
                break;
            case NX_PROTOCOL_NEXT_HEADER_FRAGMENT:
                header_length = 8;
                break;
            case NX_PROTOCOL_NEXT_HEADER_AUTHENTICATION:
                header_length = (uint16_t)((*(packet_ptr + 1) + 2) * 4);
                break;
#ifdef  CONFIG_CONNECTION_CREATE_TCP_ENABLED
            case NX_PROTOCOL_TCP:
#endif
#ifdef  CONFIG_CONNECTION_CREATE_UDP_ENABLED
            case NX_PROTOCOL_UDP:
#endif
#ifdef  CONFIG_CONNECTION_CREATE_ICMP_ENABLED
            case NX_PROTOCOL_ICMPV6:
#endif
                header_length = 0;
                found_transport_header = true;
                break;
            default:
                return NULL;
        }

        payload_length = (uint16_t)(payload_length - header_length);
        packet_ptr += header_length;
        if (!found_transport_header) {
            next_header_type = *packet_ptr;
        }

    } while (!found_transport_header);

    TRANSPORT_PROTOCOL transport_protocol;

    if (next_header_type == NX_PROTOCOL_TCP) {
        transport_protocol = TRANSPORT_PROTOCOL_TCP;
    } else if (next_header_type == NX_PROTOCOL_UDP) {
        transport_protocol = TRANSPORT_PROTOCOL_UDP;
    } else if (next_header_type == NX_PROTOCOL_ICMPV6) {
        transport_protocol = TRANSPORT_PROTOCOL_ICMP;
    } else {
        return NULL;
    }

    // For ICMP, there are no ports, thus they will get parsed incorrectly, but we don't set them.
    uint32_t* transport_header_ptr = (uint32_t*)packet_ptr;
    uint32_t ports_word = ntohl(transport_header_ptr[0]);
    uint16_t source_port = (uint16_t)(ports_word >> 16);
    uint16_t destination_port = ports_word & 0xffff;

    if (direction == NX_IP_PACKET_IN) {
        return schema_connection_create_create_ipv6(
            payload_length,     // Bytes In
            0,                  // Bytes Out
            ip_header_ptr + 6,  // Local Address
            ip_header_ptr + 2,  // Remote Address
            destination_port,   // Local Port
            source_port,        // Remote Port
            transport_protocol  // Transport Protocol
        );
    } else { // NX_IP_PACKET_OUT
        return schema_connection_create_create_ipv6(
            0,                  // Bytes In
            payload_length,     // Bytes Out
            ip_header_ptr + 2,  // Local Address
            ip_header_ptr + 6,  // Remote Address
            source_port,        // Local Port
            destination_port,   // Remote Port
            transport_protocol  // Transport Protocol
        );
    }
}