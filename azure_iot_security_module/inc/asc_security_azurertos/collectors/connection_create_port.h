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

#ifndef COLLECTOR_CONNECTION_CREATE_PORT_H
#define COLLECTOR_CONNECTION_CREATE_PORT_H

#include "asc_security_core/iotsecurity_result.h"
#include "asc_security_core/model/schema/schema_connection_create.h"
#include "nx_api.h"
#include "tx_api.h"

IOTSECURITY_RESULT collector_connection_create_port_init();
UINT collector_connection_create_port_ip_callback(VOID* ip_packet, UINT direction);
void collector_connection_create_port_deinit();

#endif /* COLLECTOR_CONNECTION_CREATE_PORT_H */
