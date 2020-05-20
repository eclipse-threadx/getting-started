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

#ifndef CORE_H
#define CORE_H

#include "asc_security_core/iotsecurity_result.h"
#include "asc_security_core/model/message.h"
#include "asc_security_core/collector_collection.h"
#include "asc_security_core/utils/collection/linked_list.h"

#define SECURITY_MESSAGE_OBJECT_POOL_COUNT ASC_CORE_MAX_MESSAGES_IN_MEMORY

typedef struct security_message {
    COLLECTION_INTERFACE(struct security_message);

    char data[ASC_MESSAGE_MAX_SIZE];
} security_message_t;

OBJECT_POOL_DECLARATIONS(security_message_t, SECURITY_MESSAGE_OBJECT_POOL_COUNT);
LINKED_LIST_DECLARATIONS(security_message_t);

typedef struct core core_t;

/**
 * @brief Initialize ClientCore handle
 *
 * @return core_t*
 */
core_t* core_init();

/**
 * @brief Collect events from all of the registered collectors.
 *
 * @param core_ptr        the client core_t handle
 *
 * @return IOTSECURITY_RESULT_OK on success, IOTSECURITY_RESULT_EXCEPTION otherwise.
 */
IOTSECURITY_RESULT core_collect(core_t* core_ptr);

/**
 * @brief Build messages from the collected events and appends them to the output_message_list.
 *
 * @param core_ptr        the client core_t handle
 * @param output_message_list       out param- list of messages
 *
 * @return IOTSECURITY_RESULT_OK on success, IOTSECURITY_RESULT_EXCEPTION otherwise.
 */
IOTSECURITY_RESULT core_get(core_t* core_ptr, linked_list_security_message_t* output_message_list);

/**
 * @brief Deinitialize ClientCore handle
 *
 * @param core_ptr        the client core_t handle
 */
void core_deinit(core_t* core_ptr);

/**
 * @brief returns the collector collection pointed by the client core_t
 *
 * @param core_ptr        the client core_t handle
 */
collector_collection_t* core_get_collector_collection(core_t* core_ptr);


#endif /* CORE_H */