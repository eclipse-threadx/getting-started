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

#include "asc_security_core/collector_collection.h"
#include "asc_security_core/collector_collection_internal.h"
#include "asc_security_core/collectors_headers.h"
#include "asc_security_core/logger.h"
#include "asc_security_core/utils/collection/linked_list.h"
#include "asc_security_core/utils/irand.h"
#include "asc_security_core/utils/itime.h"

const char *g_collector_names[COLLECTOR_TYPE_COUNT] = {
#ifdef COLLECTOR_SYSTEM_INFORMATION_ENABLED
    COLLECTOR_NAME_SYSTEM_INFORMATION,
#endif
#ifdef COLLECTOR_CONNECTION_CREATE_ENABLED
    COLLECTOR_NAME_CONNECTION_CREATE,
#endif
#ifdef COLLECTOR_LISTENING_PORTS_ENABLED
    COLLECTOR_NAME_LISTENING_PORTS,
#endif
#ifdef COLLECTOR_HEARTBEAT_ENABLED
    COLLECTOR_NAME_HEARTBEAT,
#endif
#ifdef COLLECTOR_TEST_ENABLED
    COLLECTOR_NAME_TEST
#endif
};

const uint32_t g_collector_collections_intervals[COLLECTOR_PRIORITY_COUNT] = {
    ASC_HIGH_PRIORITY_INTERVAL,
    ASC_MEDIUM_PRIORITY_INTERVAL,
    ASC_LOW_PRIORITY_INTERVAL
};

static IOTSECURITY_RESULT collector_collection_internal_set_random_collected_time(PRIORITY_COLLECTORS_HANDLE priority_collector_ptr);

IOTSECURITY_RESULT collector_collection_internal_init_startup_time(collector_collection_t* collector_collection_ptr) {
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;

    PRIORITY_COLLECTORS_HANDLE priority_collector_ptr = collector_collection_get_head_priority(collector_collection_ptr);

    while (priority_collector_ptr != NULL) {
        result = collector_collection_internal_set_random_collected_time(priority_collector_ptr);
        if (result != IOTSECURITY_RESULT_OK) {
            log_error("Failed to set random collected time to collectors, collector_priority=[%d], result=[%d]", priority_collectors_get_priority(priority_collector_ptr), result);
            goto cleanup;
        }

        priority_collector_ptr = collector_collection_get_next_priority(collector_collection_ptr, priority_collector_ptr);
    }

cleanup:
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to init collector collection init startup time, result=[%d]", result);
    }

    return result;
}

static IOTSECURITY_RESULT collector_collection_internal_set_random_collected_time(PRIORITY_COLLECTORS_HANDLE priority_collector_ptr) {
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;

    linked_list_collector_t_handle priority_collector_list = priority_collectors_get_list(priority_collector_ptr);
    uint32_t interval = priority_collectors_get_interval(priority_collector_ptr);

    collector_t* collector_ptr = linked_list_collector_t_get_first(priority_collector_list);
    while (collector_ptr != NULL) {
        time_t current_time = itime_time(NULL);
        if (current_time == (time_t)(-1)) {
            result = IOTSECURITY_RESULT_EXCEPTION;
            goto cleanup;
        }

        collector_set_last_collected_timestamp(collector_ptr, itime_time(NULL) - (irand_int() % (2 * interval) + interval));
        collector_ptr = collector_ptr->next;
    }

cleanup:
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to set random collected time, collector_priority=[%d], result=[%d]", priority_collectors_get_priority(priority_collector_ptr), result);
    }

    return result;
}