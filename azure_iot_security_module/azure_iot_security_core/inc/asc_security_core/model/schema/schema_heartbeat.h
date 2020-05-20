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

#ifndef SCHEMA_HEARTBEAT_H
#define SCHEMA_HEARTBEAT_H

#include <stdint.h>
#include "asc_security_core/asc/asc_span.h"
#include "asc_security_core/configuration.h"
#include "asc_security_core/iotsecurity_result.h"


typedef struct schema_heartbeat schema_heartbeat_t;

/**
 * @brief Initialize Heartbeat Schema
 *
 * @return schema_heartbeat_t*
 */
schema_heartbeat_t* schema_heartbeat_init();

/**
 * @brief Deinitialize Heartbeat Schema
 *
 * @param data_ptr    schema_heartbeat_t*
 *
 * @return NULL
 */
void schema_heartbeat_deinit(schema_heartbeat_t* data_ptr);

#endif /* SCHEMA_HEARTBEAT_H */
