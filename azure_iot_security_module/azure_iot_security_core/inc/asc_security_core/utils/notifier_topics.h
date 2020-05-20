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

#ifndef __NOTIFIER_GROUPS_H__
#define __NOTIFIER_GROUPS_H__

typedef enum {
    NOTIFY_TOPIC_SYSTEM,
    NOTIFY_TOPICS_NUMBER
} notify_topic_t;

enum {
    NOTIFY_MESSAGE_SYSTEM_CONFIGURATION = 0
};

#endif