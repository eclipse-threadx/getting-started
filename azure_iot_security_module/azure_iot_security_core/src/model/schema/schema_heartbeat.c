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
#include "asc_security_core/asc/asc_span.h"
#include "asc_security_core/logger.h"
#include "asc_security_core/configuration.h"
#include "asc_security_core/message_schema_consts.h"
#include "asc_security_core/model/schema/schema_heartbeat.h"
#include "asc_security_core/object_pool.h"

#define SCHEMA_HEARTBEAT_OBJECT_POOL_COUNT ASC_COLLECTOR_HEARTBEAT_MAX_OBJECTS_IN_CACHE

typedef struct schema_heartbeat {
    COLLECTION_INTERFACE(struct schema_heartbeat);
} schema_heartbeat_t;

OBJECT_POOL_DECLARATIONS(schema_heartbeat_t, SCHEMA_HEARTBEAT_OBJECT_POOL_COUNT);
OBJECT_POOL_DEFINITIONS(schema_heartbeat_t, SCHEMA_HEARTBEAT_OBJECT_POOL_COUNT);


schema_heartbeat_t* schema_heartbeat_init() {
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;
    schema_heartbeat_t* data_ptr = NULL;

    data_ptr = object_pool_get(schema_heartbeat_t);
    if (data_ptr == NULL) {
        result = IOTSECURITY_RESULT_MEMORY_EXCEPTION;
        log_error("Failed to allocate schema_heartbeat");
        goto cleanup;
    }

    memset(data_ptr, 0, sizeof(schema_heartbeat_t));

cleanup:
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to initialize heartbeat schema, result=[%d]", result);
    }

    return data_ptr;
}


void schema_heartbeat_deinit(schema_heartbeat_t* data_ptr) {
    if (data_ptr != NULL) {
        object_pool_free(schema_heartbeat_t, data_ptr);
        data_ptr = NULL;
    }
}

