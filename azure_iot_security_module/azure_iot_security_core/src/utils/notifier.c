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

#include "asc_security_core/logger.h"
#include "asc_security_core/utils/collection/linked_list.h"
#include "asc_security_core/object_pool.h"
#include "asc_security_core/utils/notifier.h"
typedef struct notifier_item_t {
    COLLECTION_INTERFACE(struct notifier_item_t);
 	notifier_t *notifier;
} notifier_item_t;

typedef struct {
    int msg_num;
	void *payload;
} notify_param_t;

LINKED_LIST_DECLARATIONS(notifier_item_t);
LINKED_LIST_DEFINITIONS(notifier_item_t);
OBJECT_POOL_DECLARATIONS(notifier_item_t, NOTIFIERS_POOL_ENTRIES);
OBJECT_POOL_DEFINITIONS(notifier_item_t, NOTIFIERS_POOL_ENTRIES);

static linked_list_notifier_item_t _notify_arr[NOTIFY_TOPICS_NUMBER] = {0};

static bool _linked_list_find_condition(notifier_item_t *handle, void* condition_input) {
	notifier_t *notifier = condition_input;

    return (notifier == handle->notifier);
}

static void _notify(notifier_item_t *handle, void *ctx) {
	notify_param_t *param = ctx;

	if (handle->notifier->notify) {
		handle->notifier->notify(handle->notifier, param->msg_num, param->payload);
	}
}

int32_t notifier_notify(notify_topic_t topic, int msg_num, void *payload) {
	linked_list_notifier_item_t_handle linked_list_handle;
	notify_param_t param;

	if (topic >= NOTIFY_TOPICS_NUMBER) {
        log_error("Failed to remove notifier due to bad argument");
        goto error;
    }
	linked_list_handle = &_notify_arr[topic];
	param.msg_num = msg_num;
	param.payload = payload;
	linked_list_notifier_item_t_foreach(linked_list_handle, _notify, &param);

	return (int32_t)linked_list_notifier_item_t_get_size(linked_list_handle);

error:
	return -1;
}

IOTSECURITY_RESULT notifier_subscribe(notify_topic_t topic, notifier_t *notifier) {
	IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;
	notifier_item_t *add;
    linked_list_notifier_item_t_handle linked_list_handle;

	if (topic >= NOTIFY_TOPICS_NUMBER) {
        log_error("Failed to initialize notifier due to bad argument");
        result = IOTSECURITY_RESULT_BAD_ARGUMENT;
		goto cleanup;
    }

	linked_list_handle = &_notify_arr[topic];
	if (linked_list_handle->initialized == false) {
		linked_list_notifier_item_t_init(linked_list_handle, object_pool_notifier_item_t_free);
	}

	add = object_pool_notifier_item_t_get();
    if (add == NULL) {
        log_error("Failed to allocate notifier");
		result = IOTSECURITY_RESULT_MEMORY_EXCEPTION;
		goto cleanup;
    }
	add->notifier = notifier;
	linked_list_notifier_item_t_add_first(linked_list_handle, add);

cleanup:
	return result;
}

IOTSECURITY_RESULT notifier_unsubscribe(notify_topic_t topic, notifier_t *notifier) {
	IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;
    linked_list_notifier_item_t_handle linked_list_handle;
	notifier_item_t *handle;

	if (topic >= NOTIFY_TOPICS_NUMBER) {
        log_error("Failed to remove notifier due to bad argument topic");
        result = IOTSECURITY_RESULT_BAD_ARGUMENT;
		goto cleanup;
    }

	linked_list_handle = &_notify_arr[topic];
	handle = linked_list_notifier_item_t_find(linked_list_handle, _linked_list_find_condition, notifier);
	if (handle == NULL) {
		log_error("Failed to remove notifier due to bad argument notifier");
        result = IOTSECURITY_RESULT_BAD_ARGUMENT;
		goto cleanup;
	}

	linked_list_notifier_item_t_remove(linked_list_handle, handle);

cleanup:
	return result;
}

IOTSECURITY_RESULT notifier_deinit(notify_topic_t topic) {
	IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;
    linked_list_notifier_item_t_handle linked_list_handle;

	if (topic >= NOTIFY_TOPICS_NUMBER) {
        log_error("Failed to remove notifier due to bad argument topic");
        result = IOTSECURITY_RESULT_BAD_ARGUMENT;
		goto cleanup;
    }

	linked_list_handle = &_notify_arr[topic];

	linked_list_notifier_item_t_deinit(linked_list_handle);

cleanup:
	return result;
}