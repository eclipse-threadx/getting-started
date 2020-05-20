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

#ifndef IOTSECURITY_COLLECTOR_COLLECTION_H
#define IOTSECURITY_COLLECTOR_COLLECTION_H

#include <stdint.h>
#include "asc_security_core/iotsecurity_result.h"
#include "asc_security_core/model/collector.h"
#include "asc_security_core/utils/collection/linked_list.h"

typedef struct priority_collectors* PRIORITY_COLLECTORS_HANDLE;
typedef struct collector_collection collector_collection_t;


/**
 * @brief Initialize collector collection
 *
 * @return A @c collector_collection_t* representing the newly created collector collection.
 */
collector_collection_t* collector_collection_init();


/**
 * @brief Deinitialize collector collection
 *
 * @param collector_collection_ptr    representing the collector collection.
 */
void collector_collection_deinit(collector_collection_t* collector_collection_ptr);


/**
 * @brief Return CollectorCollection specific priority collection
 *
 * @param collector_collection_ptr   the collector colleciton
 * @param collector_priority            the priority to retreive
 *
 * @return A @c PRIORITY_COLLECTORS_HANDLE which stands for the specific priority collectors
 */
PRIORITY_COLLECTORS_HANDLE collector_collection_get_by_priority(collector_collection_t* collector_collection_ptr, COLLECTOR_PRIORITY collector_priority);


/**
 * @brief Return CollectorCollection head priority collection
 *
 * @param collector_collection_ptr   the collector colleciton
 * @param collector_collection          the current priority collection
 *
 * @return A @c PRIORITY_COLLECTORS_HANDLE which stands for the first and highest priority collectors
 */
PRIORITY_COLLECTORS_HANDLE collector_collection_get_head_priority(collector_collection_t* collector_collection_ptr);


/**
 * @brief Return next priority
 *
 * @param current_priority  out param
 *
 * @return A @c PRIORITY_COLLECTORS_HANDLE which stands for the next priority collectors
 */
PRIORITY_COLLECTORS_HANDLE collector_collection_get_next_priority(collector_collection_t* collector_collection_ptr, PRIORITY_COLLECTORS_HANDLE priority_collectors_handle);


/**
 * @brief Return a collector handle with the same name
 *
 * @param name  The name of the collecotr
 *
 * @return A @c collector_t* collector handle with the same name
 */
collector_t* collector_collection_get_collector_by_priority(collector_collection_t* collector_collection_ptr, char* name);


/**
 * @brief traverse through the collection and calls action_function for each collector
 *
 * @param action_function   a predicate to call to for each collector
 * @param context           caller context
 */
void collector_collection_foreach(collector_collection_t* collector_collection_ptr, linked_list_collector_t_action action_function, void *context);


/**
 * @brief returns the interval in seconds of the priority collection
 *
 * @param priority_collectors_handle    the priority collection
 *
 * @return A @c uint32_t which stands for the interval in seconds of the priority collection
 */
uint32_t priority_collectors_get_interval(PRIORITY_COLLECTORS_HANDLE priority_collectors_handle);


/**
 * @brief returns the list of collectors of the priority collection
 *
 * @param priority_collectors_handle    the priority collection
 *
 * @return A @c linked_list_collector_t which stands for the list of collectors of the priority collection
 */
linked_list_collector_t_handle priority_collectors_get_list(PRIORITY_COLLECTORS_HANDLE priority_collectors_handle);


/**
 * @brief returns the priority of the priority collection
 *
 * @param priority_collectors_handle    the priority collection
 *
 * @return A @c COLLECTOR_PRIORITY which stands for the priority of the priority collection
 */
COLLECTOR_PRIORITY priority_collectors_get_priority(PRIORITY_COLLECTORS_HANDLE priority_collectors_handle);


/**
 * @brief returns the priority of the current collector collection
 *
 * @param priority_collectors_handle    the priority collection
 *
 * @return A @c collector_t* which stands for the current collector of the priority collection
 */
collector_t* priority_collectors_get_current_collector(PRIORITY_COLLECTORS_HANDLE priority_collectors_handle);


/**
 * @brief sets the current collector iterator
 *
 * @param priority_collectors_handle    the priority collection
 * @param collector_ptr              the collector to be replaced as current
 */
void priority_collectors_set_current_collector(PRIORITY_COLLECTORS_HANDLE priority_collectors_handle, collector_t* collector_ptr);


/**
 * @brief searches for the first non-empty collector
 *
 * @param priority_collectors_handle    the priority collection
 * @param collector_ptr              a pointer to the first non empty collector
 *
 * This function searches for the first non empty collector and retrives it.
 * the iterator is saved based on the last successful collector that poped an event
 *
 * @return An @c IOTSECURITY_RESULT indicating the status of the call.
 */
IOTSECURITY_RESULT priority_collectors_get_current_non_empty_collector(PRIORITY_COLLECTORS_HANDLE priority_collectors_handle, collector_t** collector_ptr);


/**
 * @brief searches for the next non-empty collector
 *
 * @param priority_collectors_handle    the priority collection
 * @param collector_ptr              a pointer to the next non empty collector
 *
 * This function searches for the next non empty collector and retrives it.
 * the iterator is saved based on the last successful collector that poped an event
 *
 * @return An @c IOTSECURITY_RESULT indicating the status of the call.
 */
IOTSECURITY_RESULT priority_collectors_get_next_non_empty_collector(PRIORITY_COLLECTORS_HANDLE priority_collectors_handle, collector_t** collector_ptr);


/**
 * @brief searches for the next non-empty collector
 *
 * @param priority_collectors_handle    the priority collection
 *
 * This function iterates through the priority collection and goes to the begining in case it reached the end
 *
 * @return A @c collector_t* indicating the position of the next cyclic collector.
 */
collector_t* priority_collectors_get_next_cyclic_collector(PRIORITY_COLLECTORS_HANDLE priority_collectors_handle);


#endif /* IOTSECURITY_COLLECTOR_COLLECTION_H */