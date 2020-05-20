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

#ifndef EVENT_H
#define EVENT_H

#include <stdbool.h>
#include <stdint.h>

#include "asc_security_core/asc/asc_span.h"

#include "asc_security_core/configuration.h"
#include "asc_security_core/iotsecurity_result.h"
#include "asc_security_core/model/schema/schemas.h"
#include "asc_security_core/object_pool.h"
#include "asc_security_core/utils/collection/linked_list.h"
#include "asc_security_core/utils/itime.h"

#define SCHEMA_EXTRA_DETAILS_BUFFER_MAX_SIZE 256
#define EVENT_OBJECT_POOL_COUNT ASC_CORE_MAX_EVENTS_IN_MEMORY

typedef struct event event_t;

OBJECT_POOL_DECLARATIONS(event_t, EVENT_OBJECT_POOL_COUNT);
LINKED_LIST_DECLARATIONS(event_t);

typedef bool (*EVENT_PREDICATE)(event_t* event, void* additional_params);


/**
 * @brief Initialize an Event
 *
 * @return event ptr
 */
event_t* event_init(const char* payload_schema_version, const char* name, const char* category, const char* event_type, time_t local_time);


/**
 * @brief Deinitialize an Event
 *
 * @param event_ptr    event ptr
 */
void event_deinit(event_t* event_ptr);


/**
 * @brief Build the event message
 *
 * @param event_ptr  event ptr
 *
 * @return IOTSECRUITY_RESULT
 */
IOTSECURITY_RESULT event_build(event_t* event_ptr);


/**
 * @brief Retrieve event data if the event is signed and valid
 *
 * @param event_ptr     event ptr
 * @param buffer        out param
 * @param size          buffer length
 *
 * @return IOTSECRUITY_RESULT
 */
IOTSECURITY_RESULT event_get_data(event_t* event_ptr, char* buffer, int32_t size);


/**
 * @brief Getter event id
 *
 * @param event_ptr    event ptr
 *
 * @return event id
 */
asc_span event_get_id(event_t* event_ptr);


/**
 * @brief Getter event name
 *
 * @param event_ptr    event ptr
 *
 * @return event name
 */
asc_span event_get_name(event_t* event_ptr);


/**
 * @brief Getter event payload schema version
 *
 * @param event_ptr    event ptr
 *
 * @return event payload schema version
 */
asc_span event_get_payload_schema_version(event_t* event_ptr);


/**
 * @brief Getter event category
 *
 * @param event_ptr    event ptr
 *
 * @return event category
 */
asc_span event_get_category(event_t* event_ptr);


/**
 * @brief Getter event type
 *
 * @param event_ptr    event ptr
 *
 * @return event type
 */
asc_span event_get_type(event_t* event_ptr);


/**
 * @brief Getter event local time
 *
 * @param event_ptr    event ptr
 *
 * @return event local time
 */
time_t event_get_local_time(event_t* event_ptr);


/**
 * @brief Predicate is event empty
 *
 * @param event_ptr    event ptr
 *
 * @return true iff the event is empty
 */
bool event_is_empty(event_t* event_ptr);


/**
 * @brief Getter event length
 *
 * @param event_ptr    event ptr
 *
 * @return event length
 */
uint32_t event_get_length(event_t* event_ptr);


#ifdef COLLECTOR_SYSTEM_INFORMATION_ENABLED
/**
 * @brief Append SystemInformation payload to the event
 *
 * @param event_ptr                  event_ptr
 * @param system_information_ptr     system_information_t*
 *
 * @return IOTSECURITY_RESULT
 */
IOTSECURITY_RESULT event_append_system_information(event_t* event_ptr, system_information_t* system_information_ptr);
#endif

#ifdef COLLECTOR_CONNECTION_CREATE_ENABLED
/**
 * @brief Append ConnectionCreate payload to the event
 *
 * @param event_ptr                 event_ptr
 * @param connection_create_ptr     connection_create ptr
 *
 * @return IOTSECURITY_RESULT
 */
IOTSECURITY_RESULT event_append_connection_create(event_t* event_ptr, connection_create_t* connection_create_ptr);
#endif

#ifdef COLLECTOR_LISTENING_PORTS_ENABLED
/**
 * @brief Append ListeningPorts payload to the event
 *
 * @param event_ptr                  event_ptr
 * @param listening_ports_handle        listening_ports_t*
 *
 * @return IOTSECURITY_RESULT
 */
IOTSECURITY_RESULT event_append_listening_ports(event_t* event_ptr, listening_ports_t* listening_ports_handle);
#endif

#ifdef COLLECTOR_HEARTBEAT_ENABLED
/**
 * @brief Append Heartbeat payload to the event
 *
 * @param event_ptr                  event_ptr
 * @param heartbeat_handle              schema_heartbeat_t*
 *
 * @return IOTSECURITY_RESULT
 */
IOTSECURITY_RESULT event_append_heartbeat(event_t* event_ptr, schema_heartbeat_t* heartbeat_handle);
#endif

#endif /* EVENT_H */
