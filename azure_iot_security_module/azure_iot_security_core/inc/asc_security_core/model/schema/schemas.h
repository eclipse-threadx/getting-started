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

#ifndef SCHEMAS_H
#define SCHEMAS_H


#ifdef COLLECTOR_SYSTEM_INFORMATION_ENABLED
#include "asc_security_core/model/schema/schema_system_information.h"
#endif

#ifdef COLLECTOR_LISTENING_PORTS_ENABLED
#include "asc_security_core/model/schema/schema_listening_ports.h"
#endif

#ifdef COLLECTOR_CONNECTION_CREATE_ENABLED
#include "asc_security_core/model/schema/schema_connection_create.h"
#endif

#ifdef COLLECTOR_HEARTBEAT_ENABLED
#include "asc_security_core/model/schema/schema_heartbeat.h"
#endif

#endif /* SCHEMAS_H */

