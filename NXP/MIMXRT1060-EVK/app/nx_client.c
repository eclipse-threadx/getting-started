/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "nx_client.h"

#include <stdio.h>

#include "nx_api.h"
#include "nx_azure_iot_hub_client.h"
#include "nx_azure_iot_provisioning_client.h"

#include "jsmn.h"

// These are sample files, user can build their own certificate and ciphersuites
#include "azure_iot_cert.h"
#include "azure_iot_ciphersuites.h"
#include "azure_iot_nx_client.h"

#include "azure_config.h"

#define TELEMETRY_INTERVAL_EVENT 1

static AZURE_IOT_NX_CLIENT azure_iot_nx_client;
static TX_EVENT_FLAGS_GROUP azure_iot_flags;

static INT telemetry_interval = 10;

static void set_led_state(bool level)
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

static void telemetry_thread_entry(ULONG parameter)
{
    NX_PACKET* packet_ptr;
    UINT status;
    ULONG events;
    float temperature = 28.5;

    NX_PARAMETER_NOT_USED(parameter);

    while (true)
    {
        /* Create a telemetry message packet. */
        if ((status = nx_azure_iot_hub_client_telemetry_message_create(
                 &azure_iot_nx_client.iothub_client, &packet_ptr, NX_WAIT_FOREVER)))
        {
            printf("Telemetry message create failed!: error code = 0x%08x\r\n", status);
            break;
        }

        azure_iot_nx_client_publish_float_telemetry(&azure_iot_nx_client, "temperature", temperature, packet_ptr);
        azure_iot_nx_client_publish_float_property(&azure_iot_nx_client, "currentTemperature", temperature);

        tx_event_flags_get(
            &azure_iot_flags, TELEMETRY_INTERVAL_EVENT, TX_OR_CLEAR, &events, telemetry_interval * NX_IP_PERIODIC_RATE);
    }
}

static void device_twin_thread_entry(ULONG parameter)
{
    NX_PACKET* packet_ptr;
    UINT status;

    jsmn_parser parser;
    jsmntok_t tokens[64];
    INT token_count;

    NX_PARAMETER_NOT_USED(parameter);

    // Request and parse the device twin properties
    if ((status = nx_azure_iot_hub_client_device_twin_properties_request(
             &azure_iot_nx_client.iothub_client, NX_WAIT_FOREVER)))
    {
        printf("device twin document request failed!: error code = 0x%08x\r\n", status);
        return;
    }

    if ((status = nx_azure_iot_hub_client_device_twin_properties_receive(
             &azure_iot_nx_client.iothub_client, &packet_ptr, NX_WAIT_FOREVER)))
    {
        printf("device twin document receive failed!: error code = 0x%08x\r\n", status);
        return;
    }

    while (true)
    {
        printf("Receive device twin properties: ");
        printf_packet(packet_ptr);

        const CHAR* json_str = (CHAR*)packet_ptr->nx_packet_prepend_ptr;
        const ULONG json_len = packet_ptr->nx_packet_append_ptr - packet_ptr->nx_packet_prepend_ptr;

        jsmn_init(&parser);
        token_count = jsmn_parse(&parser, json_str, json_len, tokens, 64);

        findJsonInt(json_str, tokens, token_count, "telemetryInterval", &telemetry_interval);

        // Set a telemetry event so we pick up the change immediately
        tx_event_flags_set(&azure_iot_flags, TELEMETRY_INTERVAL_EVENT, TX_OR);

        // Release the received packet, as ownership was passed to the application
        nx_packet_release(packet_ptr);

        // Wait for a desired property update
        if ((status = nx_azure_iot_hub_client_device_twin_desired_properties_receive(
                 &azure_iot_nx_client.iothub_client, &packet_ptr, NX_WAIT_FOREVER)))
        {
            printf("Receive desired property receive failed!: error code = 0x%08x\r\n", status);
            break;
        }
    }
}

static void direct_method_thread_entry(ULONG parameter)
{
    UINT status;
    NX_PACKET* packet_ptr;
    UCHAR* method_name_ptr;
    USHORT method_name_length;
    VOID* context_ptr;
    USHORT context_length;
    CHAR* payload_ptr;
    USHORT payload_length;

    UINT http_status;
    CHAR* http_response = "{}";

    NX_PARAMETER_NOT_USED(parameter);

    while (true)
    {
        http_status = 501;

        if ((status = nx_azure_iot_hub_client_direct_method_message_receive(&azure_iot_nx_client.iothub_client,
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

        printf("Receive method call: %.*s, with payload: ", (INT)method_name_length, (CHAR*)method_name_ptr);
        printf_packet(packet_ptr);

        payload_ptr    = (CHAR*)packet_ptr->nx_packet_prepend_ptr;
        payload_length = packet_ptr->nx_packet_append_ptr - packet_ptr->nx_packet_prepend_ptr;

        if (strncmp((CHAR*)method_name_ptr, "set_led_state", method_name_length) == 0)
        {
            // set_led_state command
            printf("received set_led_state\r\n");

            bool arg = (strncmp(payload_ptr, "true", payload_length) == 0);
            set_led_state(arg);
            http_status = 200;
        }

        if ((status = nx_azure_iot_hub_client_direct_method_message_response(&azure_iot_nx_client.iothub_client,
                 http_status,
                 context_ptr,
                 context_length,
                 (UCHAR*)http_response,
                 strlen(http_response),
                 NX_WAIT_FOREVER)))
        {
            printf("Direct method response failed!: error code = 0x%08x\r\n", status);
            break;
        }

        // Release the received packet, as ownership was passed to the application
        nx_packet_release(packet_ptr);
    }
}

static void c2d_thread_entry(ULONG parameter)
{
    NX_PACKET* packet_ptr;
    UINT status;

    NX_PARAMETER_NOT_USED(parameter);

    while (true)
    {
        if ((status = nx_azure_iot_hub_client_cloud_message_receive(
                 &azure_iot_nx_client.iothub_client, &packet_ptr, NX_WAIT_FOREVER)))
        {
            printf("C2D receive failed!: error code = 0x%08x\r\n", status);
            break;
        }

        printf("Receive message: ");
        printf_packet(packet_ptr);

        // Release the received packet, as ownership was passed to the application
        nx_packet_release(packet_ptr);
    }
}

UINT azure_iot_nx_client_entry(
    NX_IP* ip_ptr, NX_PACKET_POOL* pool_ptr, NX_DNS* dns_ptr, UINT (*unix_time_callback)(ULONG* unix_time))
{
    UINT status;

    if ((status = tx_event_flags_create(&azure_iot_flags, "Azure IoT flags")))
    {
        printf("FAIL: Unable to create nx_client event flags (0x%02x)\r\n", status);
        return status;
    }

    if ((status = azure_iot_nx_client_create(&azure_iot_nx_client,
             ip_ptr,
             pool_ptr,
             dns_ptr,
             unix_time_callback,
             IOT_HUB_HOSTNAME,
             IOT_DEVICE_ID,
             IOT_PRIMARY_KEY)))
    {
        printf("ERROR: failed to create iot client 0x%08x\r\n", status);
        return status;
    }

    // Register the callback entry points
    azure_iot_nx_client_enable_telemetry(&azure_iot_nx_client, telemetry_thread_entry);
    azure_iot_nx_client_enable_device_twin(&azure_iot_nx_client, device_twin_thread_entry);
    azure_iot_nx_client_enable_direct_method(&azure_iot_nx_client, direct_method_thread_entry);
    azure_iot_nx_client_enable_c2d(&azure_iot_nx_client, c2d_thread_entry);

    if ((status = azure_iot_nx_client_connect(&azure_iot_nx_client)))
    {
        printf("ERROR: failed to connect iot client 0x%08x\r\n", status);
        return status;
    }

    // Loop forever
    while (true)
    {
        tx_thread_sleep(NX_IP_PERIODIC_RATE);
    }

    return NX_SUCCESS;
}
