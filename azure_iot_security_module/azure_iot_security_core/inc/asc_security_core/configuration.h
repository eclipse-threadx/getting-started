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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

// Security Module ID - A unique identifier of the device
#ifndef ASC_SECURITY_MODULE_ID
#define ASC_SECURITY_MODULE_ID "asc_security_module"
#endif

// Collection interval for high priority events, in seconds
#ifndef ASC_HIGH_PRIORITY_INTERVAL
#define ASC_HIGH_PRIORITY_INTERVAL 10
#endif

// Collection interval for medium priority events, in seconds
#ifndef ASC_MEDIUM_PRIORITY_INTERVAL
#define ASC_MEDIUM_PRIORITY_INTERVAL 30
#endif

// Collection interval for low priority events, in seconds
#ifndef ASC_LOW_PRIORITY_INTERVAL
#define ASC_LOW_PRIORITY_INTERVAL 60
#endif

// The maximum number of security messages to store in memory
#ifndef ASC_CORE_MAX_MESSAGES_IN_MEMORY
#define ASC_CORE_MAX_MESSAGES_IN_MEMORY 3
#endif

// The maximum number of security events to store in memory
#ifndef ASC_CORE_MAX_EVENTS_IN_MEMORY
#define ASC_CORE_MAX_EVENTS_IN_MEMORY 200
#endif

// The maximum size of a security message (IoT Hub message max size is 262144)
#ifndef ASC_MESSAGE_MAX_SIZE
#define ASC_MESSAGE_MAX_SIZE 262144
#endif

// The maximum size of a security event
#ifndef ASC_EVENT_MAX_SIZE
#define ASC_EVENT_MAX_SIZE 2048
#endif

// The maximum number of network events to store in memory
#ifndef ASC_COLLECTOR_CONNECTION_CREATE_MAX_OBJECTS_IN_CACHE
#define ASC_COLLECTOR_CONNECTION_CREATE_MAX_OBJECTS_IN_CACHE 128
#endif

// The maximum number of listening ports events to store in memory
#ifndef ASC_COLLECTOR_LISTENING_PORTS_MAX_OBJECTS_IN_CACHE
#define ASC_COLLECTOR_LISTENING_PORTS_MAX_OBJECTS_IN_CACHE 1
#endif

// The maximum number of system information events to store in memory
#ifndef ASC_COLLECTOR_SYSTEM_INFORMATION_MAX_OBJECTS_IN_CACHE
#define ASC_COLLECTOR_SYSTEM_INFORMATION_MAX_OBJECTS_IN_CACHE 1
#endif

// The maximum number of heartbeat events to store in memory
#ifndef ASC_COLLECTOR_HEARTBEAT_MAX_OBJECTS_IN_CACHE
#define ASC_COLLECTOR_HEARTBEAT_MAX_OBJECTS_IN_CACHE 1
#endif

#endif /* CONFIGURATION_H */
