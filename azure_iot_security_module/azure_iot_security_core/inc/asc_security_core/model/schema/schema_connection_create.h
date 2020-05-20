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

#ifndef SCHEMA_CONNECTION_CREATE_H
#define SCHEMA_CONNECTION_CREATE_H

#include <stdint.h>

#include "asc_security_core/asc/asc_span.h"
#include "asc_security_core/configuration.h"
#include "asc_security_core/iotsecurity_result.h"
#include "asc_security_core/object_pool.h"
#include "asc_security_core/utils/collection/hashset.h"
#include "asc_security_core/utils/collection/linked_list.h"
#include "asc_security_core/utils/itime.h"
#include "asc_security_core/utils/network_utils.h"


#define CONNECTION_CREATE_HASHSET_SIZE (ASC_COLLECTOR_CONNECTION_CREATE_MAX_OBJECTS_IN_CACHE * 7 / 10)


typedef enum TRANSPORT_PROTOCOL_TAG {
    TRANSPORT_PROTOCOL_TCP,
    TRANSPORT_PROTOCOL_UDP,
    TRANSPORT_PROTOCOL_ICMP
} TRANSPORT_PROTOCOL;


/**
 * @brief TransportProtocol ToString
 *
 * @param protocol  TRANSPORT_PROTOCOL
 *
 * @return string protocol
 */
const char* transport_protocol_to_str(TRANSPORT_PROTOCOL protocol);


typedef struct schema_connection_create connection_create_t;

LINKED_LIST_DECLARATIONS(connection_create_t);
HASHSET_DECLARATIONS(connection_create_t);

/**
 * @brief Initialize ConnectionCreate Schema
 *
 * @return connection_create_t*
 */
connection_create_t* schema_connection_create_init();


/**
 * @brief Deinitialize ConnectionCreate Schema
 *
 * @param data_ptr      connection_create_t*
 */
void schema_connection_create_deinit(connection_create_t* data_ptr);


/**
 * @brief Create a populated IPv4 ConnectionCreate Schema
 *
 * @param local_time            The local time
 * @param bytes_in              The amount of incoming bytes
 * @param bytes_out             The amount of outgoing bytes
 * @param local_ip              The local ip address
 * @param remote_ip             The remote ip address
 * @param local_port            The local port number
 * @param remote_port           The remote port number
 * @param transport_protocol    The transport protocol
 *
 * @return connection_create_t*
 */
connection_create_t* schema_connection_create_create_ipv4(
                        uint32_t bytes_in, uint32_t bytes_out, uint32_t local_ip, uint32_t remote_ip,
                        uint16_t local_port, uint16_t remote_port, TRANSPORT_PROTOCOL transport_protocol);


/**
 * @brief Create a populated IPv6 ConnectionCreate Schema
 *
 * @param local_time            The local time
 * @param bytes_in              The amount of incoming bytes
 * @param bytes_out             The amount of outgoing bytes
 * @param local_ip              The local ip address (uint32_t[4])
 * @param remote_ip             The remote ip address (uint32_t[4])
 * @param local_port            The local port number
 * @param remote_port           The remote port number
 * @param transport_protocol    The transport protocol
 *
 * @return connection_create_t*
 */
connection_create_t* schema_connection_create_create_ipv6(
                        uint32_t bytes_in, uint32_t bytes_out, uint32_t* local_ip, uint32_t* remote_ip,
                        uint16_t local_port, uint16_t remote_port, TRANSPORT_PROTOCOL transport_protocol);


/**
 * @brief Getter schema extra details
 *
 * @param data_ptr   connection_create_t*
 *
 * @return schema extra_details
 */
asc_pair* schema_connection_create_get_extra_details(connection_create_t* data_ptr);


/**
 * @brief Setter schema extra details
 *
 * @param data_ptr       connection_create_t*
 * @param extra_details     extra details
 *
 * @return IOTSECRUITY_RESULT
 */
IOTSECURITY_RESULT schema_connection_create_set_extra_details(connection_create_t* data_ptr, asc_pair* extra_details);


/**
 * @brief Serialize local ip address into the given buffer, does not allocate memory
 *
 * @param data_ptr      connection_create_t*
 * @param buffer                        out param- buffer to populate
 *
 * @return IOTSECURITY_RESULT_Ok on success, IOTSECURITY_RESULT_BAD_ARGUMENT otherwise
 */
IOTSECURITY_RESULT schema_connection_create_serialize_local_ip(connection_create_t* data_ptr, char* buffer);


/**
 * @brief Serialize remote ip address into the given buffer, does not allocate memory
 *
 * @param data_ptr      connection_create_t*
 * @param buffer                        out param- buffer to populate
 *
 * @return IOTSECURITY_RESULT_Ok on success, IOTSECURITY_RESULT_BAD_ARGUMENT otherwise
 */
IOTSECURITY_RESULT schema_connection_create_serialize_remote_ip(connection_create_t* data_ptr, char* buffer);


/**
 * @brief Serialize local ip address into the given buffer, does not allocate memory
 *
 * @param data_ptr      connection_create_t*
 * @param buffer                        out param- buffer to populate
 *
 * @return IOTSECURITY_RESULT_Ok on success, IOTSECURITY_RESULT_BAD_ARGUMENT otherwise
 */
void schema_connection_create_log_info(connection_create_t* data_ptr);


/**
 * @brief Getter schema network protocol
 *
 * @param data_ptr   schema_connection_create_t
 *
 * @return schema network protocol
 */
NETWORK_PROTOCOL schema_connection_create_get_network_protocol(connection_create_t* data_ptr);


/**
 * @brief Setter schema network protocol
 *
 * @param data_ptr       schema_connection_create_t
 * @param network_protocol  network protocol
 *
 * @return IOTSECRUITY_RESULT
 */
IOTSECURITY_RESULT schema_connection_create_set_network_protocol(connection_create_t* data_ptr, NETWORK_PROTOCOL network_protocol);


/**
 * @brief Getter schema transport protocol
 *
 * @param data_ptr   schema_connection_create_t
 *
 * @return schema transport protocol
 */
TRANSPORT_PROTOCOL schema_connection_create_get_transport_protocol(connection_create_t* data_ptr);


/**
 * @brief Setter schema transport protocol
 *
 * @param data_ptr       schema_connection_create_t
 * @param transport_protocol  transport protocol
 *
 * @return IOTSECRUITY_RESULT
 */
IOTSECURITY_RESULT schema_connection_create_set_transport_protocol(connection_create_t* data_ptr, TRANSPORT_PROTOCOL transport_protocol);


/**
 * @brief Getter schema local ipv4
 *
 * @param data_ptr   schema_connection_create_t
 *
 * @return schema local ipv4
 */
uint32_t schema_connection_create_get_local_ipv4(connection_create_t* data_ptr);


/**
 * @brief Setter schema local ipv4
 *
 * @param data_ptr   schema_connection_create_t
 * @param local_ipv4    local_ipv4
 *
 * @return IOTSECRUITY_RESULT
 */
IOTSECURITY_RESULT schema_connection_create_set_local_ipv4(connection_create_t* data_ptr, uint32_t local_ipv4);


/**
 * @brief Getter schema local ipv6
 *
 * @param data_ptr   schema_connection_create_t
 * @param local_ipv6    local_ipv6
 *
 * @return schema local ipv6
 */
IOTSECURITY_RESULT schema_connection_create_get_local_ipv6(connection_create_t* data_ptr, uint32_t* local_ipv6);


/**
 * @brief Setter schema local ipv6
 *
 * @param data_ptr   schema_connection_create_t
 * @param local_ipv6    local_ipv6
 *
 * @return IOTSECRUITY_RESULT
 */
IOTSECURITY_RESULT schema_connection_create_set_local_ipv6(connection_create_t* data_ptr, uint32_t* local_ipv6);


/**
 * @brief Getter schema local port
 *
 * @param data_ptr   schema_connection_create_t
 *
 * @return schema local port
 */
uint16_t schema_connection_create_get_local_port(connection_create_t* data_ptr);


/**
 * @brief Setter schema local port
 *
 * @param data_ptr   schema_connection_create_t
 * @param local_port    local_port
 *
 * @return IOTSECRUITY_RESULT
 */
IOTSECURITY_RESULT schema_connection_create_set_local_port(connection_create_t* data_ptr, uint16_t local_port);


/**
 * @brief Getter schema remote ipv4
 *
 * @param data_ptr   schema_connection_create_t
 *
 * @return schema remote ipv4
 */
uint32_t schema_connection_create_get_remote_ipv4(connection_create_t* data_ptr);


/**
 * @brief Setter schema remote ipv4
 *
 * @param data_ptr   schema_connection_create_t
 * @param remote_ipv4   remote_ipv4
 *
 * @return IOTSECRUITY_RESULT
 */
IOTSECURITY_RESULT schema_connection_create_set_remote_ipv4(connection_create_t* data_ptr, uint32_t remote_ipv4);


/**
 * @brief Getter schema remote ipv6
 *
 * @param data_ptr   schema_connection_create_t
 * @param remote_ipv6   remote_ipv6
 *
 * @return schema remote ipv6
 */
IOTSECURITY_RESULT schema_connection_create_get_remote_ipv6(connection_create_t* data_ptr, uint32_t* remote_ipv6);


/**
 * @brief Setter schema remote ipv6
 *
 * @param data_ptr   schema_connection_create_t
 * @param remote_ipv6   remote_ipv6
 *
 * @return IOTSECRUITY_RESULT
 */
IOTSECURITY_RESULT schema_connection_create_set_remote_ipv6(connection_create_t* data_ptr, uint32_t* remote_ipv6);


/**
 * @brief Getter schema remote port
 *
 * @param data_ptr   schema_connection_create_t
 *
 * @return schema remote port
 */
uint16_t schema_connection_create_get_remote_port(connection_create_t* data_ptr);


/**
 * @brief Setter schema remote port
 *
 * @param data_ptr   schema_connection_create_t
 * @param remote_port   remote port
 *
 * @return IOTSECRUITY_RESULT
 */
IOTSECURITY_RESULT schema_connection_create_set_remote_port(connection_create_t* data_ptr, uint16_t remote_port);

/**
 * @brief Getter schema bytes in
 *
 * @param data_ptr   schema_connection_create_t
 *
 * @return schema bytes in
 */
uint32_t schema_connection_create_get_bytes_in(connection_create_t* data_ptr);


/**
 * @brief Setter schema bytes in
 *
 * @param data_ptr   schema_connection_create_t
 * @param bytes_in      bytes in
 *
 * @return IOTSECRUITY_RESULT
 */
IOTSECURITY_RESULT schema_connection_create_set_bytes_in(connection_create_t* data_ptr, uint32_t bytes_in);

/**
 * @brief Getter schema bytes out
 *
 * @param data_ptr   schema_connection_create_t
 *
 * @return schema bytes out
 */
uint32_t schema_connection_create_get_bytes_out(connection_create_t* data_ptr);


/**
 * @brief Setter schema bytes out
 *
 * @param data_ptr   schema_connection_create_t
 * @param bytes_in      bytes out
 *
 * @return IOTSECRUITY_RESULT
 */
IOTSECURITY_RESULT schema_connection_create_set_bytes_out(connection_create_t* data_ptr, uint32_t bytes_out);


#endif /* SCHEMA_CONNECTION_CREATE_H */
