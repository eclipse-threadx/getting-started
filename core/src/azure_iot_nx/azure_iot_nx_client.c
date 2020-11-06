// /* Copyright (c) Microsoft Corporation.
//    Licensed under the MIT License. */

// #include "azure_iot_nx_client.h"

// #include <stdio.h>

// #include "azure_iot_cert.h"
// #include "azure_iot_ciphersuites.h"
// #include "nx_azure_iot_pnp_helpers.h"

// #define NX_AZURE_IOT_THREAD_PRIORITY 4
// #define THREAD_PRIORITY              16

// // Incoming events from the middleware
// #define ALL_EVENTS                         0xFF
// #define DIRECT_METHOD_EVENT                0x01
// #define DEVICE_TWIN_GET_EVENT              0x02
// #define DEVICE_TWIN_DESIRED_PROPERTY_EVENT 0x04

// #define MODULE_ID   ""
// #define DPS_PAYLOAD "{\"modelId\":\"%s\"}"

// static VOID connection_status_callback(NX_AZURE_IOT_HUB_CLIENT* hub_client_ptr, UINT status)
// {
//     if (status)
//     {
//         printf("Disconnected from IoTHub (0x%08x)\r\n", status);
//     }
//     else
//     {
//         printf("Connected to IoTHub\r\n");
//     }
// }

// static VOID message_receive_direct_method(NX_AZURE_IOT_HUB_CLIENT* hub_client_ptr, VOID* context)
// {
//     AZURE_IOT_NX_CONTEXT* nx_context = (AZURE_IOT_NX_CONTEXT*)context;
//     tx_event_flags_set(&nx_context->events, DIRECT_METHOD_EVENT, TX_OR);
// }

// static VOID message_receive_callback_twin(NX_AZURE_IOT_HUB_CLIENT* hub_client_ptr, VOID* context)
// {
//     AZURE_IOT_NX_CONTEXT* nx_context = (AZURE_IOT_NX_CONTEXT*)context;
//     tx_event_flags_set(&nx_context->events, DEVICE_TWIN_GET_EVENT, TX_OR);
// }

// static VOID message_receive_callback_desire_property(NX_AZURE_IOT_HUB_CLIENT* hub_client_ptr, VOID* context)
// {
//     AZURE_IOT_NX_CONTEXT* nx_context = (AZURE_IOT_NX_CONTEXT*)context;
//     tx_event_flags_set(&nx_context->events, DEVICE_TWIN_DESIRED_PROPERTY_EVENT, TX_OR);
// }

// static VOID process_direct_method(AZURE_IOT_NX_CONTEXT* nx_context)
// {
//     UINT status;
//     NX_PACKET* packet;
//     const UCHAR* method_name;
//     USHORT method_name_length;
//     VOID* context;
//     USHORT context_length;
//     UCHAR* payload;
//     USHORT payload_length;

//     if ((status = nx_azure_iot_hub_client_direct_method_message_receive(&nx_context->iothub_client,
//              &method_name,
//              &method_name_length,
//              &context,
//              &context_length,
//              &packet,
//              NX_WAIT_FOREVER)))
//     {
//         printf("Direct method receive failed!: error code = 0x%08x\r\n", status);
//         return;
//     }

//     printf("Receive direct method call: %.*s\r\n", (INT)method_name_length, (CHAR*)method_name);
//     printf_packet(packet, "\tPayload: ");

//     payload        = packet->nx_packet_prepend_ptr;
//     payload_length = packet->nx_packet_append_ptr - packet->nx_packet_prepend_ptr;

//     if (nx_context->direct_method_cb)
//     {
//         nx_context->direct_method_cb(
//             nx_context, method_name, method_name_length, payload, payload_length, context, context_length);
//     }

//     // Release the received packet, as ownership was passed to the application
//     nx_packet_release(packet);
// }

// static VOID process_device_twin_get(AZURE_IOT_NX_CONTEXT* nx_context)
// {
//     UINT status;
//     NX_PACKET* packet_ptr;
//     NX_AZURE_IOT_JSON_READER json_reader;
//     UCHAR buffer[128];

//     if ((status = nx_azure_iot_hub_client_device_twin_properties_receive(
//              &nx_context->iothub_client, &packet_ptr, NX_WAIT_FOREVER)))
//     {
//         printf("Error: receive device twin property failed (0x%08x)\r\n", status);
//         return;
//     }

//     printf_packet(packet_ptr, "Receive twin properties: ");

//     if (packet_ptr->nx_packet_length > (ULONG)(packet_ptr->nx_packet_append_ptr - packet_ptr->nx_packet_prepend_ptr))
//     {
//         printf("ERROR: json is large than nx_packet\r\n");
//         nx_packet_release(packet_ptr);
//         return;
//     }

//     if ((status = nx_azure_iot_json_reader_init(&json_reader, packet_ptr)))
//     {
//         printf("ERROR: failed to initialize json reader (0x%08x)\r\n", status);
//         nx_packet_release(packet_ptr);
//         return;
//     }

//     if (nx_context->device_twin_get_cb)
//     {
//         if ((status = nx_azure_iot_pnp_helper_twin_data_parse(&json_reader,
//                  NX_FALSE,
//                  NX_NULL,
//                  0,
//                  buffer,
//                  sizeof(buffer),
//                  nx_context->device_twin_get_cb,
//                  nx_context)))
//         {
//             printf("ERROR: failed to parse twin data (0x%08x)\r\n", status);
//         }
//     }

//     // Deinit the reader, the reader owns the NX_PACKET at this point, so will release it
//     nx_azure_iot_json_reader_deinit(&json_reader);
// }

// static VOID process_device_twin_desired_property(AZURE_IOT_NX_CONTEXT* nx_context)
// {
//     UINT status;
//     NX_PACKET* packet_ptr;
//     NX_AZURE_IOT_JSON_READER json_reader;
//     UCHAR buffer[128];

//     if ((status = nx_azure_iot_hub_client_device_twin_desired_properties_receive(
//              &nx_context->iothub_client, &packet_ptr, NX_WAIT_FOREVER)))
//     {
//         printf("ERROR: receive device twin desired property receive failed (0x%08x)\r\n", status);
//         return;
//     }

//     printf_packet(packet_ptr, "Receive twin desired property: ");

//     if (packet_ptr->nx_packet_length > (ULONG)(packet_ptr->nx_packet_append_ptr - packet_ptr->nx_packet_prepend_ptr))
//     {
//         printf("ERROR: json is large than nx_packet\r\n");
//         nx_packet_release(packet_ptr);
//         return;
//     }

//     if ((status = nx_azure_iot_json_reader_init(&json_reader, packet_ptr)))
//     {
//         printf("ERROR: failed to initialize json reader (0x%08x)\r\n", status);
//         nx_packet_release(packet_ptr);
//         return;
//     }

//     if (nx_context->device_twin_desired_prop_cb)
//     {
//         if ((status = nx_azure_iot_pnp_helper_twin_data_parse(&json_reader,
//                  NX_TRUE,
//                  NX_NULL,
//                  0,
//                  buffer,
//                  sizeof(buffer),
//                  nx_context->device_twin_desired_prop_cb,
//                  nx_context)))
//         {
//             printf("ERROR: failed to parse twin data (0x%08x)\r\n", status);
//         }
//     }

//     // Deinit the reader, the reader owns the NX_PACKET at this point, so will release it
//     nx_azure_iot_json_reader_deinit(&json_reader);
// }

// static VOID event_thread(ULONG parameter)
// {
//     ULONG app_events;

//     AZURE_IOT_NX_CONTEXT* context = (AZURE_IOT_NX_CONTEXT*)parameter;

//     while (true)
//     {
//         tx_event_flags_get(&context->events, ALL_EVENTS, TX_OR_CLEAR, &app_events, NX_IP_PERIODIC_RATE);

//         if (app_events & DIRECT_METHOD_EVENT)
//         {
//             process_direct_method(context);
//         }

//         if (app_events & DEVICE_TWIN_GET_EVENT)
//         {
//             process_device_twin_get(context);
//         }

//         if (app_events & DEVICE_TWIN_DESIRED_PROPERTY_EVENT)
//         {
//             process_device_twin_desired_property(context);
//         }

//         app_events = 0;
//     }
// }

// NX_SECURE_X509_CERT device_certificate;
// // NX_SECURE_X509_CERT iot_hub_device_certificate;
// UINT azure_iot_nx_client_x509_create_common(AZURE_IOT_NX_CONTEXT* context,
//     CHAR* iot_hub_hostname,
//     CHAR* iot_device_id,
//     CHAR* device_model_id,
//     UCHAR* device_cert,
//     UINT x509_cert_len,
//     UCHAR* private_key,
//     UINT x509_key_len,
//     NX_SECURE_X509_CERT* device_certificate_ptr)
// {
//     UINT status;

//     printf("Initializing Azure IoT Hub client\r\n");
//     printf("\tHub hostname: %s\r\n", iot_hub_hostname);
//     printf("\tDevice id: %s\r\n", iot_device_id);
//     printf("\tModel id: %s\r\n", device_model_id);

//     if ((status = tx_event_flags_create(&context->events, "nx_client")))
//     {
//         printf("ERROR: failed on create nx_client event flags (0x%08x)\r\n", status);
//         return status;
//     }

//     // Initialize CA certificate.
//     printf("----- INITIALIZE ROOT CA CERTIFICATE -----\n");
//     if ((status = nx_secure_x509_certificate_initialize(&context->root_ca_cert,
//              (UCHAR*)azure_iot_root_ca,
//              (USHORT)azure_iot_root_ca_len,
//              NX_NULL,
//              0,
//              NULL,
//              0,
//              NX_SECURE_X509_KEY_TYPE_NONE)))
//     {
//         printf("Failed to initialize ROOT CA certificate!: error code = 0x%08x\r\n", status);
//         nx_azure_iot_delete(&context->nx_azure_iot);
//         return status;
//     }
    
//     // Initialize IoTHub client.
//     if ((status = nx_azure_iot_hub_client_initialize(&context->iothub_client,
//              &context->nx_azure_iot,
//              (UCHAR*)iot_hub_hostname,
//              strlen(iot_hub_hostname),
//              (UCHAR*)iot_device_id,
//              strlen(iot_device_id),
//              (UCHAR*)MODULE_ID,
//              strlen(MODULE_ID),
//              _nx_azure_iot_tls_supported_crypto,
//              _nx_azure_iot_tls_supported_crypto_size,
//              _nx_azure_iot_tls_ciphersuite_map,
//              _nx_azure_iot_tls_ciphersuite_map_size,
//              (UCHAR*)context->nx_azure_iot_tls_metadata_buffer,
//              sizeof(context->nx_azure_iot_tls_metadata_buffer),
//              &context->root_ca_cert)))
//     {
//         printf("ERROR: on nx_azure_iot_hub_client_initialize (0x%08x)\r\n", status);
//         return status;
//     }

//     /* Initialize the device certificate.  */
//     printf("----- INITIALIZE DEVICE CERTIFICATE -----\n");
//     if ((status = nx_secure_x509_certificate_initialize(device_certificate_ptr, (UCHAR*)device_cert, (USHORT)x509_cert_len, NX_NULL, 0,
//                                                         (UCHAR*)private_key, (USHORT)x509_key_len, NX_SECURE_X509_KEY_TYPE_RSA_PKCS1_DER)))
//     {
//         printf("Failed on nx_secure_x509_certificate_initialize!: error code = 0x%08x\r\n", status);
//         // return status;
//     }

//     /* Set device certificate.  */
//     else if ((status = nx_azure_iot_hub_client_device_cert_set(&context->iothub_client, device_certificate_ptr)))
//     {
//         printf("Failed on nx_azure_iot_hub_client_device_cert_set!: error code = 0x%08x\r\n", status);
//     }

//     // Set Model id
//     else if ((status = nx_azure_iot_hub_client_model_id_set(
//                   &context->iothub_client, (UCHAR*)device_model_id, strlen(device_model_id))))
//     {
//         printf("ERROR: nx_azure_iot_hub_client_model_id_set (0x%08x)\r\n", status);
//     }

//     // Set connection status callback
//     else if ((status = nx_azure_iot_hub_client_connection_status_callback_set(
//                   &context->iothub_client, connection_status_callback)))
//     {
//         printf("ERROR: failed on connection_status_callback (0x%08x)\r\n", status);
//     }

//     // Enable direct methods
//     else if ((status = nx_azure_iot_hub_client_direct_method_enable(&context->iothub_client)))
//     {
//         printf("ERROR: direct method receive enable failed (0x%08x)\r\n", status);
//     }

//     // Enable device twin
//     else if ((status = nx_azure_iot_hub_client_device_twin_enable(&context->iothub_client)))
//     {
//         printf("ERROR: device twin enabled failed (0x%08x)\r\n", status);
//     }

//     // Set device twin callback
//     else if ((status = nx_azure_iot_hub_client_receive_callback_set(&context->iothub_client,
//                   NX_AZURE_IOT_HUB_DEVICE_TWIN_PROPERTIES,
//                   message_receive_callback_twin,
//                   (VOID*)context)))
//     {
//         printf("ERROR: device twin callback set (0x%08x)\r\n", status);
//     }

//     // Set direct method callback
//     else if ((status = nx_azure_iot_hub_client_receive_callback_set(&context->iothub_client,
//                   NX_AZURE_IOT_HUB_DIRECT_METHOD,
//                   message_receive_direct_method,
//                   (VOID*)context)))
//     {
//         printf("ERROR: device method callback set (0x%08x)\r\n", status);
//     }

//     // Set the writeable property callback
//     else if ((status = nx_azure_iot_hub_client_receive_callback_set(&context->iothub_client,
//                   NX_AZURE_IOT_HUB_DEVICE_TWIN_DESIRED_PROPERTIES,
//                   message_receive_callback_desire_property,
//                   (VOID*)context)))
//     {
//         printf("ERROR: device twin desired property callback set (0x%08x)\r\n", status);
//     }

//     if (status != NX_AZURE_IOT_SUCCESS)
//     {
//         nx_azure_iot_hub_client_deinitialize(&context->iothub_client);
//     }

//     return status;
// }

// static UINT azure_iot_nx_client_create_common(AZURE_IOT_NX_CONTEXT* context,
//     CHAR* iot_hub_hostname,
//     CHAR* iot_device_id,
//     CHAR* device_sas_key,
//     CHAR* device_model_id)
// {
//     UINT status;

//     printf("Initializing Azure IoT Hub client\r\n");
//     printf("\tHub hostname: %s\r\n", iot_hub_hostname);
//     printf("\tDevice id: %s\r\n", iot_device_id);
//     printf("\tModel id: %s\r\n", device_model_id);

//     if ((status = tx_event_flags_create(&context->events, "nx_client")))
//     {
//         printf("ERROR: failed on create nx_client event flags (0x%08x)\r\n", status);
//         return status;
//     }

//     // Initialize IoTHub client.
//     if ((status = nx_azure_iot_hub_client_initialize(&context->iothub_client,
//              &context->nx_azure_iot,
//              (UCHAR*)iot_hub_hostname,
//              strlen(iot_hub_hostname),
//              (UCHAR*)iot_device_id,
//              strlen(iot_device_id),
//              (UCHAR*)MODULE_ID,
//              strlen(MODULE_ID),
//              _nx_azure_iot_tls_supported_crypto,
//              _nx_azure_iot_tls_supported_crypto_size,
//              _nx_azure_iot_tls_ciphersuite_map,
//              _nx_azure_iot_tls_ciphersuite_map_size,
//              (UCHAR*)context->nx_azure_iot_tls_metadata_buffer,
//              sizeof(context->nx_azure_iot_tls_metadata_buffer),
//              &context->root_ca_cert)))
//     {
//         printf("ERROR: on nx_azure_iot_hub_client_initialize (0x%08x)\r\n", status);
//         return status;
//     }

//     // Set SAS key
//     if ((status = nx_azure_iot_hub_client_symmetric_key_set(
//              &context->iothub_client, (UCHAR*)device_sas_key, strlen(device_sas_key))))
//     {
//         printf("ERROR: nx_azure_iot_hub_client_symmetric_key_set (0x%08x)\r\n", status);
//     }

//     // Set Model id
//     else if ((status = nx_azure_iot_hub_client_model_id_set(
//                   &context->iothub_client, (UCHAR*)device_model_id, strlen(device_model_id))))
//     {
//         printf("ERROR: nx_azure_iot_hub_client_model_id_set (0x%08x)\r\n", status);
//     }

//     // Set connection status callback
//     else if ((status = nx_azure_iot_hub_client_connection_status_callback_set(
//                   &context->iothub_client, connection_status_callback)))
//     {
//         printf("ERROR: failed on connection_status_callback (0x%08x)\r\n", status);
//     }

//     // Enable direct methods
//     else if ((status = nx_azure_iot_hub_client_direct_method_enable(&context->iothub_client)))
//     {
//         printf("ERROR: direct method receive enable failed (0x%08x)\r\n", status);
//     }

//     // Enable device twin
//     else if ((status = nx_azure_iot_hub_client_device_twin_enable(&context->iothub_client)))
//     {
//         printf("ERROR: device twin enabled failed (0x%08x)\r\n", status);
//     }

//     // Set device twin callback
//     else if ((status = nx_azure_iot_hub_client_receive_callback_set(&context->iothub_client,
//                   NX_AZURE_IOT_HUB_DEVICE_TWIN_PROPERTIES,
//                   message_receive_callback_twin,
//                   (VOID*)context)))
//     {
//         printf("ERROR: device twin callback set (0x%08x)\r\n", status);
//     }

//     // Set direct method callback
//     else if ((status = nx_azure_iot_hub_client_receive_callback_set(&context->iothub_client,
//                   NX_AZURE_IOT_HUB_DIRECT_METHOD,
//                   message_receive_direct_method,
//                   (VOID*)context)))
//     {
//         printf("ERROR: device method callback set (0x%08x)\r\n", status);
//     }

//     // Set the writeable property callback
//     else if ((status = nx_azure_iot_hub_client_receive_callback_set(&context->iothub_client,
//                   NX_AZURE_IOT_HUB_DEVICE_TWIN_DESIRED_PROPERTIES,
//                   message_receive_callback_desire_property,
//                   (VOID*)context)))
//     {
//         printf("ERROR: device twin desired property callback set (0x%08x)\r\n", status);
//     }

//     if (status != NX_AZURE_IOT_SUCCESS)
//     {
//         nx_azure_iot_hub_client_deinitialize(&context->iothub_client);
//     }

//     return status;
// }

// UINT azure_iot_nx_client_register_direct_method(AZURE_IOT_NX_CONTEXT* context, func_ptr_direct_method callback)
// {
//     if (context == NULL || context->direct_method_cb != NULL)
//     {
//         return NX_PTR_ERROR;
//     }

//     context->direct_method_cb = callback;
//     return NX_SUCCESS;
// }

// UINT azure_iot_nx_client_register_device_twin_desired_prop(
//     AZURE_IOT_NX_CONTEXT* context, func_ptr_device_twin_desired_prop callback)
// {
//     if (context == NULL || context->device_twin_desired_prop_cb != NULL)
//     {
//         return NX_PTR_ERROR;
//     }

//     context->device_twin_desired_prop_cb = callback;
//     return NX_SUCCESS;
// }

// UINT azure_iot_nx_client_register_device_twin_prop(AZURE_IOT_NX_CONTEXT* context, func_ptr_device_twin_prop callback)
// {
//     if (context == NULL || context->device_twin_get_cb != NULL)
//     {
//         return NX_PTR_ERROR;
//     }

//     context->device_twin_get_cb = callback;
//     return NX_SUCCESS;
// }

// UINT azure_iot_nx_client_create(AZURE_IOT_NX_CONTEXT* context,
//     NX_IP* nx_ip,
//     NX_PACKET_POOL* nx_pool,
//     NX_DNS* nx_dns,
//     UINT (*unix_time_callback)(ULONG* unix_time),
//     CHAR* iot_hub_hostname,
//     CHAR* iot_device_id,
//     CHAR* iot_sas_key,
//     CHAR* iot_model_id)
// {
//     UINT status;

//     if (context == NULL)
//     {
//         printf("ERROR: context is NULL\r\n");
//         return NX_PTR_ERROR;
//     }

//     if (iot_hub_hostname[0] == 0 || iot_device_id[0] == 0 || iot_sas_key[0] == 0)
//     {
//         printf("ERROR: IoT Hub connection configuration is empty\r\n");
//         return NX_PTR_ERROR;
//     }

//     memset(context, 0, sizeof(AZURE_IOT_NX_CONTEXT));

//     // Create Azure IoT handler
//     if ((status = nx_azure_iot_create(&context->nx_azure_iot,
//              (UCHAR*)"Azure IoT",
//              nx_ip,
//              nx_pool,
//              nx_dns,
//              context->nx_azure_iot_thread_stack,
//              sizeof(context->nx_azure_iot_thread_stack),
//              NX_AZURE_IOT_THREAD_PRIORITY,
//              unix_time_callback)))
//     {
//         printf("ERROR: failed on nx_azure_iot_create (0x%08x)\r\n", status);
//         return status;
//     }

//     // Initialize CA certificate.
//     if ((status = nx_secure_x509_certificate_initialize(&context->root_ca_cert,
//              (UCHAR*)azure_iot_root_ca,
//              (USHORT)azure_iot_root_ca_len,
//              NX_NULL,
//              0,
//              NULL,
//              0,
//              NX_SECURE_X509_KEY_TYPE_NONE)))
//     {
//         printf("Failed to initialize ROOT CA certificate!: error code = 0x%08x\r\n", status);
//         nx_azure_iot_delete(&context->nx_azure_iot);
//         return status;
//     }

//     return azure_iot_nx_client_create_common(context, iot_hub_hostname, iot_device_id, iot_sas_key, iot_model_id);
// }

// UINT azure_iot_nx_client_dps_create(AZURE_IOT_NX_CONTEXT* context,
//     NX_IP* nx_ip,
//     NX_PACKET_POOL* nx_pool,
//     NX_DNS* nx_dns,
//     UINT (*unix_time_callback)(ULONG* unix_time),
//     CHAR* dps_endpoint,
//     CHAR* dps_id_scope,
//     CHAR* dps_registration_id,
//     CHAR* device_sas_key,
//     CHAR* device_model_id)
// {
//     UINT status;
//     CHAR payload[200];
//     UINT iot_hub_hostname_len = AZURE_IOT_HOST_NAME_SIZE;
//     UINT iot_device_id_len    = AZURE_IOT_DEVICE_ID_SIZE;

//     printf("Initializing Azure IoT DPS client\r\n");
//     printf("\tDPS endpoint: %s\r\n", dps_endpoint);
//     printf("\tDPS ID scope: %s\r\n", dps_id_scope);
//     printf("\tRegistration ID: %s\r\n", dps_registration_id);

//     if (context == NULL)
//     {
//         printf("ERROR: context is NULL\r\n");
//         return NX_PTR_ERROR;
//     }

//     if (dps_endpoint[0] == 0 || dps_id_scope[0] == 0 || dps_registration_id[0] == 0 || device_sas_key[0] == 0)
//     {
//         printf("ERROR: IoT DPS + Hub connection configuration is empty\r\n");
//         return NX_PTR_ERROR;
//     }

//     memset(context, 0, sizeof(AZURE_IOT_NX_CONTEXT));

//     if (snprintf(payload, sizeof(payload), DPS_PAYLOAD, device_model_id) > sizeof(payload) - 1)
//     {
//         printf("ERROR: insufficient buffer size to create DPS payload\r\n");
//         return NX_SIZE_ERROR;
//     }

//     // Create Azure IoT handler
//     if ((status = nx_azure_iot_create(&context->nx_azure_iot,
//              (UCHAR*)"Azure IoT",
//              nx_ip,
//              nx_pool,
//              nx_dns,
//              context->nx_azure_iot_thread_stack,
//              sizeof(context->nx_azure_iot_thread_stack),
//              NX_AZURE_IOT_THREAD_PRIORITY,
//              unix_time_callback)))
//     {
//         printf("ERROR: failed on nx_azure_iot_create (0x%08x)\r\n", status);
//         return status;
//     }

//     // Initialize CA certificate.
//     if ((status = nx_secure_x509_certificate_initialize(&context->root_ca_cert,
//              (UCHAR*)azure_iot_root_ca,
//              (USHORT)azure_iot_root_ca_len,
//              NX_NULL,
//              0,
//              NULL,
//              0,
//              NX_SECURE_X509_KEY_TYPE_NONE)))
//     {
//         printf("Failed to initialize ROOT CA certificate!: error code = 0x%08x\r\n", status);
//         nx_azure_iot_delete(&context->nx_azure_iot);
//         return status;
//     }

//     // Initialize IoT provisioning client
//     if ((status = nx_azure_iot_provisioning_client_initialize(&context->prov_client,
//              &context->nx_azure_iot,
//              (UCHAR*)dps_endpoint,
//              strlen(dps_endpoint),
//              (UCHAR*)dps_id_scope,
//              strlen(dps_id_scope),
//              (UCHAR*)dps_registration_id,
//              strlen(dps_registration_id),
//              _nx_azure_iot_tls_supported_crypto,
//              _nx_azure_iot_tls_supported_crypto_size,
//              _nx_azure_iot_tls_ciphersuite_map,
//              _nx_azure_iot_tls_ciphersuite_map_size,
//              (UCHAR*)context->nx_azure_iot_tls_metadata_buffer,
//              sizeof(context->nx_azure_iot_tls_metadata_buffer),
//              &context->root_ca_cert)))
//     {
//         printf("Failed on nx_azure_iot_provisioning_client_initialize (0x%08x)\r\n", status);
//         return status;
//     }

//     // Set symmetric key
//     if ((status = nx_azure_iot_provisioning_client_symmetric_key_set(
//              &context->prov_client, (UCHAR*)device_sas_key, strlen(device_sas_key))))
//     {
//         printf("Failed on nx_azure_iot_hub_client_symmetric_key_set (0x%08x)\r\n", status);
//     }

//     // Set the payload containing the model Id
//     else if ((status = nx_azure_iot_provisioning_client_registration_payload_set(
//                   &context->prov_client, (UCHAR*)payload, strlen(payload))))
//     {
//         printf("Error: nx_azure_iot_provisioning_client_registration_payload_set (0x%08x\r\n", status);
//     }

//     // Register device
//     else if ((status = nx_azure_iot_provisioning_client_register(&context->prov_client, NX_WAIT_FOREVER)))
//     {
//         printf("ERROR: nx_azure_iot_provisioning_client_register (0x%08x)\r\n", status);
//     }

//     // Get Device info
//     else if ((status = nx_azure_iot_provisioning_client_iothub_device_info_get(&context->prov_client,
//                   (UCHAR*)context->azure_iot_hub_hostname,
//                   &iot_hub_hostname_len,
//                   (UCHAR*)context->azure_iot_device_id,
//                   &iot_device_id_len)))
//     {
//         printf("ERROR: nx_azure_iot_provisioning_client_iothub_device_info_get (0x%08x)\r\n", status);
//         return status;
//     }

//     // Destroy Provisioning Client
//     nx_azure_iot_provisioning_client_deinitialize(&context->prov_client);

//     if (status != NX_SUCCESS)
//     {
//         return status;
//     }

//     // Null terminate returned values
//     context->azure_iot_hub_hostname[iot_hub_hostname_len] = 0;
//     context->azure_iot_device_id[iot_device_id_len]       = 0;

//     printf("SUCCESS: Azure IoT DPS client initialized\r\n\r\n");

//     return azure_iot_nx_client_create_common(
//         context, context->azure_iot_hub_hostname, context->azure_iot_device_id, device_sas_key, device_model_id);
// }

// UINT azure_iot_nx_client_dps_x509_create(AZURE_IOT_NX_CONTEXT* context,
//     NX_IP* nx_ip,
//     NX_PACKET_POOL* nx_pool,
//     NX_DNS* nx_dns,
//     UINT (*unix_time_callback)(ULONG* unix_time),
//     CHAR* dps_endpoint,
//     CHAR* dps_id_scope,
//     CHAR* dps_registration_id,
//     UCHAR* device_cert,
//     UINT x509_cert_len,
//     UCHAR* private_key,
//     UINT x509_key_len,
//     CHAR* device_model_id)
// {
//     UINT status;
//     CHAR payload[200];
//     UINT iot_hub_hostname_len = AZURE_IOT_HOST_NAME_SIZE;
//     UINT iot_device_id_len    = AZURE_IOT_DEVICE_ID_SIZE;

//     printf("Initializing Azure IoT DPS client\r\n");
//     printf("\tDPS endpoint: %s\r\n", dps_endpoint);
//     printf("\tDPS ID scope: %s\r\n", dps_id_scope);
//     printf("\tRegistration ID: %s\r\n", dps_registration_id);

//     if (context == NULL)
//     {
//         printf("ERROR: context is NULL\r\n");
//         return NX_PTR_ERROR;
//     }

//     if (dps_endpoint[0] == 0 || dps_id_scope[0] == 0 || dps_registration_id[0] == 0 || device_cert[0] == 0 || private_key[0] == 0)
//     {
//         printf("ERROR: IoT DPS + Hub connection configuration is empty\r\n");
//         return NX_PTR_ERROR;
//     }

//     memset(context, 0, sizeof(AZURE_IOT_NX_CONTEXT));

//     if (snprintf(payload, sizeof(payload), DPS_PAYLOAD, device_model_id) > sizeof(payload) - 1)
//     {
//         printf("ERROR: insufficient buffer size to create DPS payload\r\n");
//         return NX_SIZE_ERROR;
//     }

//     // Create Azure IoT handler
//     if ((status = nx_azure_iot_create(&context->nx_azure_iot,
//              (UCHAR*)"Azure IoT",
//              nx_ip,
//              nx_pool,
//              nx_dns,
//              context->nx_azure_iot_thread_stack,
//              sizeof(context->nx_azure_iot_thread_stack),
//              NX_AZURE_IOT_THREAD_PRIORITY,
//              unix_time_callback)))
//     {
//         printf("ERROR: failed on nx_azure_iot_create (0x%08x)\r\n", status);
//         return status;
//     }

//     // Initialize CA certificate.
//     printf("----- INITIALIZE ROOT CA CERTIFICATE -----\n");
//     if ((status = nx_secure_x509_certificate_initialize(&context->root_ca_cert,
//              (UCHAR*)azure_iot_root_ca,
//              (USHORT)azure_iot_root_ca_len,
//              NX_NULL,
//              0,
//              NULL,
//              0,
//              NX_SECURE_X509_KEY_TYPE_NONE)))
//     {
//         printf("Failed to initialize ROOT CA certificate!: error code = 0x%08x\r\n", status);
//         nx_azure_iot_delete(&context->nx_azure_iot);
//         return status;
//     }

//     /* Initialize the device certificate.  */
//     printf("----- INITIALIZE DEVICE CERTIFICATE -----\n");
//     if ((status = nx_secure_x509_certificate_initialize(&device_certificate, (UCHAR*)device_cert, (USHORT)x509_cert_len, NX_NULL, 0,
//                                                         (UCHAR*)private_key, (USHORT)x509_key_len, NX_SECURE_X509_KEY_TYPE_RSA_PKCS1_DER)))
//     {
//         printf("Failed on nx_secure_x509_certificate_initialize!: error code = 0x%08x\r\n", status);
//         return status;
//     }

//     // Initialize IoT provisioning client
//     printf("----- INITIALIZE PROVISIONING CLIENT -----\n");
//     if ((status = nx_azure_iot_provisioning_client_initialize(&context->prov_client,
//              &context->nx_azure_iot,
//              (UCHAR*)dps_endpoint,
//              strlen(dps_endpoint),
//              (UCHAR*)dps_id_scope,
//              strlen(dps_id_scope),
//              (UCHAR*)dps_registration_id,
//              strlen(dps_registration_id),
//              _nx_azure_iot_tls_supported_crypto,
//              _nx_azure_iot_tls_supported_crypto_size,
//              _nx_azure_iot_tls_ciphersuite_map,
//              _nx_azure_iot_tls_ciphersuite_map_size,
//              (UCHAR*)context->nx_azure_iot_tls_metadata_buffer,
//              sizeof(context->nx_azure_iot_tls_metadata_buffer),
//              &context->root_ca_cert)))
//     {
//         printf("Failed on nx_azure_iot_provisioning_client_initialize (0x%08x)\r\n", status);
//         return status;
//     }

//     /* Set device certificate.  */
//     printf("----- SET PROVISIONING CLIENT CERT -----\n");
//     if ((status = nx_azure_iot_provisioning_client_device_cert_set(&context->prov_client, &device_certificate)))
//     {
//         printf("Failed on nx_azure_iot_provisioning_client_device_cert_set!: error code = 0x%08x\r\n", status);
//         return status;
//     }

//     // Set the payload containing the model Id
//     printf("----- SET PROVISIONING CLIENT PAYLOAD -----\n");
//     if ((status = nx_azure_iot_provisioning_client_registration_payload_set(
//                   &context->prov_client, (UCHAR*)payload, strlen(payload))))
//     {
//         printf("Error: nx_azure_iot_provisioning_client_registration_payload_set (0x%08x\r\n", status);
//         return status;
//     }

//     // Register device
//     printf("----- REGISTER PROVISIONING CLIENT -----\n");
//     if ((status = nx_azure_iot_provisioning_client_register(&context->prov_client, NX_WAIT_FOREVER)))
//     {
//         printf("ERROR: nx_azure_iot_provisioning_client_register (0x%08x)\r\n", status);
//         return status;
//     }

//     // Get Device info
//     printf("----- SUCCESS: DEVICE PROVISIONED -----\n");
//     printf("----- GET IOTHUB DEVICE INFO -----\n");
//     if ((status = nx_azure_iot_provisioning_client_iothub_device_info_get(&context->prov_client,
//                   (UCHAR*)context->azure_iot_hub_hostname,
//                   &iot_hub_hostname_len,
//                   (UCHAR*)context->azure_iot_device_id,
//                   &iot_device_id_len)))
//     {
//         printf("ERROR: nx_azure_iot_provisioning_client_iothub_device_info_get (0x%08x)\r\n", status);
//         return status;
//     }
    
//     printf("----- IOTHUB DEVICE INFO ACQUIRED -----\n");
//     printf("iot_hub_hostname = %s\n", context->azure_iot_hub_hostname);
//     printf("iot_device_id = %s\n", context->azure_iot_device_id);

//     // Destroy Provisioning Client
//     nx_azure_iot_provisioning_client_deinitialize(&context->prov_client);

//     if (status != NX_SUCCESS) {
//       return status;
//     }

//     // Null terminate returned values
//     context->azure_iot_hub_hostname[iot_hub_hostname_len] = 0;
//     context->azure_iot_device_id[iot_device_id_len] = 0;

//     printf("SUCCESS: Azure IoT DPS client initialized\r\n\r\n");

//     return azure_iot_nx_client_x509_create_common(
//         context, context->azure_iot_hub_hostname, context->azure_iot_device_id, device_model_id,
//         device_cert, x509_cert_len, private_key, x509_key_len,
//         &device_certificate);
// }

// UINT azure_iot_nx_client_delete(AZURE_IOT_NX_CONTEXT* context)
// {
//     /* Destroy IoTHub Client. */
//     nx_azure_iot_hub_client_disconnect(&context->iothub_client);
//     nx_azure_iot_hub_client_deinitialize(&context->iothub_client);
//     nx_azure_iot_delete(&context->nx_azure_iot);

//     return NX_SUCCESS;
// }

// UINT azure_iot_nx_client_connect(AZURE_IOT_NX_CONTEXT* context)
// {
//     UINT status;
//     // Connect to IoTHub client
//     if ((status = nx_azure_iot_hub_client_connect(&context->iothub_client, NX_TRUE, NX_WAIT_FOREVER)))
//     {
//         printf("Failed on nx_azure_iot_hub_client_connect (0x%08x)\r\n", status);
//         return status;
//     }
//     printf("Time to create connection thread\r\n");
//     if ((status = tx_thread_create(&context->azure_iot_thread,
//              "Nx Thread",
//              event_thread,
//              (ULONG)context,
//              (UCHAR*)context->azure_iot_thread_stack,
//              AZURE_IOT_STACK_SIZE,
//              THREAD_PRIORITY,
//              THREAD_PRIORITY,
//              1,
//              TX_AUTO_START)))
//     {
//         printf("Failed to create telemetry thread (0x%08x)\r\n", status);
//         return status;
//     }

//     printf("SUCCESS: Azure IoT Hub client initialized\r\n\r\n");

//     return NX_SUCCESS;
// }

// UINT azure_iot_nx_client_disconnect(AZURE_IOT_NX_CONTEXT* context)
// {
//     nx_azure_iot_hub_client_disconnect(&context->iothub_client);

//     return NX_SUCCESS;
// }

// UINT azure_iot_nx_client_publish_float_telemetry(AZURE_IOT_NX_CONTEXT* context, CHAR* key, float value)
// {
//     UINT status;
//     CHAR buffer[30];
//     NX_PACKET* packet_ptr;

//     if (snprintf(buffer, sizeof(buffer), "{\"%s\":%0.2f}", key, value) > sizeof(buffer) - 1)
//     {
//         printf("ERROR: insufficient buffer size to publish float telemetry\r\n");
//         return NX_SIZE_ERROR;
//     }

//     // Create a telemetry message packet
//     if ((status = nx_azure_iot_hub_client_telemetry_message_create(
//              &context->iothub_client, &packet_ptr, NX_WAIT_FOREVER)))
//     {
//         printf("Telemetry message create failed (0x%08x)\r\n", status);
//         return status;
//     }

//     if ((status = nx_azure_iot_hub_client_telemetry_send(
//              &context->iothub_client, packet_ptr, (UCHAR*)buffer, strlen(buffer), NX_WAIT_FOREVER)))
//     {
//         printf("Telemetry message send failed (0x%08x)\r\n", status);
//         nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);
//         return status;
//     }

//     printf("Telemetry message sent: %s.\r\n", buffer);

//     return NX_SUCCESS;
// }

// UINT azure_iot_nx_client_publish_float_property(AZURE_IOT_NX_CONTEXT* context, CHAR* key, float value)
// {
//     UINT status;
//     UINT response_status;
//     UINT request_id;
//     ULONG version;
//     CHAR buffer[30];

//     if (snprintf(buffer, sizeof(buffer), "{\"%s\":%0.2f}", key, value) > sizeof(buffer) - 1)
//     {
//         printf("ERROR: insufficient buffer size to publish float property\r\n");
//         return NX_SIZE_ERROR;
//     }

//     if ((status = nx_azure_iot_hub_client_device_twin_reported_properties_send(&context->iothub_client,
//              (UCHAR*)buffer,
//              strlen(buffer),
//              &request_id,
//              &response_status,
//              &version,
//              NX_WAIT_FOREVER)))
//     {
//         printf("Device twin reported properties failed (0x%08x)\r\n", status);
//         return status;
//     }

//     if ((response_status < 200) || (response_status >= 300))
//     {
//         printf("device twin report properties failed (%d)\r\n", response_status);
//         return status;
//     }

//     printf("Device twin property sent: %s\r\n", buffer);

//     return NX_SUCCESS;
// }

// UINT azure_iot_nx_client_publish_bool_property(AZURE_IOT_NX_CONTEXT* context, CHAR* key, bool value)
// {
//     UINT status;
//     UINT response_status;
//     UINT request_id;
//     ULONG version;
//     CHAR buffer[64];

//     if (snprintf(buffer, sizeof(buffer), "{\"%s\":%s}", key, (value ? "true" : "false")) > sizeof(buffer) - 1)
//     {
//         printf("ERROR: Unsufficient buffer size to publish bool property\r\n");
//         return NX_SIZE_ERROR;
//     }

//     if ((status = nx_azure_iot_hub_client_device_twin_reported_properties_send(&context->iothub_client,
//              (UCHAR*)buffer,
//              strlen(buffer),
//              &request_id,
//              &response_status,
//              &version,
//              NX_WAIT_FOREVER)))
//     {
//         printf("Error: device twin reported properties failed (0x%08x)\r\n", status);
//         return status;
//     }

//     if ((response_status < 200) || (response_status >= 300))
//     {
//         printf("Error: device twin report properties failed (%d)\r\n", response_status);
//         return status;
//     }

//     printf("Device twin property sent: %s\r\n", buffer);

//     return NX_SUCCESS;
// }

// UINT azure_nx_client_respond_int_writeable_property(
//     AZURE_IOT_NX_CONTEXT* context, CHAR* property, int value, int http_status, int version)
// {
//     UINT status;
//     UINT response_status;
//     UINT request_id;
//     ULONG version_dt;
//     CHAR message[100];

//     printf("Responding to writeable property %s = %d\r\n", property, value);

//     if (snprintf(message,
//             sizeof(message),
//             "{\"%s\":{\"value\":%d,\"ac\":%d,\"av\":%d}}",
//             property,
//             value,
//             http_status,
//             version) > sizeof(message) - 1)
//     {
//         printf("ERROR: insufficient buffer size to respond to writeable property\r\n");
//         return NX_SIZE_ERROR;
//     }

//     if ((status = nx_azure_iot_hub_client_device_twin_reported_properties_send(&context->iothub_client,
//              (UCHAR*)message,
//              strlen(message),
//              &request_id,
//              &response_status,
//              &version_dt,
//              NX_WAIT_FOREVER)))
//     {
//         printf("ERROR: device twin reported properties failed (0x%08x)\r\n", status);
//     }

//     if ((response_status < 200) || (response_status >= 300))
//     {
//         printf("ERROR: device twin report properties failed (%d)\r\n", response_status);
//         return status;
//     }

//     return status;
// }

// VOID printf_packet(NX_PACKET* packet_ptr, CHAR* prepend)
// {
//     printf("%s", prepend);

//     while (packet_ptr != NX_NULL)
//     {
//         printf("%.*s", (INT)(packet_ptr->nx_packet_length), (CHAR*)packet_ptr->nx_packet_prepend_ptr);
//         packet_ptr = packet_ptr->nx_packet_next;
//     }

//     printf("\r\n");
// }


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

#define DPS_PAYLOAD_SIZE    200
#define PUBLISH_BUFFER_SIZE 64

const unsigned char iot_x509_device_cert[] = {
    0x30, 0x82, 0x03, 0xf9, 0x30, 0x82, 0x02, 0xe1, 0x02, 0x14, 0x06, 0x53,
    0x5b, 0x9e, 0xbe, 0x07, 0x60, 0x01, 0x5e, 0xff, 0x57, 0x0a, 0x29, 0xfc,
    0x4f, 0x34, 0x5e, 0x92, 0x75, 0x19, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86,
    0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b, 0x05, 0x00, 0x30, 0x81, 0xb8,
    0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x55,
    0x53, 0x31, 0x13, 0x30, 0x11, 0x06, 0x03, 0x55, 0x04, 0x08, 0x0c, 0x0a,
    0x43, 0x61, 0x6c, 0x69, 0x66, 0x6f, 0x72, 0x6e, 0x69, 0x61, 0x31, 0x15,
    0x30, 0x13, 0x06, 0x03, 0x55, 0x04, 0x07, 0x0c, 0x0c, 0x53, 0x61, 0x6e,
    0x74, 0x61, 0x20, 0x4d, 0x6f, 0x6e, 0x69, 0x63, 0x61, 0x31, 0x12, 0x30,
    0x10, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x09, 0x4d, 0x69, 0x63, 0x72,
    0x6f, 0x73, 0x6f, 0x66, 0x74, 0x31, 0x12, 0x30, 0x10, 0x06, 0x03, 0x55,
    0x04, 0x0b, 0x0c, 0x09, 0x41, 0x7a, 0x75, 0x72, 0x65, 0x20, 0x49, 0x6f,
    0x54, 0x31, 0x2e, 0x30, 0x2c, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x25,
    0x67, 0x6c, 0x6f, 0x62, 0x61, 0x6c, 0x2e, 0x61, 0x7a, 0x75, 0x72, 0x65,
    0x2d, 0x64, 0x65, 0x76, 0x69, 0x63, 0x65, 0x73, 0x2d, 0x70, 0x72, 0x6f,
    0x76, 0x69, 0x73, 0x69, 0x6f, 0x6e, 0x69, 0x6e, 0x67, 0x2e, 0x6e, 0x65,
    0x74, 0x31, 0x25, 0x30, 0x23, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7,
    0x0d, 0x01, 0x09, 0x01, 0x16, 0x16, 0x73, 0x70, 0x6d, 0x63, 0x64, 0x6f,
    0x6e, 0x6f, 0x40, 0x6d, 0x69, 0x63, 0x72, 0x6f, 0x73, 0x6f, 0x66, 0x74,
    0x2e, 0x63, 0x6f, 0x6d, 0x30, 0x1e, 0x17, 0x0d, 0x32, 0x30, 0x30, 0x39,
    0x33, 0x30, 0x32, 0x30, 0x30, 0x35, 0x32, 0x35, 0x5a, 0x17, 0x0d, 0x32,
    0x31, 0x30, 0x39, 0x33, 0x30, 0x32, 0x30, 0x30, 0x35, 0x32, 0x35, 0x5a,
    0x30, 0x81, 0xb8, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06,
    0x13, 0x02, 0x55, 0x53, 0x31, 0x13, 0x30, 0x11, 0x06, 0x03, 0x55, 0x04,
    0x08, 0x0c, 0x0a, 0x43, 0x61, 0x6c, 0x69, 0x66, 0x6f, 0x72, 0x6e, 0x69,
    0x61, 0x31, 0x15, 0x30, 0x13, 0x06, 0x03, 0x55, 0x04, 0x07, 0x0c, 0x0c,
    0x53, 0x61, 0x6e, 0x74, 0x61, 0x20, 0x4d, 0x6f, 0x6e, 0x69, 0x63, 0x61,
    0x31, 0x12, 0x30, 0x10, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x09, 0x4d,
    0x69, 0x63, 0x72, 0x6f, 0x73, 0x6f, 0x66, 0x74, 0x31, 0x12, 0x30, 0x10,
    0x06, 0x03, 0x55, 0x04, 0x0b, 0x0c, 0x09, 0x41, 0x7a, 0x75, 0x72, 0x65,
    0x20, 0x49, 0x6f, 0x54, 0x31, 0x2e, 0x30, 0x2c, 0x06, 0x03, 0x55, 0x04,
    0x03, 0x0c, 0x25, 0x67, 0x6c, 0x6f, 0x62, 0x61, 0x6c, 0x2e, 0x61, 0x7a,
    0x75, 0x72, 0x65, 0x2d, 0x64, 0x65, 0x76, 0x69, 0x63, 0x65, 0x73, 0x2d,
    0x70, 0x72, 0x6f, 0x76, 0x69, 0x73, 0x69, 0x6f, 0x6e, 0x69, 0x6e, 0x67,
    0x2e, 0x6e, 0x65, 0x74, 0x31, 0x25, 0x30, 0x23, 0x06, 0x09, 0x2a, 0x86,
    0x48, 0x86, 0xf7, 0x0d, 0x01, 0x09, 0x01, 0x16, 0x16, 0x73, 0x70, 0x6d,
    0x63, 0x64, 0x6f, 0x6e, 0x6f, 0x40, 0x6d, 0x69, 0x63, 0x72, 0x6f, 0x73,
    0x6f, 0x66, 0x74, 0x2e, 0x63, 0x6f, 0x6d, 0x30, 0x82, 0x01, 0x22, 0x30,
    0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01,
    0x05, 0x00, 0x03, 0x82, 0x01, 0x0f, 0x00, 0x30, 0x82, 0x01, 0x0a, 0x02,
    0x82, 0x01, 0x01, 0x00, 0xca, 0xe0, 0x62, 0x70, 0x53, 0xd3, 0xc2, 0x32,
    0x0d, 0x05, 0xee, 0xc4, 0xb3, 0xa6, 0xea, 0xc8, 0xca, 0x33, 0xff, 0x78,
    0x7e, 0x0c, 0x3a, 0x30, 0xf7, 0xef, 0x2a, 0xf5, 0x48, 0x20, 0xa7, 0x8e,
    0x18, 0x36, 0xba, 0xd8, 0x00, 0x36, 0xcc, 0x0b, 0x82, 0x07, 0x8c, 0xb7,
    0x0d, 0x57, 0xb3, 0xf9, 0x82, 0xbd, 0xd1, 0xc8, 0xc2, 0xa7, 0x83, 0xa4,
    0xe1, 0xbb, 0x5c, 0xe1, 0x8f, 0x5a, 0x1e, 0x27, 0x9b, 0xc3, 0x4f, 0x0d,
    0x9e, 0x2e, 0x0e, 0x74, 0x69, 0xc2, 0x07, 0xec, 0xba, 0x1c, 0x8f, 0x48,
    0x62, 0xf2, 0x39, 0x10, 0x27, 0x7d, 0x1e, 0x21, 0x37, 0x96, 0xaa, 0x65,
    0x9f, 0xe5, 0xcd, 0x82, 0x9f, 0x68, 0x4c, 0xb8, 0x8c, 0xdd, 0x21, 0x39,
    0x81, 0x5b, 0x00, 0x94, 0xe2, 0xb7, 0x0d, 0x2c, 0xa9, 0x93, 0x15, 0x8e,
    0x42, 0xe6, 0xcf, 0xb0, 0x31, 0xea, 0xb9, 0x36, 0x40, 0x54, 0x6d, 0x71,
    0x45, 0x4b, 0x1e, 0xc0, 0x95, 0xb7, 0xe9, 0xc5, 0x7b, 0x43, 0x8c, 0xb5,
    0x3f, 0xba, 0x21, 0x79, 0xc9, 0x98, 0x2f, 0xe2, 0xb8, 0xf4, 0xf7, 0x7d,
    0x4a, 0x2f, 0x45, 0x79, 0x25, 0xa9, 0x89, 0x03, 0x77, 0x4d, 0x5f, 0xe0,
    0xa4, 0xaa, 0x5c, 0x98, 0xcb, 0xcf, 0x7a, 0x7c, 0x03, 0x78, 0xb7, 0x8a,
    0x8c, 0x6a, 0x61, 0x6c, 0x20, 0xbf, 0x34, 0xc5, 0xfa, 0x21, 0xc5, 0x65,
    0x06, 0xbb, 0x35, 0x30, 0xe1, 0x2e, 0xff, 0x11, 0x25, 0x11, 0xc9, 0xe4,
    0xd5, 0x01, 0x91, 0x01, 0xa6, 0x0d, 0x8c, 0x2c, 0x3c, 0xeb, 0x20, 0x82,
    0x06, 0x96, 0xac, 0x3b, 0x87, 0xaa, 0xb7, 0x5f, 0x1c, 0xca, 0x67, 0x92,
    0xd1, 0x84, 0x2f, 0xf2, 0x3b, 0x09, 0x9f, 0xdd, 0xc5, 0xe2, 0x90, 0x81,
    0x96, 0x7e, 0x35, 0x89, 0x96, 0xf5, 0x6f, 0xcc, 0xe4, 0x7c, 0x1c, 0xa6,
    0xbd, 0x3f, 0x23, 0xad, 0xe4, 0x7c, 0x0d, 0x05, 0x02, 0x03, 0x01, 0x00,
    0x01, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01,
    0x01, 0x0b, 0x05, 0x00, 0x03, 0x82, 0x01, 0x01, 0x00, 0xbd, 0xd4, 0xa7,
    0x95, 0xec, 0xb7, 0x09, 0x9b, 0xc5, 0xc5, 0x9f, 0xe1, 0x77, 0xd4, 0xc5,
    0x10, 0xb4, 0x5d, 0xde, 0x82, 0xf6, 0xd9, 0xe6, 0x15, 0x4f, 0x65, 0x4f,
    0xd3, 0xe2, 0x9a, 0x70, 0x38, 0x5c, 0x44, 0x79, 0xfb, 0xde, 0xbf, 0x4e,
    0x3a, 0x7e, 0x63, 0xc9, 0x72, 0xb4, 0x98, 0x37, 0x1b, 0x35, 0x24, 0x04,
    0x96, 0xbc, 0x63, 0x7c, 0x33, 0x69, 0x4f, 0xc1, 0x8b, 0x2c, 0x12, 0x55,
    0x6b, 0xf1, 0x8e, 0x64, 0xb9, 0x60, 0x84, 0x07, 0x79, 0xd7, 0x78, 0x7d,
    0x1c, 0x5c, 0x96, 0x4b, 0x96, 0x86, 0x70, 0x0a, 0xa4, 0x49, 0x21, 0xe6,
    0xcb, 0x21, 0xd6, 0xbd, 0x2f, 0x42, 0x76, 0xc3, 0xb5, 0xcf, 0xa6, 0x2b,
    0x2c, 0x51, 0x4f, 0x34, 0xda, 0xf7, 0x5d, 0x7e, 0x9b, 0x2a, 0xc2, 0xcf,
    0x5b, 0x08, 0x3e, 0xb8, 0x06, 0x8e, 0x3b, 0xbf, 0x1b, 0x13, 0xe2, 0x6b,
    0x02, 0xe1, 0xdf, 0x0c, 0x25, 0x5d, 0x83, 0xa2, 0xa7, 0x6a, 0x55, 0xff,
    0x18, 0x0a, 0x4e, 0x8a, 0xba, 0x8a, 0x61, 0x86, 0x5a, 0x7f, 0x49, 0xcf,
    0x0b, 0x8d, 0xf7, 0x64, 0xf9, 0xd0, 0xa2, 0x9e, 0xcd, 0x39, 0x19, 0x09,
    0x66, 0xf2, 0x7f, 0x93, 0xc4, 0xd2, 0xc5, 0x12, 0xea, 0x52, 0x9b, 0xb1,
    0x08, 0x83, 0x57, 0x80, 0x7b, 0xee, 0xc3, 0x09, 0x33, 0xc4, 0x19, 0x97,
    0x96, 0xfd, 0x1c, 0x3a, 0x95, 0x1e, 0xa8, 0xf2, 0xdd, 0xf9, 0xfc, 0x9b,
    0xfc, 0x4b, 0x08, 0x0d, 0x1e, 0x15, 0x1a, 0xd9, 0x83, 0xd0, 0x1d, 0x11,
    0xd0, 0xd3, 0x30, 0x94, 0x44, 0x69, 0xf4, 0x28, 0xdf, 0x6a, 0x31, 0xda,
    0x19, 0xf8, 0xab, 0x97, 0x4e, 0xae, 0x4f, 0x04, 0xf1, 0xd2, 0x8d, 0x1a,
    0xec, 0xc8, 0xc7, 0xac, 0xe9, 0xad, 0xea, 0xcb, 0x74, 0x49, 0x39, 0x44,
    0x0a, 0x75, 0x47, 0x3a, 0x32, 0xde, 0xfb, 0xad, 0x0c, 0x34, 0xe9, 0x75,
    0x44};
unsigned int iot_x509_device_cert_len = sizeof(iot_x509_device_cert);
unsigned char iot_x509_private_key[] = {
    0x30, 0x82, 0x04, 0xa3, 0x02, 0x01, 0x00, 0x02, 0x82, 0x01, 0x01, 0x00,
    0xca, 0xe0, 0x62, 0x70, 0x53, 0xd3, 0xc2, 0x32, 0x0d, 0x05, 0xee, 0xc4,
    0xb3, 0xa6, 0xea, 0xc8, 0xca, 0x33, 0xff, 0x78, 0x7e, 0x0c, 0x3a, 0x30,
    0xf7, 0xef, 0x2a, 0xf5, 0x48, 0x20, 0xa7, 0x8e, 0x18, 0x36, 0xba, 0xd8,
    0x00, 0x36, 0xcc, 0x0b, 0x82, 0x07, 0x8c, 0xb7, 0x0d, 0x57, 0xb3, 0xf9,
    0x82, 0xbd, 0xd1, 0xc8, 0xc2, 0xa7, 0x83, 0xa4, 0xe1, 0xbb, 0x5c, 0xe1,
    0x8f, 0x5a, 0x1e, 0x27, 0x9b, 0xc3, 0x4f, 0x0d, 0x9e, 0x2e, 0x0e, 0x74,
    0x69, 0xc2, 0x07, 0xec, 0xba, 0x1c, 0x8f, 0x48, 0x62, 0xf2, 0x39, 0x10,
    0x27, 0x7d, 0x1e, 0x21, 0x37, 0x96, 0xaa, 0x65, 0x9f, 0xe5, 0xcd, 0x82,
    0x9f, 0x68, 0x4c, 0xb8, 0x8c, 0xdd, 0x21, 0x39, 0x81, 0x5b, 0x00, 0x94,
    0xe2, 0xb7, 0x0d, 0x2c, 0xa9, 0x93, 0x15, 0x8e, 0x42, 0xe6, 0xcf, 0xb0,
    0x31, 0xea, 0xb9, 0x36, 0x40, 0x54, 0x6d, 0x71, 0x45, 0x4b, 0x1e, 0xc0,
    0x95, 0xb7, 0xe9, 0xc5, 0x7b, 0x43, 0x8c, 0xb5, 0x3f, 0xba, 0x21, 0x79,
    0xc9, 0x98, 0x2f, 0xe2, 0xb8, 0xf4, 0xf7, 0x7d, 0x4a, 0x2f, 0x45, 0x79,
    0x25, 0xa9, 0x89, 0x03, 0x77, 0x4d, 0x5f, 0xe0, 0xa4, 0xaa, 0x5c, 0x98,
    0xcb, 0xcf, 0x7a, 0x7c, 0x03, 0x78, 0xb7, 0x8a, 0x8c, 0x6a, 0x61, 0x6c,
    0x20, 0xbf, 0x34, 0xc5, 0xfa, 0x21, 0xc5, 0x65, 0x06, 0xbb, 0x35, 0x30,
    0xe1, 0x2e, 0xff, 0x11, 0x25, 0x11, 0xc9, 0xe4, 0xd5, 0x01, 0x91, 0x01,
    0xa6, 0x0d, 0x8c, 0x2c, 0x3c, 0xeb, 0x20, 0x82, 0x06, 0x96, 0xac, 0x3b,
    0x87, 0xaa, 0xb7, 0x5f, 0x1c, 0xca, 0x67, 0x92, 0xd1, 0x84, 0x2f, 0xf2,
    0x3b, 0x09, 0x9f, 0xdd, 0xc5, 0xe2, 0x90, 0x81, 0x96, 0x7e, 0x35, 0x89,
    0x96, 0xf5, 0x6f, 0xcc, 0xe4, 0x7c, 0x1c, 0xa6, 0xbd, 0x3f, 0x23, 0xad,
    0xe4, 0x7c, 0x0d, 0x05, 0x02, 0x03, 0x01, 0x00, 0x01, 0x02, 0x82, 0x01,
    0x00, 0x59, 0x5f, 0x86, 0x1e, 0x23, 0xdf, 0xaa, 0xb2, 0x2f, 0xe8, 0x88,
    0x2a, 0x21, 0x56, 0x8c, 0x21, 0xce, 0x6a, 0x1f, 0xc6, 0x2e, 0x09, 0x26,
    0xed, 0x81, 0x9b, 0xdc, 0x19, 0xd8, 0x1f, 0x82, 0x90, 0xe7, 0x7b, 0xdc,
    0xcd, 0xe1, 0x32, 0x42, 0xab, 0x63, 0x29, 0xfe, 0x44, 0x79, 0xdd, 0x44,
    0x0c, 0xcc, 0x1a, 0xe1, 0x3b, 0x06, 0xfd, 0x92, 0xd1, 0xe5, 0x1b, 0x8c,
    0x2c, 0x55, 0xe1, 0x86, 0x5d, 0x3e, 0x92, 0x48, 0xd6, 0x1f, 0xf3, 0xfc,
    0x2c, 0x2e, 0x96, 0xea, 0xb4, 0xcf, 0xbc, 0x5b, 0x26, 0xb1, 0x68, 0x31,
    0xd9, 0x3a, 0xfc, 0x34, 0x74, 0xbf, 0xaf, 0x60, 0xfb, 0x89, 0xf8, 0x4b,
    0x93, 0x9a, 0xb3, 0xf0, 0xf0, 0x70, 0x25, 0xce, 0x48, 0x2e, 0x03, 0x2f,
    0xfb, 0x66, 0xf0, 0xeb, 0x1d, 0xdc, 0xca, 0x4d, 0xf9, 0xb0, 0xe1, 0x4b,
    0x67, 0x3c, 0x66, 0x42, 0x17, 0x73, 0x39, 0x7e, 0x48, 0x5a, 0x38, 0x91,
    0xf1, 0xd0, 0xe8, 0x70, 0xda, 0x64, 0xbc, 0xf1, 0xf0, 0xb7, 0x11, 0x60,
    0xed, 0x09, 0x4d, 0x11, 0x84, 0x79, 0x11, 0x8d, 0x96, 0x56, 0x06, 0x12,
    0xcc, 0xa3, 0xb6, 0xd5, 0x10, 0xb1, 0xca, 0x8c, 0x7b, 0xe9, 0xcb, 0x4a,
    0xa5, 0x8b, 0x36, 0x65, 0x77, 0x07, 0x3e, 0x54, 0x94, 0x56, 0x2a, 0x34,
    0xde, 0x7e, 0x1e, 0x6f, 0xf0, 0x6a, 0x81, 0xd1, 0x78, 0x96, 0x46, 0x5e,
    0xc7, 0x18, 0x69, 0x77, 0x83, 0x88, 0x59, 0x22, 0xd6, 0x47, 0x0a, 0x4b,
    0xcf, 0xb2, 0x5f, 0x2f, 0x29, 0x7b, 0x6b, 0xa6, 0x1a, 0x04, 0xa8, 0x79,
    0xe0, 0xf4, 0x3a, 0xb4, 0xc6, 0xd5, 0x60, 0x02, 0x11, 0x53, 0x04, 0x92,
    0x70, 0xe3, 0x90, 0x04, 0xf0, 0x8b, 0x59, 0xd7, 0xe9, 0x85, 0x0b, 0xd1,
    0xd0, 0xe5, 0xd8, 0xa4, 0x7c, 0x19, 0xf9, 0x12, 0x72, 0x09, 0x67, 0x0e,
    0x89, 0x3e, 0x8c, 0x7b, 0xf5, 0x02, 0x81, 0x81, 0x00, 0xe9, 0xfd, 0x35,
    0x6d, 0xa9, 0x11, 0x06, 0x72, 0x0b, 0x83, 0x87, 0x4a, 0x6f, 0x76, 0x7e,
    0x2b, 0x47, 0xf0, 0x1f, 0xa2, 0xce, 0xbb, 0x44, 0xeb, 0x6d, 0xb0, 0xe3,
    0x69, 0xfc, 0xbf, 0x98, 0xc0, 0x61, 0x02, 0x35, 0xe5, 0xc3, 0x8a, 0x01,
    0x14, 0xc5, 0xdd, 0xe8, 0x1e, 0x99, 0x9e, 0x62, 0xfd, 0xbe, 0x2f, 0xc8,
    0xb2, 0xa4, 0xe9, 0x0b, 0xb6, 0x8c, 0x38, 0x58, 0x4d, 0xb6, 0x22, 0x67,
    0x07, 0x4f, 0xc8, 0x12, 0x2e, 0x62, 0x8d, 0x19, 0x45, 0x02, 0x6d, 0x4b,
    0x18, 0x31, 0x7a, 0x8f, 0x66, 0xff, 0x8e, 0x65, 0x0b, 0x74, 0xa0, 0x2e,
    0xc9, 0x18, 0xaf, 0x3e, 0x72, 0xa3, 0x16, 0xf7, 0x16, 0x4c, 0x25, 0xc5,
    0x98, 0x7d, 0xcb, 0x7a, 0x9f, 0x3a, 0x3f, 0x49, 0x9b, 0xb0, 0x0e, 0xf4,
    0x17, 0x96, 0x6c, 0xc0, 0x36, 0x25, 0x77, 0xc9, 0x58, 0x18, 0xc6, 0xe3,
    0x07, 0xf5, 0x04, 0xbb, 0xc7, 0x02, 0x81, 0x81, 0x00, 0xdd, 0xf5, 0xf0,
    0xae, 0x1d, 0x3f, 0xf3, 0xe8, 0xa3, 0x04, 0x28, 0x84, 0x15, 0xa1, 0x51,
    0x71, 0x4c, 0x76, 0x56, 0xba, 0x98, 0x0d, 0xec, 0x8f, 0xa5, 0xe3, 0x0b,
    0x0a, 0x6d, 0xdc, 0x12, 0x8b, 0xd3, 0x18, 0xdb, 0xe0, 0x07, 0x0e, 0xc7,
    0x74, 0x9d, 0x12, 0xe5, 0x9a, 0xd4, 0x41, 0x07, 0x9f, 0x64, 0xa4, 0xf4,
    0xcf, 0xb2, 0x1c, 0xbd, 0xce, 0x55, 0x20, 0x5b, 0x8a, 0x25, 0x26, 0x19,
    0xd8, 0xe2, 0x9c, 0x35, 0x3d, 0xa1, 0x60, 0x55, 0x5e, 0xbb, 0x15, 0xd9,
    0x2a, 0xb0, 0x40, 0x15, 0xb5, 0x1f, 0x86, 0x05, 0x7a, 0xad, 0x7f, 0xdc,
    0xb5, 0xe6, 0xdb, 0xd3, 0x6c, 0xba, 0x0c, 0xe5, 0x1a, 0x46, 0x06, 0x0f,
    0x6e, 0xcc, 0x63, 0x71, 0xd2, 0xe7, 0xf0, 0xac, 0xc6, 0xe9, 0x05, 0x50,
    0x48, 0x49, 0x24, 0x28, 0x4e, 0x8e, 0x3c, 0xe2, 0x6d, 0xcf, 0xb8, 0xe7,
    0x0f, 0xd9, 0xf0, 0x78, 0xd3, 0x02, 0x81, 0x80, 0x7b, 0x88, 0x9b, 0x34,
    0x92, 0x15, 0xd2, 0xf9, 0x52, 0xea, 0xd5, 0xfa, 0x8e, 0x6e, 0x58, 0x9e,
    0x31, 0x14, 0xad, 0x1f, 0x1f, 0x55, 0xef, 0x44, 0xfc, 0x5c, 0x6f, 0x33,
    0xfd, 0xa4, 0x89, 0xd8, 0xc7, 0x5e, 0x8b, 0x66, 0x0a, 0x9e, 0x49, 0x83,
    0xd6, 0x13, 0x1a, 0x01, 0x4d, 0xc9, 0x39, 0x2a, 0x0d, 0xc6, 0xdc, 0x86,
    0xb4, 0x59, 0xf5, 0x3d, 0x23, 0x3a, 0x7c, 0x24, 0x4b, 0x0c, 0x4e, 0xa8,
    0xa5, 0x8e, 0x53, 0x4a, 0xf9, 0xec, 0xd4, 0x72, 0x82, 0x9d, 0x81, 0x0f,
    0xfc, 0xcf, 0x5f, 0xf6, 0x58, 0xe5, 0xac, 0xa3, 0x27, 0xa7, 0x8a, 0x0b,
    0xd1, 0x64, 0x73, 0xeb, 0xc1, 0xfe, 0xd5, 0xe4, 0x87, 0xbf, 0x49, 0x5b,
    0x00, 0xce, 0xf4, 0xa6, 0x2b, 0xf2, 0x4d, 0xc2, 0x4c, 0x69, 0x18, 0xa5,
    0x7b, 0x5d, 0x5e, 0xfd, 0x37, 0x3a, 0xce, 0xaa, 0x1c, 0x2a, 0xa4, 0xa2,
    0x06, 0xd1, 0xc0, 0xf1, 0x02, 0x81, 0x80, 0x58, 0x6a, 0x71, 0xb7, 0x49,
    0xdb, 0x81, 0xd3, 0x77, 0xac, 0x5e, 0x5f, 0x4f, 0x26, 0x75, 0x84, 0xc6,
    0x4a, 0x95, 0x7d, 0xba, 0xb8, 0x41, 0x35, 0x6f, 0x85, 0x23, 0x8e, 0xc1,
    0xf8, 0x5a, 0x42, 0xec, 0x92, 0x08, 0x74, 0x13, 0x1b, 0xa6, 0x04, 0x23,
    0xd5, 0xb5, 0x34, 0x99, 0xbc, 0x55, 0xc6, 0xfe, 0x96, 0x5f, 0x16, 0xa5,
    0x02, 0xa1, 0xf1, 0x21, 0x23, 0xc4, 0x93, 0x20, 0xfd, 0x2e, 0x6a, 0xa0,
    0xca, 0x4c, 0xc3, 0x02, 0x17, 0x97, 0xaa, 0x26, 0xa8, 0x2b, 0x5b, 0xbf,
    0xdd, 0xa7, 0x48, 0x01, 0x08, 0xa8, 0x02, 0x18, 0x3e, 0xc8, 0xb2, 0x85,
    0x05, 0xa8, 0x10, 0xb5, 0x84, 0x11, 0x77, 0x07, 0xd1, 0xbe, 0x30, 0x0a,
    0xa6, 0xca, 0x5e, 0x93, 0x19, 0xe2, 0x19, 0x52, 0x15, 0x55, 0xc7, 0x95,
    0xab, 0x1b, 0x8b, 0x3f, 0x49, 0xad, 0x80, 0xc4, 0xb0, 0x09, 0x0a, 0xdd,
    0x6b, 0xd7, 0x15, 0x02, 0x81, 0x81, 0x00, 0xcb, 0xc9, 0xc4, 0x37, 0xf1,
    0x6f, 0x65, 0x7c, 0x8b, 0x95, 0x76, 0xb6, 0x7b, 0x2e, 0x83, 0xa3, 0x07,
    0xc2, 0x0a, 0x07, 0x45, 0x70, 0xfd, 0xa2, 0xd0, 0xf1, 0xea, 0x5e, 0x0a,
    0xab, 0xb7, 0xde, 0xed, 0x24, 0xed, 0xf2, 0x0d, 0xc6, 0x60, 0xbe, 0xca,
    0x8b, 0x32, 0x39, 0xee, 0x1c, 0x87, 0x53, 0xcb, 0x48, 0x27, 0xbc, 0xcc,
    0x58, 0x97, 0x14, 0x32, 0xef, 0x71, 0x93, 0x56, 0x6d, 0x34, 0xd1, 0xe4,
    0xfd, 0xa3, 0xe1, 0xbf, 0x1a, 0x26, 0xac, 0x5f, 0xd4, 0x4d, 0x90, 0xa3,
    0x83, 0x73, 0x21, 0x8c, 0x00, 0x97, 0xf5, 0x7f, 0xf8, 0xdf, 0x12, 0x7c,
    0xb3, 0x2e, 0xf1, 0x5e, 0xa0, 0xeb, 0x8b, 0xd8, 0x5c, 0x8f, 0x78, 0x1b,
    0x67, 0xaf, 0x07, 0xbe, 0xf8, 0x44, 0x2b, 0x40, 0x24, 0x56, 0x36, 0x47,
    0x75, 0x82, 0x31, 0xa0, 0xb4, 0x5a, 0x21, 0x6f, 0xbe, 0x84, 0x40, 0x16,
    0x34, 0xdb, 0x61};
const unsigned int iot_x509_private_key_len = sizeof(iot_x509_private_key);

NX_SECURE_X509_CERT device_certificate;

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
    const UCHAR* method_name;
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
    NX_AZURE_IOT_JSON_READER json_reader;
    UCHAR buffer[PUBLISH_BUFFER_SIZE];

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
        nx_packet_release(packet_ptr);
        return;
    }

    if ((status = nx_azure_iot_json_reader_init(&json_reader, packet_ptr)))
    {
        printf("ERROR: failed to initialize json reader (0x%08x)\r\n", status);
        nx_packet_release(packet_ptr);
        return;
    }

    if (nx_context->device_twin_get_cb)
    {
        if ((status = nx_azure_iot_pnp_helper_twin_data_parse(&json_reader,
                 NX_FALSE,
                 NX_NULL,
                 0,
                 buffer,
                 PUBLISH_BUFFER_SIZE,
                 nx_context->device_twin_get_cb,
                 nx_context)))
        {
            printf("ERROR: failed to parse twin data (0x%08x)\r\n", status);
        }
    }

    // Deinit the reader, the reader owns the NX_PACKET at this point, so will release it
    nx_azure_iot_json_reader_deinit(&json_reader);
}

static VOID process_device_twin_desired_property(AZURE_IOT_NX_CONTEXT* nx_context)
{
    UINT status;
    NX_PACKET* packet_ptr;
    NX_AZURE_IOT_JSON_READER json_reader;
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
        nx_packet_release(packet_ptr);
        return;
    }

    if ((status = nx_azure_iot_json_reader_init(&json_reader, packet_ptr)))
    {
        printf("ERROR: failed to initialize json reader (0x%08x)\r\n", status);
        nx_packet_release(packet_ptr);
        return;
    }

    if (nx_context->device_twin_desired_prop_cb)
    {
        if ((status = nx_azure_iot_pnp_helper_twin_data_parse(&json_reader,
                 NX_TRUE,
                 NX_NULL,
                 0,
                 buffer,
                 sizeof(buffer),
                 nx_context->device_twin_desired_prop_cb,
                 nx_context)))
        {
            printf("ERROR: failed to parse twin data (0x%08x)\r\n", status);
        }
    }

    // Deinit the reader, the reader owns the NX_PACKET at this point, so will release it
    nx_azure_iot_json_reader_deinit(&json_reader);
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
 /*  if ((status = nx_azure_iot_hub_client_symmetric_key_set(
             &context->iothub_client, (UCHAR*)device_sas_key, strlen(device_sas_key))))
    {
        printf("ERROR: nx_azure_iot_hub_client_symmetric_key_set (0x%08x)\r\n", status);
    } */

    if ((status = nx_azure_iot_hub_client_device_cert_set(
               &context->iothub_client, &device_certificate))) {
      printf("Failed on nx_azure_iot_hub_client_device_cert_set!: error code = "
             "0x%08x\r\n",
             status);
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
    CHAR payload[DPS_PAYLOAD_SIZE];
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

    if (snprintf(payload, sizeof(payload), DPS_PAYLOAD, device_model_id) > DPS_PAYLOAD_SIZE - 1)
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
 /*   if ((status = nx_azure_iot_provisioning_client_symmetric_key_set(
             &context->prov_client, (UCHAR*)device_sas_key, strlen(device_sas_key))))
    {
        printf("Failed on nx_azure_iot_hub_client_symmetric_key_set (0x%08x)\r\n", status);
    }*/

    if ((status = nx_secure_x509_certificate_initialize(
             &device_certificate, (UCHAR *)iot_x509_device_cert,
             (USHORT)iot_x509_device_cert_len, NX_NULL, 0,
             (UCHAR *)iot_x509_private_key, (USHORT)iot_x509_private_key_len,
             NX_SECURE_X509_KEY_TYPE_RSA_PKCS1_DER))) {
      printf("Failed on nx_secure_x509_certificate_initialize!: error code = "
             "0x%08x\r\n",
             status);
    }

    if ((status = nx_azure_iot_provisioning_client_device_cert_set(&context->prov_client,
                                                                   &device_certificate))) {
      printf("Failed on nx_azure_iot_hub_client_device_cert_set!: error code = "
             "0x%08x\r\n",
             status);
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

    if (status != NX_SUCCESS)
    {
        return status;
    }

    // Null terminate returned values
    context->azure_iot_hub_hostname[iot_hub_hostname_len] = 0;
    context->azure_iot_device_id[iot_device_id_len]       = 0;

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
    CHAR buffer[PUBLISH_BUFFER_SIZE];
    NX_PACKET* packet_ptr;

    int intvalue  = value;
    int fracvalue = abs(100 * (value - (long)value));

    if (snprintf(buffer, PUBLISH_BUFFER_SIZE, "{\"%s\":%d.%2d}", key, intvalue, fracvalue) > PUBLISH_BUFFER_SIZE - 1)
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
    ULONG version;
    CHAR buffer[PUBLISH_BUFFER_SIZE];

    int intvalue  = value;
    int fracvalue = abs(100 * (value - (long)value));

    if (snprintf(buffer, PUBLISH_BUFFER_SIZE, "{\"%s\":%d.%2d}", key, intvalue, fracvalue) > PUBLISH_BUFFER_SIZE - 1)
    {
        printf("ERROR: insufficient buffer size to publish float property\r\n");
        return NX_SIZE_ERROR;
    }

    if ((status = nx_azure_iot_hub_client_device_twin_reported_properties_send(&context->iothub_client,
             (UCHAR*)buffer,
             strlen(buffer),
             &request_id,
             &response_status,
             &version,
             NX_WAIT_FOREVER)))
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
    ULONG version;
    CHAR buffer[PUBLISH_BUFFER_SIZE];

    if (snprintf(buffer, PUBLISH_BUFFER_SIZE, "{\"%s\":%s}", key, (value ? "true" : "false")) > PUBLISH_BUFFER_SIZE - 1)
    {
        printf("ERROR: Unsufficient buffer size to publish bool property\r\n");
        return NX_SIZE_ERROR;
    }

    if ((status = nx_azure_iot_hub_client_device_twin_reported_properties_send(&context->iothub_client,
             (UCHAR*)buffer,
             strlen(buffer),
             &request_id,
             &response_status,
             &version,
             NX_WAIT_FOREVER)))
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
    ULONG version_dt;
    CHAR message[PUBLISH_BUFFER_SIZE];

    printf("Responding to writeable property %s = %d\r\n", property, value);

    if (snprintf(message,
            PUBLISH_BUFFER_SIZE,
            "{\"%s\":{\"value\":%d,\"ac\":%d,\"av\":%d}}",
            property,
            value,
            http_status,
            version) > PUBLISH_BUFFER_SIZE - 1)
    {
        printf("ERROR: insufficient buffer size to respond to writeable property\r\n");
        return NX_SIZE_ERROR;
    }

    if ((status = nx_azure_iot_hub_client_device_twin_reported_properties_send(&context->iothub_client,
             (UCHAR*)message,
             strlen(message),
             &request_id,
             &response_status,
             &version_dt,
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
