/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "azure_iot_nx_client.h"

#include <stdio.h>

#include "nx_azure_iot_hub_client.h"
#include "nx_azure_iot_hub_client_properties.h"

#include "azure_iot_cert.h"
#include "azure_iot_ciphersuites.h"

#define NX_AZURE_IOT_THREAD_PRIORITY 4
#define THREAD_PRIORITY              16

// Incoming events from the middleware
#define ALL_EVENTS                        0xFF
#define COMMAND_RECEIVE_EVENT             0x01
#define PROPERTIES_RECEIVE_EVENT          0x02
#define WRITABLE_PROPERTIES_RECEIVE_EVENT 0x04
#define PROPERTIES_COMPLETE_EVENT         0x08

#define AZURE_IOT_DPS_ENDPOINT "global.azure-devices-provisioning.net"

#define MODULE_ID   ""
#define DPS_PAYLOAD "{\"modelId\":\"%s\"}"

#define MAX_EXPONENTIAL_BACKOFF_JITTER_PERCENT 60
#define MAX_EXPONENTIAL_BACKOFF_IN_SEC         (10 * 60)
#define INITIAL_EXPONENTIAL_BACKOFF_IN_SEC     3

// Connection timeouts in threadx ticks
#define HUB_CONNECT_TIMEOUT_TICKS  (10 * TX_TIMER_TICKS_PER_SECOND)
#define DPS_REGISTER_TIMEOUT_TICKS (3 * TX_TIMER_TICKS_PER_SECOND)

#define DPS_PAYLOAD_SIZE       200
#define TELEMETRY_BUFFER_SIZE  256
#define PROPERTIES_BUFFER_SIZE 128

static UCHAR telemetry_buffer[TELEMETRY_BUFFER_SIZE];
static UCHAR properties_buffer[PROPERTIES_BUFFER_SIZE];

static VOID printf_packet(CHAR* prepend, NX_PACKET* packet_ptr)
{
    printf("%s", prepend);

    while (packet_ptr != NX_NULL)
    {
        printf("%.*s", (INT)(packet_ptr->nx_packet_length), (CHAR*)packet_ptr->nx_packet_prepend_ptr);
        packet_ptr = packet_ptr->nx_packet_next;
    }

    printf("\r\n");
}

static UINT exponential_backoff_with_jitter(UINT* exponential_retry_count)
{
    float jitter_percent = (MAX_EXPONENTIAL_BACKOFF_JITTER_PERCENT / 100.0f) * (rand() / ((float)RAND_MAX));
    UINT base_delay      = MAX_EXPONENTIAL_BACKOFF_IN_SEC;

    base_delay = (1 << *exponential_retry_count) * INITIAL_EXPONENTIAL_BACKOFF_IN_SEC;

    if (base_delay > MAX_EXPONENTIAL_BACKOFF_IN_SEC)
    {
        base_delay = MAX_EXPONENTIAL_BACKOFF_IN_SEC;
    }
    else
    {
        (*exponential_retry_count)++;
    }

    return (base_delay * (1 + jitter_percent)) * TX_TIMER_TICKS_PER_SECOND;
}

static VOID connection_status_callback(NX_AZURE_IOT_HUB_CLIENT* hub_client_ptr, UINT status)
{
    if (status == NX_SUCCESS)
    {
        printf("Connected to IoT Hub\r\n");
    }
    else
    {
        printf("Connection failure from IoT Hub (0x%08x)\r\n", status);

        UINT connect_status = NX_AZURE_IOT_FAILURE;
        UINT retry_count    = 0;
        while (connect_status)
        {
            printf("Reconnecting to IoT Hub...\r\n");

            if ((connect_status = nx_azure_iot_hub_client_connect(hub_client_ptr, NX_TRUE, HUB_CONNECT_TIMEOUT_TICKS)))
            {
                printf("Failed reconnect on nx_azure_iot_hub_client_connect (0x%08x)\r\n", connect_status);
                tx_thread_sleep(exponential_backoff_with_jitter(&retry_count));
            }
        }
    }
}

static VOID message_receive_command(NX_AZURE_IOT_HUB_CLIENT* hub_client_ptr, VOID* context)
{
    AZURE_IOT_NX_CONTEXT* nx_context = (AZURE_IOT_NX_CONTEXT*)context;
    tx_event_flags_set(&nx_context->events, COMMAND_RECEIVE_EVENT, TX_OR);
}

static VOID message_receive_callback_properties(NX_AZURE_IOT_HUB_CLIENT* hub_client_ptr, VOID* context)
{
    AZURE_IOT_NX_CONTEXT* nx_context = (AZURE_IOT_NX_CONTEXT*)context;
    tx_event_flags_set(&nx_context->events, PROPERTIES_RECEIVE_EVENT, TX_OR);
}

static VOID message_receive_callback_writable_property(NX_AZURE_IOT_HUB_CLIENT* hub_client_ptr, VOID* context)
{
    AZURE_IOT_NX_CONTEXT* nx_context = (AZURE_IOT_NX_CONTEXT*)context;
    tx_event_flags_set(&nx_context->events, WRITABLE_PROPERTIES_RECEIVE_EVENT, TX_OR);
}

static VOID process_command(AZURE_IOT_NX_CONTEXT* nx_context)
{
    UINT status;
    const UCHAR* component_name_ptr;
    USHORT component_name_length;
    const UCHAR* command_name_ptr;
    USHORT command_name_length;
    VOID* context_ptr;
    USHORT context_length;
    UCHAR* payload_ptr;
    USHORT payload_length;
    NX_PACKET* packet_ptr;

    while ((status = nx_azure_iot_hub_client_command_message_receive(&nx_context->iothub_client,
                &component_name_ptr,
                &component_name_length,
                &command_name_ptr,
                &command_name_length,
                &context_ptr,
                &context_length,
                &packet_ptr,
                NX_NO_WAIT)) == NX_AZURE_IOT_SUCCESS)
    {
        printf("Received command: %.*s\r\n", (INT)command_name_length, (CHAR*)command_name_ptr);
        printf_packet("\tPayload: ", packet_ptr);

        payload_ptr    = packet_ptr->nx_packet_prepend_ptr;
        payload_length = packet_ptr->nx_packet_append_ptr - packet_ptr->nx_packet_prepend_ptr;

        if (nx_context->command_received_cb)
        {
            nx_context->command_received_cb(nx_context,
                command_name_ptr,
                command_name_length,
                payload_ptr,
                payload_length,
                context_ptr,
                context_length);
        }

        // Release the received packet, as ownership was passed to the application from the middleware
        nx_packet_release(packet_ptr);
    }

    // If we failed for anything other than no packet, then report error
    if (status != NX_AZURE_IOT_NO_PACKET)
    {
        printf("Error: Command receive failed (0x%08x)\r\n", status);
        return;
    }
}

static UINT process_properties_shared(AZURE_IOT_NX_CONTEXT* nx_context,
    NX_PACKET* packet_ptr,
    UINT message_type,
    UINT property_type,
    UCHAR* scratch_buffer,
    UINT scratch_buffer_len,
    func_ptr_property_received property_received_cb)
{
    UINT status;
    const UCHAR* component_name_ptr;
    USHORT component_name_length = 0;
    UINT property_name_length;
    ULONG properties_version;
    NX_AZURE_IOT_JSON_READER json_reader;

    if ((status = nx_azure_iot_json_reader_init(&json_reader, packet_ptr)))
    {
        printf("Error: failed to initialize json reader (0x%08x)\r\n", status);
        nx_packet_release(packet_ptr);
        return status;
    }

    // Get the version
    if ((status = nx_azure_iot_hub_client_properties_version_get(
             &nx_context->iothub_client, &json_reader, message_type, &properties_version)))
    {
        printf("Error: Properties version get failed (0x%08x)\r\n", status);
        nx_packet_release(packet_ptr);
        return status;
    }

    if ((status = nx_azure_iot_json_reader_init(&json_reader, packet_ptr)))
    {
        printf("Init json reader failed!: error code = 0x%08x\r\n", status);
        nx_packet_release(packet_ptr);
        return status;
    }

    if ((status = nx_azure_iot_json_reader_init(&json_reader, packet_ptr)))
    {
        printf("Error: failed to initialize json reader (0x%08x)\r\n", status);
        nx_packet_release(packet_ptr);
        return status;
    }

    while ((status = nx_azure_iot_hub_client_properties_component_property_next_get(&nx_context->iothub_client,
                &json_reader,
                message_type,
                property_type,
                &component_name_ptr,
                &component_name_length)) == NX_AZURE_IOT_SUCCESS)
    {
        if (nx_azure_iot_json_reader_token_string_get(
                &json_reader, scratch_buffer, scratch_buffer_len, &property_name_length))
        {
            printf("Failed to get string property value\r\n");
            return NX_NOT_SUCCESSFUL;
        }

        nx_azure_iot_json_reader_next_token(&json_reader);

        property_received_cb(nx_context,
            component_name_ptr,
            component_name_length,
            scratch_buffer,
            property_name_length,
            &json_reader,
            properties_version);

        // If we are still looking at the value, then skip over it (including if it has children)
        if (nx_azure_iot_json_reader_token_type(&json_reader) == NX_AZURE_IOT_READER_TOKEN_BEGIN_OBJECT)
        {
            nx_azure_iot_json_reader_skip_children(&json_reader);
        }

        nx_azure_iot_json_reader_next_token(&json_reader);
    }

    return NX_AZURE_IOT_SUCCESS;
}

static VOID process_properties(AZURE_IOT_NX_CONTEXT* nx_context)
{
    UINT status;
    NX_PACKET* packet_ptr;

    if ((status = nx_azure_iot_hub_client_properties_receive(&nx_context->iothub_client, &packet_ptr, NX_WAIT_FOREVER)))
    {
        printf("Error: nx_azure_iot_hub_client_properties_receive failed (0x%08x)\r\n", status);
        return;
    }

    printf_packet("Receive properties: ", packet_ptr);

    if (nx_context->property_received_cb)
    {
        // Parse the writable properties from the device twin receive receive message
        if ((status = process_properties_shared(nx_context,
                 packet_ptr,
                 NX_AZURE_IOT_HUB_PROPERTIES,
                 NX_AZURE_IOT_HUB_CLIENT_PROPERTY_WRITABLE,
                 properties_buffer,
                 sizeof(properties_buffer),
                 nx_context->property_received_cb)))
        {
            printf("Error: failed to parse properties (0x%08x)\r\n", status);
        }
    }

    // Release the received packet, as ownership was passed to the application from the middleware
    nx_packet_release(packet_ptr);

    // Send event to notify device twin received
    tx_event_flags_set(&nx_context->events, PROPERTIES_COMPLETE_EVENT, TX_OR);
}

static VOID process_writable_properties(AZURE_IOT_NX_CONTEXT* nx_context)
{
    UINT status;
    NX_PACKET* packet_ptr;

    if ((status = nx_azure_iot_hub_client_writable_properties_receive(
             &nx_context->iothub_client, &packet_ptr, NX_WAIT_FOREVER)))
    {
        printf("Error: nx_azure_iot_hub_client_writable_properties_receive failed (0x%08x)\r\n", status);
        return;
    }

    printf_packet("Receive properties: ", packet_ptr);

    if (nx_context->writable_property_received_cb)
    {
        // Parse the writable properties from the writable receive message
        if ((status = process_properties_shared(nx_context,
                 packet_ptr,
                 NX_AZURE_IOT_HUB_WRITABLE_PROPERTIES,
                 NX_AZURE_IOT_HUB_CLIENT_PROPERTY_WRITABLE,
                 properties_buffer,
                 sizeof(properties_buffer),
                 nx_context->writable_property_received_cb)))
        {
            printf("Error: failed to parse properties (0x%08x)\r\n", status);
        }
    }

    // Release the received packet, as ownership was passed to the application from the middleware
    nx_packet_release(packet_ptr);
}

static VOID event_thread(ULONG parameter)
{
    ULONG app_events = 0;

    AZURE_IOT_NX_CONTEXT* context = (AZURE_IOT_NX_CONTEXT*)parameter;

    while (true)
    {
        tx_event_flags_get(&context->events, ALL_EVENTS, TX_OR_CLEAR, &app_events, NX_IP_PERIODIC_RATE);

        if (app_events & COMMAND_RECEIVE_EVENT)
        {
            process_command(context);
        }

        if (app_events & PROPERTIES_RECEIVE_EVENT)
        {
            process_properties(context);
        }

        if (app_events & WRITABLE_PROPERTIES_RECEIVE_EVENT)
        {
            process_writable_properties(context);
        }

        app_events = 0;
    }
}

static UINT azure_iot_nx_client_hub_create_internal(AZURE_IOT_NX_CONTEXT* context)
{
    UINT status;

    printf("Initializing Azure IoT Hub client\r\n");
    printf("\tHub hostname: %s\r\n", context->azure_iot_hub_hostname);
    printf("\tDevice id: %s\r\n", context->azure_iot_device_id);
    printf("\tModel id: %s\r\n", context->azure_iot_model_id);

    // Initialize IoT Hub client.
    if ((status = nx_azure_iot_hub_client_initialize(&context->iothub_client,
             &context->nx_azure_iot,
             (UCHAR*)context->azure_iot_hub_hostname,
             strlen(context->azure_iot_hub_hostname),
             (UCHAR*)context->azure_iot_device_id,
             strlen(context->azure_iot_device_id),
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
        printf("Error: on nx_azure_iot_hub_client_initialize (0x%08x)\r\n", status);
        return status;
    }

    // Set credentials
    if (context->azure_iot_auth_mode == AZURE_IOT_AUTH_MODE_SAS)
    {
        // Symmetric (SAS) Key
        if ((status = nx_azure_iot_hub_client_symmetric_key_set(&context->iothub_client,
                 (UCHAR*)context->azure_iot_device_sas_key,
                 context->azure_iot_device_sas_key_len)))
        {
            printf("Error: failed on nx_azure_iot_hub_client_symmetric_key_set (0x%08x)\r\n", status);
        }
    }
    else if (context->azure_iot_auth_mode == AZURE_IOT_AUTH_MODE_CERT)
    {
        // X509 Certificate
        if ((status = nx_azure_iot_hub_client_device_cert_set(&context->iothub_client, &context->device_certificate)))
        {
            printf("Error: failed on nx_azure_iot_hub_client_device_cert_set!: error code = 0x%08x\r\n", status);
        }
    }

    // Set Model id
    if ((status = nx_azure_iot_hub_client_model_id_set(
             &context->iothub_client, (UCHAR*)context->azure_iot_model_id, strlen(context->azure_iot_model_id))))
    {
        printf("Error: nx_azure_iot_hub_client_model_id_set (0x%08x)\r\n", status);
    }

    // Set connection status callback
    else if ((status = nx_azure_iot_hub_client_connection_status_callback_set(
                  &context->iothub_client, connection_status_callback)))
    {
        printf("Error: failed on connection_status_callback (0x%08x)\r\n", status);
    }

    // Enable commands
    else if ((status = nx_azure_iot_hub_client_command_enable(&context->iothub_client)))
    {
        printf("Error: command receive enable failed (0x%08x)\r\n", status);
    }

    // Enable properties
    else if ((status = nx_azure_iot_hub_client_properties_enable(&context->iothub_client)))
    {
        printf("Failed on nx_azure_iot_hub_client_properties_enable!: error code = 0x%08x\r\n", status);
    }

    // Set properties callback
    else if ((status = nx_azure_iot_hub_client_receive_callback_set(&context->iothub_client,
                  NX_AZURE_IOT_HUB_PROPERTIES,
                  message_receive_callback_properties,
                  (VOID*)context)))
    {
        printf("Error: device twin callback set (0x%08x)\r\n", status);
    }

    // Set command callback
    else if ((status = nx_azure_iot_hub_client_receive_callback_set(
                  &context->iothub_client, NX_AZURE_IOT_HUB_COMMAND, message_receive_command, (VOID*)context)))
    {
        printf("Error: device method callback set (0x%08x)\r\n", status);
    }

    // Set the writable property callback
    else if ((status = nx_azure_iot_hub_client_receive_callback_set(&context->iothub_client,
                  NX_AZURE_IOT_HUB_WRITABLE_PROPERTIES,
                  message_receive_callback_writable_property,
                  (VOID*)context)))
    {
        printf("Error: device twin desired property callback set (0x%08x)\r\n", status);
    }

    if (status != NX_AZURE_IOT_SUCCESS)
    {
        nx_azure_iot_hub_client_deinitialize(&context->iothub_client);
    }

    return status;
}

UINT azure_iot_nx_client_register_command_callback(AZURE_IOT_NX_CONTEXT* context, func_ptr_command_received callback)
{
    if (context == NULL || context->command_received_cb != NULL)
    {
        return NX_PTR_ERROR;
    }

    context->command_received_cb = callback;
    return NX_SUCCESS;
}

UINT azure_iot_nx_client_register_writable_property_callback(
    AZURE_IOT_NX_CONTEXT* context, func_ptr_writable_property_received callback)
{
    if (context == NULL || context->writable_property_received_cb != NULL)
    {
        return NX_PTR_ERROR;
    }

    context->writable_property_received_cb = callback;
    return NX_SUCCESS;
}

UINT azure_iot_nx_client_register_property_callback(AZURE_IOT_NX_CONTEXT* context, func_ptr_property_received callback)
{
    if (context == NULL || context->property_received_cb != NULL)
    {
        return NX_PTR_ERROR;
    }

    context->property_received_cb = callback;
    return NX_SUCCESS;
}

UINT azure_iot_nx_client_sas_set(AZURE_IOT_NX_CONTEXT* context, CHAR* device_sas_key)
{
    if (device_sas_key[0] == 0)
    {
        printf("Error: azure_iot_nx_client_sas_set device_sas_key is null\r\n");
        return NX_PTR_ERROR;
    }

    context->azure_iot_auth_mode          = AZURE_IOT_AUTH_MODE_SAS;
    context->azure_iot_device_sas_key     = device_sas_key;
    context->azure_iot_device_sas_key_len = strlen(device_sas_key);

    return NX_SUCCESS;
}

UINT azure_iot_nx_client_cert_set(AZURE_IOT_NX_CONTEXT* context,
    UCHAR* device_x509_cert,
    UINT device_x509_cert_len,
    UCHAR* device_x509_key,
    UINT device_x509_key_len)
{
    UINT status;

    if (device_x509_cert[0] == 0 || device_x509_cert_len == 0 || device_x509_key[0] == 0 || device_x509_key_len == 0)
    {
        printf("Error: azure_iot_nx_client_cert_set cert/key is null\r\n");
        return NX_PTR_ERROR;
    }

    context->azure_iot_auth_mode = AZURE_IOT_AUTH_MODE_CERT;

    // Create the device certificate
    if ((status = nx_secure_x509_certificate_initialize(&context->device_certificate,
             (UCHAR*)device_x509_cert,
             (USHORT)device_x509_cert_len,
             NX_NULL,
             0,
             (UCHAR*)device_x509_key,
             (USHORT)device_x509_key_len,
             NX_SECURE_X509_KEY_TYPE_RSA_PKCS1_DER)))
    {
        printf("Error: Failed on device nx_secure_x509_certificate_initialize!: error code = 0x%08x\r\n", status);
    }

    return NX_SUCCESS;
}

UINT azure_iot_nx_client_create(AZURE_IOT_NX_CONTEXT* context,
    NX_IP* nx_ip,
    NX_PACKET_POOL* nx_pool,
    NX_DNS* nx_dns,
    UINT (*unix_time_callback)(ULONG* unix_time),
    CHAR* iot_model_id)
{
    UINT status;

    if (iot_model_id[0] == 0)
    {
        printf("Error: UINT azure_iot_nx_client_create_new empty device_id or model_id\r\n");
        return NX_PTR_ERROR;
    }

    // Initialise the context
    memset(context, 0, sizeof(AZURE_IOT_NX_CONTEXT));

    // Stash parameters
    context->azure_iot_model_id = iot_model_id;

    if ((status = tx_event_flags_create(&context->events, "nx_client")))
    {
        printf("Error: failed on create nx_client event flags (0x%08x)\r\n", status);
        return status;
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
        printf("Error: failed on nx_azure_iot_create (0x%08x)\r\n", status);
        return status;
    }

    // Initialize CA root certificate
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

    return NX_AZURE_IOT_SUCCESS;
}

UINT azure_iot_nx_client_hub_create(AZURE_IOT_NX_CONTEXT* context, CHAR* iot_hub_hostname, CHAR* iot_device_id)
{
    if (context == NULL)
    {
        printf("Error: context is NULL\r\n");
        return NX_PTR_ERROR;
    }

    // Return error if empty hostname or device id
    if (iot_hub_hostname[0] == 0 || iot_device_id[0] == 0)
    {
        printf("Error: azure_iot_nx_client_hub_create iot_hub_hostname is null\r\n");
        return NX_PTR_ERROR;
    }

    // Stash parameters
    memcpy(context->azure_iot_hub_hostname, iot_hub_hostname, AZURE_IOT_DEVICE_ID_SIZE);
    memcpy(context->azure_iot_device_id, iot_device_id, AZURE_IOT_DEVICE_ID_SIZE);

    return azure_iot_nx_client_hub_create_internal(context);
}

UINT azure_iot_nx_client_dps_create(AZURE_IOT_NX_CONTEXT* context, CHAR* dps_id_scope, CHAR* dps_registration_id)
{
    UINT status;
    CHAR payload[DPS_PAYLOAD_SIZE];
    UINT iot_hub_hostname_len = AZURE_IOT_HOST_NAME_SIZE;
    UINT iot_device_id_len    = AZURE_IOT_DEVICE_ID_SIZE;

    printf("Initializing Azure IoT DPS client\r\n");
    printf("\tDPS endpoint: %s\r\n", AZURE_IOT_DPS_ENDPOINT);
    printf("\tDPS ID scope: %s\r\n", dps_id_scope);
    printf("\tRegistration ID: %s\r\n", dps_registration_id);

    if (context == NULL)
    {
        printf("Error: context is NULL\r\n");
        return NX_PTR_ERROR;
    }

    // Return error if empty credentials
    if (dps_id_scope[0] == 0 || dps_registration_id[0] == 0)
    {
        printf("Error: azure_iot_nx_client_dps_create incorrect parameters\r\n");
        return NX_PTR_ERROR;
    }

    if (snprintf(payload, sizeof(payload), DPS_PAYLOAD, context->azure_iot_model_id) > DPS_PAYLOAD_SIZE - 1)
    {
        printf("Error: insufficient buffer size to create DPS payload\r\n");
        return NX_SIZE_ERROR;
    }

    // Initialize IoT provisioning client
    if ((status = nx_azure_iot_provisioning_client_initialize(&context->dps_client,
             &context->nx_azure_iot,
             (UCHAR*)AZURE_IOT_DPS_ENDPOINT,
             strlen(AZURE_IOT_DPS_ENDPOINT),
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

    // Set credentials
    if (context->azure_iot_auth_mode == AZURE_IOT_AUTH_MODE_SAS)
    {
        // Symmetric (SAS) Key
        if ((status = nx_azure_iot_provisioning_client_symmetric_key_set(&context->dps_client,
                 (UCHAR*)context->azure_iot_device_sas_key,
                 context->azure_iot_device_sas_key_len)))
        {
            printf("Failed on nx_azure_iot_hub_client_symmetric_key_set (0x%08x)\r\n", status);
        }
    }
    else if (context->azure_iot_auth_mode == AZURE_IOT_AUTH_MODE_CERT)
    {
        // X509 Certificate
        if ((status = nx_azure_iot_provisioning_client_device_cert_set(
                 &context->dps_client, &context->device_certificate)))
        {
            printf("Failed on nx_azure_iot_hub_client_device_cert_set! (0x%08x)\r\n", status);
        }
    }

    // Set the payload containing the model Id
    if ((status = nx_azure_iot_provisioning_client_registration_payload_set(
             &context->dps_client, (UCHAR*)payload, strlen(payload))))
    {
        printf("Error: nx_azure_iot_provisioning_client_registration_payload_set (0x%08x\r\n", status);
    }

    // Register device
    else
    {
        while (true)
        {
            status = nx_azure_iot_provisioning_client_register(&context->dps_client, DPS_REGISTER_TIMEOUT_TICKS);
            if (status == NX_AZURE_IOT_PENDING)
            {
                printf("\tPending DPS connection, retrying\r\n");
                continue;
            }

            // Registration complete
            break;
        }
    }

    if (status != NX_AZURE_IOT_SUCCESS)
    {
        printf("\tERROR: nx_azure_iot_provisioning_client_register (0x%08x)\r\n", status);
    }

    // Get Device info
    else if ((status = nx_azure_iot_provisioning_client_iothub_device_info_get(&context->dps_client,
                  (UCHAR*)context->azure_iot_hub_hostname,
                  &iot_hub_hostname_len,
                  (UCHAR*)context->azure_iot_device_id,
                  &iot_device_id_len)))
    {
        printf("Error: nx_azure_iot_provisioning_client_iothub_device_info_get (0x%08x)\r\n", status);
    }

    // Destroy Provisioning Client
    nx_azure_iot_provisioning_client_deinitialize(&context->dps_client);

    if (status != NX_SUCCESS)
    {
        return status;
    }

    // Null terminate returned values
    context->azure_iot_hub_hostname[iot_hub_hostname_len] = 0;
    context->azure_iot_device_id[iot_device_id_len]       = 0;

    printf("SUCCESS: Azure IoT DPS client initialized\r\n\r\n");

    return azure_iot_nx_client_hub_create_internal(context);
}

UINT azure_iot_nx_client_delete(AZURE_IOT_NX_CONTEXT* context)
{
    // Destroy IoTHub Client
    nx_azure_iot_hub_client_disconnect(&context->iothub_client);
    nx_azure_iot_hub_client_deinitialize(&context->iothub_client);

    // Destroy the common object
    nx_azure_iot_delete(&context->nx_azure_iot);

    return NX_SUCCESS;
}

UINT azure_iot_nx_client_connect(AZURE_IOT_NX_CONTEXT* context)
{
    UINT status;

    // Connect to IoTHub client
    if ((status = nx_azure_iot_hub_client_connect(&context->iothub_client, NX_TRUE, HUB_CONNECT_TIMEOUT_TICKS)))
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

UINT azure_iot_nx_client_properties_request_and_wait(AZURE_IOT_NX_CONTEXT* context)
{
    UINT status;
    ULONG app_events = 0;

    // Request the properties
    if ((status = nx_azure_iot_hub_client_properties_request(&context->iothub_client, NX_WAIT_FOREVER)))
    {
        printf("Error: failed to request properties (0x%08x)\r\n", status);
        return status;
    }

    // Wait for the device twin to be processed
    if ((status = tx_event_flags_get(
             &context->events, PROPERTIES_COMPLETE_EVENT, TX_OR_CLEAR, &app_events, 10 * NX_IP_PERIODIC_RATE)))
    {
        printf("Error: failed to execute tx_event_flags_get (0x%08x)\r\n", status);
        return status;
    }

    return NX_SUCCESS;
}

UINT azure_iot_nx_client_publish_telemetry(AZURE_IOT_NX_CONTEXT* context_ptr,
    CHAR* component_name_ptr,
    UINT (*append_properties)(NX_AZURE_IOT_JSON_WRITER* json_builder_ptr))
{
    UINT status;
    UINT telemetry_length;
    NX_PACKET* packet_ptr;
    NX_AZURE_IOT_JSON_WRITER json_writer;

    if ((status = nx_azure_iot_hub_client_telemetry_message_create(
             &context_ptr->iothub_client, &packet_ptr, NX_WAIT_FOREVER)))
    {
        printf("Error: nx_azure_iot_hub_client_telemetry_message_create failed (0x%08x)\r\n", status);
    }

    if ((status = nx_azure_iot_json_writer_with_buffer_init(&json_writer, telemetry_buffer, sizeof(telemetry_buffer))))
    {
        printf("Error: Failed to initialize json writer (0x%08x)\r\n", status);
        nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);
        return status;
    }

    if ((status = nx_azure_iot_json_writer_append_begin_object(&json_writer)) ||
        (component_name_ptr != NX_NULL &&
            (status = nx_azure_iot_hub_client_reported_properties_component_begin(
                 &context_ptr->iothub_client, &json_writer, (UCHAR*)component_name_ptr, strlen(component_name_ptr)))) ||
        (status = append_properties(&json_writer)) ||
        (component_name_ptr != NX_NULL && (status = nx_azure_iot_hub_client_reported_properties_component_end(
                                               &context_ptr->iothub_client, &json_writer))) ||
        (status = nx_azure_iot_json_writer_append_end_object(&json_writer)))
    {
        printf("Error: Failed to build telemetry (0x%08x)\r\n", status);
        nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);
        return status;
    }

    telemetry_length = nx_azure_iot_json_writer_get_bytes_used(&json_writer);
    if ((status = nx_azure_iot_hub_client_telemetry_send(
             &context_ptr->iothub_client, packet_ptr, telemetry_buffer, telemetry_length, NX_WAIT_FOREVER)))
    {
        printf("Error: Telemetry message send failed (0x%08x)\r\n", status);
        nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);
        return status;
    }

    printf("Telemetry message sent: %.*s.\r\n", telemetry_length, telemetry_buffer);

    return status;
}

UINT azure_iot_nx_client_publish_properties(AZURE_IOT_NX_CONTEXT* context_ptr,
    CHAR* component_name_ptr,
    UINT (*append_properties)(NX_AZURE_IOT_JSON_WRITER* json_writer_ptr))
{
    UINT status;
    UINT response_status = 0;
    NX_PACKET* packet_ptr;
    NX_AZURE_IOT_JSON_WRITER json_writer;

    if ((status = nx_azure_iot_hub_client_reported_properties_create(
             &context_ptr->iothub_client, &packet_ptr, NX_WAIT_FOREVER)))
    {
        printf("Error: Failed create reported properties (0x%08x)\r\n", status);
        return status;
    }

    if ((status = nx_azure_iot_json_writer_init(&json_writer, packet_ptr, NX_WAIT_FOREVER)))
    {
        printf("Error: Failed to initialize json writer (0x%08x)\r\n", status);
        nx_packet_release(packet_ptr);
        return status;
    }

    if ((status = nx_azure_iot_json_writer_append_begin_object(&json_writer)) ||
        (component_name_ptr != NX_NULL &&
            (status = nx_azure_iot_hub_client_reported_properties_component_begin(
                 &context_ptr->iothub_client, &json_writer, (UCHAR*)component_name_ptr, strlen(component_name_ptr)))) ||
        (status = append_properties(&json_writer)) ||
        (component_name_ptr != NX_NULL && (status = nx_azure_iot_hub_client_reported_properties_component_end(
                                               &context_ptr->iothub_client, &json_writer))) ||
        (status = nx_azure_iot_json_writer_append_end_object(&json_writer)))
    {
        printf("Error: Failed to build reported property (0x%08x)\r\n", status);
        nx_packet_release(packet_ptr);
        return status;
    }

    printf_packet("Sending property: ", packet_ptr);

    if ((status = nx_azure_iot_hub_client_reported_properties_send(
             &context_ptr->iothub_client, packet_ptr, NX_NULL, &response_status, NX_NULL, (5 * NX_IP_PERIODIC_RATE))))
    {
        printf("Error: nx_azure_iot_hub_client_reported_properties_send failed (0x%08x)\r\n", status);
        nx_packet_release(packet_ptr);
        return status;
    }

    if ((response_status < 200) || (response_status >= 300))
    {
        printf("Error: Property sent response status failed (%d)\r\n", response_status);
        return NX_NOT_SUCCESSFUL;
    }

    return NX_SUCCESS;
}

UINT azure_iot_nx_client_publish_bool_property(AZURE_IOT_NX_CONTEXT* context, CHAR* property, bool value)
{
    UINT status;
    UINT response_status;
    UINT request_id;
    ULONG reported_property_version;
    NX_AZURE_IOT_JSON_WRITER json_writer;
    NX_PACKET* packet_ptr;

    if ((status = nx_azure_iot_hub_client_reported_properties_create(
             &context->iothub_client, &packet_ptr, NX_WAIT_FOREVER)))
    {
        printf("Error: Failed create reported properties (0x%08x)\r\n", status);
        return status;
    }

    if ((status = nx_azure_iot_json_writer_init(&json_writer, packet_ptr, NX_WAIT_FOREVER)))
    {
        printf("Error: Failed to initialize json writer (0x%08x)\r\n", status);
        nx_packet_release(packet_ptr);
        return status;
    }

    if ((status = nx_azure_iot_json_writer_append_begin_object(&json_writer)) ||
        (status = nx_azure_iot_json_writer_append_property_with_bool_value(
             &json_writer, (const UCHAR*)property, strlen(property), value)) ||
        (status = nx_azure_iot_json_writer_append_end_object(&json_writer)))
    {
        printf("Error: Failed to build bool property (0x%08x)\r\n", status);
        nx_packet_release(packet_ptr);
        return status;
    }

    printf_packet("Sending property: ", packet_ptr);

    if ((status = nx_azure_iot_hub_client_reported_properties_send(&context->iothub_client,
             packet_ptr,
             &request_id,
             &response_status,
             &reported_property_version,
             5 * NX_IP_PERIODIC_RATE)))
    {
        printf("Error: nx_azure_iot_hub_client_reported_properties_send failed (0x%08x)\r\n", status);
        nx_packet_release(packet_ptr);
        return status;
    }

    if ((response_status < 200) || (response_status >= 300))
    {
        printf("Error: device twin report properties failed (%d)\r\n", response_status);
        return status;
    }

    return NX_SUCCESS;
}

UINT azure_nx_client_respond_int_writable_property(
    AZURE_IOT_NX_CONTEXT* context, CHAR* property, INT value, INT http_status, INT version)
{
    UINT status;
    UINT response_status;
    UINT request_id;
    NX_AZURE_IOT_JSON_WRITER json_writer;
    NX_PACKET* packet_ptr;

    if ((status = nx_azure_iot_hub_client_reported_properties_create(
             &context->iothub_client, &packet_ptr, NX_WAIT_FOREVER)))
    {
        printf("Error: Failed create reported properties (0x%08x)\r\n", status);
        return status;
    }

    if ((status = nx_azure_iot_json_writer_init(&json_writer, packet_ptr, NX_WAIT_FOREVER)))
    {
        printf("Error: Failed to initialize json writer (0x%08x)\r\n", status);
        nx_packet_release(packet_ptr);
        return status;
    }

    if ((status = nx_azure_iot_json_writer_append_begin_object(&json_writer)) ||
        (status = nx_azure_iot_hub_client_reported_properties_status_begin(&context->iothub_client,
             &json_writer,
             (const UCHAR*)property,
             strlen(property),
             http_status,
             version,
             NULL,
             0)) ||
        (status = nx_azure_iot_json_writer_append_int32(&json_writer, value)) ||
        (status = nx_azure_iot_hub_client_reported_properties_status_end(&context->iothub_client, &json_writer)) ||
        (status = nx_azure_iot_json_writer_append_end_object(&json_writer)))
    {
        printf("Error: Failed to build uint writable property response (0x%08x)\r\n", status);
        nx_packet_release(packet_ptr);
        return status;
    }

    printf_packet("Sending writable property: ", packet_ptr);

    if (nx_azure_iot_hub_client_reported_properties_send(
            &context->iothub_client, packet_ptr, &request_id, &response_status, NX_NULL, 5 * NX_IP_PERIODIC_RATE))
    {
        printf("Error: nx_azure_iot_hub_client_reported_properties_send failed (0x%08x)\r\n", status);
        nx_packet_release(packet_ptr);
        return status;
    }

    if ((response_status < 200) || (response_status >= 300))
    {
        printf("Error: device twin report properties failed (%d)\r\n", response_status);
        return status;
    }

    return NX_SUCCESS;
}

UINT azure_iot_nx_client_publish_int_writable_property(AZURE_IOT_NX_CONTEXT* context, CHAR* property, UINT value)
{
    return azure_nx_client_respond_int_writable_property(context, property, value, 200, 1);
}
