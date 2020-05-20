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

#include "asc_security_core/asc/asc_json.h"

#include "asc_security_core/configuration.h"
#include "asc_security_core/logger.h"
#include "asc_security_core/message_schema_consts.h"
#include "asc_security_core/model/message.h"
#include "asc_security_core/object_pool.h"
#include "asc_security_core/utils/string_utils.h"

typedef enum MESSAGE_STATUS_TAG {
    MESSAGE_STATUS_UNINITIALIZED    = 0,
    MESSAGE_STATUS_PROCESSING       = 1,
    MESSAGE_STATUS_EXCEPTION        = 2,
    MESSAGE_STATUS_OK               = 3,
} MESSAGE_STATUS;

typedef struct message {
    COLLECTION_INTERFACE(struct message);

    // buffer workspace
    uint8_t buffer[ASC_MESSAGE_MAX_SIZE];

    // JSON builder on top of the message buffer workspace
    asc_json_builder builder;

    MESSAGE_STATUS status;
    bool has_events;

    // message schema properties
    asc_span agent_id;
    asc_span agent_version;
    asc_span message_schema_version;

} message_t;

OBJECT_POOL_DECLARATIONS(message_t, MESSAGE_OBJECT_POOL_COUNT);
OBJECT_POOL_DEFINITIONS(message_t, MESSAGE_OBJECT_POOL_COUNT);

/**
 * @brief Build the message
 *
 * @param message_ptr    message ptr
 *
 * @return IOTSECRUITY_RESULT
 */
static IOTSECURITY_RESULT _Message_Build(message_t* message_ptr);
static IOTSECURITY_RESULT _Message_SetStatus(message_t* message_ptr, MESSAGE_STATUS status);


message_t* message_init(const char* agent_id, const char* agent_version) {
    log_debug("Creating message, agent_id=[%s], agent_version=[%s]",
        string_utils_value_or_empty(agent_id),
        string_utils_value_or_empty(agent_version)
    );
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;
    message_t* message_ptr = NULL;

    if (string_utils_is_blank(agent_id) || string_utils_is_blank(agent_version)) {
        log_error("Failed to create a new message due to bad argument");
        result = IOTSECURITY_RESULT_BAD_ARGUMENT;
        goto cleanup;
    }

    message_ptr = object_pool_get(message_t);
    if (message_ptr == NULL) {
        log_error("Failed to allocate memory for message");
        goto cleanup;
    }
    memset(message_ptr, 0, sizeof(message_t));

    message_ptr->agent_id = asc_span_from_str((char*)agent_id);
    message_ptr->agent_version = asc_span_from_str((char*)agent_version);
    message_ptr->message_schema_version = asc_span_from_str(MESSAGE_SCHEMA_VERSION);
    message_ptr->status = MESSAGE_STATUS_PROCESSING;
    message_ptr->has_events = false;
    memset(message_ptr->buffer, 0, message_get_capacity(message_ptr));

    asc_json_builder* builder = NULL;
    builder = &message_ptr->builder;

    if (asc_json_builder_init(builder, ASC_SPAN_FROM_BUFFER(message_ptr->buffer)) != ASC_OK) {
        log_error("Failed to initialize a new message");
        result = IOTSECURITY_RESULT_EXCEPTION;
        goto cleanup;
    }

    if (asc_json_builder_append_token(builder, asc_json_token_object_start()) != ASC_OK) {
        log_error("Failed to set message property=[%s]", EVENT_NAME_KEY);
        result = IOTSECURITY_RESULT_EXCEPTION;
        goto cleanup;
    }

    if (asc_json_builder_append_object(builder, asc_span_from_str((char*)AGENT_VERSION_KEY), asc_json_token_string(message_ptr->agent_version)) != ASC_OK) {
        log_error("Failed to set message property=[%s]", AGENT_VERSION_KEY);
        result = IOTSECURITY_RESULT_EXCEPTION;
        goto cleanup;
    }

    if (asc_json_builder_append_object(builder, asc_span_from_str((char*)AGENT_ID_KEY), asc_json_token_string(message_ptr->agent_id)) != ASC_OK) {
        log_error("Failed to set message property=[%s]", AGENT_ID_KEY);
        result = IOTSECURITY_RESULT_EXCEPTION;
        goto cleanup;
    }

    if (asc_json_builder_append_object(builder, asc_span_from_str((char*)MESSAGE_SCHEMA_VERSION_KEY), asc_json_token_string(message_ptr->message_schema_version)) != ASC_OK) {
        log_error("Failed to set message property=[%s]", MESSAGE_SCHEMA_VERSION_KEY);
        result = IOTSECURITY_RESULT_EXCEPTION;
        goto cleanup;
    }

    if (asc_json_builder_append_object(builder, asc_span_from_str((char*)EVENTS_KEY), asc_json_token_array_start()) != ASC_OK) {
        log_error("Failed to set message property=[%s]", EVENTS_KEY);
        result = IOTSECURITY_RESULT_EXCEPTION;
        goto cleanup;
    }

cleanup:
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to create a message, result=[%d]", result);

        _Message_SetStatus(message_ptr, MESSAGE_STATUS_EXCEPTION);

        message_deinit(message_ptr);
        message_ptr = NULL;
    } else {
        log_debug("Message has been created successfully, result=[%d]", result);
    }

    return message_ptr;
}


void message_deinit(message_t* message_ptr) {
    if (message_ptr != NULL) {
        if (message_ptr->status == MESSAGE_STATUS_EXCEPTION) {
            log_error("Deinitialize message with status=[%d]", message_ptr->status);
        }

        object_pool_free(message_t, message_ptr);
        message_ptr = NULL;
    }
}


static IOTSECURITY_RESULT _Message_Build(message_t* message_ptr) {
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;

    if (message_ptr == NULL) {
        result = IOTSECURITY_RESULT_BAD_ARGUMENT;
        log_error("Failed to build message, result=[%d]", result);
        goto cleanup;
    }

    switch (message_ptr->status) {
        case MESSAGE_STATUS_EXCEPTION:
            result = IOTSECURITY_RESULT_EXCEPTION;
            log_error("Cannot build the message, status=[%d]", message_ptr->status);
            goto cleanup;
        case MESSAGE_STATUS_OK:
            result = IOTSECURITY_RESULT_OK;
            goto cleanup;
        case MESSAGE_STATUS_PROCESSING:
        default:
            // continue processing and build the message
            break;
    }

    asc_json_builder* builder = &message_ptr->builder;

    if (asc_json_builder_append_token(builder, asc_json_token_array_end()) != ASC_OK) {
        log_error("Failed to close json array, property=[%s]", EVENT_ID_KEY);
        result = IOTSECURITY_RESULT_EXCEPTION;
        goto cleanup;
    }

    if (asc_json_builder_append_token(builder, asc_json_token_object_end()) != ASC_OK) {
        log_error("Failed to close object=[message]");
        result = IOTSECURITY_RESULT_EXCEPTION;
        goto cleanup;
    }

    result = _Message_SetStatus(message_ptr, MESSAGE_STATUS_OK);
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to set message status, result=[%d]", result);
        goto cleanup;
    }
cleanup:
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to build message, result=[%d]", result);

        _Message_SetStatus(message_ptr, MESSAGE_STATUS_EXCEPTION);
    }

    return result;
}


static IOTSECURITY_RESULT _Message_SetStatus(message_t* message_ptr, MESSAGE_STATUS status) {
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;

    if (message_ptr != NULL) {
        message_ptr->status = status;
    }

    return result;
}

IOTSECURITY_RESULT message_to_json(message_t* message_ptr, char* buffer, uint32_t size) {
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;

    if (message_ptr == NULL) {
        result = IOTSECURITY_RESULT_BAD_ARGUMENT;
        log_error("Failed to retrieve event data, result=[%d]", result);
        goto cleanup;
    }

    if (buffer == NULL) {
        result = IOTSECURITY_RESULT_BAD_ARGUMENT;
        log_error("Buffer cannot be null, result=[%d]", result);
        goto cleanup;
    }

    // build the message if needed
    result = _Message_Build(message_ptr);
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to build message, result=[%d]", result);
        goto cleanup;
    }

    if (message_ptr->status != MESSAGE_STATUS_OK) {
        result = IOTSECURITY_RESULT_EXCEPTION;
        log_error("Cannot retrieve message data, state=[%d]", message_ptr->status);
        goto cleanup;
    }

    if (asc_span_to_str(buffer, (int32_t)size, asc_json_builder_span_get(&message_ptr->builder)) != ASC_OK) {
        result = IOTSECURITY_RESULT_EXCEPTION;
        log_error("Failed to extract event data");
        goto cleanup;
    }

cleanup:
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to retrieve message data, result=[%d]", result);

        _Message_SetStatus(message_ptr, MESSAGE_STATUS_EXCEPTION);
    }

    return result;
}


IOTSECURITY_RESULT message_append(message_t* message_ptr, event_t* event_ptr) {
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;

    if (message_ptr == NULL) {
        result = IOTSECURITY_RESULT_BAD_ARGUMENT;
        log_error("message_append bad argument exception");
        goto cleanup;
    }

    // Retrieve the asc_json_builder
    asc_json_builder* builder = &message_ptr->builder;

    if (!message_can_append(message_ptr, event_ptr)) {
        log_error("the event exceeded the size of the message");
        result = IOTSECURITY_RESULT_EXCEPTION;
        goto cleanup;
    }

    char event_data[ASC_EVENT_MAX_SIZE];
    result = event_get_data(event_ptr, event_data, ASC_EVENT_MAX_SIZE);
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to retrieve event data");
        goto cleanup;
    }

    asc_span event = asc_span_from_str(event_data);

    if (asc_json_builder_append_array_item(builder, asc_json_token_object(event)) != ASC_OK) {
        log_error("Failed to append event to message");
        result = IOTSECURITY_RESULT_EXCEPTION;
        goto cleanup;
    }

    // Update message status
    message_ptr->has_events = true;

cleanup:
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to append an event to message, result=[%d]", result);
    }

    return result;
}


uint32_t message_get_length(message_t* message_ptr) {
    if (message_ptr != NULL) {
        return (uint32_t)asc_span_length(asc_json_builder_span_get((asc_json_builder const*)&message_ptr->builder));
    }

    return 0;
}


uint32_t message_get_capacity(message_t* message_ptr) {
    if (message_ptr != NULL) {
        return (uint32_t)asc_span_capacity(asc_json_builder_span_get((asc_json_builder const*)&message_ptr->builder));
    }

    return 0;
}

bool message_can_append(message_t* message_ptr, event_t* event_ptr) {
    return (event_get_length(event_ptr) + message_get_length(message_ptr) < message_get_capacity(message_ptr));
}


bool message_has_events(message_t* message_ptr) {
    return message_ptr->has_events;
}