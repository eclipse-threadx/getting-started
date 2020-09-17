/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "azure_iot_nx_client.h"

#include <stdio.h>

#include "azure_iot_cert.h"
#include "azure_iot_ciphersuites.h"
#include "nx_azure_iot_pnp_helpers.h"

#define NX_AZURE_IOT_THREAD_PRIORITY 4
#define THREAD_PRIORITY              16

// Incoming events from the middleware
#define ALL_EVENTS                         0xFF
#define DIRECT_METHOD_EVENT                0x01
#define DEVICE_TWIN_GET_EVENT              0x02
#define DEVICE_TWIN_DESIRED_PROPERTY_EVENT 0x04

#define MODULE_ID   ""
#define DPS_PAYLOAD "{\"modelId\":\"%s\"}"

static VOID connection_status_callback(NX_AZURE_IOT_HUB_CLIENT* hub_client_ptr, UINT status)
{
    if (status)
    {
        printf("Disconnected from IoTHub (0x%08x)\r\n", status);
    }
    else
    {
        printf("Connected to IoTHub\r\n");
    }
}

static VOID message_receive_direct_method(NX_AZURE_IOT_HUB_CLIENT* hub_client_ptr, VOID* context)
{
    AZURE_IOT_NX_CONTEXT* nx_context = (AZURE_IOT_NX_CONTEXT*)context;
    tx_event_flags_set(&nx_context->events, DIRECT_METHOD_EVENT, TX_OR);
}

static VOID message_receive_callback_twin(NX_AZURE_IOT_HUB_CLIENT* hub_client_ptr, VOID* context)
{
    AZURE_IOT_NX_CONTEXT* nx_context = (AZURE_IOT_NX_CONTEXT*)context;
    tx_event_flags_set(&nx_context->events, DEVICE_TWIN_GET_EVENT, TX_OR);
}

static VOID message_receive_callback_desire_property(NX_AZURE_IOT_HUB_CLIENT* hub_client_ptr, VOID* context)
{
    AZURE_IOT_NX_CONTEXT* nx_context = (AZURE_IOT_NX_CONTEXT*)context;
    tx_event_flags_set(&nx_context->events, DEVICE_TWIN_DESIRED_PROPERTY_EVENT, TX_OR);
}

static VOID process_direct_method(AZURE_IOT_NX_CONTEXT* nx_context)
{
    UINT status;
    NX_PACKET* packet;
    UCHAR* method_name;
    USHORT method_name_length;
    VOID* context;
    USHORT context_length;
    UCHAR* payload;
    USHORT payload_length;

    if ((status = nx_azure_iot_hub_client_direct_method_message_receive(&nx_context->iothub_client,
             &method_name,
             &method_name_length,
             &context,
             &context_length,
             &packet,
             NX_WAIT_FOREVER)))
    {
        printf("Direct method receive failed!: error code = 0x%08x\r\n", status);
        return;
    }

    printf("Receive direct method call: %.*s\r\n", (INT)method_name_length, (CHAR*)method_name);
    printf_packet(packet, "\tPayload: ");

    payload        = packet->nx_packet_prepend_ptr;
    payload_length = packet->nx_packet_append_ptr - packet->nx_packet_prepend_ptr;

    if (nx_context->direct_method_cb)
    {
        nx_context->direct_method_cb(
            nx_context, method_name, method_name_length, payload, payload_length, context, context_length);
    }

    // Release the received packet, as ownership was passed to the application
    nx_packet_release(packet);
}

static VOID process_device_twin_get(AZURE_IOT_NX_CONTEXT* nx_context)
{
    UINT status;
    NX_PACKET* packet_ptr;
    UCHAR buffer[128];

    if ((status = nx_azure_iot_hub_client_device_twin_properties_receive(
             &nx_context->iothub_client, &packet_ptr, NX_WAIT_FOREVER)))
    {
        printf("Error: receive device twin property failed (0x%08x)\r\n", status);
        return;
    }

    printf_packet(packet_ptr, "Receive twin properties: ");

    if (packet_ptr->nx_packet_length > (ULONG)(packet_ptr->nx_packet_append_ptr - packet_ptr->nx_packet_prepend_ptr))
    {
        printf("ERROR: json is large than nx_packet\r\n");
        return;
    }

    if (nx_context->device_twin_get_cb)
    {
        if ((status = nx_azure_iot_pnp_helper_twin_data_parse(
                 packet_ptr, NX_FALSE, NX_NULL, 0, buffer, sizeof(buffer), nx_context->device_twin_get_cb, nx_context)))
        {
            printf("ERROR: failed to parse twin data!: error code = 0x%08x\r\n", status);
        }
    }

    // Release the received packet, as ownership was passed to the application
    nx_packet_release(packet_ptr);
}

static VOID process_device_twin_desired_property(AZURE_IOT_NX_CONTEXT* nx_context)
{
    UINT status;
    NX_PACKET* packet_ptr;
    UCHAR buffer[128];
    if ((status = nx_azure_iot_hub_client_device_twin_desired_properties_receive(
             &nx_context->iothub_client, &packet_ptr, NX_WAIT_FOREVER)))
    {
        printf("ERROR: receive device twin desired property receive failed (0x%08x)\r\n", status);
        return;
    }

    printf_packet(packet_ptr, "Receive twin desired property: ");

    if (packet_ptr->nx_packet_length > (ULONG)(packet_ptr->nx_packet_append_ptr - packet_ptr->nx_packet_prepend_ptr))
    {
        printf("ERROR: json is large than nx_packet\r\n");
        return;
    }

    if (nx_context->device_twin_desired_prop_cb)
    {
        if ((status = nx_azure_iot_pnp_helper_twin_data_parse(packet_ptr,
                 NX_TRUE,
                 NX_NULL,
                 0,
                 buffer,
                 sizeof(buffer),
                 nx_context->device_twin_desired_prop_cb,
                 nx_context)))
        {
            printf("ERROR: failed to parse twin data!: error code = 0x%08x\r\n", status);
        }
    }

    // Release the received packet, as ownership was passed to the application
    nx_packet_release(packet_ptr);
}

static VOID event_thread(ULONG parameter)
{
    ULONG app_events;

    AZURE_IOT_NX_CONTEXT* context = (AZURE_IOT_NX_CONTEXT*)parameter;

    while (true)
    {
        tx_event_flags_get(&context->events, ALL_EVENTS, TX_OR_CLEAR, &app_events, NX_IP_PERIODIC_RATE);

        if (app_events & DIRECT_METHOD_EVENT)
        {
            process_direct_method(context);
        }

        if (app_events & DEVICE_TWIN_GET_EVENT)
        {
            process_device_twin_get(context);
        }

        if (app_events & DEVICE_TWIN_DESIRED_PROPERTY_EVENT)
        {
            process_device_twin_desired_property(context);
        }

        app_events = 0;
    }
}

static UINT azure_iot_nx_client_create_common(AZURE_IOT_NX_CONTEXT* context,
    CHAR* iot_hub_hostname,
    CHAR* iot_device_id,
    CHAR* device_sas_key,
    CHAR* device_model_id)
{
    UINT status;

    printf("Initializing Azure IoT Hub client\r\n");
    printf("\tHub hostname: %s\r\n", iot_hub_hostname);
    printf("\tDevice id: %s\r\n", iot_device_id);
    printf("\tModel id: %s\r\n", device_model_id);

    if ((status = tx_event_flags_create(&context->events, "nx_client")))
    {
        printf("ERROR: failed on create nx_client event flags (0x%08x)\r\n", status);
        return status;
    }

    // Initialize IoTHub client.
    if ((status = nx_azure_iot_hub_client_initialize(&context->iothub_client,
             &context->nx_azure_iot,
             (UCHAR*)iot_hub_hostname,
             strlen(iot_hub_hostname),
             (UCHAR*)iot_device_id,
             strlen(iot_device_id),
             (UCHAR*)MODULE_ID,
             strlen(MODULE_ID),
             _nx_azure_iot_tls_supported_crypto,
             _nx_azure_iot_tls_supported_crypto_size,
             _nx_azure_iot_tls_ciphersuite_map,
             _nx_azure_iot_tls_ciphersuite_map_size,
             (UCHAR*)context->nx_azure_iot_tls_metadata_buffer,
             sizeof(context->nx_azure_iot_tls_metadata_buffer),
             &context->root_ca_cert)))
    {
        printf("ERROR: on nx_azure_iot_hub_client_initialize (0x%08x)\r\n", status);
        return status;
    }

    // Set SAS key
    if ((status = nx_azure_iot_hub_client_symmetric_key_set(
             &context->iothub_client, (UCHAR*)device_sas_key, strlen(device_sas_key))))
    {
        printf("ERROR: nx_azure_iot_hub_client_symmetric_key_set (0x%08x)\r\n", status);
    }

    // Set Model id
    else if ((status = nx_azure_iot_hub_client_model_id_set(
                  &context->iothub_client, (UCHAR*)device_model_id, strlen(device_model_id))))
    {
        printf("ERROR: nx_azure_iot_hub_client_model_id_set (0x%08x)\r\n", status);
    }

    // Set connection status callback
    else if ((status = nx_azure_iot_hub_client_connection_status_callback_set(
                  &context->iothub_client, connection_status_callback)))
    {
        printf("ERROR: failed on connection_status_callback (0x%08x)\r\n", status);
    }

    // Enable direct methods
    else if ((status = nx_azure_iot_hub_client_direct_method_enable(&context->iothub_client)))
    {
        printf("ERROR: direct method receive enable failed (0x%08x)\r\n", status);
    }

    // Enable device twin
    else if ((status = nx_azure_iot_hub_client_device_twin_enable(&context->iothub_client)))
    {
        printf("ERROR: device twin enabled failed (0x%08x)\r\n", status);
    }

    // Set device twin callback
    else if ((status = nx_azure_iot_hub_client_receive_callback_set(&context->iothub_client,
                  NX_AZURE_IOT_HUB_DEVICE_TWIN_PROPERTIES,
                  message_receive_callback_twin,
                  (VOID*)context)))
    {
        printf("ERROR: device twin callback set (0x%08x)\r\n", status);
    }

    // Set direct method callback
    else if ((status = nx_azure_iot_hub_client_receive_callback_set(&context->iothub_client,
                  NX_AZURE_IOT_HUB_DIRECT_METHOD,
                  message_receive_direct_method,
                  (VOID*)context)))
    {
        printf("ERROR: device method callback set (0x%08x)\r\n", status);
    }

    // Set the writeable property callback
    else if ((status = nx_azure_iot_hub_client_receive_callback_set(&context->iothub_client,
                  NX_AZURE_IOT_HUB_DEVICE_TWIN_DESIRED_PROPERTIES,
                  message_receive_callback_desire_property,
                  (VOID*)context)))
    {
        printf("ERROR: device twin desired property callback set (0x%08x)\r\n", status);
    }

    if (status != NX_AZURE_IOT_SUCCESS)
    {
        nx_azure_iot_hub_client_deinitialize(&context->iothub_client);
    }

    return status;
}

UINT azure_iot_nx_client_register_direct_method(AZURE_IOT_NX_CONTEXT* context, func_ptr_direct_method callback)
{
    if (context == NULL || context->direct_method_cb != NULL)
    {
        return NX_PTR_ERROR;
    }

    context->direct_method_cb = callback;
    return NX_SUCCESS;
}

UINT azure_iot_nx_client_register_device_twin_desired_prop(
    AZURE_IOT_NX_CONTEXT* context, func_ptr_device_twin_desired_prop callback)
{
    if (context == NULL || context->device_twin_desired_prop_cb != NULL)
    {
        return NX_PTR_ERROR;
    }

    context->device_twin_desired_prop_cb = callback;
    return NX_SUCCESS;
}

UINT azure_iot_nx_client_register_device_twin_prop(AZURE_IOT_NX_CONTEXT* context, func_ptr_device_twin_prop callback)
{
    if (context == NULL || context->device_twin_get_cb != NULL)
    {
        return NX_PTR_ERROR;
    }

    context->device_twin_get_cb = callback;
    return NX_SUCCESS;
}

UINT azure_iot_nx_client_create(AZURE_IOT_NX_CONTEXT* context,
    NX_IP* nx_ip,
    NX_PACKET_POOL* nx_pool,
    NX_DNS* nx_dns,
    UINT (*unix_time_callback)(ULONG* unix_time),
    CHAR* iot_hub_hostname,
    CHAR* iot_device_id,
    CHAR* iot_sas_key,
    CHAR* iot_model_id)
{
    UINT status;

    if (context == NULL)
    {
        printf("ERROR: context is NULL\r\n");
        return NX_PTR_ERROR;
    }

    if (iot_hub_hostname[0] == 0 || iot_device_id[0] == 0 || iot_sas_key[0] == 0)
    {
        printf("ERROR: IoT Hub connection configuration is empty\r\n");
        return NX_PTR_ERROR;
    }

    memset(context, 0, sizeof(AZURE_IOT_NX_CONTEXT));

    // Create Azure IoT handler
    if ((status = nx_azure_iot_create(&context->nx_azure_iot,
             (UCHAR*)"Azure IoT",
             nx_ip,
             nx_pool,
             nx_dns,
             context->nx_azure_iot_thread_stack,
             sizeof(context->nx_azure_iot_thread_stack),
             NX_AZURE_IOT_THREAD_PRIORITY,
             unix_time_callback)))
    {
        printf("ERROR: failed on nx_azure_iot_create (0x%08x)\r\n", status);
        return status;
    }

    // Initialize CA certificate.
    if ((status = nx_secure_x509_certificate_initialize(&context->root_ca_cert,
             (UCHAR*)azure_iot_root_ca,
             (USHORT)azure_iot_root_ca_len,
             NX_NULL,
             0,
             NULL,
             0,
             NX_SECURE_X509_KEY_TYPE_NONE)))
    {
        printf("Failed to initialize ROOT CA certificate!: error code = 0x%08x\r\n", status);
        nx_azure_iot_delete(&context->nx_azure_iot);
        return status;
    }

    return azure_iot_nx_client_create_common(context, iot_hub_hostname, iot_device_id, iot_sas_key, iot_model_id);
}

UINT azure_iot_nx_client_dps_create(AZURE_IOT_NX_CONTEXT* context,
    NX_IP* nx_ip,
    NX_PACKET_POOL* nx_pool,
    NX_DNS* nx_dns,
    UINT (*unix_time_callback)(ULONG* unix_time),
    CHAR* dps_endpoint,
    CHAR* dps_id_scope,
    CHAR* dps_registration_id,
    CHAR* device_sas_key,
    CHAR* device_model_id)
{
    UINT status;
    CHAR payload[200];
    UINT iot_hub_hostname_len = AZURE_IOT_HOST_NAME_SIZE;
    UINT iot_device_id_len    = AZURE_IOT_DEVICE_ID_SIZE;

    printf("Initializing Azure IoT DPS client\r\n");
    printf("\tDPS endpoint: %s\r\n", dps_endpoint);
    printf("\tDPS ID scope: %s\r\n", dps_id_scope);
    printf("\tRegistration ID: %s\r\n", dps_registration_id);

    if (context == NULL)
    {
        printf("ERROR: context is NULL\r\n");
        return NX_PTR_ERROR;
    }

    if (dps_endpoint[0] == 0 || dps_id_scope[0] == 0 || dps_registration_id[0] == 0 || device_sas_key[0] == 0)
    {
        printf("ERROR: IoT DPS + Hub connection configuration is empty\r\n");
        return NX_PTR_ERROR;
    }

    memset(context, 0, sizeof(AZURE_IOT_NX_CONTEXT));

    if (snprintf(payload, sizeof(payload), DPS_PAYLOAD, device_model_id) > sizeof(payload) - 1)
    {
        printf("ERROR: insufficient buffer size to create DPS payload\r\n");
        return NX_SIZE_ERROR;
    }

    // Create Azure IoT handler
    if ((status = nx_azure_iot_create(&context->nx_azure_iot,
             (UCHAR*)"Azure IoT",
             nx_ip,
             nx_pool,
             nx_dns,
             context->nx_azure_iot_thread_stack,
             sizeof(context->nx_azure_iot_thread_stack),
             NX_AZURE_IOT_THREAD_PRIORITY,
             unix_time_callback)))
    {
        printf("ERROR: failed on nx_azure_iot_create (0x%08x)\r\n", status);
        return status;
    }

    // Initialize CA certificate.
    if ((status = nx_secure_x509_certificate_initialize(&context->root_ca_cert,
             (UCHAR*)azure_iot_root_ca,
             (USHORT)azure_iot_root_ca_len,
             NX_NULL,
             0,
             NULL,
             0,
             NX_SECURE_X509_KEY_TYPE_NONE)))
    {
        printf("Failed to initialize ROOT CA certificate!: error code = 0x%08x\r\n", status);
        nx_azure_iot_delete(&context->nx_azure_iot);
        return status;
    }

    // Initialize IoT provisioning client
    if ((status = nx_azure_iot_provisioning_client_initialize(&context->prov_client,
             &context->nx_azure_iot,
             (UCHAR*)dps_endpoint,
             strlen(dps_endpoint),
             (UCHAR*)dps_id_scope,
             strlen(dps_id_scope),
             (UCHAR*)dps_registration_id,
             strlen(dps_registration_id),
             _nx_azure_iot_tls_supported_crypto,
             _nx_azure_iot_tls_supported_crypto_size,
             _nx_azure_iot_tls_ciphersuite_map,
             _nx_azure_iot_tls_ciphersuite_map_size,
             (UCHAR*)context->nx_azure_iot_tls_metadata_buffer,
             sizeof(context->nx_azure_iot_tls_metadata_buffer),
             &context->root_ca_cert)))
    {
        printf("Failed on nx_azure_iot_provisioning_client_initialize (0x%08x)\r\n", status);
        return status;
    }

    // Set symmetric key
    if ((status = nx_azure_iot_provisioning_client_symmetric_key_set(
             &context->prov_client, (UCHAR*)device_sas_key, strlen(device_sas_key))))
    {
        printf("Failed on nx_azure_iot_hub_client_symmetric_key_set (0x%08x)\r\n", status);
    }

    // Set the payload containing the model Id
    else if ((status = nx_azure_iot_provisioning_client_registration_payload_set(
                  &context->prov_client, (UCHAR*)payload, strlen(payload))))
    {
        printf("Error: nx_azure_iot_provisioning_client_registration_payload_set (0x%08x\r\n", status);
    }

    // Register device
    else if ((status = nx_azure_iot_provisioning_client_register(&context->prov_client, NX_WAIT_FOREVER)))
    {
        printf("ERROR: nx_azure_iot_provisioning_client_register (0x%08x)\r\n", status);
    }

    // Get Device info
    else if ((status = nx_azure_iot_provisioning_client_iothub_device_info_get(&context->prov_client,
                  (UCHAR*)context->azure_iot_hub_hostname,
                  &iot_hub_hostname_len,
                  (UCHAR*)context->azure_iot_device_id,
                  &iot_device_id_len)))
    {
        printf("ERROR: nx_azure_iot_provisioning_client_iothub_device_info_get (0x%08x)\r\n", status);
        return status;
    }

    // Destroy Provisioning Client
    nx_azure_iot_provisioning_client_deinitialize(&context->prov_client);

    if (status != NX_SUCCESS) {
      return status;
    }

    // Null terminate returned values
    context->azure_iot_hub_hostname[iot_hub_hostname_len] = 0;
    context->azure_iot_device_id[iot_device_id_len] = 0;

    printf("SUCCESS: Azure IoT DPS client initialized\r\n\r\n");

    return azure_iot_nx_client_create_common(
        context, context->azure_iot_hub_hostname, context->azure_iot_device_id, device_sas_key, device_model_id);
}

UINT azure_iot_nx_client_delete(AZURE_IOT_NX_CONTEXT* context)
{
    /* Destroy IoTHub Client. */
    nx_azure_iot_hub_client_disconnect(&context->iothub_client);
    nx_azure_iot_hub_client_deinitialize(&context->iothub_client);
    nx_azure_iot_delete(&context->nx_azure_iot);

    return NX_SUCCESS;
}

UINT azure_iot_nx_client_connect(AZURE_IOT_NX_CONTEXT* context)
{
    UINT status;

    // Connect to IoTHub client
    if ((status = nx_azure_iot_hub_client_connect(&context->iothub_client, NX_TRUE, NX_WAIT_FOREVER)))
    {
        printf("Failed on nx_azure_iot_hub_client_connect (0x%08x)\r\n", status);
        return status;
    }

    if ((status = tx_thread_create(&context->azure_iot_thread,
             "Nx Thread",
             event_thread,
             (ULONG)context,
             (UCHAR*)context->azure_iot_thread_stack,
             AZURE_IOT_STACK_SIZE,
             THREAD_PRIORITY,
             THREAD_PRIORITY,
             1,
             TX_AUTO_START)))
    {
        printf("Failed to create telemetry thread (0x%08x)\r\n", status);
        return status;
    }

    printf("SUCCESS: Azure IoT Hub client initialized\r\n\r\n");

    return NX_SUCCESS;
}

UINT azure_iot_nx_client_disconnect(AZURE_IOT_NX_CONTEXT* context)
{
    nx_azure_iot_hub_client_disconnect(&context->iothub_client);

    return NX_SUCCESS;
}

UINT azure_iot_nx_client_publish_float_telemetry(AZURE_IOT_NX_CONTEXT* context, CHAR* key, float value)
{
    UINT status;
    CHAR buffer[30];
    NX_PACKET* packet_ptr;

    if (snprintf(buffer, sizeof(buffer), "{\"%s\":%0.2f}", key, value) > sizeof(buffer) - 1)
    {
        printf("ERROR: insufficient buffer size to publish float telemetry\r\n");
        return NX_SIZE_ERROR;
    }

    // Create a telemetry message packet
    if ((status = nx_azure_iot_hub_client_telemetry_message_create(
             &context->iothub_client, &packet_ptr, NX_WAIT_FOREVER)))
    {
        printf("Telemetry message create failed (0x%08x)\r\n", status);
        return status;
    }

    if ((status = nx_azure_iot_hub_client_telemetry_send(
             &context->iothub_client, packet_ptr, (UCHAR*)buffer, strlen(buffer), NX_WAIT_FOREVER)))
    {
        printf("Telemetry message send failed (0x%08x)\r\n", status);
        nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);
        return status;
    }

    printf("Telemetry message sent: %s.\r\n", buffer);

    return NX_SUCCESS;
}

UINT azure_iot_nx_client_publish_float_property(AZURE_IOT_NX_CONTEXT* context, CHAR* key, float value)
{
    UINT status;
    UINT response_status;
    UINT request_id;
    CHAR buffer[30];

    if (snprintf(buffer, sizeof(buffer), "{\"%s\":%0.2f}", key, value) > sizeof(buffer) - 1)
    {
        printf("ERROR: insufficient buffer size to publish float property\r\n");
        return NX_SIZE_ERROR;
    }

    if ((status = nx_azure_iot_hub_client_device_twin_reported_properties_send(
             &context->iothub_client, (UCHAR*)buffer, strlen(buffer), &request_id, &response_status, NX_WAIT_FOREVER)))
    {
        printf("Device twin reported properties failed (0x%08x)\r\n", status);
        return status;
    }

    if ((response_status < 200) || (response_status >= 300))
    {
        printf("device twin report properties failed (%d)\r\n", response_status);
        return status;
    }

    printf("Device twin property sent: %s\r\n", buffer);

    return NX_SUCCESS;
}

UINT azure_iot_nx_client_publish_bool_property(AZURE_IOT_NX_CONTEXT* context, CHAR* key, bool value)
{
    UINT status;
    UINT response_status;
    UINT request_id;
    CHAR buffer[64];

    if (snprintf(buffer, sizeof(buffer), "{\"%s\":%s}", key, (value ? "true" : "false")) > sizeof(buffer) - 1)
    {
        printf("ERROR: Unsufficient buffer size to publish bool property\r\n");
        return NX_SIZE_ERROR;
    }

    if ((status = nx_azure_iot_hub_client_device_twin_reported_properties_send(
             &context->iothub_client, (UCHAR*)buffer, strlen(buffer), &request_id, &response_status, NX_WAIT_FOREVER)))
    {
        printf("Error: device twin reported properties failed (0x%08x)\r\n", status);
        return status;
    }

    if ((response_status < 200) || (response_status >= 300))
    {
        printf("Error: device twin report properties failed (%d)\r\n", response_status);
        return status;
    }

    printf("Device twin property sent: %s\r\n", buffer);

    return NX_SUCCESS;
}

UINT azure_nx_client_respond_int_writeable_property(
    AZURE_IOT_NX_CONTEXT* context, CHAR* property, int value, int http_status, int version)
{
    UINT status;
    UINT response_status;
    UINT request_id;
    CHAR message[100];

    printf("Responding to writeable property %s = %d\r\n", property, value);

    if (snprintf(message,
            sizeof(message),
            "{\"%s\":{\"value\":%d,\"ac\":%d,\"av\":%d}}",
            property,
            value,
            http_status,
            version) > sizeof(message) - 1)
    {
        printf("ERROR: insufficient buffer size to respond to writeable property\r\n");
        return NX_SIZE_ERROR;
    }

    if ((status = nx_azure_iot_hub_client_device_twin_reported_properties_send(&context->iothub_client,
             (UCHAR*)message,
             strlen(message),
             &request_id,
             &response_status,
             NX_WAIT_FOREVER)))
    {
        printf("ERROR: device twin reported properties failed (0x%08x)\r\n", status);
    }

    if ((response_status < 200) || (response_status >= 300))
    {
        printf("ERROR: device twin report properties failed (%d)\r\n", response_status);
        return status;
    }

    return status;
}

VOID printf_packet(NX_PACKET* packet_ptr, CHAR* prepend)
{
    printf("%s", prepend);

    while (packet_ptr != NX_NULL)
    {
        printf("%.*s", (INT)(packet_ptr->nx_packet_length), (CHAR*)packet_ptr->nx_packet_prepend_ptr);
        packet_ptr = packet_ptr->nx_packet_next;
    }

    printf("\r\n");
}
