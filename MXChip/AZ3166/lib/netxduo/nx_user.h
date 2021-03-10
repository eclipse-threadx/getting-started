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

#ifndef NX_USER_H
#define NX_USER_H

#define NX_SECURE_ENABLE
#define NX_ENABLE_EXTENDED_NOTIFY_SUPPORT
#define NX_ENABLE_IP_PACKET_FILTER
#define NX_DISABLE_IPV6
#define NX_DNS_CLIENT_USER_CREATE_PACKET_POOL

#define NXD_MQTT_CLOUD_ENABLE

#define NX_SNTP_CLIENT_MIN_SERVER_STRATUM 3

extern UINT nx_rand16( void );
#define NX_RAND                         nx_rand16

#define NX_ASSERT_FAIL for(;;){}

/* Symbols for Wiced.  */

/* This define specifies the size of the physical packet header. The default value is 16 (based on
   a typical 16-byte Ethernet header).  */
#define NX_PHYSICAL_HEADER              (14 + 12 + 18)

/* This define specifies the size of the physical packet trailer and is typically used to reserve storage
   for things like Ethernet CRCs, etc.  */
#define NX_PHYSICAL_TRAILER             (0)

#define NX_LINK_PTP_SEND                51      /* Precision Time Protocol */

#endif /* NX_USER_H */
