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

#ifndef SECURITY_MODULE_IOTHUB_H
#define SECURITY_MODULE_IOTHUB_H

#include "nx_api.h"
#include "nx_cloud.h"
#include "azrtos_iot_hub_client.h"

#include "asc_security_core/security_module.h"


typedef struct AZRTOS_IOT_SECURITY_MODULE_TAG {
    AZRTOS_IOT_HUB_CLIENT* azrtos_iot_hub_client;

    NX_CLOUD_MODULE azrtos_iot_security_module;
} AZRTOS_IOT_SECURITY_MODULE;

UINT security_module_iothub_init(AZRTOS_IOT_SECURITY_MODULE* security_module, AZRTOS_IOT_HUB_CLIENT* azrtos_iot_hub_client);
UINT security_module_iothub_deinit(AZRTOS_IOT_SECURITY_MODULE* security_module);


#endif /* SECURITY_MODULE_IOTHUB_H */
