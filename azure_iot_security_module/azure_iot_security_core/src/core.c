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

#include <stdint.h>
#include <stdlib.h>
#include "asc_security_core/utils/itime.h"
#include "asc_security_core/logger.h"
#include "asc_security_core/core.h"
#include "asc_security_core/collector_collection.h"
#include "asc_security_core/configuration.h"
#include "asc_security_core/model/collector.h"
#include "asc_security_core/model/event.h"
#include "asc_security_core/model/message.h"
#include "asc_security_core/utils/os_utils.h"
#include "asc_security_core/object_pool.h"

#define CORE_OBJECT_POOL_COUNT 1

// Security Module Version
#ifndef ASC_SECURITY_MODULE_VERSION
#define ASC_SECURITY_MODULE_VERSION "0.0.1"
#endif

typedef struct core {
    COLLECTION_INTERFACE(struct core);

    asc_span agent_id;
    asc_span agent_version;
    collector_collection_t* collector_collection_ptr;
} core_t;

OBJECT_POOL_DECLARATIONS(core_t, CORE_OBJECT_POOL_COUNT);
OBJECT_POOL_DEFINITIONS(core_t, CORE_OBJECT_POOL_COUNT);

OBJECT_POOL_DEFINITIONS(security_message_t, SECURITY_MESSAGE_OBJECT_POOL_COUNT);

LINKED_LIST_DEFINITIONS(security_message_t);

static IOTSECURITY_RESULT _core_get_message(linked_list_security_message_t* message_list, message_t* message_ptr);


core_t* core_init() {
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;
    core_t* core_ptr = NULL;
    const char* security_module_id = NULL;

    core_ptr = object_pool_get(core_t);
    if (core_ptr == NULL) {
        log_error("Failed to init client core_t");
        result = IOTSECURITY_RESULT_MEMORY_EXCEPTION;
        goto cleanup;
    }

    security_module_id = os_utils_get_security_module_id();
    if (security_module_id == NULL) {
        log_error("Failed to retrieve security module id");
        result = IOTSECURITY_RESULT_EXCEPTION;
        goto cleanup;
    }

    core_ptr->agent_id = asc_span_from_str((char*)security_module_id);
    if (asc_span_is_content_equal(ASC_SPAN_NULL, core_ptr->agent_id)) {
        log_error("Failed to set client core_t id");
        result = IOTSECURITY_RESULT_EXCEPTION;
        goto cleanup;
    }

    core_ptr->agent_version = asc_span_from_str(ASC_SECURITY_MODULE_VERSION);
    if (asc_span_is_content_equal(ASC_SPAN_NULL, core_ptr->agent_version)) {
        log_error("Failed to set client core_t agent_id");
        result = IOTSECURITY_RESULT_EXCEPTION;
        goto cleanup;
    }

    core_ptr->collector_collection_ptr = collector_collection_init();
    if (core_ptr->collector_collection_ptr == NULL) {
        log_error("Failed to init client core_t collectors");
        result = IOTSECURITY_RESULT_MEMORY_EXCEPTION;
        goto cleanup;
    }

cleanup:
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to init client core_t");
        core_deinit(core_ptr);
        core_ptr = NULL;
    }

    return core_ptr;
}

void core_deinit(core_t* core_ptr) {
    if (core_ptr != NULL) {
        if (core_ptr->collector_collection_ptr != NULL) {
            core_ptr->agent_id = ASC_SPAN_NULL;
            core_ptr->agent_version = ASC_SPAN_NULL;

            collector_collection_deinit(core_ptr->collector_collection_ptr);
            core_ptr->collector_collection_ptr = NULL;
        }

        object_pool_free(core_t, core_ptr);
        core_ptr = NULL;
    }
}

IOTSECURITY_RESULT core_collect(core_t* core_ptr) {
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;
    time_t current_snapshot = itime_time(NULL);
    bool at_least_one_success = false;
    bool time_passed = false;

    for (   PRIORITY_COLLECTORS_HANDLE prioritized_collectors = collector_collection_get_head_priority(core_ptr->collector_collection_ptr);
            prioritized_collectors != NULL;
            prioritized_collectors = collector_collection_get_next_priority(core_ptr->collector_collection_ptr, prioritized_collectors)
        ) {
        linked_list_collector_t_handle collector_list = priority_collectors_get_list(prioritized_collectors);

        for (   collector_t* current_collector=linked_list_collector_t_get_first(collector_list);
                current_collector!=NULL;
                current_collector=current_collector->next
            ) {
            time_t last_collected = collector_get_last_collected_timestamp(current_collector);
            double interval = priority_collectors_get_interval(prioritized_collectors);

            if (itime_difftime(current_snapshot, last_collected) >= interval) {
                time_passed = true;
                result = collector_collect(current_collector);
                if (result != IOTSECURITY_RESULT_OK) {
                    log_error("Failed to collect, collector=[%s]", collector_get_name(current_collector));
                    continue;
                }
                at_least_one_success = true;
            }
        }
    }

    return (!time_passed || at_least_one_success) ? IOTSECURITY_RESULT_OK : result;
}


IOTSECURITY_RESULT core_get(core_t* core_ptr, linked_list_security_message_t* output_message_list_handle) {
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;
    event_t* event_ptr = NULL;
    collector_t* current_collector = NULL;
    PRIORITY_COLLECTORS_HANDLE prioritized_collectors = NULL;

    message_t* message_ptr = message_init((const char *)asc_span_ptr(core_ptr->agent_id), (const char *)asc_span_ptr(core_ptr->agent_version));
    if (message_ptr == NULL) {
        log_error("Failed to init message ptrr in ClientCore_CreateMessages");
        result = IOTSECURITY_RESULT_MEMORY_EXCEPTION;
        goto cleanup;
    }

    linked_list_security_message_t message_list = { 0 };
    linked_list_security_message_t* message_list_handle = &message_list;
    linked_list_security_message_t_init(message_list_handle, object_pool_security_message_t_free);

    uint32_t num_of_messages_in_queue = 0;

    prioritized_collectors = collector_collection_get_head_priority(core_ptr->collector_collection_ptr);

    while (prioritized_collectors != NULL && num_of_messages_in_queue < ASC_CORE_MAX_MESSAGES_IN_MEMORY) {
        result = priority_collectors_get_current_non_empty_collector(prioritized_collectors, &current_collector);
        if (result != IOTSECURITY_RESULT_OK) {
            goto cleanup;
        }

        while (current_collector != NULL && num_of_messages_in_queue < ASC_CORE_MAX_MESSAGES_IN_MEMORY) {
            result = collector_peek_event(current_collector, &event_ptr);
            if (result != IOTSECURITY_RESULT_OK) {
                log_error("peeking event at collector [%s] failed", collector_get_name(current_collector));
                goto cleanup;
            }

            bool can_append = message_can_append(message_ptr, event_ptr);
            bool is_message_too_big = !message_has_events(message_ptr) && !can_append;

            // if the message is empty, it means that the event is too big
            // we'll discard it and issue warning and try to collect the next event in the same collector
            if (can_append || is_message_too_big) {
                event_t* popped_event_ptr = NULL;
                result = collector_pop_event(current_collector, &popped_event_ptr);
                if (result != IOTSECURITY_RESULT_OK) {
                    goto cleanup;
                }

                if (is_message_too_big) {
                    log_error("event exceeded message size limit, discarding it for the overall good");
                    event_deinit(event_ptr);
                    event_ptr = NULL;
                    continue;
                }
            }

            if (can_append) {
                result = message_append(message_ptr, event_ptr);

                event_deinit(event_ptr);
                event_ptr = NULL;

                if (result != IOTSECURITY_RESULT_OK) {
                    log_error("Failed to append event to message, result=[%d]", result);
                    goto cleanup;
                }

                priority_collectors_get_next_non_empty_collector(prioritized_collectors, &current_collector);
                if (result != IOTSECURITY_RESULT_OK) {
                    log_error("Failed to get collector ptr in core_get, result=[%d]", result);
                    goto cleanup;
                }
            } else {
                // message is full

                // insert the message
                result = _core_get_message(message_list_handle, message_ptr);
                if (result != IOTSECURITY_RESULT_OK) {
                    log_error("Failed to get message ptr in core_get, result=[%d]", result);
                    goto cleanup;
                }

                num_of_messages_in_queue++;
                if (message_ptr != NULL) {
                    message_deinit(message_ptr);
                    message_ptr = NULL;
                }

                // reinit message ptrr
                message_ptr = message_init((const char *)asc_span_ptr(core_ptr->agent_id), (const char *)asc_span_ptr(core_ptr->agent_version));
                if (message_ptr == NULL) {
                    log_error("Failed to init message ptr in core_get");
                    result = IOTSECURITY_RESULT_MEMORY_EXCEPTION;
                    goto cleanup;
                }
            }
        }

        prioritized_collectors = collector_collection_get_next_priority(core_ptr->collector_collection_ptr, prioritized_collectors);
    }

    // build whatever there is in message_ptr into a message
    // in there are no messaged whatsoever, destroy the allocated buffer
    if (message_has_events(message_ptr)) {
        result = _core_get_message(message_list_handle, message_ptr);

        goto cleanup;
    } else {
        result = (num_of_messages_in_queue > 0) ? IOTSECURITY_RESULT_OK : IOTSECURITY_RESULT_EMPTY;
        goto cleanup;
    }

cleanup:
    if (result == IOTSECURITY_RESULT_OK) {
        linked_list_security_message_t_concat(output_message_list_handle, message_list_handle);
    }

    if (message_ptr != NULL) {
        message_deinit(message_ptr);
        message_ptr = NULL;
    }

    return result;
}

collector_collection_t* core_get_collector_collection(core_t* core_ptr) {
    return core_ptr->collector_collection_ptr;
}

static IOTSECURITY_RESULT _core_get_message(linked_list_security_message_t* message_list, message_t* message_ptr) {
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;

    security_message_t* security_message_ptr = object_pool_get(security_message_t);
    if (security_message_ptr == NULL) {
        log_error("creating a message failed due to memory allocation");
        result = IOTSECURITY_RESULT_MEMORY_EXCEPTION;
        goto cleanup;
    }

    memset(security_message_ptr, 0, sizeof(security_message_t));

    result = message_to_json(message_ptr, security_message_ptr->data, ASC_MESSAGE_MAX_SIZE);
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("creating a message failed");
        goto cleanup;
    }

    if (linked_list_security_message_t_add_last(message_list, security_message_ptr) == NULL) {
        result = IOTSECURITY_RESULT_EXCEPTION;
        goto cleanup;
    }
cleanup:
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to append message, result=[%d]", result);
    }

    return result;
}
