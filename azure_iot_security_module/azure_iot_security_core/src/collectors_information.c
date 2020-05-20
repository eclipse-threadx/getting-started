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

#include <stdio.h>

#include "asc_security_core/asc/asc_span.h"

#include "asc_security_core/logger.h"
#include "asc_security_core/object_pool.h"
#include "asc_security_core/collectors/collectors_information.h"
#include "asc_security_core/model/collector_enums.h"
#include "asc_security_core/model/collector.h"
#include "asc_security_core/utils/containerof.h"
#include "asc_security_core/utils/notifier.h"


/* String representation of uint_32 */
#define COLLECTOR_INFO_LENGTH 11

typedef struct collector_info_t {
    char data[COLLECTOR_INFO_LENGTH];
} collector_info_t;

typedef struct notifier_container_t {
    COLLECTION_INTERFACE(struct notifier_container_t);

    notifier_t notifier;
    collector_info_t info[COLLECTOR_TYPE_COUNT];
} notifier_container_t;

OBJECT_POOL_DECLARATIONS(notifier_container_t, 1);
OBJECT_POOL_DEFINITIONS(notifier_container_t, 1);

static void _collector_info_cb(notifier_t *notifier, int message_num, void *payload) {
    notifier_container_t *container = containerof(notifier, notifier_container_t, notifier);
    collector_info_t *info = container->info;
    collector_internal_t* collector_internal_ptr = payload;

    if (collector_internal_ptr == NULL) {
        log_error("Wrong (NULL) data was recieved");
        return;
    }

    if (collector_internal_ptr->type < 0 ||
        collector_internal_ptr->type >= COLLECTOR_TYPE_COUNT ||
        collector_internal_ptr->priority < 0 ||
        collector_internal_ptr->priority >= COLLECTOR_PRIORITY_COUNT) {
        log_error("Wrong collector type=[%d] or priority=[%d]", collector_internal_ptr->type, collector_internal_ptr->priority);
    } else {
        // Here need to be aligned with COLLECTOR_INFO_LENGTH
        sprintf(info[collector_internal_ptr->type].data, "%d", g_collector_collections_intervals[collector_internal_ptr->priority]);
        log_debug("Updated configuration for collector=[%s] with data=[%s]\n",
            g_collector_names[collector_internal_ptr->type],
            info[collector_internal_ptr->type].data);
    }
}

collectors_info_handle collectors_info_init() {
    notifier_container_t *container = object_pool_get(notifier_container_t);

    if (container == NULL) {
        log_error("Failed to allocate notifier container object");
        return 0;
    }
    memset(container, 0, sizeof(notifier_container_t));
    container->notifier.notify = _collector_info_cb;
    notifier_subscribe(NOTIFY_TOPIC_SYSTEM, &container->notifier);
    return (intptr_t)container;
}

void collectors_info_deinit(collectors_info_handle collectors_info) {
    notifier_container_t *container = (notifier_container_t *)collectors_info;

    if (container == NULL) {
        log_error("collectors_info_handle is NULL");
        return;
    }
    notifier_unsubscribe(NOTIFY_TOPIC_SYSTEM, &container->notifier);
    object_pool_free(notifier_container_t, container);
}

void collectors_info_append(collectors_info_handle collectors_info, asc_pair* pairs, int max_pairs) {
    notifier_container_t *container = (notifier_container_t *)collectors_info;
    collector_info_t *info;
    int type, pairs_index;

    if (container == NULL) {
        log_error("collectors_info_handle is NULL");
        return;
    }
    info = container->info;

    for (pairs_index = 0; pairs_index < max_pairs; pairs_index++) {
        asc_pair entry = pairs[pairs_index];

        if (asc_span_length(entry.key) == 0 || asc_span_length(entry.value) == 0) {
            break;
        }
    }
    for (type = 0; (pairs_index < max_pairs && type < COLLECTOR_TYPE_COUNT); type++ ) {
        if (info[type].data[0] != '\0') {
            pairs[pairs_index++] = asc_pair_from_str((char *)g_collector_names[type], info[type].data);
        }
    }

    if (type < COLLECTOR_TYPE_COUNT) {
        log_error("No room to insert=[%d] collectors information to extra details", COLLECTOR_TYPE_COUNT - type);
    }
}

