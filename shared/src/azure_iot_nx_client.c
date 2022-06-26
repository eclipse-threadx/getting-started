/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "azure_iot_nx_client.h"

#include <stdio.h>

#include "nx_azure_iot_hub_client.h"
#include "nx_azure_iot_hub_client_properties.h"

#include "azure_iot_cert.h"
#include "azure_iot_ciphersuites.h"
#include "azure_iot_connect.h"

#define NX_AZURE_IOT_THREAD_PRIORITY 4

// Incoming events from the middleware
#define HUB_ALL_EVENTS                        0xFF
#define HUB_CONNECT_EVENT                     0x01
#define HUB_DISCONNECT_EVENT                  0x02
#define HUB_COMMAND_RECEIVE_EVENT             0x04
#define HUB_PROPERTIES_RECEIVE_EVENT          0x08
#define HUB_WRITABLE_PROPERTIES_RECEIVE_EVENT 0x10
#define HUB_PROPERTIES_COMPLETE_EVENT         0x20
#define HUB_PERIODIC_TIMER_EVENT              0x40

#define AZURE_IOT_DPS_ENDPOINT "global.azure-devices-provisioning.net"

#define MODULE_ID   ""
#define DPS_PAYLOAD "{\"modelId\":\"%s\"}"

// Connection timeouts in threadx ticks
#define HUB_CONNECT_TIMEOUT_TICKS  (10 * TX_TIMER_TICKS_PER_SECOND)
#define DPS_REGISTER_TIMEOUT_TICKS (30 * TX_TIMER_TICKS_PER_SECOND)

#define DPS_PAYLOAD_SIZE       (15 + 128)
#define TELEMETRY_BUFFER_SIZE  256
#define PROPERTIES_BUFFER_SIZE 128

// define static strings for content type and -encoding on message property bag
static const UCHAR content_type_property[]     = "$.ct";
static const UCHAR content_encoding_property[] = "$.ce";
static const UCHAR content_type_json[]         = "application%2Fjson";
static const UCHAR content_encoding_utf8[]     = "utf-8";

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

static VOID connection_status_callback(NX_AZURE_IOT_HUB_CLIENT* hub_client_ptr, UINT status)
{
    // :HACK: This callback doesn't allow us to provide context, pinch it from the command message callback args
    AZURE_IOT_NX_CONTEXT* nx_context = hub_client_ptr->nx_azure_iot_hub_client_command_message.message_callback_args;

    if (status == NX_SUCCESS)
    {
        tx_event_flags_set(&nx_context->events, HUB_CONNECT_EVENT, TX_OR);
    }
    else
    {
        tx_event_flags_set(&nx_context->events, HUB_DISCONNECT_EVENT, TX_OR);
    }

    // update the connection status in the connect workflow
    connection_status_set(nx_context, status);
}

static VOID message_receive_command(NX_AZURE_IOT_HUB_CLIENT* hub_client_ptr, VOID* context)
{
    AZURE_IOT_NX_CONTEXT* nx_context = (AZURE_IOT_NX_CONTEXT*)context;
    tx_event_flags_set(&nx_context->events, HUB_COMMAND_RECEIVE_EVENT, TX_OR);
}

static VOID message_receive_callback_properties(NX_AZURE_IOT_HUB_CLIENT* hub_client_ptr, VOID* context)
{
    AZURE_IOT_NX_CONTEXT* nx_context = (AZURE_IOT_NX_CONTEXT*)context;
    tx_event_flags_set(&nx_context->events, HUB_PROPERTIES_RECEIVE_EVENT, TX_OR);
}

static VOID message_receive_callback_writable_property(NX_AZURE_IOT_HUB_CLIENT* hub_client_ptr, VOID* context)
{
    AZURE_IOT_NX_CONTEXT* nx_context = (AZURE_IOT_NX_CONTEXT*)context;
    tx_event_flags_set(&nx_context->events, HUB_WRITABLE_PROPERTIES_RECEIVE_EVENT, TX_OR);
}

static VOID periodic_timer_entry(ULONG context)
{
    AZURE_IOT_NX_CONTEXT* nx_context = (AZURE_IOT_NX_CONTEXT*)context;
    tx_event_flags_set(&nx_context->events, HUB_PERIODIC_TIMER_EVENT, TX_OR);
}

static UINT iot_hub_initialize(AZURE_IOT_NX_CONTEXT* nx_context)
{
    UINT status;

    // Initialize IoT Hub client.
    if ((status = nx_azure_iot_hub_client_initialize(&nx_context->iothub_client,
             &nx_context->nx_azure_iot,
             (UCHAR*)nx_context->azure_iot_hub_hostname,
             nx_context->azure_iot_hub_hostname_len,
             (UCHAR*)nx_context->azure_iot_hub_device_id,
             nx_context->azure_iot_hub_device_id_len,
             (UCHAR*)MODULE_ID,
             sizeof(MODULE_ID) - 1,
             _nx_azure_iot_tls_supported_crypto,
             _nx_azure_iot_tls_supported_crypto_size,
             _nx_azure_iot_tls_ciphersuite_map,
             _nx_azure_iot_tls_ciphersuite_map_size,
             (UCHAR*)nx_context->nx_azure_iot_tls_metadata_buffer,
             sizeof(nx_context->nx_azure_iot_tls_metadata_buffer),
             &nx_context->root_ca_cert)))
    {
        printf("Error: on nx_azure_iot_hub_client_initialize (0x%08x)\r\n", status);
        return status;
    }

    // Set credentials
    if (nx_context->azure_iot_auth_mode == AZURE_IOT_AUTH_MODE_SAS)
    {
        // Symmetric (SAS) Key
        if ((status = nx_azure_iot_hub_client_symmetric_key_set(&nx_context->iothub_client,
                 (UCHAR*)nx_context->azure_iot_device_sas_key,
                 nx_context->azure_iot_device_sas_key_len)))
        {
            printf("Error: failed on nx_azure_iot_hub_client_symmetric_key_set (0x%08x)\r\n", status);
        }
    }
    else if (nx_context->azure_iot_auth_mode == AZURE_IOT_AUTH_MODE_CERT)
    {
        // X509 Certificate
        if ((status = nx_azure_iot_hub_client_device_cert_set(
                 &nx_context->iothub_client, &nx_context->device_certificate)))
        {
            printf("Error: failed on nx_azure_iot_hub_client_device_cert_set!: error code = 0x%08x\r\n", status);
        }
    }

    if (status != NX_AZURE_IOT_SUCCESS)
    {
        printf("Failed to set auth credentials\r\n");
    }

    // Add more CA certificates
    else if ((status =
                     nx_azure_iot_hub_client_trusted_cert_add(&nx_context->iothub_client, &nx_context->root_ca_cert_2)))
    {
        printf("Failed on nx_azure_iot_hub_client_trusted_cert_add!: error code = 0x%08x\r\n", status);
    }
    else if ((status =
                     nx_azure_iot_hub_client_trusted_cert_add(&nx_context->iothub_client, &nx_context->root_ca_cert_3)))
    {
        printf("Failed on nx_azure_iot_hub_client_trusted_cert_add!: error code = 0x%08x\r\n", status);
    }

    // Set Model id
    else if ((status = nx_azure_iot_hub_client_model_id_set(&nx_context->iothub_client,
                  (UCHAR*)nx_context->azure_iot_model_id,
                  nx_context->azure_iot_model_id_len)))
    {
        printf("Error: nx_azure_iot_hub_client_model_id_set (0x%08x)\r\n", status);
    }

    // Set connection status callback
    else if ((status = nx_azure_iot_hub_client_connection_status_callback_set(
                  &nx_context->iothub_client, connection_status_callback)))
    {
        printf("Error: failed on connection_status_callback (0x%08x)\r\n", status);
    }

    // Enable commands
    else if ((status = nx_azure_iot_hub_client_command_enable(&nx_context->iothub_client)))
    {
        printf("Error: command receive enable failed (0x%08x)\r\n", status);
    }

    // Enable properties
    else if ((status = nx_azure_iot_hub_client_properties_enable(&nx_context->iothub_client)))
    {
        printf("Failed on nx_azure_iot_hub_client_properties_enable!: error code = 0x%08x\r\n", status);
    }

    // Set properties callback
    else if ((status = nx_azure_iot_hub_client_receive_callback_set(&nx_context->iothub_client,
                  NX_AZURE_IOT_HUB_PROPERTIES,
                  message_receive_callback_properties,
                  (VOID*)nx_context)))
    {
        printf("Error: device twin callback set (0x%08x)\r\n", status);
    }

    // Set command callback
    else if ((status = nx_azure_iot_hub_client_receive_callback_set(
                  &nx_context->iothub_client, NX_AZURE_IOT_HUB_COMMAND, message_receive_command, (VOID*)nx_context)))
    {
        printf("Error: device method callback set (0x%08x)\r\n", status);
    }

    // Set the writable property callback
    else if ((status = nx_azure_iot_hub_client_receive_callback_set(&nx_context->iothub_client,
                  NX_AZURE_IOT_HUB_WRITABLE_PROPERTIES,
                  message_receive_callback_writable_property,
                  (VOID*)nx_context)))
    {
        printf("Error: device twin desired property callback set (0x%08x)\r\n", status);
    }

    // Register the pnp components for receiving
    for (int i = 0; i < nx_context->azure_iot_component_count; ++i)
    {
        if ((status = nx_azure_iot_hub_client_component_add(&nx_context->iothub_client,
                 (UCHAR*)nx_context->azure_iot_components[i],
                 strlen(nx_context->azure_iot_components[i]))))
        {
            printf("ERROR: nx_azure_iot_hub_client_component_add failed (0x%08x)\r\n", status);
            break;
        }
    }

    if (status != NX_AZURE_IOT_SUCCESS)
    {
        nx_azure_iot_hub_client_deinitialize(&nx_context->iothub_client);
    }

    return status;
}

static UINT dps_initialize(AZURE_IOT_NX_CONTEXT* nx_context)
{
    UINT status;
    CHAR payload[DPS_PAYLOAD_SIZE];

    if (nx_context == NULL)
    {
        printf("ERROR: context is NULL\r\n");
        return NX_PTR_ERROR;
    }

    // Return error if empty credentials
    if (nx_context->azure_iot_dps_id_scope_len == 0 || nx_context->azure_iot_dps_registration_id_len == 0)
    {
        printf("ERROR: azure_iot_nx_client_dps_entry incorrect parameters\r\n");
        return NX_PTR_ERROR;
    }

    printf("\r\nInitializing Azure IoT DPS client\r\n");
    printf("\tDPS endpoint: %s\r\n", AZURE_IOT_DPS_ENDPOINT);
    printf("\tDPS ID scope: %.*s\r\n", nx_context->azure_iot_dps_id_scope_len, nx_context->azure_iot_dps_id_scope);
    printf("\tRegistration ID: %.*s\r\n",
        nx_context->azure_iot_dps_registration_id_len,
        nx_context->azure_iot_dps_registration_id);

    // Initialise the length of the return buffers
    nx_context->azure_iot_hub_hostname_len  = sizeof(nx_context->azure_iot_hub_hostname);
    nx_context->azure_iot_hub_device_id_len = sizeof(nx_context->azure_iot_hub_device_id);

    if (snprintf(payload, sizeof(payload), DPS_PAYLOAD, nx_context->azure_iot_model_id) > DPS_PAYLOAD_SIZE - 1)
    {
        printf("ERROR: insufficient buffer size to create DPS payload\r\n");
        return NX_SIZE_ERROR;
    }

    // Initialize IoT provisioning client
    if ((status = nx_azure_iot_provisioning_client_initialize(&nx_context->dps_client,
             &nx_context->nx_azure_iot,
             (UCHAR*)AZURE_IOT_DPS_ENDPOINT,
             strlen(AZURE_IOT_DPS_ENDPOINT),
             (UCHAR*)nx_context->azure_iot_dps_id_scope,
             nx_context->azure_iot_dps_id_scope_len,
             (UCHAR*)nx_context->azure_iot_dps_registration_id,
             nx_context->azure_iot_dps_registration_id_len,
             _nx_azure_iot_tls_supported_crypto,
             _nx_azure_iot_tls_supported_crypto_size,
             _nx_azure_iot_tls_ciphersuite_map,
             _nx_azure_iot_tls_ciphersuite_map_size,
             (UCHAR*)nx_context->nx_azure_iot_tls_metadata_buffer,
             sizeof(nx_context->nx_azure_iot_tls_metadata_buffer),
             &nx_context->root_ca_cert)))
    {
        printf("ERROR: nx_azure_iot_provisioning_client_initialize (0x%08x)\r\n", status);
        return status;
    }

    // Add more CA certificates
    else if ((status = nx_azure_iot_provisioning_client_trusted_cert_add(
                  &nx_context->dps_client, &nx_context->root_ca_cert_2)))
    {
        printf("ERROR: nx_azure_iot_provisioning_client_trusted_cert_add!: error code = 0x%08x\r\n", status);
    }
    else if ((status = nx_azure_iot_provisioning_client_trusted_cert_add(
                  &nx_context->dps_client, &nx_context->root_ca_cert_3)))
    {
        printf("ERROR: nx_azure_iot_provisioning_client_trusted_cert_add!: error code = 0x%08x\r\n", status);
    }

    else
    {
        switch (nx_context->azure_iot_auth_mode)
        {
            // Symmetric (SAS) Key
            case AZURE_IOT_AUTH_MODE_SAS:
                if ((status = nx_azure_iot_provisioning_client_symmetric_key_set(&nx_context->dps_client,
                         (UCHAR*)nx_context->azure_iot_device_sas_key,
                         nx_context->azure_iot_device_sas_key_len)))
                {
                    printf("ERROR: nx_azure_iot_provisioning_client_symmetric_key_set (0x%08x)\r\n", status);
                }
                break;

            // X509 Certificate
            case AZURE_IOT_AUTH_MODE_CERT:
                if ((status = nx_azure_iot_provisioning_client_device_cert_set(
                         &nx_context->dps_client, &nx_context->device_certificate)))
                {
                    printf("ERROR: nx_azure_iot_provisioning_client_device_cert_set (0x%08x)\r\n", status);
                }
                break;
        }
    }

    if (status != NX_AZURE_IOT_SUCCESS)
    {
        printf("ERROR: failed to set initialize DPS\r\n");
    }

    // Set the payload containing the model Id
    else if ((status = nx_azure_iot_provisioning_client_registration_payload_set(
                  &nx_context->dps_client, (UCHAR*)payload, strlen(payload))))
    {
        printf("ERROR: nx_azure_iot_provisioning_client_registration_payload_set (0x%08x\r\n", status);
    }

    else if ((status = nx_azure_iot_provisioning_client_register(&nx_context->dps_client, DPS_REGISTER_TIMEOUT_TICKS)))
    {
        printf("\tERROR: nx_azure_iot_provisioning_client_register (0x%08x)\r\n", status);
    }

    // Stash IoT Hub Device info
    else if ((status = nx_azure_iot_provisioning_client_iothub_device_info_get(&nx_context->dps_client,
                  (UCHAR*)nx_context->azure_iot_hub_hostname,
                  &nx_context->azure_iot_hub_hostname_len,
                  (UCHAR*)nx_context->azure_iot_hub_device_id,
                  &nx_context->azure_iot_hub_device_id_len)))
    {
        printf("ERROR: nx_azure_iot_provisioning_client_iothub_device_info_get (0x%08x)\r\n", status);
    }

    // Destroy Provisioning Client
    nx_azure_iot_provisioning_client_deinitialize(&nx_context->dps_client);

    if (status != NX_SUCCESS)
    {
        return status;
    }

    printf("SUCCESS: Azure IoT DPS client initialized\r\n");

    return iot_hub_initialize(nx_context);
}

static VOID process_connect(AZURE_IOT_NX_CONTEXT* nx_context)
{
    UINT status;

    // Request the client properties
    if ((status = nx_azure_iot_hub_client_properties_request(&nx_context->iothub_client, NX_WAIT_FOREVER)))
    {
        printf("ERROR: failed to request properties (0x%08x)\r\n", status);
    }

    // Start the periodic timer
    if ((status = tx_timer_activate(&nx_context->periodic_timer)))
    {
        printf("ERROR: tx_timer_activate (0x%08x)\r\n", status);
    }
}

static VOID process_disconnect(AZURE_IOT_NX_CONTEXT* nx_context)
{
    UINT status;

    printf("Disconnected from IoT Hub\r\n");

    // Stop the periodic timer
    if ((status = tx_timer_deactivate(&nx_context->periodic_timer)))
    {
        printf("ERROR: tx_timer_deactivate (0x%08x)\r\n", status);
    }
}

static VOID process_properties_complete(AZURE_IOT_NX_CONTEXT* nx_context)
{
    if (nx_context->properties_complete_cb)
    {
        nx_context->properties_complete_cb(nx_context);
    }
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
                component_name_ptr,
                component_name_length,
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

    // reinitialize the json reader after reading the version to reset
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
        printf("ERROR: nx_azure_iot_hub_client_properties_receive failed (0x%08x)\r\n", status);
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
    tx_event_flags_set(&nx_context->events, HUB_PROPERTIES_COMPLETE_EVENT, TX_OR);
}

static VOID process_writable_properties(AZURE_IOT_NX_CONTEXT* nx_context)
{
    UINT status;
    NX_PACKET* packet_ptr;

    if ((status = nx_azure_iot_hub_client_writable_properties_receive(
             &nx_context->iothub_client, &packet_ptr, NX_WAIT_FOREVER)))
    {
        printf("ERROR: nx_azure_iot_hub_client_writable_properties_receive (0x%08x)\r\n", status);
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
            printf("ERROR: failed to parse properties (0x%08x)\r\n", status);
        }
    }

    // Release the received packet, as ownership was passed to the application from the middleware
    nx_packet_release(packet_ptr);
}

static VOID process_timer_event(AZURE_IOT_NX_CONTEXT* nx_context)
{
    if (nx_context->timer_cb)
    {
        nx_context->timer_cb(nx_context);
    }
}

UINT azure_nx_client_periodic_interval_set(AZURE_IOT_NX_CONTEXT* nx_context, INT interval)
{
    UINT status;
    UINT active;
    UINT ticks = interval * TX_TIMER_TICKS_PER_SECOND;

    if ((status = tx_timer_info_get(&nx_context->periodic_timer, NULL, &active, NULL, NULL, NULL)))
    {
        printf("ERROR: tx_timer_deactivate (0x%08x)\r\n", status);
        return status;
    }

    if (active == TX_TRUE && (status = tx_timer_deactivate(&nx_context->periodic_timer)))
    {
        printf("ERROR: tx_timer_deactivate (0x%08x)\r\n", status);
    }

    else if ((status = tx_timer_change(&nx_context->periodic_timer, ticks, ticks)))
    {
        printf("ERROR: tx_timer_change (0x%08x)\r\n", status);
    }

    else if (active == TX_TRUE && (status = tx_timer_activate(&nx_context->periodic_timer)))
    {
        printf("ERROR: tx_timer_activate (0x%08x)\r\n", status);
    }

    return status;
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

    if (component_name_ptr != NX_NULL)
    {
        printf("appending component name: %s\r\n", component_name_ptr);
        if ((status = nx_azure_iot_hub_client_telemetry_component_set(
                 packet_ptr, (UCHAR*)component_name_ptr, strlen(component_name_ptr), NX_WAIT_FOREVER)))
        {
            printf("Error: nx_azure_iot_hub_client_telemetry_component_set failed (0x%08x)\r\n", status);
            nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);
            return status;
        }
    }

    if ((status = nx_azure_iot_json_writer_with_buffer_init(&json_writer, telemetry_buffer, sizeof(telemetry_buffer))))
    {
        printf("Error: Failed to initialize json writer (0x%08x)\r\n", status);
        nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);
        return status;
    }

    if ((status = nx_azure_iot_json_writer_append_begin_object(&json_writer)) ||
        (status = append_properties(&json_writer)) ||
        (status = nx_azure_iot_json_writer_append_end_object(&json_writer)))
    {
        printf("Error: Failed to build telemetry (0x%08x)\r\n", status);
        nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);
        return status;
    }

    // set the ContentType property on the message to "application/json" (url-encoded)
    if ((status = nx_azure_iot_hub_client_telemetry_property_add(packet_ptr,
             content_type_property,
             sizeof(content_type_property) - 1,
             content_type_json,
             sizeof(content_type_json) - 1,
             NX_WAIT_FOREVER)))
    {
        printf("Error: Cant set ContentType message property (0x%08X)\r\n", status);
        nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);
        return status;
    }

    // set the ContentEncoding property on the message to "utf-8"
    if ((status = nx_azure_iot_hub_client_telemetry_property_add(packet_ptr,
             content_encoding_property,
             sizeof(content_encoding_property) - 1,
             content_encoding_utf8,
             sizeof(content_encoding_utf8) - 1,
             NX_WAIT_FOREVER)))
    {
        printf("Error: Cant set ContentEncoding message property (0x%08X)\r\n", status);
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

static UINT reported_properties_begin(AZURE_IOT_NX_CONTEXT* context_ptr,
    NX_AZURE_IOT_JSON_WRITER* json_writer,
    NX_PACKET** packet_ptr,
    CHAR* component_name_ptr)
{
    UINT status;

    if ((status = nx_azure_iot_hub_client_reported_properties_create(
             &context_ptr->iothub_client, packet_ptr, NX_WAIT_FOREVER)))
    {
        printf("Error: Failed create reported properties (0x%08x)\r\n", status);
    }

    else if ((status = nx_azure_iot_json_writer_init(json_writer, *packet_ptr, NX_WAIT_FOREVER)))
    {
        printf("Error: Failed to initialize json writer (0x%08x)\r\n", status);
    }

    else if ((status = nx_azure_iot_json_writer_append_begin_object(json_writer)))
    {
        printf("Error: Failed to append object begin (0x%08x)\r\n", status);
    }

    else if (component_name_ptr != NX_NULL &&
             (status = nx_azure_iot_hub_client_reported_properties_component_begin(
                  &context_ptr->iothub_client, json_writer, (UCHAR*)component_name_ptr, strlen(component_name_ptr))))
    {
        printf("Error: Failed to append component begin (0x%08x)\r\n", status);
    }

    return status;
}

static UINT reported_properties_end(AZURE_IOT_NX_CONTEXT* nx_context,
    NX_AZURE_IOT_JSON_WRITER* json_writer,
    NX_PACKET** packet_ptr,
    CHAR* component_name_ptr)
{
    UINT status;
    UINT response_status = 0;

    if ((component_name_ptr != NX_NULL && (status = nx_azure_iot_hub_client_reported_properties_component_end(
                                               &nx_context->iothub_client, json_writer))))
    {
        printf("Error: Failed to append component end (0x%08x)\r\n", status);
        return status;
    }

    if ((status = nx_azure_iot_json_writer_append_end_object(json_writer)))
    {
        printf("Error: Failed to append object end (0x%08x)\r\n", status);
        return status;
    }

    printf_packet("Sending property: ", *packet_ptr);

    if ((status = nx_azure_iot_hub_client_reported_properties_send(
             &nx_context->iothub_client, *packet_ptr, NX_NULL, &response_status, NX_NULL, 5 * NX_IP_PERIODIC_RATE)))
    {
        printf("Error: nx_azure_iot_hub_client_reported_properties_send failed (0x%08x)\r\n", status);
        return status;
    }

    else if ((response_status < 200) || (response_status >= 300))
    {
        printf("Error: Property sent response status failed (%d)\r\n", response_status);
        return NX_NOT_SUCCESSFUL;
    }

    return NX_SUCCESS;
}

UINT azure_iot_nx_client_publish_properties(AZURE_IOT_NX_CONTEXT* nx_context,
    CHAR* component_name_ptr,
    UINT (*append_properties)(NX_AZURE_IOT_JSON_WRITER* json_writer_ptr))
{
    UINT status;
    NX_PACKET* packet_ptr;
    NX_AZURE_IOT_JSON_WRITER json_writer;

    if ((status = reported_properties_begin(nx_context, &json_writer, &packet_ptr, component_name_ptr)) ||

        (status = append_properties(&json_writer)) ||

        (status = reported_properties_end(nx_context, &json_writer, &packet_ptr, component_name_ptr)))
    {
        printf("ERROR: azure_iot_nx_client_publish_properties (0x%08x)", status);
        nx_packet_release(packet_ptr);
    }

    return status;
}

UINT azure_iot_nx_client_publish_bool_property(
    AZURE_IOT_NX_CONTEXT* nx_context, CHAR* component_name_ptr, CHAR* property_ptr, bool value)
{
    UINT status;
    NX_AZURE_IOT_JSON_WRITER json_writer;
    NX_PACKET* packet_ptr;

    if ((status = reported_properties_begin(nx_context, &json_writer, &packet_ptr, component_name_ptr)) ||

        (status = nx_azure_iot_json_writer_append_property_with_bool_value(
             &json_writer, (const UCHAR*)property_ptr, strlen(property_ptr), value)) ||

        (status = reported_properties_end(nx_context, &json_writer, &packet_ptr, component_name_ptr)))
    {
        printf("ERROR: azure_iot_nx_client_publish_bool_property (0x%08x)", status);
        nx_packet_release(packet_ptr);
    }

    return status;
}

UINT azure_nx_client_respond_int_writable_property(AZURE_IOT_NX_CONTEXT* nx_context,
    CHAR* component_name_ptr,
    CHAR* property_ptr,
    INT value,
    INT http_status,
    INT version)
{
    UINT status;
    NX_AZURE_IOT_JSON_WRITER json_writer;
    NX_PACKET* packet_ptr;

    if ((status = reported_properties_begin(nx_context, &json_writer, &packet_ptr, component_name_ptr)) ||

        (status = nx_azure_iot_hub_client_reported_properties_status_begin(&nx_context->iothub_client,
             &json_writer,
             (const UCHAR*)property_ptr,
             strlen(property_ptr),
             http_status,
             version,
             NULL,
             0)) ||

        (status = nx_azure_iot_json_writer_append_int32(&json_writer, value)) ||

        (status = nx_azure_iot_hub_client_reported_properties_status_end(&nx_context->iothub_client, &json_writer)) ||

        (status = reported_properties_end(nx_context, &json_writer, &packet_ptr, component_name_ptr)))
    {
        printf("ERROR: azure_nx_client_respond_int_writable_property (0x%08x)", status);
        nx_packet_release(packet_ptr);
    }

    return status;
}

UINT azure_iot_nx_client_publish_int_writable_property(
    AZURE_IOT_NX_CONTEXT* nx_context, CHAR* component_ptr, CHAR* property_ptr, UINT value)
{
    // Pass in a version of 1, as we a reporting the writable property, not responding to a server request
    return azure_nx_client_respond_int_writable_property(nx_context, component_ptr, property_ptr, value, 200, 1);
}

UINT azure_iot_nx_client_register_command_callback(AZURE_IOT_NX_CONTEXT* nx_context, func_ptr_command_received callback)
{
    if (nx_context == NULL || nx_context->command_received_cb != NULL)
    {
        return NX_PTR_ERROR;
    }

    nx_context->command_received_cb = callback;
    return NX_SUCCESS;
}

UINT azure_iot_nx_client_register_writable_property_callback(
    AZURE_IOT_NX_CONTEXT* nx_context, func_ptr_writable_property_received callback)
{
    if (nx_context == NULL || nx_context->writable_property_received_cb != NULL)
    {
        return NX_PTR_ERROR;
    }

    nx_context->writable_property_received_cb = callback;
    return NX_SUCCESS;
}

UINT azure_iot_nx_client_register_property_callback(
    AZURE_IOT_NX_CONTEXT* nx_context, func_ptr_property_received callback)
{
    if (nx_context == NULL || nx_context->property_received_cb != NULL)
    {
        return NX_PTR_ERROR;
    }

    nx_context->property_received_cb = callback;
    return NX_SUCCESS;
}

UINT azure_iot_nx_client_register_properties_complete_callback(
    AZURE_IOT_NX_CONTEXT* nx_context, func_ptr_properties_complete callback)
{
    if (nx_context == NULL || nx_context->properties_complete_cb != NULL)
    {
        return NX_PTR_ERROR;
    }

    nx_context->properties_complete_cb = callback;
    return NX_SUCCESS;
}

UINT azure_iot_nx_client_register_timer_callback(
    AZURE_IOT_NX_CONTEXT* nx_context, func_ptr_timer callback, int32_t interval)
{
    if (nx_context == NULL || nx_context->timer_cb != NULL)
    {
        return NX_PTR_ERROR;
    }

    azure_nx_client_periodic_interval_set(nx_context, interval);

    nx_context->timer_cb = callback;

    return NX_SUCCESS;
}

UINT azure_iot_nx_client_add_component(AZURE_IOT_NX_CONTEXT* nx_context, CHAR* component_name)
{
    if (nx_context == NULL || component_name == NULL)
    {
        return NX_PTR_ERROR;
    }

    if (nx_context->azure_iot_component_count >= NX_AZURE_IOT_HUB_CLIENT_MAX_COMPONENT_LIST)
    {
        return NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE;
    }

    nx_context->azure_iot_components[nx_context->azure_iot_component_count] = component_name;
    nx_context->azure_iot_component_count++;
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

UINT azure_iot_nx_client_cert_set(AZURE_IOT_NX_CONTEXT* nx_context,
    UCHAR* device_x509_cert,
    UINT device_x509_cert_len,
    UCHAR* device_x509_key,
    UINT device_x509_key_len)
{
    UINT status;

    if (device_x509_cert_len == 0 || device_x509_key_len == 0)
    {
        printf("ERROR: azure_iot_nx_client_cert_set cert/key is null\r\n");
        return NX_PTR_ERROR;
    }

    nx_context->azure_iot_auth_mode = AZURE_IOT_AUTH_MODE_CERT;

    // Create the device certificate
    if ((status = nx_secure_x509_certificate_initialize(&nx_context->device_certificate,
             (UCHAR*)device_x509_cert,
             (USHORT)device_x509_cert_len,
             NX_NULL,
             0,
             (UCHAR*)device_x509_key,
             (USHORT)device_x509_key_len,
             NX_SECURE_X509_KEY_TYPE_RSA_PKCS1_DER)))
    {
        printf("ERROR: nx_secure_x509_certificate_initialize (0x%08x)\r\n", status);
    }

    return NX_SUCCESS;
}

UINT azure_iot_nx_client_create(AZURE_IOT_NX_CONTEXT* nx_context,
    NX_IP* nx_ip,
    NX_PACKET_POOL* nx_pool,
    NX_DNS* nx_dns,
    UINT (*unix_time_callback)(ULONG* unix_time),
    CHAR* iot_model_id,
    UINT iot_model_id_len)
{
    UINT status;

    if (iot_model_id_len == 0)
    {
        printf("ERROR: azure_iot_nx_client_create_new empty model_id\r\n");
        return NX_PTR_ERROR;
    }

    // Initialise the context
    memset(nx_context, 0, sizeof(AZURE_IOT_NX_CONTEXT));

    // Stash parameters
    nx_context->azure_iot_connection_status = NX_AZURE_IOT_NOT_INITIALIZED;
    nx_context->azure_iot_nx_ip             = nx_ip;
    nx_context->azure_iot_model_id          = iot_model_id;
    nx_context->azure_iot_model_id_len      = iot_model_id_len;

    // Initialize CA root certificates
    if ((status = nx_secure_x509_certificate_initialize(&nx_context->root_ca_cert,
             (UCHAR*)azure_iot_root_cert,
             (USHORT)azure_iot_root_cert_size,
             NX_NULL,
             0,
             NULL,
             0,
             NX_SECURE_X509_KEY_TYPE_NONE)))
    {
        printf("ERROR: nx_secure_x509_certificate_initialize (0x%08x)\r\n", status);
    }

    else if ((status = nx_secure_x509_certificate_initialize(&nx_context->root_ca_cert_2,
                  (UCHAR*)azure_iot_root_cert_2,
                  (USHORT)azure_iot_root_cert_size_2,
                  NX_NULL,
                  0,
                  NULL,
                  0,
                  NX_SECURE_X509_KEY_TYPE_NONE)))
    {
        printf("ERROR: nx_secure_x509_certificate_initialize (0x%08x)\r\n", status);
    }

    else if ((status = nx_secure_x509_certificate_initialize(&nx_context->root_ca_cert_3,
                  (UCHAR*)azure_iot_root_cert_3,
                  (USHORT)azure_iot_root_cert_size_3,
                  NX_NULL,
                  0,
                  NULL,
                  0,
                  NX_SECURE_X509_KEY_TYPE_NONE)))
    {
        printf("ERROR: nx_secure_x509_certificate_initialize (0x%08x)\r\n", status);
    }

    if ((status = tx_event_flags_create(&nx_context->events, "nx_client")))
    {
        printf("ERROR: tx_event_flags_creates (0x%08x)\r\n", status);
    }

    else if ((status = tx_timer_create(&nx_context->periodic_timer,
                  "periodic_timer",
                  periodic_timer_entry,
                  (ULONG)nx_context,
                  60 * NX_IP_PERIODIC_RATE,
                  60 * NX_IP_PERIODIC_RATE,
                  TX_NO_ACTIVATE)))
    {
        printf("ERROR: tx_timer_create (0x%08x)\r\n", status);
        tx_event_flags_delete(&nx_context->events);
    }

    // Create Azure IoT handler
    else if ((status = nx_azure_iot_create(&nx_context->nx_azure_iot,
                  (UCHAR*)"Azure IoT",
                  nx_ip,
                  nx_pool,
                  nx_dns,
                  nx_context->nx_azure_iot_thread_stack,
                  sizeof(nx_context->nx_azure_iot_thread_stack),
                  NX_AZURE_IOT_THREAD_PRIORITY,
                  unix_time_callback)))
    {
        printf("ERROR: failed on nx_azure_iot_create (0x%08x)\r\n", status);
        tx_event_flags_delete(&nx_context->events);
        tx_timer_delete(&nx_context->periodic_timer);
    }

    return status;
}

static UINT client_run(
    AZURE_IOT_NX_CONTEXT* nx_context, UINT (*iot_initialize)(AZURE_IOT_NX_CONTEXT*), UINT (*network_connect)())
{
    ULONG app_events;

    while (true)
    {
        app_events = 0;
        tx_event_flags_get(&nx_context->events, HUB_ALL_EVENTS, TX_OR_CLEAR, &app_events, NX_IP_PERIODIC_RATE);

        if (app_events & HUB_DISCONNECT_EVENT)
        {
            process_disconnect(nx_context);
        }

        if (app_events & HUB_CONNECT_EVENT)
        {
            process_connect(nx_context);
        }

        if (app_events & HUB_PERIODIC_TIMER_EVENT)
        {
            process_timer_event(nx_context);
        }

        if (app_events & HUB_PROPERTIES_COMPLETE_EVENT)
        {
            process_properties_complete(nx_context);
        }

        if (app_events & HUB_COMMAND_RECEIVE_EVENT)
        {
            process_command(nx_context);
        }

        if (app_events & HUB_PROPERTIES_RECEIVE_EVENT)
        {
            process_properties(nx_context);
        }

        if (app_events & HUB_WRITABLE_PROPERTIES_RECEIVE_EVENT)
        {
            process_writable_properties(nx_context);
        }

        // Monitor and reconnect where possible
        connection_monitor(nx_context, iot_initialize, network_connect);
    }

    return NX_SUCCESS;
}

UINT azure_iot_nx_client_hub_run(
    AZURE_IOT_NX_CONTEXT* nx_context, CHAR* iot_hub_hostname, CHAR* iot_hub_device_id, UINT (*network_connect)())
{
    if (iot_hub_hostname == 0 || iot_hub_device_id == 0)
    {
        printf("ERROR: azure_iot_nx_client_hub_run hub config is null\r\n");
        return NX_PTR_ERROR;
    }

    if (strlen(iot_hub_hostname) > AZURE_IOT_HOST_NAME_SIZE || strlen(iot_hub_device_id) > AZURE_IOT_DEVICE_ID_SIZE)
    {
        printf("ERROR: azure_iot_nx_client_hub_run hub config exceeds buffer size\r\n");
        return NX_SIZE_ERROR;
    }

    // take a copy of the hub config
    memcpy(nx_context->azure_iot_hub_hostname, iot_hub_hostname, AZURE_IOT_HOST_NAME_SIZE);
    memcpy(nx_context->azure_iot_hub_device_id, iot_hub_device_id, AZURE_IOT_DEVICE_ID_SIZE);
    nx_context->azure_iot_hub_hostname_len  = strlen(iot_hub_hostname);
    nx_context->azure_iot_hub_device_id_len = strlen(iot_hub_device_id);

    return client_run(nx_context, iot_hub_initialize, network_connect);
}

UINT azure_iot_nx_client_dps_run(
    AZURE_IOT_NX_CONTEXT* nx_context, CHAR* dps_id_scope, CHAR* dps_registration_id, UINT (*network_connect)())
{
    if (dps_id_scope == 0 || dps_registration_id == 0)
    {
        printf("ERROR: azure_iot_nx_client_dps_run dps config is null\r\n");
        return NX_PTR_ERROR;
    }

    // keep a reference to the dps config
    nx_context->azure_iot_dps_id_scope            = dps_id_scope;
    nx_context->azure_iot_dps_registration_id     = dps_registration_id;
    nx_context->azure_iot_dps_id_scope_len        = strlen(dps_id_scope);
    nx_context->azure_iot_dps_registration_id_len = strlen(dps_registration_id);

    return client_run(nx_context, dps_initialize, network_connect);
}
