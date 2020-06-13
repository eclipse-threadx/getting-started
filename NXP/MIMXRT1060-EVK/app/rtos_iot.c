/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */
   
#include "rtos_iot.h"

#include <stdio.h>

#include "tx_api.h"
#include "nx_api.h"
#include "nx_azure_iot_hub_client.h"

#include "board.h"
#include "networking.h"
#include "sntp_client.h"
#include "nx_azure_iot_ciphersuites.h"

#include "cert.h"
//#include "azure/azure_mqtt.h"

#define MODULE_ID                                   ""
#define MAX_PROPERTY_COUNT                          2

#define NX_AZURE_IOT_STACK_SIZE                     (2048)
#define NX_AZURE_IOT_THREAD_PRIORITY                (3)

static ULONG nx_azure_iot_thread_stack[NX_AZURE_IOT_STACK_SIZE / sizeof(ULONG)];

static NX_SECURE_X509_CERT root_ca_cert;
//static AZURE_MQTT azure_mqtt;
static UCHAR nx_azure_iot_tls_metadata_buffer[NX_AZURE_IOT_TLS_METADATA_BUFFER_SIZE];
static NX_AZURE_IOT nx_azure_iot;
static NX_AZURE_IOT_HUB_CLIENT iothub_client;

static VOID printf_packet(NX_PACKET *packet_ptr)
{
    while (packet_ptr != NX_NULL)
    {
        printf("%.*s",
            (INT)(packet_ptr->nx_packet_append_ptr - packet_ptr->nx_packet_prepend_ptr),
            (CHAR *)packet_ptr-> nx_packet_prepend_ptr);
        packet_ptr = packet_ptr->nx_packet_next;
    }
}

/*static void set_led_state(bool level)
{
    if (level)
    {
        // Pin level set to "low" state
        printf("LED is turned ON\r\n");
        
        // The User LED on the board shares the same pin as ENET RST so is unusable
        // USER_LED_ON();
    }
    else
    {
        // Pin level set to "high" state
        printf("LED is turned OFF\r\n");

        // The User LED on the board shares the same pin as ENET RST so is unusable
        // USER_LED_OFF();
    }
}

static void mqtt_direct_method(CHAR *direct_method_name, CHAR *message, MQTT_DIRECT_METHOD_RESPONSE *response)
{
    // Default response - 501 Not Implemented
    int status = 501;
    if (strcmp(direct_method_name, "set_led_state") == 0)
    {
        // 'false' - turn LED off
        // 'true'  - turn LED on
        bool arg = (strcmp(message, "true") == 0);

        set_led_state(arg);

        // 204 No Content, the server successfully processed the request and is not returning any content.
        status = 204;

        // Update device twin property
        azure_mqtt_publish_bool_property(&azure_mqtt, "led0State", arg);

        printf("Direct method=%s invoked\r\n", direct_method_name);
    }
    else
    {
        printf("Received direct menthod=%s is unknown\r\n", direct_method_name);
    }
    
    response->status = status;
    strcpy(response->message, "{}");
    return;
}

static void mqtt_c2d_message(CHAR *key, CHAR *value)
{
    if (strcmp(key, "led0State") == 0)
    {
        // 'false' - turn LED off
        // 'true'  - turn LED on
        bool arg = (strcmp(value, "true") == 0);

        set_led_state(arg);

        // Update device twin property
        azure_mqtt_publish_bool_property(&azure_mqtt, key, arg);
    }
    else
    {
        // Update device twin property
        azure_mqtt_publish_string_property(&azure_mqtt, key, value);
    }

    printf("Property=%s updated with value=%s\r\n", key, value);
}

static void mqtt_device_twin_desired_prop(CHAR *message)
{
    printf("Received device twin updated properties: %s\r\n", message);
}*/

static CHAR method_response_payload[] = "{\"status\": \"OK\"}";

static void azure_iot_hub_direct_method_callback(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr, VOID *args)
{
    UINT status;
    NX_PACKET *packet_ptr;
    UCHAR *method_name_ptr;
    USHORT method_name_length;
    VOID *context_ptr;
    USHORT context_length;

    status = nx_azure_iot_hub_client_direct_method_message_receive(hub_client_ptr,
        &method_name_ptr, &method_name_length,
        &context_ptr, &context_length,
        &packet_ptr, NX_WAIT_FOREVER);
    if (status != NX_SUCCESS)
    {
        printf("Direct method receive failed!: error code = 0x%08x\r\n", status);
        return;
    }

    printf("Receive method call: %.*s, with payload:", (INT)method_name_length, (CHAR *)method_name_ptr);
    printf_packet(packet_ptr);
    printf("\r\n");

    status = nx_azure_iot_hub_client_direct_method_message_response(&iothub_client, 200 /* method status */,
        context_ptr, context_length,
        (UCHAR *)method_response_payload, sizeof(method_response_payload) - 1,
        NX_WAIT_FOREVER);
    if (status != NX_SUCCESS)
    {
        printf("Direct method response failed!: error code = 0x%08x\r\n", status);
        return;
    }

    nx_packet_release(packet_ptr);
}

static VOID connection_status_callback(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr, UINT status)
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

static UINT unix_time_get(ULONG* unix_time)
{
    *unix_time = sntp_time_get();
    return NX_SUCCESS;
}

UINT rtos_iot_run(CHAR *iot_hub_hostname, CHAR *iot_device_id, CHAR *iot_sas_key)
{
    UINT status;
    
    // Create Azure IoT handler
    if ((status = nx_azure_iot_create(&nx_azure_iot,
        (UCHAR*)"Azure IoT",
        &nx_ip,
        &nx_pool,
        &nx_dns_client,
        nx_azure_iot_thread_stack,
        sizeof(nx_azure_iot_thread_stack),
        NX_AZURE_IOT_THREAD_PRIORITY,
        unix_time_get)))
    {
        printf("Failed on nx_azure_iot_create!: error code = 0x%08x\r\n", status);
        return 1;
    }
    
    // Initialize CA certificate.
    if ((status = nx_secure_x509_certificate_initialize(
        &root_ca_cert,
        (UCHAR *)azure_iot_root_ca,
        (USHORT)azure_iot_root_ca_len,
        NX_NULL,
        0,
        NULL,
        0,
        NX_SECURE_X509_KEY_TYPE_NONE)))
    {
        printf("Failed to initialize ROOT CA certificate!: error code = 0x%08x\r\n", status);
        nx_azure_iot_delete(&nx_azure_iot);
        return 1;
    }
    
/*    if ((status = sample_dps_entry(
        iot_hub_hostname, strlen(iot_hub_hostname),
        iot_device_id, strlen(iot_device_id))
    {
        printf("Failed on sample_dps_entry!: error code = 0x%08x\r\n", status);
        nx_azure_iot_delete(&nx_azure_iot);
        return 1;
    }*/
    
    status = nx_azure_iot_hub_client_initialize(&iothub_client,
        &nx_azure_iot,
        (UCHAR *)iot_hub_hostname, strlen(iot_hub_hostname),
        (UCHAR *)iot_device_id, strlen(iot_device_id),
        (UCHAR *)MODULE_ID,
        sizeof(MODULE_ID) - 1,
        _nx_azure_iot_tls_supported_crypto,
        _nx_azure_iot_tls_supported_crypto_size,
        _nx_azure_iot_tls_ciphersuite_map,
        _nx_azure_iot_tls_ciphersuite_map_size,
        nx_azure_iot_tls_metadata_buffer,
        sizeof(nx_azure_iot_tls_metadata_buffer),
        &root_ca_cert);
    if (status != NX_SUCCESS)
    {
        printf("Failed on nx_azure_iot_hub_client_initialize!: error code = 0x%08x\r\n", status);
        nx_azure_iot_delete(&nx_azure_iot);
        return status;
    }
    
    status = nx_azure_iot_hub_client_symmetric_key_set(&iothub_client,
        (UCHAR *)iot_sas_key,
        strlen(iot_sas_key));
    if (status != NX_SUCCESS)
    {
        printf("Failed on nx_azure_iot_hub_client_symmetric_key_set!\r\n");
        return status;
    }
    
    // Set connection status callback
    status = nx_azure_iot_hub_client_connection_status_callback_set(&iothub_client, connection_status_callback);
    if (status != NX_SUCCESS)
    {
        printf("Failed on connection_status_callback!\r\n");
        return status;
    }

    // Direct method callback
    status = nx_azure_iot_hub_client_receive_callback_set(&iothub_client, NX_AZURE_IOT_HUB_DIRECT_METHOD, azure_iot_hub_direct_method_callback, NULL);
    if (status != NX_SUCCESS)
    {
        printf("Failed on set direct method callback\r\n");
        return status;
    }

    // Connect to IoTHub client
    if (nx_azure_iot_hub_client_connect(&iothub_client, NX_TRUE, NX_WAIT_FOREVER))
    {
        printf("Failed on nx_azure_iot_hub_client_connect!\r\n");
        return status;
    }

    // Direct method setup
    status = nx_azure_iot_hub_client_direct_method_enable(&iothub_client);
    if (status != NX_SUCCESS)
    {
        printf("Failed on enable direct methods\r\n");
        return status;
    }

    NX_PACKET *packet_ptr;
    CHAR buffer[30];
    UINT buffer_length;
    UINT i = 0;

    CHAR* property = "temperature";
    CHAR* value = "28.5";

    while (true)
    {
        // Create a telemetry message packet.
        if ((status = nx_azure_iot_hub_client_telemetry_message_create(&iothub_client, &packet_ptr, NX_WAIT_FOREVER)))
        {
            printf("Telemetry message create failed!: error code = 0x%08x\r\n", status);
            break;
        }

        // Add properties to telemetry message.
        status = nx_azure_iot_hub_client_telemetry_property_add(packet_ptr,
            (UCHAR *)property, strlen(property),
            (UCHAR *)value, strlen(value),
            NX_WAIT_FOREVER);
        if (status != NX_SUCCESS)
        {
            printf("Telemetry property add failed!: error code = 0x%08x\r\n", status);
            nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);
            break;
        }

        buffer_length = (UINT)snprintf(buffer, sizeof(buffer), "{\"Message ID\":%u}", i++);
        status = nx_azure_iot_hub_client_telemetry_send(&iothub_client,
            packet_ptr,
            (UCHAR *)buffer,
            buffer_length,
            NX_WAIT_FOREVER);
        if (status != NX_SUCCESS)
        {
            printf("Telemetry message send failed!: error code = 0x%08x\r\n", status);
            nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);
            break;
        }

        printf("Telemetry message send: %s.\r\n", buffer);
        tx_thread_sleep(10 * NX_IP_PERIODIC_RATE);
    }
    
    return NXD_MQTT_SUCCESS;
}
