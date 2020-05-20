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
#include "asc_security_core/model/message.h"
#include "asc_security_core/utils/irand.h"
#include "asc_security_core/utils/itime.h"

#include "asc_security_azurertos/security_module_mqtt.h"

static VOID _azrtos_iot_security_module_event_process(VOID* security_module_ptr, ULONG common_events, ULONG module_own_events);


UINT security_module_mqtt_init(AZRTOS_IOT_SECURITY_MODULE* security_module, NXD_MQTT_CLIENT* mqtt_client_secure, char* mqtt_topic_name, unix_time_callback_t unix_time_callback) {
    UINT status;

    if (security_module == NULL || mqtt_client_secure == NULL || mqtt_client_secure->nxd_mqtt_client_cloud_ptr == NX_NULL) {
        log_error("INVALID POINTER");
        return(NX_NOT_SUCCESSFUL);
    }

    if (mqtt_client_secure->nxd_mqtt_client_state != NXD_MQTT_CLIENT_STATE_CONNECTED) {
        log_error("MQTT client is not connected");
        return(NX_NOT_SUCCESSFUL);
    }

    memset(security_module, 0, sizeof(AZRTOS_IOT_SECURITY_MODULE));

    itime_init(unix_time_callback);

    security_module->core_ptr = core_init();
    if (security_module->core_ptr == NULL) {
        log_error("Failed to initialize core");
        return(NX_NOT_SUCCESSFUL);
    }

    linked_list_security_message_t_init(&security_module->ll_security_message, object_pool_security_message_t_free);

    security_module->mqtt_client_secure = mqtt_client_secure;
    security_module->mqtt_topic_name = mqtt_topic_name;

    /* Register Security module module on cloud helper.  */
    status = nx_cloud_module_register(mqtt_client_secure->nxd_mqtt_client_cloud_ptr, &(security_module->azrtos_iot_cloud_asc_module),
                                      "ASC", NX_CLOUD_MODULE_AZURE_ASC_EVENT | NX_CLOUD_COMMON_PERIODIC_EVENT,
                                      _azrtos_iot_security_module_event_process, security_module);
    if (status) {
        log_error("Security module register fail, error=[s0x%02x]", status);
        return(NX_NOT_SUCCESSFUL);
    }

    return (NX_SUCCESS);
}


UINT security_module_mqtt_deinit(AZRTOS_IOT_SECURITY_MODULE* security_module) {
    UINT status;

    if (security_module == NULL) {
        log_error("INVALID POINTER");
        return(NX_NOT_SUCCESSFUL);
    }

    /* Deregister Security Module on cloud helper.  */
    status = nx_cloud_module_deregister(security_module->mqtt_client_secure->nxd_mqtt_client_cloud_ptr, &(security_module->azrtos_iot_cloud_asc_module));

    return(status);
}

static VOID _azrtos_iot_security_module_event_process(VOID* security_module_ptr, ULONG common_events, ULONG module_own_events) {
    UINT status;
    AZRTOS_IOT_SECURITY_MODULE* security_module = (AZRTOS_IOT_SECURITY_MODULE*)security_module_ptr;
    NXD_MQTT_CLIENT* mqtt_client_secure = security_module->mqtt_client_secure;

    /* Process common events.  */
    NX_PARAMETER_NOT_USED(common_events);


    if (common_events & NX_CLOUD_COMMON_PERIODIC_EVENT) {
        IOTSECURITY_RESULT result = IOTSECURITY_RESULT_OK;

        result = core_collect(security_module->core_ptr);
        if (result != IOTSECURITY_RESULT_OK) {
            log_error("core collect failed, result=[%d]", result);
        }

        result = core_get(security_module->core_ptr,  &security_module->ll_security_message);
        if (result == IOTSECURITY_RESULT_EMPTY) {
            // no security messages
        } else if (result == IOTSECURITY_RESULT_OK) {
            // send security messages
            UINT counter = linked_list_security_message_t_get_size(&security_module->ll_security_message);
            security_message_t* security_message_ptr = NULL;

            while (counter--) {
                security_message_ptr = linked_list_security_message_t_remove_first(&security_module->ll_security_message);
                char* message = security_message_ptr->data;

                /* Publish security message.  */
                status = nxd_mqtt_client_publish(
                    mqtt_client_secure, security_module->mqtt_topic_name, strlen(security_module->mqtt_topic_name),
                    (CHAR*)message, strlen((CHAR const*)message),
                    0, 1, NX_WAIT_FOREVER
                );

                if (status) {
                    log_error("MQTT publish security message failed, error=[0x%02x]", status);
                } else {
                    log_info("[Security Message] %s", (CHAR *)message);
                }

                object_pool_security_message_t_free(security_message_ptr);
                security_message_ptr = NULL;
            }
        } else {
            log_error("core get operation failed, result=[%d]", result);
        }
    }
}
