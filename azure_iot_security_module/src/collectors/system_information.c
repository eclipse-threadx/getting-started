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

#include <time.h>

#include "nx_api.h"
#include "tx_api.h"

#include "asc_security_core/logger.h"
#include "asc_security_core/message_schema_consts.h"
#include "asc_security_core/collectors/collectors_information.h"
#include "asc_security_core/collectors/system_information.h"
#include "asc_security_core/model/collector_enums.h"
#include "asc_security_core/model/collector.h"
#include "asc_security_core/model/event.h"
#include "asc_security_core/model/schema/schema_system_information.h"

#include "asc_security_core/utils/notifier.h"
#include "asc_security_core/utils/os_utils.h"

#define LIB_NX_FORMAT "lib-NetXDuox-%u.%u"
#define DEPENDENCIES "Dependencies"
#define LIB_TX_FORMAT "tx-%u.%u"

static char tx_version[sizeof(LIB_TX_FORMAT) + 5] = {0};
static char nx_version[sizeof(LIB_NX_FORMAT) + 5] = {0};
static const char *UNDEFINED = "Undefined";
static asc_pair extra_details[SYSTEM_INFORMATION_SCHEMA_EXTRA_DETAILS_ENTRIES];

static void _collector_system_information_deinit(collector_internal_t* collector_internal_ptr);
static IOTSECURITY_RESULT _collector_system_information_get_events(collector_internal_t* collector_internal_ptr, linked_list_event_t_handle events);
static IOTSECURITY_RESULT _collect_operation_system_information(collector_internal_t* collector_internal_ptr, system_information_t* data_ptr);

IOTSECURITY_RESULT collector_system_information_init(collector_internal_t* collector_internal_ptr) {
    if (collector_internal_ptr == NULL) {
        log_error("Could not initialize collector_system_information, bad argument");
        return IOTSECURITY_RESULT_BAD_ARGUMENT;
    }

    memset(collector_internal_ptr, 0, sizeof(*collector_internal_ptr));
    strncpy(collector_internal_ptr->name, SYSTEM_INFORMATION_NAME, COLLECTOR_NAME_LENGTH);
    collector_internal_ptr->type = COLLECTOR_TYPE_SYSTEM_INFORMATION;
    collector_internal_ptr->priority = COLLECTOR_PRIORITY_LOW;
    collector_internal_ptr->collect_function = _collector_system_information_get_events;
    collector_internal_ptr->deinit_function = _collector_system_information_deinit;
    collector_internal_ptr->state = (void*)collectors_info_init();

    snprintf(tx_version, sizeof(tx_version), LIB_TX_FORMAT, THREADX_MAJOR_VERSION, THREADX_MINOR_VERSION);
    snprintf(nx_version, sizeof(nx_version), LIB_NX_FORMAT, NETXDUO_MAJOR_VERSION, NETXDUO_MINOR_VERSION);

    memset(extra_details, 0, sizeof(extra_details));
    extra_details[0].key = asc_span_from_str((char *)DEPENDENCIES);
    extra_details[0].value = asc_span_from_str(nx_version);
    notifier_notify(NOTIFY_TOPIC_SYSTEM, NOTIFY_MESSAGE_SYSTEM_CONFIGURATION, collector_internal_ptr);

    return IOTSECURITY_RESULT_OK;
}

static void _collector_system_information_deinit(collector_internal_t* collector_internal_ptr) {
    if (collector_internal_ptr == NULL) {
        log_error("Could not deinitialize collector_system_information, bad argument");
        return;
    }
    collectors_info_handle collectors_info = (collectors_info_handle)collector_internal_ptr->state;

    collectors_info_deinit(collectors_info);
    memset(collector_internal_ptr, 0, sizeof(*collector_internal_ptr));
}

static IOTSECURITY_RESULT _collector_system_information_get_events(collector_internal_t* collector_internal_ptr, linked_list_event_t_handle events) {
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;
    system_information_t* system_information_ptr = NULL;
    event_t* event_ptr = NULL;

    log_debug("Start Collector%s_GetEvents", SYSTEM_INFORMATION_NAME);

    system_information_ptr = schema_system_information_init();
    if (system_information_ptr == NULL) {
        log_error("Failed to allocate SYSTEM_INFORMATION_SCHEMA");
        result = IOTSECURITY_RESULT_MEMORY_EXCEPTION;
        goto cleanup;
    }

    log_debug("Collecting Operation System Information");
    result = _collect_operation_system_information(collector_internal_ptr, system_information_ptr);
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to collect Operation System information, result=[%d]", result);
        goto cleanup;
    }

    event_ptr = event_init(SYSTEM_INFORMATION_PAYLOAD_SCHEMA_VERSION, SYSTEM_INFORMATION_NAME, EVENT_PERIODIC_CATEGORY, EVENT_TYPE_SECURITY_VALUE, itime_time(NULL));
    if (event_ptr == NULL) {
        log_error("Failed to allocate event_t*");
        result = IOTSECURITY_RESULT_MEMORY_EXCEPTION;
        goto cleanup;
    }

    result = event_append_system_information(event_ptr, system_information_ptr);
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to append Operation System information to event, result=[%d]", result);
        goto cleanup;
    }

    result = event_build(event_ptr);
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to build event, result=[%d]", result);
        goto cleanup;
    }

    if (linked_list_event_t_add_last(events, event_ptr) == NULL) {
        result = IOTSECURITY_RESULT_EXCEPTION;
        log_error("Failed to append event to the events list, result=[%d]", result);
        goto cleanup;
    }

cleanup:
    schema_system_information_deinit(system_information_ptr);
    system_information_ptr = NULL;
    if (result != IOTSECURITY_RESULT_OK) {
        event_deinit(event_ptr);
        event_ptr = NULL;
    }

    log_debug("Done Collector%s_GetEvents, result=[%d]", SYSTEM_INFORMATION_NAME, result);
    return result;
}

static IOTSECURITY_RESULT _collect_operation_system_information(collector_internal_t* collector_internal_ptr, system_information_t* data_ptr) {
    log_debug("Start _Collector%s_CollectOperationSystemInformation", SYSTEM_INFORMATION_NAME);
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;
    collectors_info_handle collectors_info = (collectors_info_handle)collector_internal_ptr->state;

    result = schema_system_information_set_os_name(data_ptr, (char*)os_utils_get_os_name());
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("schema_system_information_set_os_name failed, result=[%d]", result);
        goto cleanup;
    }

    result = schema_system_information_set_os_version(data_ptr, tx_version);
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("schema_system_information_set_os_version failed, result=[%d]", result);
        goto cleanup;
    }

    result = schema_system_information_set_os_architecture(data_ptr, (char*)UNDEFINED);
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("schema_system_information_set_os_architecture failed, result=[%d]", result);
        goto cleanup;
    }

    result = schema_system_information_set_hostname(data_ptr, (char*)UNDEFINED);
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("schema_system_information_set_os_hostname failed, result=[%d]", result);
        goto cleanup;
    }

    collectors_info_append(collectors_info, extra_details, SYSTEM_INFORMATION_SCHEMA_EXTRA_DETAILS_ENTRIES);

    result = schema_system_information_set_extra_details(data_ptr, extra_details);
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("schema_system_information_set_extra_details failed, result=[%d]", result);
        goto cleanup;
    }

cleanup:
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed _collect_operation_system_information, result=[%d]", SYSTEM_INFORMATION_NAME, result);
    } else {
        log_debug("Done _collect_operation_system_information, result=[%d]", SYSTEM_INFORMATION_NAME, result);
    }

    return result;
}
