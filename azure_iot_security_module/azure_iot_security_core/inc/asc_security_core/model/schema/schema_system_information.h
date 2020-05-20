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

#ifndef SCHEMA_SYSTEM_INFORMATION_H
#define SCHEMA_SYSTEM_INFORMATION_H


#include <stdint.h>
#include "asc_security_core/asc/asc_span.h"
#include "asc_security_core/configuration.h"
#include "asc_security_core/iotsecurity_result.h"
#include "asc_security_core/model/collector_enums.h"

#define SYSTEM_INFORMATION_SCHEMA_EXTRA_DETAILS_ENTRIES (COLLECTOR_TYPE_COUNT + 1)

typedef struct schema_system_information system_information_t;

/**
 * @brief Initialize SystemInformation Schema
 *
 * @return system_information_t*
 */
system_information_t* schema_system_information_init();

/**
 * @brief Deinitialize SystemInformation Schema
 *
 * @param data_ptr    system_information_t*
 *
 * @return NULL
 */
void schema_system_information_deinit(system_information_t* data_ptr);


/**
 * @brief Getter schema extra details
 *
 * @param data_ptr   system information data handle
 *
 * @return schema extra_details
 */
asc_pair* schema_system_information_get_extra_details(system_information_t* data_ptr);


/**
 * @brief Setter schema extra details
 *
 * @param data_ptr       system_information_t*
 * @param extra_details     extra details
 *
 * @return IOTSECRUITY_RESULT
 */
IOTSECURITY_RESULT schema_system_information_set_extra_details(system_information_t* data_ptr, asc_pair* extra_details);


/**
 * @brief Getter schema os_name
 *
 * @param data_ptr   system information data handle
 *
 * @return schema os_name
 */
asc_span schema_system_information_get_os_name(system_information_t* data_ptr);


/**
 * @brief Setter schema os_name
 *
 * @param data_ptr   system information data handle
 * @param os_name       os_name
 *
 * @return IOTSECRUITY_RESULT
 */
IOTSECURITY_RESULT schema_system_information_set_os_name(system_information_t* data_ptr, char* os_name);


/**
 * @brief Getter schema os_version
 *
 * @param data_ptr   system information data handle
 *
 * @return schema os_version
 */
asc_span schema_system_information_get_os_version(system_information_t* data_ptr);


/**
 * @brief Setter schema os_version
 *
 * @param data_ptr   system information data handle
 * @param os_version    os_version
 *
 * @return IOTSECRUITY_RESULT
 */
IOTSECURITY_RESULT schema_system_information_set_os_version(system_information_t* data_ptr, char* os_version);


/**
 * @brief Getter schema os_architecture
 *
 * @param data_ptr   system information data handle
 *
 * @return schema os_architecture
 */
asc_span schema_system_information_get_os_architecture(system_information_t* data_ptr);


/**
 * @brief Setter schema os_architecture
 *
 * @param data_ptr       system_information_t*
 * @param os_architecture   os_architecture
 *
 * @return IOTSECRUITY_RESULT
 */
IOTSECURITY_RESULT schema_system_information_set_os_architecture(system_information_t* data_ptr, char* os_architecture);


/**
 * @brief Getter schema hostname
 *
 * @param data_ptr   system information data handle
 *
 * @return schema hostname
 */
asc_span schema_system_information_get_hostname(system_information_t* data_ptr);


/**
 * @brief Setter schema hostname
 *
 * @param data_ptr   system information data handle
 * @param hostname      hostname
 *
 * @return IOTSECRUITY_RESULT
 */
IOTSECURITY_RESULT schema_system_information_set_hostname(system_information_t* data_ptr, char* hostname);


/**
 * @brief Getter schema memory_total_physical_in_kb
 *
 * @param data_ptr   system information data handle
 *
 * @return schema memory_total_physical_in_kb
 */
uint32_t schema_system_information_get_memory_total_physical_in_kb(system_information_t* data_ptr);


/**
 * @brief Setter schema memory_total_physical_in_kb
 *
 * @param data_ptr                   system information data handle
 * @param memory_total_physical_in_kb   memory total physical in kb
 *
 * @return IOTSECRUITY_RESULT
 */
IOTSECURITY_RESULT schema_system_information_set_memory_total_physical_in_kb(system_information_t* data_ptr, uint32_t memory_total_physical_in_kb);


/**
 * @brief Getter schema memory_free_physical_in_kb
 *
 * @param data_ptr   system information data handle
 *
 * @return schema memory_free_physical_in_kb
 */
uint32_t schema_system_information_get_memory_free_physical_in_kb(system_information_t* data_ptr);


/**
 * @brief Setter schema memory_free_physical_in_kb
 *
 * @param data_ptr                   system information data handle
 * @param memory_free_physical_in_kb    memory free physical in kb
 *
 * @return IOTSECRUITY_RESULT
 */
IOTSECURITY_RESULT schema_system_information_set_memory_free_physical_in_kb(system_information_t* data_ptr, uint32_t memory_free_physical_in_kb);


#endif /* SCHEMA_SYSTEM_INFORMATION_H */
