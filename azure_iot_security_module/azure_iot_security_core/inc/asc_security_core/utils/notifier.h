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

#ifndef __NOTIFIER_H__
#define __NOTIFIER_H__

#include "asc_security_core/iotsecurity_result.h"
#include "asc_security_core/configuration.h"
#include "asc_security_core/utils/notifier_topics.h"

#ifndef NOTIFIERS_POOL_ENTRIES
#define NOTIFIERS_POOL_ENTRIES 1
#endif

typedef struct notifier_t notifier_t;

struct notifier_t {
    void (*notify)(notifier_t *notifier, int msg_num, void *payload);
};

/** @brief Notify subscribers of a specific topic that an event has occurred.
 *         msg_num, payload and payload_len are topic specific.
 *
 *  @param topic Topic enumerator
 *  @param msg_num Message number that notifier callback will receive
 *  @param payload Pointer to data that notifier callback will receive
 *  @return Number of active subscribers
 */
int32_t notifier_notify(notify_topic_t topic, int msg_num, void *payload);

/** @brief Subscribe a notifier for a topic.
 *         Topics should be defined in notifier.h
 *  @param topic topic Topic enumerator
 *  @param notifier Topic notifier with callback filled in
 *  @return IOTSECURITY_RESULT_OK on success, IOTSECURITY_RESULT_EXCEPTION otherwise.
 */
IOTSECURITY_RESULT notifier_subscribe(notify_topic_t topic, notifier_t *notifier);

/** @brief Unsubscribe a notifier for a topic
 *
 *  @param topic Topic enumerator
 *  @param notifier Topic notifier with callback filled in
 *  @return IOTSECURITY_RESULT_OK on success, IOTSECURITY_RESULT_EXCEPTION otherwise.
 */
IOTSECURITY_RESULT notifier_unsubscribe(notify_topic_t topic, notifier_t *notifier);

/** @brief Unsubscribe all notifiers for a topic
 *
 *  @param topic Topic enumerator
 *  @return IOTSECURITY_RESULT_OK on success, IOTSECURITY_RESULT_EXCEPTION otherwise.
 */IOTSECURITY_RESULT notifier_deinit(notify_topic_t topic);
#endif