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
#include "asc_security_core/asc/asc_span.h"
#include "asc_security_core/logger.h"
#include "asc_security_core/configuration.h"
#include "asc_security_core/model/schema/schema_system_information.h"
#include "asc_security_core/object_pool.h"

#define SCHEMA_SYSTEM_INFORMATION_OBJECT_POOL_COUNT ASC_COLLECTOR_SYSTEM_INFORMATION_MAX_OBJECTS_IN_CACHE

typedef struct schema_system_information {
    COLLECTION_INTERFACE(struct schema_system_information);
    asc_pair extra_details[SYSTEM_INFORMATION_SCHEMA_EXTRA_DETAILS_ENTRIES];

    asc_span os_name;
    asc_span os_version;
    asc_span os_architecture;
    asc_span hostname;
    uint32_t memory_total_physical_in_kb;
    uint32_t memory_free_physical_in_kb;
} schema_system_information_t;

OBJECT_POOL_DECLARATIONS(schema_system_information_t, SCHEMA_SYSTEM_INFORMATION_OBJECT_POOL_COUNT);
OBJECT_POOL_DEFINITIONS(schema_system_information_t, SCHEMA_SYSTEM_INFORMATION_OBJECT_POOL_COUNT);


system_information_t* schema_system_information_init() {
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;
    system_information_t* data_ptr = NULL;

    data_ptr = object_pool_get(schema_system_information_t);
    if (data_ptr == NULL) {
        result = IOTSECURITY_RESULT_MEMORY_EXCEPTION;
        log_error("Failed to allocate schema_system_information_t");
        goto cleanup;
    }

    memset(data_ptr, 0, sizeof(schema_system_information_t));

    data_ptr->os_name = ASC_SPAN_NULL;
    data_ptr->os_version = ASC_SPAN_NULL;
    data_ptr->os_architecture = ASC_SPAN_NULL;
    data_ptr->hostname = ASC_SPAN_NULL;

cleanup:
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to initialize system information schema, result=[%d]", result);
    }

    return data_ptr;
}


void schema_system_information_deinit(system_information_t* data_ptr) {
    if (data_ptr != NULL) {
        data_ptr->os_name = ASC_SPAN_NULL;
        data_ptr->os_version = ASC_SPAN_NULL;
        data_ptr->os_architecture = ASC_SPAN_NULL;
        data_ptr->hostname = ASC_SPAN_NULL;

        object_pool_free(schema_system_information_t, data_ptr);
        data_ptr = NULL;
    }
}


asc_pair* schema_system_information_get_extra_details(system_information_t* data_ptr) {
    asc_pair* result = NULL;

    if (data_ptr != NULL) {
        result = data_ptr->extra_details;
    }

    return result;
}


IOTSECURITY_RESULT schema_system_information_set_extra_details(system_information_t* data_ptr, asc_pair* extra_details) {
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;

    if (data_ptr == NULL || extra_details == NULL) {
        log_error("Failed to set system information schema extra details due to bad argument");
        result = IOTSECURITY_RESULT_BAD_ARGUMENT;
        goto cleanup;
    }

    memcpy(data_ptr->extra_details, extra_details, sizeof(asc_pair) * SYSTEM_INFORMATION_SCHEMA_EXTRA_DETAILS_ENTRIES);

cleanup:
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to set system information schema extra details, result=[%d]", result);
    }

    return result;
}


asc_span schema_system_information_get_os_name(system_information_t* data_ptr) {
    asc_span result = ASC_SPAN_NULL;

    if (data_ptr != NULL) {
        result = data_ptr->os_name;
    }

    return result;
}


IOTSECURITY_RESULT schema_system_information_set_os_name(system_information_t* data_ptr, char* os_name) {
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;

    if (data_ptr == NULL || os_name == NULL) {
        log_error("Failed to set system information schema os_name due to bad argument");
        result = IOTSECURITY_RESULT_BAD_ARGUMENT;
        goto cleanup;
    }

    data_ptr->os_name = asc_span_from_str(os_name);

cleanup:
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to set system information schema os_name, result=[%d]", result);
    }

    return result;
}


asc_span schema_system_information_get_os_version(system_information_t* data_ptr) {
    asc_span result = ASC_SPAN_NULL;

    if (data_ptr != NULL) {
        result = data_ptr->os_version;
    }

    return result;
}


IOTSECURITY_RESULT schema_system_information_set_os_version(system_information_t* data_ptr, char* os_version) {
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;

    if (data_ptr == NULL || os_version == NULL) {
        log_error("Failed to set system information schema os_version due to bad argument");
        result = IOTSECURITY_RESULT_BAD_ARGUMENT;
        goto cleanup;
    }

    data_ptr->os_version = asc_span_from_str(os_version);

cleanup:
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to set system information schema os_version, result=[%d]", result);
    }

    return result;
}


asc_span schema_system_information_get_os_architecture(system_information_t* data_ptr) {
    asc_span result = ASC_SPAN_NULL;

    if (data_ptr != NULL) {
        result = data_ptr->os_architecture;
    }

    return result;
}


IOTSECURITY_RESULT schema_system_information_set_os_architecture(system_information_t* data_ptr, char* os_architecture) {
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;

    if (data_ptr == NULL || os_architecture == NULL) {
        log_error("Failed to set system information schema os_architecture due to bad argument");
        result = IOTSECURITY_RESULT_BAD_ARGUMENT;
        goto cleanup;
    }

    data_ptr->os_architecture = asc_span_from_str(os_architecture);

cleanup:
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to set system information schema os_architecture, result=[%d]", result);
    }

    return result;
}


asc_span schema_system_information_get_hostname(system_information_t* data_ptr) {
    asc_span result = ASC_SPAN_NULL;

    if (data_ptr != NULL) {
        result = data_ptr->hostname;
    }

    return result;
}


IOTSECURITY_RESULT schema_system_information_set_hostname(system_information_t* data_ptr, char* hostname) {
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;

    if (data_ptr == NULL || hostname == NULL) {
        log_error("Failed to set system information schema hostname due to bad argument");
        result = IOTSECURITY_RESULT_BAD_ARGUMENT;
        goto cleanup;
    }

    data_ptr->hostname = asc_span_from_str(hostname);

cleanup:
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to set system information schema hostname, result=[%d]", result);
    }

    return result;
}


uint32_t schema_system_information_get_memory_total_physical_in_kb(system_information_t* data_ptr) {
    uint32_t result = 0;

    if (data_ptr != NULL) {
        result = data_ptr->memory_total_physical_in_kb;
    }

    return result;
}


IOTSECURITY_RESULT schema_system_information_set_memory_total_physical_in_kb(system_information_t* data_ptr, uint32_t memory_total_physical_in_kb) {
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;

    if (data_ptr == NULL) {
        log_error("Failed to set listening ports schema memory_total_physical_in_kb due to bad argument");
        result = IOTSECURITY_RESULT_BAD_ARGUMENT;
        goto cleanup;
    }

    data_ptr->memory_total_physical_in_kb = memory_total_physical_in_kb;

cleanup:
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to set listening ports schema memory_total_physical_in_kb, result=[%d]", result);
    }

    return result;
}


uint32_t schema_system_information_get_memory_free_physical_in_kb(system_information_t* data_ptr) {
    uint32_t result = 0;

    if (data_ptr != NULL) {
        result = data_ptr->memory_free_physical_in_kb;
    }

    return result;
}


IOTSECURITY_RESULT schema_system_information_set_memory_free_physical_in_kb(system_information_t* data_ptr, uint32_t memory_free_physical_in_kb) {
    IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;

    if (data_ptr == NULL) {
        log_error("Failed to set listening ports schema memory_free_physical_in_kb due to bad argument");
        result = IOTSECURITY_RESULT_BAD_ARGUMENT;
        goto cleanup;
    }

    data_ptr->memory_free_physical_in_kb = memory_free_physical_in_kb;

cleanup:
    if (result != IOTSECURITY_RESULT_OK) {
        log_error("Failed to set listening ports schema memory_free_physical_in_kb, result=[%d]", result);
    }

    return result;
}
