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

#ifndef _HEARTBEAT_INFORMATION_H
#define _HEARTBEAT_INFORMATION_H

#include "asc_security_core/model/collector.h"

/**
 * @brief Initialize Heartbeat Collector
 *
 * @param collector_internal_ptr   A handle to the collector internal to initialize.
 *
 * @return IOTSECURITY_RESULT_OK on success
 */
IOTSECURITY_RESULT collector_heartbeat_init(collector_internal_t* collector_internal_ptr);

#endif /* _HEARTBEAT_INFORMATION_H */
