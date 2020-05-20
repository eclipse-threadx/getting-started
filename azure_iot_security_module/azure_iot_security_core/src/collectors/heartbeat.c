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

#include "asc_security_core/logger.h"
#include "asc_security_core/collectors/heartbeat.h"
#include "asc_security_core/message_schema_consts.h"
#include "asc_security_core/model/event.h"
#include "asc_security_core/object_pool.h"
#include "asc_security_core/utils/notifier.h"

static void _collector_heartbeat_deinit(collector_internal_t* collector_internal_ptr);
static IOTSECURITY_RESULT _collector_heartbeat_get_events(collector_internal_t* collector_internal_ptr, linked_list_event_t_handle events);

IOTSECURITY_RESULT collector_heartbeat_init(collector_internal_t* collector_internal_ptr) {
    if (collector_internal_ptr == NULL) {
        log_error("Could not initialize collector_heartbeat, bad argument");
        return IOTSECURITY_RESULT_BAD_ARGUMENT;
    }

    memset(collector_internal_ptr, 0, sizeof(*collector_internal_ptr));
    strncpy(collector_internal_ptr->name, HEARTBEAT_NAME, COLLECTOR_NAME_LENGTH);
    collector_internal_ptr->type = COLLECTOR_TYPE_HEARTBEAT;
    collector_internal_ptr->priority = COLLECTOR_PRIORITY_HIGH;
    collector_internal_ptr->collect_function = _collector_heartbeat_get_events;
    collector_internal_ptr->deinit_function = _collector_heartbeat_deinit;

    return IOTSECURITY_RESULT_OK;
}

static void _collector_heartbeat_deinit(collector_internal_t* collector_internal_ptr) {
    memset(collector_internal_ptr, 0, sizeof(*collector_internal_ptr));
}

static IOTSECURITY_RESULT _collector_heartbeat_get_events(collector_internal_t* collector_internal_ptr, linked_list_event_t_handle events) {
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;
    schema_heartbeat_t* heartbeat_ptr = NULL;
    event_t* event_ptr = NULL;

    log_debug("Start Collector%s_GetEvents", HEARTBEAT_NAME);

    heartbeat_ptr = schema_heartbeat_init();
    if (heartbeat_ptr == NULL) {
        log_error("Failed to allocate schema heartbeat_t");
        result = IOTSECURITY_RESULT_MEMORY_EXCEPTION;
        goto cleanup;
    }

    event_ptr = event_init(HEARTBEAT_PAYLOAD_SCHEMA_VERSION, HEARTBEAT_NAME, EVENT_PERIODIC_CATEGORY, EVENT_TYPE_SECURITY_VALUE, itime_time(NULL));
    if (event_ptr == NULL) {
        log_error("Failed to allocate event_ptr");
        result = IOTSECURITY_RESULT_MEMORY_EXCEPTION;
        goto cleanup;
    }

    result = event_append_heartbeat(event_ptr, heartbeat_ptr);
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to append Operation Heartbeat to event, result=[%d]", result);
        goto cleanup;
    }

    result = event_build(event_ptr);
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to build event, result=[%d]", result);
        goto cleanup;
    }

    if (linked_list_event_t_add_last(events, event_ptr) == NULL)
    {
        log_error("Failed to add event", result);
        result = IOTSECURITY_RESULT_FULL;
        goto cleanup;
    }

cleanup:
    if (heartbeat_ptr != NULL) {
        schema_heartbeat_deinit(heartbeat_ptr);
        heartbeat_ptr = NULL;
    }

    if (result != IOTSECURITY_RESULT_OK) {
        event_deinit(event_ptr);
        event_ptr = NULL;
    }

    log_debug("Done Collector%s_GetEvents, result=[%d]", HEARTBEAT_NAME, result);
    return result;
}
