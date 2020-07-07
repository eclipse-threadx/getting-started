/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "azure_iot_nx_client.h"

#include <stdio.h>

#include "azure_iot_cert.h"
#include "azure_iot_ciphersuites.h"

#define NX_AZURE_IOT_THREAD_PRIORITY 3
#define THREAD_PRIORITY              16

#define MODULE_ID ""

static VOID connection_status_callback(NX_AZURE_IOT_HUB_CLIENT* hub_client_ptr, UINT status)
{
    NX_PARAMETER_NOT_USED(hub_client_ptr);
    if (status)
    {
        printf("Disconnected from IoTHub!: error code = 0x%08x\r\n", status);
    }
    else
    {
        printf("Connected to IoTHub.\r\n");
    }
}

UINT azure_iot_nx_client_create(AZURE_IOT_NX_CLIENT* azure_iot_nx_client,
    NX_IP* nx_ip,
    NX_PACKET_POOL* nx_pool,
    NX_DNS* nx_dns,
    UINT (*unix_time_callback)(ULONG* unix_time),
    CHAR* iot_hub_hostname,
    CHAR* iot_device_id,
    CHAR* iot_sas_key)
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

    if (iot_hub_hostname[0] == 0 || iot_device_id[0] == 0 || iot_sas_key[0] == 0)
    {
        printf("ERROR: IoT Hub connection configuration is empty\r\n");
        return NX_PTR_ERROR;
    }

    memset(azure_iot_nx_client, 0, sizeof(&azure_iot_nx_client));

    /* Create Azure IoT handler. */
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
        printf("Failed on nx_azure_iot_create!: error code = 0x%08x\r\n", status);
        return status;
    }

    /* Initialize CA certificate. */
    if ((status = nx_secure_x509_certificate_initialize(&azure_iot_nx_client->root_ca_cert,
             (UCHAR*)azure_iot_root_ca,
             (USHORT)azure_iot_root_ca_len,
             NX_NULL,
             0,
             NULL,
             0,
             NX_SECURE_X509_KEY_TYPE_NONE)))
    {
        printf("Failed to initialize ROOT CA certificate!: error code = 0x%08x\r\n", status);
        nx_azure_iot_delete(&azure_iot_nx_client->nx_azure_iot);
        return status;
    }

    /* Initialize IoTHub client. */
    if ((status = nx_azure_iot_hub_client_initialize(&azure_iot_nx_client->iothub_client,
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
             azure_iot_nx_client->nx_azure_iot_tls_metadata_buffer,
             sizeof(azure_iot_nx_client->nx_azure_iot_tls_metadata_buffer),
             &azure_iot_nx_client->root_ca_cert)))
    {
        printf("Failed on nx_azure_iot_hub_client_initialize!: error code = 0x%08x\r\n", status);
        nx_azure_iot_delete(&azure_iot_nx_client->nx_azure_iot);
        return status;
    }

    /* Set symmetric key.  */
    if ((status = nx_azure_iot_hub_client_symmetric_key_set(
             &azure_iot_nx_client->iothub_client, (UCHAR*)iot_sas_key, strlen(iot_sas_key))))
    {
        printf("Failed on nx_azure_iot_hub_client_symmetric_key_set!\r\n");
        return status;
    }

    /* Set connection status callback. */
    if (nx_azure_iot_hub_client_connection_status_callback_set(
            &azure_iot_nx_client->iothub_client, connection_status_callback))
    {
        printf("Failed on connection_status_callback!\r\n");
        return status;
    }

    return NX_SUCCESS;
}

UINT azure_iot_nx_client_delete(AZURE_IOT_NX_CLIENT* azure_iot_nx_client)
{
    /* Destroy IoTHub Client. */
    nx_azure_iot_hub_client_disconnect(&azure_iot_nx_client->iothub_client);
    nx_azure_iot_hub_client_deinitialize(&azure_iot_nx_client->iothub_client);
    nx_azure_iot_delete(&azure_iot_nx_client->nx_azure_iot);

    return NX_SUCCESS;
}

UINT azure_iot_nx_client_connect(AZURE_IOT_NX_CLIENT* azure_iot_nx_client)
{
    UINT status;

    // Connect to IoTHub client
    if ((status = nx_azure_iot_hub_client_connect(&azure_iot_nx_client->iothub_client, NX_TRUE, NX_WAIT_FOREVER)))
    {
        printf("Failed on nx_azure_iot_hub_client_connect = 0x%08x\r\n", status);
        return status;
    }

    // Enable the device twin
    if ((status = nx_azure_iot_hub_client_device_twin_enable(&azure_iot_nx_client->iothub_client)))
    {
        printf("device twin enabled failed!: error code = 0x%08x\r\n", status);
        return status;
    }

    // Telemetry thread
    if (azure_iot_nx_client->telemetry_thread_entry != NULL)
    {
        if ((status = tx_thread_create(&azure_iot_nx_client->telemetry_thread,
                 "Telemetry Thread",
                 azure_iot_nx_client->telemetry_thread_entry,
                 0,
                 (UCHAR*)azure_iot_nx_client->telemetry_thread_stack,
                 AZURE_IOT_NX_STACK_SIZE,
                 THREAD_PRIORITY,
                 THREAD_PRIORITY,
                 1,
                 TX_AUTO_START)))
        {
            printf("Failed to create telemetry thread!: error code = 0x%08x\r\n", status);
            return status;
        }
    }

    // Device twin thread
    if (azure_iot_nx_client->device_twin_thread_entry != NULL)
    {
        if ((status = tx_thread_create(&azure_iot_nx_client->device_twin_thread,
                 "Device Twin Thread",
                 azure_iot_nx_client->device_twin_thread_entry,
                 0,
                 (UCHAR*)azure_iot_nx_client->device_twin_thread_stack,
                 AZURE_IOT_NX_STACK_SIZE,
                 THREAD_PRIORITY,
                 THREAD_PRIORITY,
                 1,
                 TX_AUTO_START)))
        {
            printf("Failed to create device twin thread!: error code = 0x%08x\r\n", status);
            return status;
        }
    }

    // Direct Method thread
    if (azure_iot_nx_client->direct_method_thread_entry != NULL)
    {
        if ((status = nx_azure_iot_hub_client_direct_method_enable(&azure_iot_nx_client->iothub_client)))
        {
            printf("Direct method receive enable failed!: error code = 0x%08x\r\n", status);
            return status;
        }
        if ((status = tx_thread_create(&azure_iot_nx_client->direct_method_thread,
                 "Direct Method Thread ",
                 azure_iot_nx_client->direct_method_thread_entry,
                 0,
                 (UCHAR*)azure_iot_nx_client->direct_method_thread_stack,
                 AZURE_IOT_NX_STACK_SIZE,
                 THREAD_PRIORITY,
                 THREAD_PRIORITY,
                 1,
                 TX_AUTO_START)))
        {
            printf("Failed to create direct method thread!: error code = 0x%08x\r\n", status);
            return status;
        }
    }

    // C2D thread
    if (azure_iot_nx_client->c2d_thread_entry != NULL)
    {
        if ((status = nx_azure_iot_hub_client_cloud_message_enable(&azure_iot_nx_client->iothub_client)))
        {
            printf("C2D receive enable failed!: error code = 0x%08x\r\n", status);
            return status;
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
            printf("Failed to create c2d thread!: error code = 0x%08x\r\n", status);
            return status;
        }
    }

    return NX_SUCCESS;
}

UINT azure_iot_nx_client_disconnect(AZURE_IOT_NX_CLIENT* azure_iot_nx_client)
{
    nx_azure_iot_hub_client_disconnect(&azure_iot_nx_client->iothub_client);

    return NX_SUCCESS;
}

UINT azure_iot_nx_client_enable_telemetry(AZURE_IOT_NX_CLIENT* azure_iot_nx_client, threadx_entry telemetry_entry)
{
    azure_iot_nx_client->telemetry_thread_entry = telemetry_entry;
    return NX_SUCCESS;
}

UINT azure_iot_nx_client_enable_device_twin(AZURE_IOT_NX_CLIENT* azure_iot_nx_client, threadx_entry device_twin_entry)
{
    azure_iot_nx_client->device_twin_thread_entry = device_twin_entry;
    return NX_SUCCESS;
}

UINT azure_iot_nx_client_enable_direct_method(
    AZURE_IOT_NX_CLIENT* azure_iot_nx_client, threadx_entry direct_method_entry)
{
    azure_iot_nx_client->direct_method_thread_entry = direct_method_entry;
    return NX_SUCCESS;
}

UINT azure_iot_nx_client_enable_c2d(AZURE_IOT_NX_CLIENT* azure_iot_nx_client, threadx_entry c2d_entry)
{
    azure_iot_nx_client->c2d_thread_entry = c2d_entry;
    return NX_SUCCESS;
}

UINT azure_iot_nx_client_publish_float_telemetry(AZURE_IOT_NX_CLIENT* azure_iot_nx_client, CHAR* key, float value, NX_PACKET* packet_ptr)
{
    UINT status;
    CHAR buffer[30];

    snprintf(buffer, sizeof(buffer), "{\"%s\":%0.2f}", key, value);

    if ((status = nx_azure_iot_hub_client_telemetry_send(
             &azure_iot_nx_client->iothub_client, packet_ptr, (UCHAR*)buffer, strlen(buffer), NX_WAIT_FOREVER)))
    {
        printf("Telemetry message send failed!: error code = 0x%08x\r\n", status);
        nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);
        return status;
    }

    printf("Telemetry message sent: %s.\r\n", buffer);

    return NX_SUCCESS;
}

UINT azure_iot_nx_client_publish_float_property(AZURE_IOT_NX_CLIENT* azure_iot_nx_client, CHAR* key, float value)
{
    UINT status;
    UINT response_status;
    UINT request_id;
    CHAR buffer[30];

    snprintf(buffer, sizeof(buffer), "{\"%s\":%0.2f}", key, value);

    if ((status = nx_azure_iot_hub_client_device_twin_reported_properties_send(&azure_iot_nx_client->iothub_client,
             (UCHAR*)buffer,
             strlen(buffer),
             &request_id,
             &response_status,
             NX_WAIT_FOREVER)))
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

VOID printf_packet(NX_PACKET* packet_ptr)
{
    while (packet_ptr != NX_NULL)
    {
        printf("%.*s", (INT)(packet_ptr->nx_packet_length), (CHAR*)packet_ptr->nx_packet_prepend_ptr);
        packet_ptr = packet_ptr->nx_packet_next;
    }
    printf("\r\n");
}

bool findJsonInt(const char* json, jsmntok_t* tokens, int tokens_count, const char* s, int* value)
{
    for (int i = 1; i < tokens_count; i++)
    {
        if ((tokens[i].type == JSMN_STRING) && (strlen(s) == tokens[i].end - tokens[i].start) &&
            (strncmp(json + tokens[i].start, s, tokens[i].end - tokens[i].start) == 0))
        {
            *value = atoi(json + tokens[i + 1].start);

            printf("Desired property %s = %d\r\n", "telemetryInterval", *value);
            return true;
        }
    }

    return false;
}
