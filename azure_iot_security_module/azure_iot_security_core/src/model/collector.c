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
#include "asc_security_core/logger.h"
#include "asc_security_core/model/collector.h"

OBJECT_POOL_DEFINITIONS(collector_t, COLLECTOR_OBJECT_POOL_COUNT);
LINKED_LIST_DEFINITIONS(collector_t);

collector_t* collector_init(INIT_FUNCTION collector_internal_init_function) {
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;
    collector_t* collector_ptr = NULL;

    if (collector_internal_init_function == NULL) {
        log_error("Failed to initialize collector due to bad argument");
        result = IOTSECURITY_RESULT_BAD_ARGUMENT;
        goto cleanup;
    }

    collector_ptr = object_pool_get(collector_t);
    if (collector_ptr == NULL) {
        log_error("Failed to allocate collector");
        result = IOTSECURITY_RESULT_MEMORY_EXCEPTION;
        goto cleanup;
    }

    memset(collector_ptr, 0, sizeof(collector_t));

    result = collector_internal_init_function(&collector_ptr->internal);
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to initialize collector internal, result=[%d]", result);
        goto cleanup;
    }

    linked_list_event_t_init(&(collector_ptr->event_list), event_deinit);

    log_debug("Initialize collector, name=[%s], priority=[%d]", collector_get_name(collector_ptr), collector_get_priority(collector_ptr));

    // default parameters
    collector_ptr->enabled = true;
    collector_ptr->status = COLLECTOR_STATUS_OK;
    collector_ptr->failure_count = 0;
    collector_ptr->last_collected_timestamp = 0;
    collector_ptr->last_sent_timestamp = 0;

cleanup:
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to initialize collector, result=[%d]", result);
        collector_deinit(collector_ptr);
        collector_ptr = NULL;
    }

    return collector_ptr;
}

void collector_deinit(collector_t* collector_ptr) {
    if (collector_ptr == NULL) {
        return;
    }

    linked_list_event_t_deinit(&(collector_ptr->event_list));

    if (collector_ptr->internal.deinit_function != NULL) {
        collector_ptr->internal.deinit_function(&collector_ptr->internal);
    }

    object_pool_free(collector_t, collector_ptr);
    collector_ptr = NULL;
}

const char* collector_get_name(collector_t* collector_ptr) {
    const char* name = NULL;

    if (collector_ptr == NULL) {
        log_error("Failed to retrieve collector name, bad argument");
    } else {
        name = collector_ptr->internal.name;
    }

    return name;
}

COLLECTOR_PRIORITY collector_get_priority(collector_t* collector_ptr) {
    COLLECTOR_PRIORITY priority = COLLECTOR_PRIORITY_HIGH;

    if (collector_ptr == NULL) {
        log_error("Failed to retrieve collector priority, bad argument");
    }else {
        priority = collector_ptr->internal.priority;
    }

    return priority;
}

time_t collector_get_last_collected_timestamp(collector_t* collector_ptr) {
    time_t last_collected_timestamp = 0;

    if (collector_ptr == NULL) {
        log_error("Failed to retrieve collector last collected timestamp, bad argument");
    } else {
        last_collected_timestamp = collector_ptr->last_collected_timestamp;
    }

    return last_collected_timestamp;
}

IOTSECURITY_RESULT collector_set_last_collected_timestamp(collector_t* collector_ptr, time_t last_collected_timestamp) {
    if (collector_ptr == NULL) {
        log_error("Failed to set collector last collected timestamp, bad argument");
        return IOTSECURITY_RESULT_BAD_ARGUMENT;
    }

    collector_ptr->last_collected_timestamp = last_collected_timestamp;

    return IOTSECURITY_RESULT_OK;
}


IOTSECURITY_RESULT collector_collect(collector_t* collector_ptr) {
    if (collector_ptr == NULL) {
        log_error("Collector failed to collect, bad argument");
        return IOTSECURITY_RESULT_BAD_ARGUMENT;
    }

    collector_ptr->last_collected_timestamp = itime_time(NULL);

    return collector_ptr->internal.collect_function(&collector_ptr->internal, &(collector_ptr->event_list));
}

IOTSECURITY_RESULT collector_peek_event(collector_t* collector_ptr, event_t** event) {
    if (collector_ptr == NULL) {
        log_error("Collector failed to peek event, bad argument");
        return IOTSECURITY_RESULT_BAD_ARGUMENT;
    }

    event_t* peeked_event = linked_list_event_t_get_first(&(collector_ptr->event_list));
    if (peeked_event == NULL) {
        log_debug("Collector=[%s] is empty", collector_get_name(collector_ptr));
        return IOTSECURITY_RESULT_EMPTY;
    }

    *event = peeked_event;

    return IOTSECURITY_RESULT_OK;
}

IOTSECURITY_RESULT collector_pop_event(collector_t* collector_ptr, event_t** event) {
    if (collector_ptr == NULL) {
        log_error("Collector failed to pop event, bad argument");
        return IOTSECURITY_RESULT_BAD_ARGUMENT;
    }

    event_t* popped_event_ptr = linked_list_event_t_remove_first(&(collector_ptr->event_list));
    if (popped_event_ptr == NULL) {
        log_debug("Collector=[%s] is empty", collector_get_name(collector_ptr));
        return IOTSECURITY_RESULT_EMPTY;
    }

    if (event != NULL) {
        *event = popped_event_ptr;
    }

    return IOTSECURITY_RESULT_OK;
}
