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

#include <stdlib.h>
#include "asc_security_core/logger.h"
#include "asc_security_core/collector_collection.h"
#include "asc_security_core/object_pool.h"
#include "asc_security_core/collector_collection_internal.h"
#include "asc_security_core/collector_collection_factory.h"
#include "asc_security_core/collectors_headers.h"

#define COLLECTOR_COLLECTION_OBJECT_POOL_COUNT 1

typedef struct priority_collectors {
    uint32_t interval;
    COLLECTOR_PRIORITY priority;
    collector_t* current_collector_ptr;

    linked_list_collector_t collector_list;
} priority_collectors_t;

typedef struct collector_collection {
    COLLECTION_INTERFACE(struct collector_collection);

    priority_collectors_t collector_array[COLLECTOR_PRIORITY_COUNT];
} collector_collection_t;

OBJECT_POOL_DECLARATIONS(collector_collection_t, COLLECTOR_COLLECTION_OBJECT_POOL_COUNT);
OBJECT_POOL_DEFINITIONS(collector_collection_t, COLLECTOR_COLLECTION_OBJECT_POOL_COUNT);

static IOTSECURITY_RESULT _collector_collection_init_collector_lists(collector_collection_t* collector_collection_ptr, INIT_FUNCTION* collector_init_array, uint32_t array_size);
static void _collector_collection_deinit_collector_lists(linked_list_collector_t_handle collector_list_ptr);
static bool _collector_collection_name_match_function(collector_t* collector_ptr, void* match_context);

collector_collection_t* collector_collection_init() {
    log_debug("Init collector collection");
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;
    collector_collection_t* collector_collection_ptr = NULL;

    collector_collection_ptr = object_pool_get(collector_collection_t);
    if (collector_collection_ptr == NULL) {
        log_error("Failed to initialized collector collection");
        result = IOTSECURITY_RESULT_MEMORY_EXCEPTION;
        goto cleanup;
    }

    memset(collector_collection_ptr, 0, sizeof(collector_collection_t));

    INIT_FUNCTION* collector_init_array = NULL;
    uint32_t collector_init_array_size = 0;

    result = collector_collection_factory_get_initialization_array(&collector_init_array, &collector_init_array_size);
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Collector collection array is not being initialized properly");
        goto cleanup;
    }

    result = _collector_collection_init_collector_lists(collector_collection_ptr, collector_init_array, collector_init_array_size);
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Collector collection failed to initialize collector lists, result=[%d]", result);
        goto cleanup;
    }

    result = collector_collection_internal_init_startup_time(collector_collection_ptr);
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Collector collection failed to init collectors startup time, result=[%d]", result);
        goto cleanup;
    }


cleanup:
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to initialize collector collection, result=[%d]", result);
        collector_collection_ptr = NULL;
    }

    return collector_collection_ptr;
}


void collector_collection_deinit(collector_collection_t* collector_collection_ptr) {
    if (collector_collection_ptr == NULL) {
        return;
    }

    for (int priority=0; priority < COLLECTOR_PRIORITY_COUNT; priority++) {
        _collector_collection_deinit_collector_lists(&(collector_collection_ptr->collector_array[priority].collector_list));
        collector_collection_ptr->collector_array[priority].current_collector_ptr = NULL;
    }

    object_pool_free(collector_collection_t, collector_collection_ptr);
    collector_collection_ptr = NULL;
}


PRIORITY_COLLECTORS_HANDLE collector_collection_get_head_priority(collector_collection_t* collector_collection_ptr) {
    return &(collector_collection_ptr->collector_array[COLLECTOR_PRIORITY_HIGH]);
}


PRIORITY_COLLECTORS_HANDLE collector_collection_get_next_priority(collector_collection_t* collector_collection_ptr, PRIORITY_COLLECTORS_HANDLE priority_collectors_handle) {
    uint32_t current_priority = priority_collectors_handle->priority +1;
    if (current_priority == COLLECTOR_PRIORITY_COUNT) {
        return NULL;
    }

    return &(collector_collection_ptr->collector_array[current_priority]);
}

PRIORITY_COLLECTORS_HANDLE collector_collection_get_by_priority(collector_collection_t* collector_collection_ptr, COLLECTOR_PRIORITY collector_priority) {
    if (collector_priority >= COLLECTOR_PRIORITY_COUNT) {
        return NULL;
    }

    return &(collector_collection_ptr->collector_array[collector_priority]);
}

static bool _collector_collection_name_match_function(collector_t* collector_ptr, void* match_context) {
    return collector_ptr == NULL ? false : (strcmp(collector_ptr->internal.name, (char*)match_context) == 0);
}

collector_t* collector_collection_get_collector_by_priority(collector_collection_t* collector_collection_ptr, char* name) {
    collector_t* collector_ptr = NULL;
    PRIORITY_COLLECTORS_HANDLE priority_collector_ptr = collector_collection_get_head_priority(collector_collection_ptr);

    while (priority_collector_ptr != NULL) {
        linked_list_collector_t_handle collector_list = priority_collectors_get_list(priority_collector_ptr);

        collector_ptr = linked_list_collector_t_find(collector_list, _collector_collection_name_match_function, (void*)name);
        if (collector_ptr != NULL) {
            goto cleanup;
        }

        priority_collector_ptr = collector_collection_get_next_priority(collector_collection_ptr, priority_collector_ptr);
    }

cleanup:
    return collector_ptr;
}

void collector_collection_foreach(collector_collection_t* collector_collection_ptr, linked_list_collector_t_action action_function, void *context) {
    for (PRIORITY_COLLECTORS_HANDLE prioritized_collectors = collector_collection_get_head_priority(collector_collection_ptr) ; prioritized_collectors != NULL; prioritized_collectors = collector_collection_get_next_priority(collector_collection_ptr, prioritized_collectors)) {
        linked_list_collector_t_foreach(priority_collectors_get_list(prioritized_collectors), action_function, context);
    }
}


uint32_t priority_collectors_get_interval(PRIORITY_COLLECTORS_HANDLE priority_collectors_handle) {
    return priority_collectors_handle->interval;
}

COLLECTOR_PRIORITY priority_collectors_get_priority(PRIORITY_COLLECTORS_HANDLE priority_collectors_handle) {
    return priority_collectors_handle->priority;
}

linked_list_collector_t_handle priority_collectors_get_list(PRIORITY_COLLECTORS_HANDLE priority_collectors_handle) {
    return &(priority_collectors_handle->collector_list);
}

IOTSECURITY_RESULT _collector_collection_init_collector_lists(collector_collection_t* collector_collection_ptr, INIT_FUNCTION* collector_init_array, uint32_t collector_init_array_size) {
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;

    for (int priority=0; priority < COLLECTOR_PRIORITY_COUNT; priority++) {
        linked_list_collector_t_init(&(collector_collection_ptr->collector_array[priority].collector_list), NULL);
        collector_collection_ptr->collector_array[priority].interval = g_collector_collections_intervals[priority];
        collector_collection_ptr->collector_array[priority].current_collector_ptr = NULL;
        collector_collection_ptr->collector_array[priority].priority = priority;
    }

    uint32_t collector_count = collector_init_array_size;
    for (unsigned int i=0; i < collector_count; i++){
        collector_t* collector_ptr = collector_init(collector_init_array[i]);

        if (collector_ptr == NULL) {
            result = IOTSECURITY_RESULT_MEMORY_EXCEPTION;
            goto cleanup;
        }

        COLLECTOR_PRIORITY priority = collector_get_priority(collector_ptr);

        linked_list_collector_t_handle current_collector_list_handle = &(collector_collection_ptr->collector_array[priority].collector_list);

        if (linked_list_collector_t_add_last(current_collector_list_handle, collector_ptr) == NULL){
            log_error("Could not append Collector=[%s] to collector list", collector_get_name(collector_ptr));
            result = IOTSECURITY_RESULT_EXCEPTION;
            goto cleanup;
        }
        collector_collection_ptr->collector_array[priority].current_collector_ptr = linked_list_collector_t_get_first(current_collector_list_handle);
    }

cleanup:

    return result;
}

IOTSECURITY_RESULT priority_collectors_get_current_non_empty_collector(PRIORITY_COLLECTORS_HANDLE priority_collectors_handle, collector_t** collector_ptr) {
    event_t* event_ptr;
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;
    collector_t* current_collector = NULL;
    uint32_t number_of_collectors_passed = 0;

    while(number_of_collectors_passed < linked_list_collector_t_get_size(&(priority_collectors_handle->collector_list))) {
        current_collector = priority_collectors_handle->current_collector_ptr;
        result = collector_peek_event(current_collector, &event_ptr);
        if (result == IOTSECURITY_RESULT_EMPTY) {
            priority_collectors_handle->current_collector_ptr = priority_collectors_get_next_cyclic_collector(priority_collectors_handle);
            number_of_collectors_passed++;
            continue;
        } else if (result != IOTSECURITY_RESULT_OK) {
            log_error("peeking event at collector [%s] failed", collector_get_name(current_collector));
            goto cleanup;
        }

        goto cleanup;
    }

cleanup:
    if (result == IOTSECURITY_RESULT_OK) {
        *collector_ptr = current_collector;
    }

    if (number_of_collectors_passed == linked_list_collector_t_get_size(&(priority_collectors_handle->collector_list))) {
        *collector_ptr = NULL;
        result = IOTSECURITY_RESULT_OK;
    }

    return result;
}

IOTSECURITY_RESULT priority_collectors_get_next_non_empty_collector(PRIORITY_COLLECTORS_HANDLE priority_collectors_handle, collector_t** collector_ptr) {
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;
    priority_collectors_handle->current_collector_ptr = priority_collectors_get_next_cyclic_collector(priority_collectors_handle);

    result = priority_collectors_get_current_non_empty_collector(priority_collectors_handle, collector_ptr);
    if (result != IOTSECURITY_RESULT_OK) {
        goto cleanup;
    }

cleanup:
    return result;
}

collector_t* priority_collectors_get_current_collector(PRIORITY_COLLECTORS_HANDLE priority_collectors_handle) {
    return priority_collectors_handle->current_collector_ptr;
}

void priority_collectors_set_current_collector(PRIORITY_COLLECTORS_HANDLE priority_collectors_handle, collector_t* current_item) {
    priority_collectors_handle->current_collector_ptr = current_item;
}

collector_t* priority_collectors_get_next_cyclic_collector(PRIORITY_COLLECTORS_HANDLE priority_collectors_handle) {
    collector_t* current_item = priority_collectors_handle->current_collector_ptr;

    if (current_item == NULL) {
        return NULL;
    }

    if (current_item->next == NULL) {
        current_item = linked_list_collector_t_get_first(&(priority_collectors_handle->collector_list));
    } else {
        current_item = current_item->next;
    }

    return current_item;
}

static void _collector_collection_deinit_collector_lists(linked_list_collector_t_handle collector_list_ptr)
{
    collector_t* collector_ptr = linked_list_collector_t_get_first(collector_list_ptr);
    while (collector_ptr != NULL) {
        linked_list_collector_t_remove(collector_list_ptr, collector_ptr);

        if (collector_ptr != NULL) {
            collector_deinit(collector_ptr);
        }

        collector_ptr = linked_list_collector_t_get_first(collector_list_ptr);
    }

}