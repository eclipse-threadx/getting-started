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

#include <stdio.h>
#include <stdbool.h>
#include "asc_security_core/asc/asc_span.h"
#include "asc_security_core/logger.h"
#include "asc_security_core/configuration.h"
#include "asc_security_core/message_schema_consts.h"
#include "asc_security_core/model/schema/schema_connection_create.h"
#include "asc_security_core/object_pool.h"


#define MAX_IPV6_STRING_LENGTH (sizeof "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")
#define SCHEMA_CONNECTION_CREATE_OBJECT_POOL_COUNT ASC_COLLECTOR_CONNECTION_CREATE_MAX_OBJECTS_IN_CACHE

typedef struct schema_connection_create {
    COLLECTION_INTERFACE(struct schema_connection_create);

    uint32_t bytes_in;
    uint32_t bytes_out;
    union {
        uint32_t local_ipv4;
        uint32_t local_ipv6[4];
    };
    union {
        uint32_t remote_ipv4;
        uint32_t remote_ipv6[4];
    };
    uint16_t local_port;
    uint16_t remote_port;
    NETWORK_PROTOCOL network_protocol;
    TRANSPORT_PROTOCOL transport_protocol;
} schema_connection_create_t;

OBJECT_POOL_DECLARATIONS(schema_connection_create_t, SCHEMA_CONNECTION_CREATE_OBJECT_POOL_COUNT);
OBJECT_POOL_DEFINITIONS(schema_connection_create_t, SCHEMA_CONNECTION_CREATE_OBJECT_POOL_COUNT);

static IOTSECURITY_RESULT _schema_connection_create_serialize_ip(connection_create_t* data_ptr, char* buffer, bool local);


const char* transport_protocol_to_str(TRANSPORT_PROTOCOL protocol) {
    const char* ret;

    switch (protocol) {
        case TRANSPORT_PROTOCOL_TCP:
            ret = CONNECTION_CREATE_PROTOCOL_TCP_NAME;
            break;
        case TRANSPORT_PROTOCOL_UDP:
            ret = CONNECTION_CREATE_PROTOCOL_UDP_NAME;
            break;
        case TRANSPORT_PROTOCOL_ICMP:
            ret = CONNECTION_CREATE_PROTOCOL_ICMP_NAME;
            break;
    }

    return ret;
}

LINKED_LIST_DEFINITIONS(connection_create_t);
HASHSET_DEFINITIONS(connection_create_t, CONNECTION_CREATE_HASHSET_SIZE);


int hashset_connection_create_t_equals(connection_create_t* a, connection_create_t* b) {
    if (a == b) return 1;
    if (a == NULL || b == NULL) return 0;
    if (a->network_protocol != b->network_protocol) return 0;
    int result = (
            a->local_port == b->local_port &&
            a->remote_port == b->remote_port &&
            a->transport_protocol == b->transport_protocol
        );
    if (a->network_protocol == NETWORK_PROTOCOL_IPV4) {
        return (result &&
            a->local_ipv4 == b->local_ipv4 &&
            a->remote_ipv4 == b->remote_ipv4
        );
    } else { // IPv6
        return (result &&
            IPV6_ADDRESS_EQUALS(a->local_ipv6, b->local_ipv6) &&
            IPV6_ADDRESS_EQUALS(a->remote_ipv6, b->remote_ipv6)
        );
    }
}


unsigned int hashset_connection_create_t_hash(connection_create_t* data_ptr) {
    unsigned int result = 0;
    if (data_ptr->network_protocol == NETWORK_PROTOCOL_IPV4) {
        result ^= data_ptr->local_ipv4;
        result ^= data_ptr->remote_ipv4;
    } else { // IPv6
        result ^= IPV6_ADDRESS_HASH(data_ptr->local_ipv6);
        result ^= IPV6_ADDRESS_HASH(data_ptr->remote_ipv6);
    }

    result ^= (unsigned int)(((data_ptr->local_port << 16) | data_ptr->remote_port));
    result ^= data_ptr->transport_protocol;

    return result;
}


void hashset_connection_create_t_update(connection_create_t* old_data, connection_create_t* new_data) {
    old_data->bytes_in += new_data->bytes_in;
    old_data->bytes_out += new_data->bytes_out;
    object_pool_free(schema_connection_create_t, new_data);
}


connection_create_t* schema_connection_create_init() {
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;
    connection_create_t* data_ptr = NULL;

    data_ptr = object_pool_get(schema_connection_create_t);
    if (data_ptr == NULL) {
        result = IOTSECURITY_RESULT_MEMORY_EXCEPTION;
        log_error("Failed to allocate connection_create_t*");
        goto cleanup;
    }

    memset(data_ptr, 0, sizeof(schema_connection_create_t));

cleanup:
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to initialize connection create schema, result=[%d]", result);
    }

    return data_ptr;
}


connection_create_t* schema_connection_create_create_ipv4(uint32_t bytes_in, uint32_t bytes_out, uint32_t local_ip, uint32_t remote_ip,
                                                        uint16_t local_port, uint16_t remote_port, TRANSPORT_PROTOCOL transport_protocol) {
    connection_create_t* data_ptr = object_pool_get(schema_connection_create_t);
    if (data_ptr == NULL) {
        log_error("Failed to allocate connection_create_t*");
        return NULL;
    }

    data_ptr->bytes_in = bytes_in;
    data_ptr->bytes_out = bytes_out;
    data_ptr->local_ipv4 = local_ip;
    data_ptr->remote_ipv4 = remote_ip;

    if (transport_protocol != TRANSPORT_PROTOCOL_ICMP) {
        data_ptr->local_port = local_port;
        data_ptr->remote_port = remote_port;
    }

    data_ptr->transport_protocol = transport_protocol;
    data_ptr->network_protocol = NETWORK_PROTOCOL_IPV4;

    return data_ptr;
}


connection_create_t* schema_connection_create_create_ipv6(uint32_t bytes_in, uint32_t bytes_out, uint32_t* local_ip, uint32_t* remote_ip,
                                                        uint16_t local_port, uint16_t remote_port, TRANSPORT_PROTOCOL transport_protocol) {
    connection_create_t* data_ptr = object_pool_get(schema_connection_create_t);
    if (data_ptr == NULL) {
        log_error("Failed to allocate connection_create_t*");
        return NULL;
    }

    data_ptr->bytes_in = bytes_in;
    data_ptr->bytes_out = bytes_out;

    IPV6_ADDRESS_COPY(local_ip, data_ptr->local_ipv6);
    IPV6_ADDRESS_COPY(remote_ip, data_ptr->remote_ipv6);

    if (transport_protocol != TRANSPORT_PROTOCOL_ICMP) {
        data_ptr->local_port = local_port;
        data_ptr->remote_port = remote_port;
    }

    data_ptr->transport_protocol = transport_protocol;
    data_ptr->network_protocol = NETWORK_PROTOCOL_IPV6;

    return data_ptr;
}


void schema_connection_create_deinit(connection_create_t* data_ptr) {
    if (data_ptr == NULL) {
        return;
    }

    object_pool_free(schema_connection_create_t, data_ptr);
    data_ptr = NULL;
}

IOTSECURITY_RESULT schema_connection_create_serialize_local_ip(connection_create_t* data_ptr, char* buffer) {
    return _schema_connection_create_serialize_ip(data_ptr, buffer, true);
}


IOTSECURITY_RESULT schema_connection_create_serialize_remote_ip(connection_create_t* data_ptr, char* buffer) {
    return _schema_connection_create_serialize_ip(data_ptr, buffer, false);
}


void schema_connection_create_log_info(connection_create_t* data_ptr) {
    if (data_ptr == NULL) {
        return;
    }

    char local_ip[MAX_IPV6_STRING_LENGTH] = { 0 };
    char remote_ip[MAX_IPV6_STRING_LENGTH] = { 0 };

    _schema_connection_create_serialize_ip(data_ptr, local_ip, true);
    _schema_connection_create_serialize_ip(data_ptr, remote_ip, false);

    log_debug("ConnectionCreateSchema: local_ip=[%s], local_port=[%u], remote_ip=[%s], remote_port=[%u], protocol=[%s]",
        local_ip,
        data_ptr->local_port,
        remote_ip,
        data_ptr->remote_port,
        transport_protocol_to_str(data_ptr->transport_protocol)
    );
}


static IOTSECURITY_RESULT _schema_connection_create_serialize_ip(connection_create_t* data_ptr, char* buffer, bool local) {
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;
    if (data_ptr == NULL || buffer == NULL) {
        log_error("ConnectionCreateSchema failed to serialize IP address");
        result = IOTSECURITY_RESULT_BAD_ARGUMENT;
        goto cleanup;
    }

    const void* source = local ? data_ptr->local_ipv6 : data_ptr->remote_ipv6;

    if (network_utils_inet_ntop(data_ptr->network_protocol, source, buffer, MAX_IPV6_STRING_LENGTH) == NULL) {
        log_error("ConnectionCreateSchema failed to serialize IP address");
        result = IOTSECURITY_RESULT_EXCEPTION;
        goto cleanup;
    }

cleanup:
    return result;
}


IOTSECURITY_RESULT schema_connection_create_set_network_protocol(connection_create_t* data_ptr, NETWORK_PROTOCOL network_protocol) {
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;

    if (data_ptr == NULL) {
        log_error("Failed to set connection create schema network_protocol due to bad argument");
        result = IOTSECURITY_RESULT_BAD_ARGUMENT;
        goto cleanup;
    }

    data_ptr->network_protocol = network_protocol;

cleanup:
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to set connection create schema network_protocol, result=[%d]", result);
    }

    return result;
}


NETWORK_PROTOCOL schema_connection_create_get_network_protocol(connection_create_t* data_ptr) {
    NETWORK_PROTOCOL result = 0;

    if (data_ptr != NULL) {
        result = data_ptr->network_protocol;
    }

    return result;
}


IOTSECURITY_RESULT schema_connection_create_set_transport_protocol(connection_create_t* data_ptr, TRANSPORT_PROTOCOL transport_protocol) {
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;

    if (data_ptr == NULL) {
        log_error("Failed to set connection create schema transport_protocol due to bad argument");
        result = IOTSECURITY_RESULT_BAD_ARGUMENT;
        goto cleanup;
    }

    data_ptr->transport_protocol = transport_protocol;

cleanup:
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to set connection create schema transport_protocol, result=[%d]", result);
    }

    return result;
}


TRANSPORT_PROTOCOL schema_connection_create_get_transport_protocol(connection_create_t* data_ptr) {
    TRANSPORT_PROTOCOL result = 0;

    if (data_ptr != NULL) {
        result = data_ptr->transport_protocol;
    }

    return result;
}


uint32_t schema_connection_create_get_local_ipv4(connection_create_t* data_ptr) {
    uint32_t result = 0;

    if (data_ptr != NULL) {
        result = data_ptr->local_ipv4;
    }

    return result;
}


IOTSECURITY_RESULT schema_connection_create_set_local_ipv4(connection_create_t* data_ptr, uint32_t local_ipv4) {
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;

    if (data_ptr == NULL) {
        log_error("Failed to set connection create schema local_ipv4 due to bad argument");
        result = IOTSECURITY_RESULT_BAD_ARGUMENT;
        goto cleanup;
    }

    data_ptr->local_ipv4 = local_ipv4;

cleanup:
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to set connection create schema local_ipv4, result=[%d]", result);
    }

    return result;
}


IOTSECURITY_RESULT schema_connection_create_get_local_ipv6(connection_create_t* data_ptr, uint32_t* local_ipv6) {
    if (data_ptr == NULL || local_ipv6 == NULL) {
        return IOTSECURITY_RESULT_BAD_ARGUMENT;
    }

    IPV6_ADDRESS_COPY(data_ptr->local_ipv6, local_ipv6);

    return IOTSECURITY_RESULT_OK;
}


IOTSECURITY_RESULT schema_connection_create_set_local_ipv6(connection_create_t* data_ptr, uint32_t* local_ipv6) {
    if (data_ptr == NULL || local_ipv6 == NULL) {
        return IOTSECURITY_RESULT_BAD_ARGUMENT;
    }

    IPV6_ADDRESS_COPY(local_ipv6, data_ptr->local_ipv6);

    return IOTSECURITY_RESULT_OK;
}


uint16_t schema_connection_create_get_local_port(connection_create_t* data_ptr) {
    uint16_t result = 0;

    if (data_ptr != NULL) {
        result = data_ptr->local_port;
    }

    return result;
}


IOTSECURITY_RESULT schema_connection_create_set_local_port(connection_create_t* data_ptr, uint16_t local_port) {
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;

    if (data_ptr == NULL) {
        log_error("Failed to set connection create schema local_port due to bad argument");
        result = IOTSECURITY_RESULT_BAD_ARGUMENT;
        goto cleanup;
    }

    data_ptr->local_port = local_port;

cleanup:
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to set connection create schema local_port, result=[%d]", result);
    }

    return result;
}


uint32_t schema_connection_create_get_remote_ipv4(connection_create_t* data_ptr) {
    uint32_t result = 0;

    if (data_ptr != NULL) {
        result = data_ptr->remote_ipv4;
    }

    return result;
}


IOTSECURITY_RESULT schema_connection_create_set_remote_ipv4(connection_create_t* data_ptr, uint32_t remote_ipv4) {
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;

    if (data_ptr == NULL) {
        log_error("Failed to set connection create schema remote_ipv4 due to bad argument");
        result = IOTSECURITY_RESULT_BAD_ARGUMENT;
        goto cleanup;
    }

    data_ptr->remote_ipv4 = remote_ipv4;

cleanup:
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to set connection create schema remote_ipv4, result=[%d]", result);
    }

    return result;
}


IOTSECURITY_RESULT schema_connection_create_get_remote_ipv6(connection_create_t* data_ptr, uint32_t* remote_ipv6) {
    if (data_ptr == NULL || remote_ipv6 == NULL) {
        return IOTSECURITY_RESULT_BAD_ARGUMENT;
    }

    IPV6_ADDRESS_COPY(data_ptr->remote_ipv6, remote_ipv6);

    return IOTSECURITY_RESULT_OK;
}


IOTSECURITY_RESULT schema_connection_create_set_remote_ipv6(connection_create_t* data_ptr, uint32_t* remote_ipv6) {
    if (data_ptr == NULL || remote_ipv6 == NULL) {
        return IOTSECURITY_RESULT_BAD_ARGUMENT;
    }

    IPV6_ADDRESS_COPY(remote_ipv6, data_ptr->remote_ipv6);

    return IOTSECURITY_RESULT_OK;
}


uint16_t schema_connection_create_get_remote_port(connection_create_t* data_ptr) {
    uint16_t result = 0;

    if (data_ptr != NULL) {
        result = data_ptr->remote_port;
    }

    return result;
}


IOTSECURITY_RESULT schema_connection_create_set_remote_port(connection_create_t* data_ptr, uint16_t remote_port) {
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;

    if (data_ptr == NULL) {
        log_error("Failed to set connection create schema remote_port due to bad argument");
        result = IOTSECURITY_RESULT_BAD_ARGUMENT;
        goto cleanup;
    }

    data_ptr->remote_port = remote_port;

cleanup:
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to set connection create schema remote_port, result=[%d]", result);
    }

    return result;
}

uint32_t schema_connection_create_get_bytes_in(connection_create_t* data_ptr) {
    uint32_t result = 0;

    if (data_ptr != NULL) {
        result = data_ptr->bytes_in;
    }

    return result;
}


IOTSECURITY_RESULT schema_connection_create_set_bytes_in(connection_create_t* data_ptr, uint32_t bytes_in) {
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;

    if (data_ptr == NULL) {
        log_error("Failed to set connection create schema bytes_in due to bad argument");
        result = IOTSECURITY_RESULT_BAD_ARGUMENT;
        goto cleanup;
    }

    data_ptr->bytes_in = bytes_in;

cleanup:
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to set connection create schema bytes_in, result=[%d]", result);
    }

    return result;
}


uint32_t schema_connection_create_get_bytes_out(connection_create_t* data_ptr) {
    uint32_t result = 0;

    if (data_ptr != NULL) {
        result = data_ptr->bytes_out;
    }

    return result;
}


IOTSECURITY_RESULT schema_connection_create_set_bytes_out(connection_create_t* data_ptr, uint32_t bytes_out) {
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;

    if (data_ptr == NULL) {
        log_error("Failed to set connection create schema bytes_out due to bad argument");
        result = IOTSECURITY_RESULT_BAD_ARGUMENT;
        goto cleanup;
    }

    data_ptr->bytes_out = bytes_out;

cleanup:
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to set connection create schema bytes_out, result=[%d]", result);
    }

    return result;
}
