/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

// https://docs.microsoft.com/en-us/azure/iot-dps/iot-dps-mqtt-support

#include "azure_iot_dps_mqtt.h"

#include <string.h>

#define MQTT_PRIORITY 2

static VOID mqtt_notify_cb(NXD_MQTT_CLIENT* client_ptr, UINT number_of_messages)
{
    UINT status;
/*    CHAR mqtt_topic_buffer[MQTT_TOPIC_NAME_LENGTH] = {0};
    UINT mqtt_topic_length;
    CHAR mqtt_message_buffer[MQTT_MESSAGE_NAME_LENGTH] = {0};
    UINT mqtt_message_length;

    AZURE_IOT_MQTT* azure_iot_mqtt = (AZURE_IOT_MQTT*)client_ptr->nxd_mqtt_packet_receive_context;

    tx_mutex_get(&azure_iot_mqtt->mqtt_mutex, TX_WAIT_FOREVER);

    // Get the mqtt client message
    status = nxd_mqtt_client_message_get(client_ptr,
        (UCHAR*)mqtt_topic_buffer,
        sizeof(mqtt_topic_buffer),
        &mqtt_topic_length,
        (UCHAR*)mqtt_message_buffer,
        sizeof(mqtt_message_buffer),
        &mqtt_message_length);
    if (status == NXD_MQTT_SUCCESS)
    {
        // Append null string terminators
        mqtt_topic_buffer[mqtt_topic_length]     = 0;
        mqtt_message_buffer[mqtt_message_length] = 0;

        printf("[MQTT Received] topic = %s\r\n", mqtt_topic_buffer);

        if (strstr((CHAR*)mqtt_topic_buffer, DIRECT_METHOD_RECEIVE))
        {
            process_direct_method(azure_iot_mqtt, mqtt_topic_buffer, mqtt_message_buffer);
        }
        else if (strstr((CHAR*)mqtt_topic_buffer, DEVICE_TWIN_RES_BASE))
        {
            process_device_twin_response(azure_iot_mqtt, mqtt_topic_buffer, mqtt_message_buffer);
        }
        else if (strstr((CHAR*)mqtt_topic_buffer, DEVICE_MESSAGE_BASE))
        {
            process_c2d_message(azure_iot_mqtt, mqtt_topic_buffer);
        }
        else if (strstr((CHAR*)mqtt_topic_buffer, DEVICE_TWIN_DESIRED_PROP_RES_BASE))
        {
            process_device_twin_desired_prop_update(azure_iot_mqtt, mqtt_topic_buffer, mqtt_message_buffer);
        }
        else
        {
            printf("Unknown topic, no custom processing specified\r\n");
        }
    }

    tx_mutex_put(&azure_iot_mqtt->mqtt_mutex);*/
}

UINT azure_iot_dps_create(AZURE_IOT_DPS_MQTT* dps_client_ptr,
    UCHAR* mqtt_stack,
    UINT mqtt_stack_size,
    NX_IP* nx_ip,
    NX_PACKET_POOL* nx_pool,
    NX_DNS* nx_dns,
    CHAR* endpoint,
    CHAR* id_scope,
    CHAR* registration_id)
{
    UINT status;

    printf("Initializing MQTT DPS client\r\n");

    memset(dps_client_ptr, 0, sizeof(*dps_client_ptr));

    dps_client_ptr->nx_dns                 = nx_dns;
    dps_client_ptr->dps_endpoint           = endpoint;
    dps_client_ptr->dps_id_scope           = id_scope;
    dps_client_ptr->device_registration_id = registration_id;

    status = nxd_mqtt_client_create(&dps_client_ptr->nxd_mqtt_client,
        "MQTT DPS client",
        registration_id,
        strlen(registration_id),
        nx_ip,
        nx_pool,
        mqtt_stack,
        mqtt_stack_size,
        MQTT_PRIORITY,
        NX_NULL,
        0);
    if (status)
    {
        printf("Failed to create MQTT Client (0x%02x)\r\n", status);
        return status;
    }

    status = nxd_mqtt_client_receive_notify_set(&dps_client_ptr->nxd_mqtt_client, mqtt_notify_cb);
    if (status)
    {
        printf("Error in setting receive notify (0x%02x)\r\n", status);
        nxd_mqtt_client_delete(&dps_client_ptr->nxd_mqtt_client);
        return status;
    }

    status = tx_mutex_create(&dps_client_ptr->mqtt_mutex, "Azure MQTT", TX_NO_INHERIT);
    if (status != TX_SUCCESS)
    {
        printf("Error in creating DPS MQTT mutex (0x%02x)\r\n", status);
        nxd_mqtt_client_delete(&dps_client_ptr->nxd_mqtt_client);
        return status;
    }

    // Set the receive context (highjacking the packet_receive_context) for callbacks
    dps_client_ptr->nxd_mqtt_client.nxd_mqtt_packet_receive_context = dps_client_ptr;

    return NX_SUCCESS;
}

UINT azure_iot_dps_delete(AZURE_IOT_DPS_MQTT* dps_client_ptr)
{
    if (dps_client_ptr == NX_NULL)
    {
        printf("Fail to delete DPS, null pointer\r\n");
        return NX_PTR_ERROR;
    }

    nxd_mqtt_client_disconnect(&dps_client_ptr->nxd_mqtt_client);
    nxd_mqtt_client_delete(&dps_client_ptr->nxd_mqtt_client);
    tx_mutex_delete(&dps_client_ptr->mqtt_mutex);

    return NX_SUCCESS;
}

UINT azure_iot_dps_symmetric_key_set(AZURE_IOT_DPS_MQTT* dps_client_ptr, CHAR* symmetric_key)
{
    if (dps_client_ptr == NX_NULL)
    {
        printf("Fail to delete DPS, null pointer\r\n");
        return NX_PTR_ERROR;
    }

    dps_client_ptr->device_sas_key = symmetric_key;

    return NX_SUCCESS;
}
