/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "nx_client.h"

#include <stdio.h>

#include "nx_api.h"
#include "nx_azure_iot_hub_client.h"
#include "nx_azure_iot_provisioning_client.h"

/* These are sample files, user can build their own certificate and
 * ciphersuites.  */
#include "azure_iot_cert.h"
#include "azure_iot_ciphersuites.h"
//#include "sample_config.h"

#include "azure_config.h"

#define NX_AZURE_IOT_STACK_SIZE      2048
#define NX_AZURE_IOT_THREAD_PRIORITY 3
#define SAMPLE_STACK_SIZE            2048
#define SAMPLE_THREAD_PRIORITY       16

//#define MAX_PROPERTY_COUNT     2

#define MODULE_ID ""

/* Define Azure RTOS TLS info.  */
static NX_SECURE_X509_CERT root_ca_cert;
static UCHAR nx_azure_iot_tls_metadata_buffer[NX_AZURE_IOT_TLS_METADATA_BUFFER_SIZE];
static ULONG nx_azure_iot_thread_stack[NX_AZURE_IOT_STACK_SIZE / sizeof(ULONG)];

/* Define the prototypes for AZ IoT.  */
static NX_AZURE_IOT nx_azure_iot;
static NX_AZURE_IOT_HUB_CLIENT iothub_client;
#ifdef ENABLE_DPS_SAMPLE
static NX_AZURE_IOT_PROVISIONING_CLIENT prov_client;
#endif /* ENABLE_DPS_SAMPLE */

/* Define buffer for IoTHub info. */
#ifdef ENABLE_DPS_SAMPLE
static UCHAR sample_iothub_hostname[SAMPLE_MAX_BUFFER];
static UCHAR sample_iothub_device_id[SAMPLE_MAX_BUFFER];
#endif /* ENABLE_DPS_SAMPLE */

/* Define sample threads. */
static TX_THREAD sample_telemetry_thread;
static ULONG sample_telemetry_thread_stack[SAMPLE_STACK_SIZE / sizeof(ULONG)];

static TX_THREAD sample_c2d_thread;
static ULONG sample_c2d_thread_stack[SAMPLE_STACK_SIZE / sizeof(ULONG)];

/* Define sample properties.  */
// static const CHAR* sample_properties[MAX_PROPERTY_COUNT][2] = {{"propertyA", "valueA"}, {"propertyB", "valueB"}};

static CHAR method_response_payload[] = "{\"status\": \"OK\"}";
static TX_THREAD sample_direct_method_thread;
static ULONG sample_direct_method_thread_stack[SAMPLE_STACK_SIZE / sizeof(ULONG)];

static CHAR fixed_reported_properties[] = "{\"sample_report\": \"OK\"}";
static TX_THREAD sample_device_twin_thread;
static ULONG sample_device_twin_thread_stack[SAMPLE_STACK_SIZE / sizeof(ULONG)];

#ifdef ENABLE_DPS_SAMPLE
static UINT sample_dps_entry(
    UCHAR** iothub_hostname, UINT* iothub_hostname_length, UCHAR** iothub_device_id, UINT* iothub_device_id_length);
#endif /* ENABLE_DPS_SAMPLE */

static void sample_telemetry_thread_entry(ULONG parameter);
static void sample_c2d_thread_entry(ULONG parameter);
static void sample_direct_method_thread_entry(ULONG parameter);
static void sample_device_twin_thread_entry(ULONG parameter);

static VOID printf_packet(NX_PACKET* packet_ptr)
{
    while (packet_ptr != NX_NULL)
    {
        printf("%.*s",
            (INT)(packet_ptr->nx_packet_append_ptr - packet_ptr->nx_packet_prepend_ptr),
            (CHAR*)packet_ptr->nx_packet_prepend_ptr);
        packet_ptr = packet_ptr->nx_packet_next;
    }
}

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

UINT azure_iot_embedded_sdk_entry(
    NX_IP* ip_ptr, NX_PACKET_POOL* pool_ptr, NX_DNS* dns_ptr, UINT (*unix_time_callback)(ULONG* unix_time))
{
    UINT status = 0;
#ifdef ENABLE_DPS_SAMPLE
    UCHAR* iothub_hostname       = NX_NULL;
    UCHAR* iothub_device_id      = NX_NULL;
    UINT iothub_hostname_length  = 0;
    UINT iothub_device_id_length = 0;
#else
    UCHAR* iothub_hostname       = (UCHAR*)IOT_HUB_HOSTNAME;
    UCHAR* iothub_device_id      = (UCHAR*)IOT_DEVICE_ID;
    UINT iothub_hostname_length  = sizeof(IOT_HUB_HOSTNAME) - 1;
    UINT iothub_device_id_length = sizeof(IOT_DEVICE_ID) - 1;
#endif /* ENABLE_DPS_SAMPLE */

    /* Create Azure IoT handler.  */
    if ((status = nx_azure_iot_create(&nx_azure_iot,
             (UCHAR*)"Azure IoT",
             ip_ptr,
             pool_ptr,
             dns_ptr,
             nx_azure_iot_thread_stack,
             sizeof(nx_azure_iot_thread_stack),
             NX_AZURE_IOT_THREAD_PRIORITY,
             unix_time_callback)))
    {
        printf("Failed on nx_azure_iot_create!: error code = 0x%08x\r\n", status);
        return status;
    }

    /* Initialize CA certificate. */
    if ((status = nx_secure_x509_certificate_initialize(&root_ca_cert,
             (UCHAR*)azure_iot_root_ca,
             (USHORT)azure_iot_root_ca_len,
             NX_NULL,
             0,
             NULL,
             0,
             NX_SECURE_X509_KEY_TYPE_NONE)))
    {
        printf("Failed to initialize ROOT CA certificate!: error code = 0x%08x\r\n", status);
        nx_azure_iot_delete(&nx_azure_iot);
        return status;
    }

#ifdef ENABLE_DPS_SAMPLE
    /* Run DPS. */
    if ((status = sample_dps_entry(
             &iothub_hostname, &iothub_hostname_length, &iothub_device_id, &iothub_device_id_length)))
    {
        printf("Failed on sample_dps_entry!: error code = 0x%08x\r\n", status);
        nx_azure_iot_delete(&nx_azure_iot);
        return status;
    }
#endif /* ENABLE_DPS_SAMPLE */

    /* Initialize IoTHub client. */
    if ((status = nx_azure_iot_hub_client_initialize(&iothub_client,
             &nx_azure_iot,
             iothub_hostname,
             iothub_hostname_length,
             iothub_device_id,
             iothub_device_id_length,
             (UCHAR*)MODULE_ID,
             sizeof(MODULE_ID) - 1,
             _nx_azure_iot_tls_supported_crypto,
             _nx_azure_iot_tls_supported_crypto_size,
             _nx_azure_iot_tls_ciphersuite_map,
             _nx_azure_iot_tls_ciphersuite_map_size,
             nx_azure_iot_tls_metadata_buffer,
             sizeof(nx_azure_iot_tls_metadata_buffer),
             &root_ca_cert)))
    {
        printf("Failed on nx_azure_iot_hub_client_initialize!: error code = "
               "0x%08x\r\n",
            status);
        nx_azure_iot_delete(&nx_azure_iot);
        return status;
    }

    /* Set symmetric key.  */
    if ((status = nx_azure_iot_hub_client_symmetric_key_set(
             &iothub_client, (UCHAR*)IOT_PRIMARY_KEY, sizeof(IOT_PRIMARY_KEY) - 1)))
    {
        printf("Failed on nx_azure_iot_hub_client_symmetric_key_set!\r\n");
        return status;
    }

    /* Set connection status callback. */
    if (nx_azure_iot_hub_client_connection_status_callback_set(&iothub_client, connection_status_callback))
    {
        printf("Failed on connection_status_callback!\r\n");
        return status;
    }

    /* Connect to IoTHub client. */
    if (nx_azure_iot_hub_client_connect(&iothub_client, NX_TRUE, NX_WAIT_FOREVER))
    {
        printf("Failed on nx_azure_iot_hub_client_connect!\r\n");
        return status;
    }

    /* Create Telemetry sample thread. */
    if ((status = tx_thread_create(&sample_telemetry_thread,
             "Sample Telemetry Thread",
             sample_telemetry_thread_entry,
             0,
             (UCHAR*)sample_telemetry_thread_stack,
             SAMPLE_STACK_SIZE,
             SAMPLE_THREAD_PRIORITY,
             SAMPLE_THREAD_PRIORITY,
             1,
             TX_AUTO_START)))
    {
        printf("Failed to create telemetry sample thread!: error code = 0x%08x\r\n", status);
        return status;
    }

    /* Create C2D sample thread. */
    if ((status = tx_thread_create(&sample_c2d_thread,
             "Sample C2D Thread",
             sample_c2d_thread_entry,
             0,
             (UCHAR*)sample_c2d_thread_stack,
             SAMPLE_STACK_SIZE,
             SAMPLE_THREAD_PRIORITY,
             SAMPLE_THREAD_PRIORITY,
             1,
             TX_AUTO_START)))
    {
        printf("Failed to create c2d sample thread!: error code = 0x%08x\r\n", status);
        return status;
    }

    /* Create Direct Method sample thread. */
    if ((status = tx_thread_create(&sample_direct_method_thread,
             "Sample Direct Method Thread",
             sample_direct_method_thread_entry,
             0,
             (UCHAR*)sample_direct_method_thread_stack,
             SAMPLE_STACK_SIZE,
             SAMPLE_THREAD_PRIORITY,
             SAMPLE_THREAD_PRIORITY,
             1,
             TX_AUTO_START)))
    {
        printf("Failed to create direct method sample thread!: error code = "
               "0x%08x\r\n",
            status);
        return status;
    }

    /* Create Device twin sample thread. */
    if ((status = tx_thread_create(&sample_device_twin_thread,
             "Sample Device Twin Thread",
             sample_device_twin_thread_entry,
             0,
             (UCHAR*)sample_device_twin_thread_stack,
             SAMPLE_STACK_SIZE,
             SAMPLE_THREAD_PRIORITY,
             SAMPLE_THREAD_PRIORITY,
             1,
             TX_AUTO_START)))
    {
        printf("Failed to create device twin sample thread!: error code = "
               "0x%08x\r\n",
            status);
        return status;
    }

    /* Simply loop in sample. */
    while (true)
    {
        tx_thread_sleep(NX_IP_PERIODIC_RATE);
    }

    /* Destroy IoTHub Client.  */
    nx_azure_iot_hub_client_disconnect(&iothub_client);
    nx_azure_iot_hub_client_deinitialize(&iothub_client);
    nx_azure_iot_delete(&nx_azure_iot);
}

#ifdef ENABLE_DPS_SAMPLE
static UINT sample_dps_entry(
    UCHAR** iothub_hostname, UINT* iothub_hostname_length, UCHAR** iothub_device_id, UINT* iothub_device_id_length)
{
    UINT status;

    /* Initialize IoT provisioning client.  */
    if ((status = nx_azure_iot_provisioning_client_initialize(&prov_client,
             &nx_azure_iot,
             (UCHAR*)ENDPOINT,
             sizeof(ENDPOINT) - 1,
             (UCHAR*)ID_SCOPE,
             sizeof(ID_SCOPE) - 1,
             (UCHAR*)REGISTRATION_ID,
             sizeof(REGISTRATION_ID) - 1,
             _nx_azure_iot_tls_supported_crypto,
             _nx_azure_iot_tls_supported_crypto_size,
             _nx_azure_iot_tls_ciphersuite_map,
             _nx_azure_iot_tls_ciphersuite_map_size,
             nx_azure_iot_tls_metadata_buffer,
             sizeof(nx_azure_iot_tls_metadata_buffer),
             &root_ca_cert)))
    {
        printf("Failed on nx_azure_iot_provisioning_client_initialize!: error code "
               "= 0x%08x\r\n",
            status);
        return (status);
    }

    /* Initialize length of hostname and device ID. */
    *iothub_hostname_length  = sizeof(sample_iothub_hostname);
    *iothub_device_id_length = sizeof(sample_iothub_device_id);

    /* Set symmetric key.  */
    if ((status = nx_azure_iot_provisioning_client_symmetric_key_set(
             &prov_client, (UCHAR*)DEVICE_SYMMETRIC_KEY, sizeof(DEVICE_SYMMETRIC_KEY) - 1)))
    {
        printf("Failed on nx_azure_iot_hub_client_symmetric_key_set!: error code = "
               "0x%08x\r\n",
            status);
    }

    /* Register device */
    else if ((status = nx_azure_iot_provisioning_client_register(&prov_client, NX_WAIT_FOREVER)))
    {
        printf("Failed on nx_azure_iot_provisioning_client_register!: error code = "
               "0x%08x\r\n",
            status);
    }

    /* Get Device info */
    else if ((status = nx_azure_iot_provisioning_client_iothub_device_info_get(&prov_client,
                  sample_iothub_hostname,
                  iothub_hostname_length,
                  sample_iothub_device_id,
                  iothub_device_id_length)))
    {
        printf("Failed on nx_azure_iot_provisioning_client_iothub_device_info_get!: "
               "error code = 0x%08x\r\n",
            status);
    }
    else
    {
        *iothub_hostname  = sample_iothub_hostname;
        *iothub_device_id = sample_iothub_device_id;
    }

    /* Destroy Provisioning Client.  */
    nx_azure_iot_provisioning_client_deinitialize(&prov_client);

    return (status);
}
#endif /* ENABLE_DPS_SAMPLE */

void sample_telemetry_thread_entry(ULONG parameter)
{
    UINT i      = 0;
    UINT status = 0;
    CHAR buffer[30];
    UINT buffer_length;
    NX_PACKET* packet_ptr;

    CHAR* temperature_property = "temperature";
    CHAR* temperature_value    = "28.5";

    NX_PARAMETER_NOT_USED(parameter);

    /* Loop to send telemetry message.  */
    while (true)
    {

        /* Create a telemetry message packet. */
        if ((status = nx_azure_iot_hub_client_telemetry_message_create(&iothub_client, &packet_ptr, NX_WAIT_FOREVER)))
        {
            printf("Telemetry message create failed!: error code = 0x%08x\r\n", status);
            break;
        }

        /* Add properties to telemetry message. */
        if ((status = nx_azure_iot_hub_client_telemetry_property_add(packet_ptr,
                 (UCHAR*)temperature_property,
                 strlen(temperature_property),
                 (UCHAR*)temperature_value,
                 strlen(temperature_value),
                 NX_WAIT_FOREVER)))
        {
            printf("Telemetry property add failed!: error code = 0x%08x\r\n", status);
            nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);
            break;
        }

        buffer_length = (UINT)snprintf(buffer, sizeof(buffer), "{\"Message ID\":%u}", i++);
        if ((status = nx_azure_iot_hub_client_telemetry_send(
                 &iothub_client, packet_ptr, (UCHAR*)buffer, buffer_length, NX_WAIT_FOREVER)))
        {
            printf("Telemetry message send failed!: error code = 0x%08x\r\n", status);
            nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);
            break;
        }
        printf("Telemetry message send: %s.\r\n", buffer);

        tx_thread_sleep(10 * NX_IP_PERIODIC_RATE);
    }
}

void sample_c2d_thread_entry(ULONG parameter)
{
    NX_PACKET* packet_ptr;
    UINT status = 0;
    USHORT property_buf_size;
    UCHAR* property_buf;
    CHAR* temperature_property = "temperature";

    NX_PARAMETER_NOT_USED(parameter);

    if ((status = nx_azure_iot_hub_client_cloud_message_enable(&iothub_client)))
    {
        printf("C2D receive enable failed!: error code = 0x%08x\r\n", status);
        return;
    }

    /* Loop to receive c2d message.  */
    while (true)
    {
        if ((status = nx_azure_iot_hub_client_cloud_message_receive(&iothub_client, &packet_ptr, NX_WAIT_FOREVER)))
        {
            printf("C2D receive failed!: error code = 0x%08x\r\n", status);
            break;
        }

        if ((status = nx_azure_iot_hub_client_cloud_message_property_get(&iothub_client,
                 packet_ptr,
                 (UCHAR*)temperature_property,
                 (USHORT)strlen(temperature_property),
                 &property_buf,
                 &property_buf_size)))
        {
            printf("Property [%s] not found: 0x%08x\r\n", temperature_property, status);
        }
        else
        {
            printf("Receive property: %s = %.*s\r\n", temperature_property, (INT)property_buf_size, property_buf);
        }

        printf("Receive message:");
        printf_packet(packet_ptr);
        printf("\r\n");

        nx_packet_release(packet_ptr);
    }
}

void sample_direct_method_thread_entry(ULONG parameter)
{
    NX_PACKET* packet_ptr;
    UINT status = 0;
    USHORT method_name_length;
    UCHAR* method_name_ptr;
    USHORT context_length;
    VOID* context_ptr;

    NX_PARAMETER_NOT_USED(parameter);

    if ((status = nx_azure_iot_hub_client_direct_method_enable(&iothub_client)))
    {
        printf("Direct method receive enable failed!: error code = 0x%08x\r\n", status);
        return;
    }

    /* Loop to receive direct method message.  */
    while (true)
    {
        if ((status = nx_azure_iot_hub_client_direct_method_message_receive(&iothub_client,
                 &method_name_ptr,
                 &method_name_length,
                 &context_ptr,
                 &context_length,
                 &packet_ptr,
                 NX_WAIT_FOREVER)))
        {
            printf("Direct method receive failed!: error code = 0x%08x\r\n", status);
            break;
        }

        printf("Receive method call: %.*s, with payload:", (INT)method_name_length, (CHAR*)method_name_ptr);
        printf_packet(packet_ptr);
        printf("\r\n");

        if ((status = nx_azure_iot_hub_client_direct_method_message_response(&iothub_client,
                 200 /* method status */,
                 context_ptr,
                 context_length,
                 (UCHAR*)method_response_payload,
                 sizeof(method_response_payload) - 1,
                 NX_WAIT_FOREVER)))
        {
            printf("Direct method response failed!: error code = 0x%08x\r\n", status);
            break;
        }

        nx_packet_release(packet_ptr);
    }
}

void sample_device_twin_thread_entry(ULONG parameter)
{
    NX_PACKET* packet_ptr;
    UINT status = 0;
    UINT response_status;
    UINT request_id;

    NX_PARAMETER_NOT_USED(parameter);

    if ((status = nx_azure_iot_hub_client_device_twin_enable(&iothub_client)))
    {
        printf("device twin enabled failed!: error code = 0x%08x\r\n", status);
        return;
    }

    if ((status = nx_azure_iot_hub_client_device_twin_properties_request(&iothub_client, NX_WAIT_FOREVER)))
    {
        printf("device twin document request failed!: error code = 0x%08x\r\n", status);
        return;
    }

    if ((status = nx_azure_iot_hub_client_device_twin_properties_receive(&iothub_client, &packet_ptr, NX_WAIT_FOREVER)))
    {
        printf("device twin document receive failed!: error code = 0x%08x\r\n", status);
        return;
    }

    printf("Receive twin properties :");
    printf_packet(packet_ptr);
    printf("\r\n");
    nx_packet_release(packet_ptr);

    /* Loop to receive device twin message.  */
    while (true)
    {
        if ((status = nx_azure_iot_hub_client_device_twin_desired_properties_receive(
                 &iothub_client, &packet_ptr, NX_WAIT_FOREVER)))
        {
            printf("Receive desired property receive failed!: error code = 0x%08x\r\n", status);
            break;
        }

        printf("Receive desired property call: ");
        printf_packet(packet_ptr);
        printf("\r\n");
        nx_packet_release(packet_ptr);

        if ((status = nx_azure_iot_hub_client_device_twin_reported_properties_send(&iothub_client,
                 (UCHAR*)fixed_reported_properties,
                 sizeof(fixed_reported_properties) - 1,
                 &request_id,
                 &response_status,
                 NX_WAIT_FOREVER)))
        {
            printf("Device twin reported properties failed!: error code = 0x%08x\r\n", status);
            break;
        }

        if ((response_status < 200) || (response_status >= 300))
        {
            printf("device twin report properties failed with code : %d\r\n", response_status);
            break;
        }
    }
}
