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
#define ALL_EVENTS 0xFFF
//#define CONNECT_EVENT 0x001
//#define INITIALIZATION_EVENT 0x002
#define METHOD_DIRECT_METHOD_EVENT          0x004
#define DEVICE_TWIN_GET_EVENT               0x008
#define DEVICE_TWIN_DESIRED_PROPERTY_EVENT  0x010
#define TELEMETRY_SEND_EVENT                0x020
#define DEVICE_TWIN_REPORTED_PROPERTY_EVENT 0x040
//#define DISCONNECT_EVENT 0x080
//#define RECONNECT_EVENT 0x100
//#define CONNECTED_EVENT 0x200

#define MODULE_ID ""

static VOID connection_status_callback(NX_AZURE_IOT_HUB_CLIENT* hub_client_ptr, UINT status)
{
    /*  sample_connection_status = status;

      if (status) {
        printf("Disconnected from IoTHub!: error code = 0x%08x\r\n", status);
        tx_event_flags_set(&(sample_context.sample_events),
      SAMPLE_DISCONNECT_EVENT, TX_OR); } else { printf("Connected to
      IoTHub.\r\n"); tx_event_flags_set(&(sample_context.sample_events),
      SAMPLE_CONNECTED_EVENT, TX_OR); exponential_backoff_reset();
      }*/
}

/*static VOID connection_status_callback(NX_AZURE_IOT_HUB_CLIENT
*hub_client_ptr, UINT status) { NX_PARAMETER_NOT_USED(hub_client_ptr); if
(status) { printf("Disconnected from IoTHub!: error code = 0x%08x\r\n", status);
  } else {
    printf("Connected to IoTHub.\r\n");
  }
}*/

static VOID message_receive_callback_twin(NX_AZURE_IOT_HUB_CLIENT* hub_client_ptr, VOID* context)
{
    AZURE_IOT_NX_CONTEXT* nx_context = (AZURE_IOT_NX_CONTEXT*)context;
    tx_event_flags_set(&nx_context->events, DEVICE_TWIN_GET_EVENT, TX_OR);
}

static VOID message_receive_direct_method(NX_AZURE_IOT_HUB_CLIENT* hub_client_ptr, VOID* context)
{
    AZURE_IOT_NX_CONTEXT* nx_context = (AZURE_IOT_NX_CONTEXT*)context;
    tx_event_flags_set(&nx_context->events, METHOD_DIRECT_METHOD_EVENT, TX_OR);
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

    printf("Received direct method event\r\n");

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
    printf_packet(packet, "\tpayload: ");

    payload        = packet->nx_packet_prepend_ptr;
    payload_length = packet->nx_packet_append_ptr - packet->nx_packet_prepend_ptr;

    if (nx_context->direct_method_cb)
    {
        nx_context->direct_method_cb(
            nx_context, method_name, method_name_length, payload, payload_length, context, context_length);
    }

    // release the received packet, as ownership was passed to the application
    nx_packet_release(packet);
}

static VOID process_device_twin_get(AZURE_IOT_NX_CONTEXT* nx_context)
{
    UINT status;
    NX_PACKET* packet_ptr;
    UCHAR buffer[128];
    //    az_span twin_span;

    /*  if (context->state != SAMPLE_STATE_CONNECTED) {
        return;
      }*/

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

    nx_packet_release(packet_ptr);
}

static VOID event_thread(ULONG parameter)
{
    ULONG app_events;

    AZURE_IOT_NX_CONTEXT* context = (AZURE_IOT_NX_CONTEXT*)parameter;

    while (true)
    {
        tx_event_flags_get(&context->events, ALL_EVENTS, TX_OR_CLEAR, &app_events, NX_IP_PERIODIC_RATE);

        if (app_events & DEVICE_TWIN_GET_EVENT)
        {
            process_device_twin_get(context);
        }

        if (app_events & METHOD_DIRECT_METHOD_EVENT)
        {
            process_direct_method(context);
        }

        if (app_events & DEVICE_TWIN_DESIRED_PROPERTY_EVENT)
        {
            process_device_twin_desired_property(context);
        }

        app_events = 0;
    }
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

UINT azure_iot_nx_client_register_c2d_message(AZURE_IOT_NX_CONTEXT* context, func_ptr_c2d_message callback)
{
    if (context == NULL || context->cloud_to_device_cb != NULL)
    {
        return NX_PTR_ERROR;
    }

    context->cloud_to_device_cb = callback;
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

UINT azure_iot_nx_client_create2(AZURE_IOT_NX_CONTEXT* context,
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
    //    UCHAR* iothub_hostname       = (UCHAR*)iot_hub_hostname;
    //    UCHAR* iothub_device_id      = (UCHAR*)iot_device_id;
    //    UINT iothub_hostname_length  = strlen(iot_hub_hostname);
    //    UINT iothub_device_id_length = strlen(iot_device_id);

    printf("Initializing Azure IoT Hub client\r\n");
    printf("\tHub hostname: %s\r\n", iot_hub_hostname);
    printf("\tDevice id: %s\r\n", iot_device_id);

    if (iot_hub_hostname[0] == 0 || iot_device_id[0] == 0 || iot_sas_key[0] == 0)
    {
        printf("ERROR: IoT Hub connection configuration is empty\r\n");
        return NX_PTR_ERROR;
    }

    memset(context, 0, sizeof(AZURE_IOT_NX_CONTEXT));

    if ((status = tx_event_flags_create(&context->events, "nx_client")))
    {
        printf("ERROR: failed on create nx_client event flags (0x%08x)\r\n", status);
        return status;
    }

    // Create Azure IoT handler.
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

    /*  if (context->state != SAMPLE_STATE_INIT)
        {
        return;
      }*/

    /* Initialize IoTHub client. */
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
        //    context->action_result = status;
        return status;
    }

    if ((status = nx_azure_iot_hub_client_symmetric_key_set(
             &context->iothub_client, (UCHAR*)iot_sas_key, strlen(iot_sas_key))))
    {
        printf("ERROR:nx_azure_iot_hub_client_symmetric_key_set (0x%08x)\r\n", status);
    }
    else if ((status = nx_azure_iot_hub_client_connection_status_callback_set(
                  &context->iothub_client, connection_status_callback)))
    {
        printf("ERROR: failed on connection_status_callback (0x%08x)\r\n", status);
    }
    else if ((status = nx_azure_iot_hub_client_direct_method_enable(&context->iothub_client)))
    {
        printf("ERROR: direct method receive enable failed (0x%08x)\r\n", status);
    }
    else if ((status = nx_azure_iot_hub_client_device_twin_enable(&context->iothub_client)))
    {
        printf("ERROR: device twin enabled failed (0x%08x)\r\n", status);
    }
    else if ((status = nx_azure_iot_hub_client_receive_callback_set(&context->iothub_client,
                  NX_AZURE_IOT_HUB_DEVICE_TWIN_PROPERTIES,
                  message_receive_callback_twin,
                  (VOID*)context)))
    {
        printf("ERROR: device twin callback set (0x%08x)\r\n", status);
    }
    else if ((status = nx_azure_iot_hub_client_receive_callback_set(&context->iothub_client,
                  NX_AZURE_IOT_HUB_DIRECT_METHOD,
                  message_receive_direct_method,
                  (VOID*)context)))
    {
        printf("ERROR: device method callback set (0x%08x)\r\n", status);
    }
    else if ((status = nx_azure_iot_hub_client_receive_callback_set(&context->iothub_client,
                  NX_AZURE_IOT_HUB_DEVICE_TWIN_DESIRED_PROPERTIES,
                  message_receive_callback_desire_property,
                  (VOID*)context)))
    {
        printf("ERROR: device twin desired property callback set (0x%08x)\r\n", status);
    }
    else if ((status = nx_azure_iot_hub_client_model_id_set(
                  &context->iothub_client, (UCHAR*)iot_model_id, strlen(iot_model_id))))
    {
        printf("ERROR: digital twin modelId set (0x%08x)\r\n", status);
    }

    if (status != NX_AZURE_IOT_SUCCESS)
    {
        nx_azure_iot_hub_client_deinitialize(&context->iothub_client);
    }

    //        context->state = SAMPLE_STATE_CONNECT;
    return status;
}

/*UINT azure_iot_nx_client_dps_create(AZURE_IOT_NX_CLIENT* azure_iot_nx_client,
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
    UINT status = 0;

    printf("Initializing Azure IoT DPS + Hub client\r\n");
    printf("\tDPS endpoint: %s\r\n", dps_endpoint);
    printf("\tDPS ID scope: %s\r\n", dps_id_scope);
    printf("\tDevice ID: %s\r\n", dps_registration_id);

    if (azure_iot_nx_client == NULL)
    {
        printf("ERROR: azure_iot_nx_client is NULL\r\n");
        return NX_PTR_ERROR;
    }

    if (dps_endpoint[0] == 0 || dps_id_scope[0] == 0 || dps_registration_id[0]
== 0 || device_sas_key[0] == 0)
    {
        printf("ERROR: IoT DPS + Hub connection configuration is empty\r\n");
        return NX_PTR_ERROR;
    }

    memset(azure_iot_nx_client, 0, sizeof(*azure_iot_nx_client));

    // Create Azure IoT handler
    if ((status = nx_azure_iot_create(&azure_iot_nx_client->nx_azure_iot,
             (UCHAR*)"Azure IoT",
             nx_ip,
             nx_pool,
             nx_dns,
             azure_iot_nx_client->nx_azure_iot_thread_stack,
             sizeof(azure_iot_nx_client->nx_azure_iot_thread_stack),
             NX_AZURE_IOT_THREAD_PRIORITY,
             unix_time_callback)))
    {
        printf("Failed on nx_azure_iot_create!: error code = 0x%08x\r\n",
status); return status;
    }

    // Initialize CA certificate.
    if ((status =
nx_secure_x509_certificate_initialize(&azure_iot_nx_client->root_ca_cert,
             (UCHAR*)azure_iot_root_ca,
             (USHORT)azure_iot_root_ca_len,
             NX_NULL,
             0,
             NULL,
             0,
             NX_SECURE_X509_KEY_TYPE_NONE)))
    {
        printf("Failed to initialize ROOT CA certificate!: error code =
0x%08x\r\n", status); nx_azure_iot_delete(&azure_iot_nx_client->nx_azure_iot);
        return status;
    }

    CHAR hostname[200];
    CHAR device_id[200];
    UINT hostname_len;
    UINT device_id_len;

    // Initialize IoT provisioning client
    if ((status =
nx_azure_iot_provisioning_client_initialize(&azure_iot_nx_client->prov_client,
             &azure_iot_nx_client->nx_azure_iot,
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
             (UCHAR*)azure_iot_nx_client->nx_azure_iot_tls_metadata_buffer,
             sizeof(azure_iot_nx_client->nx_azure_iot_tls_metadata_buffer),
             &azure_iot_nx_client->root_ca_cert)))
    {
        printf("Failed on nx_azure_iot_provisioning_client_initialize!: error
code = 0x%08x\r\n", status); return status;
    }

    // Set symmetric key
    if ((status = nx_azure_iot_provisioning_client_symmetric_key_set(
             &azure_iot_nx_client->prov_client, (UCHAR*)device_sas_key,
strlen(device_sas_key))))
    {
        printf("Failed on nx_azure_iot_hub_client_symmetric_key_set!: error code
= 0x%08x\r\n", status); return status;
    }

    // Register device
    if ((status =
nx_azure_iot_provisioning_client_register(&azure_iot_nx_client->prov_client,
NX_WAIT_FOREVER)))
    {
        printf("ERROR: nx_azure_iot_provisioning_client_register!: error code =
0x%08x\r\n", status); return status;
    }

    // Get Device info
    if ((status = nx_azure_iot_provisioning_client_iothub_device_info_get(
             &azure_iot_nx_client->prov_client, (UCHAR*)hostname, &hostname_len,
(UCHAR*)device_id, &device_id_len)))
    {
        printf("Failed on
nx_azure_iot_provisioning_client_iothub_device_info_get!: error code =
0x%08x\r\n", status); return status;
    }

    // Destroy Provisioning Client.
    nx_azure_iot_provisioning_client_deinitialize(&azure_iot_nx_client->prov_client);

    // Initialize IoTHub client.
    if ((status =
nx_azure_iot_hub_client_initialize(&azure_iot_nx_client->iothub_client,
             &azure_iot_nx_client->nx_azure_iot,
             (UCHAR*)hostname,
             hostname_len,
             (UCHAR*)device_id,
             device_id_len,
             (UCHAR*)MODULE_ID,
             strlen(MODULE_ID),
             _nx_azure_iot_tls_supported_crypto,
             _nx_azure_iot_tls_supported_crypto_size,
             _nx_azure_iot_tls_ciphersuite_map,
             _nx_azure_iot_tls_ciphersuite_map_size,
             (UCHAR*)azure_iot_nx_client->nx_azure_iot_tls_metadata_buffer,
             sizeof(azure_iot_nx_client->nx_azure_iot_tls_metadata_buffer),
             &azure_iot_nx_client->root_ca_cert)))
    {
        printf("Failed on nx_azure_iot_hub_client_initialize!: error code =
0x%08x\r\n", status); nx_azure_iot_delete(&azure_iot_nx_client->nx_azure_iot);
        return status;
    }

    // Set symmetric key
    if ((status = nx_azure_iot_hub_client_symmetric_key_set(
             &azure_iot_nx_client->iothub_client, (UCHAR*)device_sas_key,
strlen(device_sas_key))))
    {
        printf("Failed on nx_azure_iot_hub_client_symmetric_key_set!\r\n");
        return status;
    }

    // Set the model Id
    if ((status = nx_azure_iot_hub_client_model_id_set(
             &azure_iot_nx_client->iothub_client, (UCHAR*)device_model_id,
strlen(device_model_id))))
    {
        printf("Failed on nx_azure_iot_hub_client_model_id_set!\r\n");
        return status;
    }

    // Set connection status callback
    if (nx_azure_iot_hub_client_connection_status_callback_set(
            &azure_iot_nx_client->iothub_client, connection_status_callback))
    {
        printf("Failed on connection_status_callback!\r\n");
        return status;
    }

    return NX_SUCCESS;
}*/

/*UINT azure_iot_nx_client_create(AZURE_IOT_NX_CLIENT* azure_iot_nx_client,
    NX_IP* nx_ip,
    NX_PACKET_POOL* nx_pool,
    NX_DNS* nx_dns,
    UINT (*unix_time_callback)(ULONG* unix_time),
    CHAR* iot_hub_hostname,
    CHAR* iot_device_id,
    CHAR* iot_sas_key,
    CHAR* iot_model_id)
{
    UINT status = 0;

    printf("Initializing Azure IoT Hub client\r\n");
    printf("\tHub hostname: %s\r\n", iot_hub_hostname);
    printf("\tDevice id: %s\r\n", iot_device_id);

    if (azure_iot_nx_client == NULL)
    {
        printf("ERROR: azure_iot_nx_client is NULL\r\n");
        return NX_PTR_ERROR;
    }

    if (iot_hub_hostname[0] == 0 || iot_device_id[0] == 0 || iot_sas_key[0] ==
0)
    {
        printf("ERROR: IoT Hub connection configuration is empty\r\n");
        return NX_PTR_ERROR;
    }

    memset(azure_iot_nx_client, 0, sizeof(*azure_iot_nx_client));

    // Create Azure IoT handler.
    if ((status = nx_azure_iot_create(&azure_iot_nx_client->nx_azure_iot,
             (UCHAR*)"Azure IoT",
             nx_ip,
             nx_pool,
             nx_dns,
             azure_iot_nx_client->nx_azure_iot_thread_stack,
             sizeof(azure_iot_nx_client->nx_azure_iot_thread_stack),
             NX_AZURE_IOT_THREAD_PRIORITY,
             unix_time_callback)))
    {
        printf("Failed on nx_azure_iot_create!: error code = 0x%08x\r\n",
status); return status;
    }

    // Initialize CA certificate.
    if ((status =
nx_secure_x509_certificate_initialize(&azure_iot_nx_client->root_ca_cert,
             (UCHAR*)azure_iot_root_ca,
             (USHORT)azure_iot_root_ca_len,
             NX_NULL,
             0,
             NULL,
             0,
             NX_SECURE_X509_KEY_TYPE_NONE)))
    {
        printf("Failed to initialize ROOT CA certificate!: error code =
0x%08x\r\n", status); nx_azure_iot_delete(&azure_iot_nx_client->nx_azure_iot);
        return status;
    }

    // Initialize IoTHub client.
    if ((status =
nx_azure_iot_hub_client_initialize(&azure_iot_nx_client->iothub_client,
             &azure_iot_nx_client->nx_azure_iot,
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
             (UCHAR*)azure_iot_nx_client->nx_azure_iot_tls_metadata_buffer,
             sizeof(azure_iot_nx_client->nx_azure_iot_tls_metadata_buffer),
             &azure_iot_nx_client->root_ca_cert)))
    {
        printf("Failed on nx_azure_iot_hub_client_initialize!: error code = "
               "0x%08x\r\n",
            status);
        nx_azure_iot_delete(&azure_iot_nx_client->nx_azure_iot);
        return status;
    }

    // Set symmetric key.
    if ((status = nx_azure_iot_hub_client_symmetric_key_set(
             &azure_iot_nx_client->iothub_client, (UCHAR*)iot_sas_key,
strlen(iot_sas_key))))
    {
        printf("Failed on nx_azure_iot_hub_client_symmetric_key_set!\r\n");
        return status;
    }

    // Set the model Id.
    if ((status = nx_azure_iot_hub_client_model_id_set(
             &azure_iot_nx_client->iothub_client, (UCHAR*)iot_model_id,
strlen(iot_model_id))))
    {
        printf("Failed on nx_azure_iot_hub_client_model_id_set!\r\n");
        return status;
    }

    // Set connection status callback.
    if (nx_azure_iot_hub_client_connection_status_callback_set(
            &azure_iot_nx_client->iothub_client, connection_status_callback))
    {
        printf("Failed on connection_status_callback!\r\n");
        return status;
    }

    return NX_SUCCESS;
}*/

UINT azure_iot_nx_client_delete(AZURE_IOT_NX_CONTEXT* context)
{
    /* Destroy IoTHub Client. */
    nx_azure_iot_hub_client_disconnect(&context->client.iothub);
    nx_azure_iot_hub_client_deinitialize(&context->client.iothub);
    nx_azure_iot_delete(&context->nx_azure_iot);

    return NX_SUCCESS;
}

UINT azure_iot_nx_client_connect(AZURE_IOT_NX_CONTEXT* context)
{
    UINT status;

    // Connect to IoTHub client
    if ((status = nx_azure_iot_hub_client_connect(&context->client.iothub, NX_TRUE, NX_WAIT_FOREVER)))
    {
        printf("Failed on nx_azure_iot_hub_client_connect = 0x%08x\r\n", status);
        return status;
    }

    if ((status = tx_thread_create(&context->azure_iot_thread,
             "Telemetry Thread",
             event_thread,
             (ULONG)context,
             (UCHAR*)context->azure_iot_thread_stack,
             AZURE_IOT_STACK_SIZE,
             THREAD_PRIORITY,
             THREAD_PRIORITY,
             1,
             TX_AUTO_START)))
    {
        printf("Failed to create telemetry thread!: error code = 0x%08x\r\n", status);
        return status;
    }

    // Telemetry thread
    /*    if (azure_iot_nx_client->telemetry_thread_entry != NULL)
        {
            if ((status =
       tx_thread_create(&azure_iot_nx_client->telemetry_thread, "Telemetry
       Thread", azure_iot_nx_client->telemetry_thread_entry, 0,
                     (UCHAR*)azure_iot_nx_client->telemetry_thread_stack,
                     AZURE_IOT_NX_STACK_SIZE,
                     THREAD_PRIORITY,
                     THREAD_PRIORITY,
                     1,
                     TX_AUTO_START)))
            {
                printf("Failed to create telemetry thread!: error code =
       0x%08x\r\n", status); return status;
            }
        }

        // Device twin thread
        if (azure_iot_nx_client->device_twin_thread_entry != NULL)
        {
            if ((status =
       nx_azure_iot_hub_client_device_twin_enable(&azure_iot_nx_client->iothub_client)))
            {
                printf("device twin enabled failed!: error code = 0x%08x\r\n",
       status); return status;
            }
            if ((status =
       tx_thread_create(&azure_iot_nx_client->device_twin_thread, "Device Twin
       Thread", azure_iot_nx_client->device_twin_thread_entry, 0,
                     (UCHAR*)azure_iot_nx_client->device_twin_thread_stack,
                     AZURE_IOT_NX_STACK_SIZE,
                     THREAD_PRIORITY,
                     THREAD_PRIORITY,
                     0,
                     TX_AUTO_START)))
            {
                printf("Failed to create device twin thread!: error code =
       0x%08x\r\n", status); return status;
            }
        }

        // Direct Method thread
        if (azure_iot_nx_client->direct_method_thread_entry != NULL)
        {
            if ((status =
       nx_azure_iot_hub_client_direct_method_enable(&azure_iot_nx_client->iothub_client)))
            {
                printf("Direct method receive enable failed!: error code =
       0x%08x\r\n", status); return status;
            }
            if ((status =
       tx_thread_create(&azure_iot_nx_client->direct_method_thread, "Direct
       Method Thread ", azure_iot_nx_client->direct_method_thread_entry, 0,
                     (UCHAR*)azure_iot_nx_client->direct_method_thread_stack,
                     AZURE_IOT_NX_STACK_SIZE,
                     THREAD_PRIORITY,
                     THREAD_PRIORITY,
                     1,
                     TX_AUTO_START)))
            {
                printf("Failed to create direct method thread!: error code =
       0x%08x\r\n", status); return status;
            }
        }

        // C2D thread
        if (azure_iot_nx_client->c2d_thread_entry != NULL)
        {
            if ((status =
       nx_azure_iot_hub_client_cloud_message_enable(&azure_iot_nx_client->iothub_client)))
            {
                printf("C2D receive enable failed!: error code = 0x%08x\r\n",
       status); return status;
            }

            if ((status = tx_thread_create(&azure_iot_nx_client->c2d_thread,
                     "C2D Thread",
                     azure_iot_nx_client->c2d_thread_entry,
                     0,
                     (UCHAR*)azure_iot_nx_client->c2d_thread_stack,
                     AZURE_IOT_NX_STACK_SIZE,
                     THREAD_PRIORITY,
                     THREAD_PRIORITY,
                     1,
                     TX_AUTO_START)))
            {
                printf("Failed to create c2d thread!: error code = 0x%08x\r\n",
       status); return status;
            }
        }*/

    printf("SUCCESS: Azure IoT Hub client initialized\r\n");

    return NX_SUCCESS;
}

UINT azure_iot_nx_client_disconnect(AZURE_IOT_NX_CONTEXT* context)
{
    nx_azure_iot_hub_client_disconnect(&context->client.iothub);

    return NX_SUCCESS;
}

/*UINT azure_iot_nx_client_enable_telemetry(AZURE_IOT_NX_CONTEXT* context,
threadx_entry telemetry_entry)
{
    context->telemetry_thread_entry = telemetry_entry;
    return NX_SUCCESS;
}

UINT azure_iot_nx_client_enable_device_twin(AZURE_IOT_NX_CONTEXT* context,
threadx_entry device_twin_entry)
{
    context->device_twin_thread_entry = device_twin_entry;
    return NX_SUCCESS;
}

UINT azure_iot_nx_client_enable_direct_method(AZURE_IOT_NX_CONTEXT* context,
threadx_entry direct_method_entry)
{
    context->direct_method_thread_entry = direct_method_entry;
    return NX_SUCCESS;
}

UINT azure_iot_nx_client_enable_c2d(AZURE_IOT_NX_CONTEXT* context, threadx_entry
c2d_entry)
{
{
    context->c2d_thread_entry = c2d_entry;
    return NX_SUCCESS;
}*/

UINT azure_iot_nx_client_publish_float_telemetry(AZURE_IOT_NX_CONTEXT* context, CHAR* key, float value)
{
    UINT status;
    CHAR buffer[30];
    NX_PACKET* packet_ptr;

    snprintf(buffer, sizeof(buffer), "{\"%s\":%0.2f}", key, value);

    // Create a telemetry message packet
    if ((status = nx_azure_iot_hub_client_telemetry_message_create(
             &context->iothub_client, &packet_ptr, NX_WAIT_FOREVER)))
    {
        printf("Telemetry message create failed!: error code = 0x%08x\r\n", status);
        return status;
    }

    if ((status = nx_azure_iot_hub_client_telemetry_send(
             &context->client.iothub, packet_ptr, (UCHAR*)buffer, strlen(buffer), NX_WAIT_FOREVER)))
    {
        printf("Telemetry message send failed!: error code = 0x%08x\r\n", status);
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

    snprintf(buffer, sizeof(buffer), "{\"%s\":%0.2f}", key, value);

    if ((status = nx_azure_iot_hub_client_device_twin_reported_properties_send(
             &context->client.iothub, (UCHAR*)buffer, strlen(buffer), &request_id, &response_status, NX_WAIT_FOREVER)))
    {
        printf("Device twin reported properties failed!: error code = 0x%08x\r\n", status);
        return status;
    }

    if ((response_status < 200) || (response_status >= 300))
    {
        printf("device twin report properties failed with code : %d\r\n", response_status);
        return status;
    }

    printf("Device twin property sent: %s.\r\n", buffer);

    return NX_SUCCESS;
}

UINT azure_iot_nx_client_publish_bool_property(AZURE_IOT_NX_CONTEXT* context, CHAR* key, bool value)
{
    UINT status;
    UINT response_status;
    UINT request_id;
    CHAR buffer[30];

    snprintf(buffer, sizeof(buffer), "{\"%s\":%s}", key, (value ? "true" : "false"));

    if ((status = nx_azure_iot_hub_client_device_twin_reported_properties_send(
             &context->client.iothub, (UCHAR*)buffer, strlen(buffer), &request_id, &response_status, NX_WAIT_FOREVER)))
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

/*UINT azure_nx_client_respond_int_desired_property(
    AZURE_IOT_NX_CONTEXT* context, CHAR* label, INT value, UINT http_status, UINT version)
{
    // CHAR mqtt_publish_topic[100];
    UINT status;
    UINT response_status;
    UINT request_id;
    CHAR message[100];

    snprintf(
        message, sizeof(message), "{\"%s\":{\"value\":%d,\"ac\":%d,\"av\":%d}}", label, value, http_status, version);

    if ((status = nx_azure_iot_hub_client_device_twin_reported_properties_send(&context->client.iothub,
             (UCHAR*)message,
             strlen(message),
             &request_id,
             &response_status,
             NX_WAIT_FOREVER)))
    {
        printf("Device twin reported properties failed!: error code = 0x%08x\r\n", status);
    }

    if ((response_status < 200) || (response_status >= 300))
    {
        printf("device twin report properties failed with code : %d\r\n", response_status);
        return status;
    }

    printf("Reported desired property with int value %s\r\n", message);

    return status;
}*/

/*UINT azure_nx_client_publish_int_desired_property(AZURE_IOT_NX_CONTEXT* context, CHAR* label, UINT value)
{
    return azure_nx_client_respond_int_desired_property(context, label, value, 200, 1);
}*/

UINT azure_nx_client_respond_int_writeable_property(
    AZURE_IOT_NX_CONTEXT* context, CHAR* property, int value, int http_status, int version)
{
    // CHAR mqtt_publish_topic[100];
    UINT status;
    UINT response_status;
    UINT request_id;
    CHAR message[100];

    printf("Responding to writeable property %s as %d\r\n", property, value);

    snprintf(
        message, sizeof(message), "{\"%s\":{\"value\":%d,\"ac\":%d,\"av\":%d}}", property, value, http_status, version);

    if ((status = nx_azure_iot_hub_client_device_twin_reported_properties_send(&context->iothub_client,
             (UCHAR*)message,
             strlen(message),
             &request_id,
             &response_status,
             NX_WAIT_FOREVER)))
    {
        printf("Device twin reported properties failed (0x%08x)\r\n", status);
    }

    if ((response_status < 200) || (response_status >= 300))
    {
        printf("device twin report properties failed with code: %d\r\n", response_status);
        return status;
    }

    return status;
}

VOID printf_packet(NX_PACKET* packet_ptr, CHAR* prepend)
{
    printf(prepend);

    while (packet_ptr != NX_NULL)
    {
        printf("%.*s", (INT)(packet_ptr->nx_packet_length), (CHAR*)packet_ptr->nx_packet_prepend_ptr);
        packet_ptr = packet_ptr->nx_packet_next;
    }

    printf("\r\n");
}
