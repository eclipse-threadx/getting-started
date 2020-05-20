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

#ifndef COLLECTOR_H
#define COLLECTOR_H

#include <assert.h>
#include <stdbool.h>
#include "asc_security_core/utils/itime.h"
#include "asc_security_core/iotsecurity_result.h"
#include "asc_security_core/model/event.h"
#include "asc_security_core/model/collector_enums.h"
#include "asc_security_core/object_pool.h"
#include "asc_security_core/utils/collection/linked_list.h"

#ifndef COLLECTOR_OBJECT_POOL_COUNT
#define COLLECTOR_OBJECT_POOL_COUNT COLLECTOR_TYPE_COUNT
#endif

#define RECORD_VALUE_MAX_LENGTH 512
#define COLLECTOR_NAME_LENGTH 20

typedef struct collector_internal_ptr collector_internal_t;

/**
 * @brief Initialize the collector internal
 *
 * @param collector_internal_ptr   A handle to the collector internal to initialize.
 *
 * @return IOTSECURITY_RESULT_OK on success
 */
typedef IOTSECURITY_RESULT (*INIT_FUNCTION)(collector_internal_t* collector_internal_ptr);

/**
 * @brief Get events from the collector
 *
 * @param collector_internal_ptr   A handle to the collector internal.
 *
 * @param events   A list to which the Event[s] should be added.
 *
 * @return   IOTSECURITY_RESULT_OK on success
 *           IOTSECURITY_RESULT_EMPTY when there are no events. In that case, @events will be null.
 *           IOTSECURITY_RESULT_EXCEPTION otherwise
 */
typedef IOTSECURITY_RESULT (*COLLECT_FUNCTION)(collector_internal_t* collector_internal_ptr, linked_list_event_t_handle events);

/**
 * @brief Function which used in order to free a specific collector.
 *
 * @param collector_internal_ptr   A handle to the collector internal to deinitialize.
 */
typedef void (*DEINIT_FUNCTION)(collector_internal_t* collector_internal_ptr);


typedef struct collector_internal_ptr {
    char name[COLLECTOR_NAME_LENGTH];
    COLLECTOR_TYPE type;
    COLLECTOR_PRIORITY priority;

    COLLECT_FUNCTION collect_function;
    DEINIT_FUNCTION deinit_function;

    void* state;
} collector_internal_t;


typedef enum COLLECTOR_STATUS_TAG {
    COLLECTOR_STATUS_OK,
    COLLECTOR_STATUS_EXCEPTION
} COLLECTOR_STATUS;


typedef struct collector {
    COLLECTION_INTERFACE(struct collector);

    bool enabled;
    COLLECTOR_STATUS status;
    unsigned int failure_count;
    time_t last_collected_timestamp;
    time_t last_sent_timestamp;

    collector_internal_t internal;

    linked_list_event_t event_list;
} collector_t;

OBJECT_POOL_DECLARATIONS(collector_t, COLLECTOR_OBJECT_POOL_COUNT);
LINKED_LIST_DECLARATIONS(collector_t);


/**
 * @brief Initialize a Collector
 *
 * @param init_function      The initialization function of the collector internal
 *
 * @return collector ptr
 */
collector_t* collector_init(INIT_FUNCTION init_function);


/**
 * @brief Deinitialize Collector
 *
 * @param collector_ptr  collector ptr
 */
void collector_deinit(collector_t* collector_ptr);


/**
 * @brief Collector name getter
 *
 * @param collector_ptr  collector ptr
 *
 * @return Collector name
 */
const char* collector_get_name(collector_t* collector_ptr);


/**
 * @brief Collector priority getter
 *
 * @param collector_ptr  collector ptr
 *
 * @return Collector priority
 */
COLLECTOR_PRIORITY collector_get_priority(collector_t* collector_ptr);


/**
 * @brief Collector last collected timestamp getter
 *
 * @param collector_ptr  collector ptr
 *
 * @return Collector last collected timestamp
 */
time_t collector_get_last_collected_timestamp(collector_t* collector_ptr);


/**
 * @brief Collector last collected timestamp setter
 *
 * @param collector_ptr  collector_t*
 * @param last_collected_timestamp  the timestamp
 *
 * @return IOTSECURITY_RESULT_OK on success, IOTSECURITY_RESULT_EXCEPTION otherwise
 */
IOTSECURITY_RESULT collector_set_last_collected_timestamp(collector_t* collector_ptr, time_t last_collected_timestamp);


/**
 * @brief Collect events
 *
 * @param collector_ptr    The collector handle
 *
 * @return IOTSECURITY_RESULT_OK on success, IOTSECURITY_RESULT_EXCEPTION otherwise
 */
IOTSECURITY_RESULT collector_collect(collector_t* collector_ptr);


/**
 * @brief Peek the top event
 *
 * @param collector_ptr    The collector handle
 * @param event    Out param. The peeked event.
 *
 * @return  IOTSECURITY_RESULT_OK on success
 *          IOTSECURITY_RESULT_EMPTY when there are no events. In that case, @event will be null.
 *          IOTSECURITY_RESULT_EXCEPTION otherwise
 */
IOTSECURITY_RESULT collector_peek_event(collector_t* collector_ptr, event_t** event);


/**
 * @brief Pop the top event
 *
 * @param collector_ptr    The collector handle
 * @param event    Out param. The popped event.
 *
 * @return  IOTSECURITY_RESULT_OK on success
 *          IOTSECURITY_RESULT_EMPTY when there are no events. In that case, @event will be null.
 *          IOTSECURITY_RESULT_EXCEPTION otherwise
 */
IOTSECURITY_RESULT collector_pop_event(collector_t* collector, event_t** event);


#endif /* COLLECTOR_H */
