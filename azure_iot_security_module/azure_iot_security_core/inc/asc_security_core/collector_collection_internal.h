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

#ifndef IOTSECURITY_COLLECTOR_COLLECTION_INTERNAL_H
#define IOTSECURITY_COLLECTOR_COLLECTION_INTERNAL_H

#include <stdint.h>
#include "asc_security_core/collector_collection.h"
#include "asc_security_core/iotsecurity_result.h"

/**
 * @brief Initialize startup time for each collector in the collection
 *
 * @param collector_collection      collector collection
 *
 * @return An @c IOTSECURITY_RESULT indicating the status of the call.
 */
IOTSECURITY_RESULT collector_collection_internal_init_startup_time(collector_collection_t* collector_collection_ptr);


#endif /* IOTSECURITY_COLLECTOR_COLLECTION_INTERNAL_H */