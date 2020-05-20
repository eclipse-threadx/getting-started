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

#ifndef SECURITY_MODULE_MQTT_H
#define SECURITY_MODULE_MQTT_H

#include "nx_api.h"
#include "nx_cloud.h"
#include "nxd_mqtt_client.h"

#include "asc_security_core/core.h"
#include "asc_security_azurertos/security_module.h"


typedef struct AZRTOS_IOT_SECURITY_MODULE_TAG {
    core_t* core_ptr;
    linked_list_security_message_t ll_security_message;

    NXD_MQTT_CLIENT* mqtt_client_secure;
    char* mqtt_topic_name;
    NX_CLOUD_MODULE azrtos_iot_cloud_asc_module;
} AZRTOS_IOT_SECURITY_MODULE;


UINT security_module_mqtt_init(AZRTOS_IOT_SECURITY_MODULE* security_module, NXD_MQTT_CLIENT* mqtt_client_secure, char* mqtt_topic_name, unix_time_callback_t unix_time_callback);
UINT security_module_mqtt_deinit(AZRTOS_IOT_SECURITY_MODULE* security_module);


#endif /* SECURITY_MODULE_MQTT_H */
