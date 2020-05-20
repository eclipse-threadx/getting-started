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

#include "asc_security_azurertos/security_module_iothub.h"
#include "asc_security_core/model/schema/schema_connection_create.h"
#include "asc_security_core/model/schema/schema_connection_create.h"
#include "asc_security_core/message_schema_consts.h"


/* Define the prototypes for Security Module.  */
AZRTOS_IOT_SECURITY_MODULE* _security_module_created_ptr;

static VOID azrtos_iot_security_module_event_process(VOID *security_module, ULONG common_events, ULONG module_own_events);

UINT security_module_iothub_init(AZRTOS_IOT_SECURITY_MODULE* security_module, AZRTOS_IOT_HUB_CLIENT* azrtos_iot_hub_client) {
    UINT status;

    if (security_module == NULL || azrtos_iot_hub_client == NULL || hub_client_ptr->azrtos_iot_ptr == NX_NULL) {
        log_error("INVALID POINTER");
        return(NX_NOT_SUCCESSFUL);
    }

    if (azrtos_iot_hub_client->azrtos_iot_hub_client_state != AZRTOS_IOT_HUB_CLIENT_STATUS_CONNECTED) {
        log_error("IoT Hub is not connected");
        return(NX_NOT_SUCCESSFUL);
    }

    memset(security_module, 0, sizeof(AZRTOS_IOT_SECURITY_MODULE));

    security_module->azrtos_iot_hub_client = azrtos_iot_hub_client;

    /* Register Security module module on cloud helper.  */
    status = nx_cloud_module_register(&(hub_client_ptr->azrtos_iot_ptr->azrtos_iot_cloud), &(security_module->azrtos_iot_security_module),
                                      "Azure Security Module", NX_CLOUD_MODULE_AZURE_ASC_EVENT,
                                      azrtos_iot_security_module_event_process, security_module);
    if (status) {
        log_error("Security module register fail s0x%02x", status);
        return(NX_NOT_SUCCESSFUL);
    }

    /* Record the created security client.  */
    _security_module_created_ptr  = security_module;

    return (NX_SUCCESS);
}


UINT security_module_iothub_deinit(AZRTOS_IOT_SECURITY_MODULE* security_module) {
    UINT status;

    if (security_module == NULL) {
        log_error("INVALID POINTER");
        return(NX_NOT_SUCCESSFUL);
    }

    /* Deregister Security Module on cloud helper.  */
    status = nx_cloud_module_deregister(&(security_module->azrtos_iot_hub_client->azrtos_iot_ptr->azrtos_iot_cloud), &(security_module->azrtos_iot_security_module));

    return(status);
}

CHAR    telemetry_data[1024];
UINT    telemetry_data_size;
char    local_ip[40] = { 0 };
char    remote_ip[40] = { 0 };
char    local_port[8];
char    remote_port[8];
NX_PACKET *packet_ptr;

static VOID azrtos_iot_security_module_event_process(VOID *security_module, ULONG common_events, ULONG module_own_events) {
    AZRTOS_IOT_SECURITY_MODULE* azrtos_iot_ptr = (AZRTOS_IOT_SECURITY_MODULE *)security_module;
    AZRTOS_IOT_HUB_CLIENT* hub_client_ptr = azrtos_iot_ptr->azrtos_iot_hub_client;
    connection_create_t* current_schema_handle = azrtos_iot_ptr->schema_list_header;
    connection_create_t* next_schema_handle;


    /* Process module own events.  */
    NX_PARAMETER_NOT_USED(common_events);

    /* Process module own events.  */
    if (module_own_events & AZRTOS_IOT_HUB_CLIENT_CONNECT_EVENT)
    {
        while(current_schema_handle)
        {

            /* Get the next schema handle.  */
            next_schema_handle = current_schema_handle->schema_next;

            /* Output log.  */
            ConnectionCreateSchema_LogInfo(current_schema_handle);

            /* Create a telemetry message packet. */
            if (azrtos_iot_hub_client_telemetry_message_create(hub_client_ptr, &packet_ptr, NX_NO_WAIT))
            {
                break;
            }

            /* FIXME: Create schema info as json into buffer, this is just a sample for sending partial data.  */
            ConnectionCreateSchema_SerializeLocalIP(current_schema_handle, local_ip);
            ConnectionCreateSchema_SerializeRemoteIP(current_schema_handle, remote_ip);
            snprintf(local_port, sizeof(local_port), "%u", current_schema_handle->local_port);
            snprintf(remote_port, sizeof(remote_port), "%u", current_schema_handle->remote_port);
            const char* protocol_value = TransportProtocol_ToString(current_schema_handle->transport_protocol);
            const char* direction_value = ConnectionDirection_ToString(current_schema_handle->direction);
            telemetry_data_size = snprintf(telemetry_data, sizeof(telemetry_data), "{\"%s\":\"%s\", \"%s\":\"%s\", \"%s\":\"%s\", \"%s\":\"%s\", \"%s\":\"%s\", \"%s\":\"%s\", \"%s\":\"%s\", \"%s\":\"%s\"}",
                                           CONNECTION_CREATE_LOCAL_ADDRESS_KEY, local_ip,
                                           CONNECTION_CREATE_REMOTE_ADDRESS_KEY, remote_ip,
                                           CONNECTION_CREATE_LOCAL_PORT_KEY, local_port,
                                           CONNECTION_CREATE_REMOTE_PORT_KEY, remote_port,
                                           CONNECTION_CREATE_PROTOCOL_KEY, protocol_value,
                                           CONNECTION_CREATE_DIRECTION_KEY, direction_value,
                                           CONNECTION_CREATE_PROCESS_ID_KEY, "0",
                                           CONNECTION_CREATE_EXECUTABLE_KEY, "0");

            /* Send it out.   */
            if (azrtos_iot_hub_client_telemetry_send(hub_client_ptr, packet_ptr, (UCHAR *)telemetry_data, telemetry_data_size, NX_NO_WAIT))
            {
                azrtos_iot_hub_client_telemetry_message_delete(packet_ptr);
            }

            /* Release current schema handle.  */
            schema_connection_create_deinit(azrtos_iot_ptr, current_schema_handle);

            /* Remove to next schema handle.  */
            current_schema_handle = next_schema_handle;
        }
    }
}