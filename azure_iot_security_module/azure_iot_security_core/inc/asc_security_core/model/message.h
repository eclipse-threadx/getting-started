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

#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdint.h>
#include <stdbool.h>
#include "asc_security_core/iotsecurity_result.h"
#include "asc_security_core/model/event.h"

#define MESSAGE_SCHEMA_VERSION "1.0"
#define MESSAGE_OBJECT_POOL_COUNT ASC_CORE_MAX_MESSAGES_IN_MEMORY

typedef struct message message_t;


/**
 * @brief Initialize a Message
 *
 * @param agent_id                  Unique GUID for the message
 * @param agent_version             Agent version
 * @param message_schema_version    Message schema version
 *
 * @return message_t*
 */
message_t* message_init(const char* agent_id, const char* agent_version);


/**
 * @brief Deinitialize Message
 *
 * @param message_ptr    event ptr
 */
void message_deinit(message_t* message_ptr);


/**
 * @brief Serialize the given message to a json string.
 *        Allocate memory for the given buffer.
 *
 * @param message_ptr    message ptr
 * @param buffer            out param: the message string in json
 * @param size              buffer size
 *
 * @return An @c IOTSECURITY_RESULT indicating the status of the call.
 */
IOTSECURITY_RESULT message_to_json(message_t* message_ptr, char* buffer, uint32_t size);


/**
 * @brief Add the event to the given Message.
 *
 * @param message_ptr    message ptr
 * @param event_ptr      event ptr
 *
 * @return An @c IOTSECURITY_RESULT indicating the status of the call.
 */
IOTSECURITY_RESULT message_append(message_t* message_ptr, event_t* event_ptr);


/**
 * @brief Check if is it possible to append the event to the given message
 *
 * @param message_ptr    message ptr
 * @param event_ptr      event ptr
 *
 * @return true if there is enough space, false otherwise
 */
bool message_can_append(message_t* message_ptr, event_t* event_ptr);


/**
 * @brief Return the message capacity
 *
 * @param message_ptr    message ptr
 *
 * @return the message capacity
 */
uint32_t message_get_capacity(message_t* message_ptr);


/**
 * @brief Return the message length
 *
 * @param message_ptr    message ptr
 *
 * @return the message length
 */
uint32_t message_get_length(message_t* message_ptr);


/**
 * @brief Check whether the given message has events.
 *
 * @param message_ptr    message ptr
 *
 * @return true if it has events, false otherwise.
 */
bool message_has_events(message_t* message_ptr);


#endif /* MESSAGE_H */