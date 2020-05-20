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

#ifndef SECURITY_MODULE_H
#define SECURITY_MODULE_H

/* Required. */
#ifndef SECURITY_MODULE_VERSION
// #error "Azure Security Module required version."
#define SECURITY_MODULE_VERSIONN     "0.0.0-azrtos"
#else
#define SECURITY_MODULE_VERSIONN     SECURITY_MODULE_VERSION"-azrtos"
#endif /* SECURITY_MODULE_VERSION */
// TODO solve redefind version

#ifndef NX_ENABLE_IP_PACKET_FILTER
#error "Azure Security Module is not supported if NX_ENABLE_IP_PACKET_FILTER is not defined."
#endif /* NX_ENABLE_IP_PACKET_FILTER */

/* Define AZ IoT ASC event flags. These events are processed by the Cloud thread.  */
#define AZ_IOT_SECURITY_MODULE_SEND_EVENT               ((ULONG)0x00000001)       /* Security send event      */


#endif /* SECURITY_MODULE_H */