/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include <stdio.h>

#include "Bosch_BME280.h"
#include "atmel_start.h"

#include "nx_api.h"
#include "nx_azure_iot_hub_client.h"
#include "nx_azure_iot_provisioning_client.h"

#include "jsmn.h"

/* These are sample files, user can build their own certificate and
 * ciphersuites.  */
#include "azure_iot/azure_iot_nx_client.h"
#include "azure_iot_cert.h"
#include "azure_iot_ciphersuites.h"

#include "azure_config.h"

//#define IOT_MODEL_ID "dtmi:microsoft:gsg;1"
#define IOT_MODEL_ID ""

static AZURE_IOT_NX_CLIENT azure_iot_nx_client;
static INT telemetry_interval = 10;

static void set_led_state(bool level)
{
    if (level)
    {
        // Pin level set to "high" state
        printf("LED0 is turned OFF\r\n");
    }
    else
    {
        // Pin level set to "low" state
        printf("LED0 is turned ON\r\n");
    }

    gpio_set_pin_level(PC18, level);
}

static bool findJsonInt(const char* json, jsmntok_t* tokens, int tokens_count, const char* s, int* value)
{
    for (int i = 1; i < tokens_count; i++)
    {
        if ((tokens[i].type == JSMN_STRING) && (strlen(s) == tokens[i].end - tokens[i].start) &&
            (strncmp(json + tokens[i].start, s, tokens[i].end - tokens[i].start) == 0))
        {
            *value = atoi(json + tokens[i + 1].start);

            printf("Desired property %s = %d\r\n", s, *value);
            return true;
        }
    }

    return false;
}

static void telemetry_thread_entry(ULONG parameter)
{
    NX_PACKET* packet_ptr;
    UINT status;
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

#if __SENSOR_BME280__ == 1
        WeatherClick_waitforRead();
        temperature = Weather_getTemperatureDegC();
#endif

        report_telemetry_float(&azure_iot_nx_client, "temperature", temperature, packet_ptr);
        report_device_twin_property_float(&azure_iot_nx_client, "currentTemperature", temperature);

        nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);

        tx_thread_sleep(telemetry_interval * NX_IP_PERIODIC_RATE);
    }
}

static void device_twin_thread_entry(ULONG parameter)
{
    NX_PACKET* packet_ptr;
    UINT status;

    jsmn_parser parser;
    jsmntok_t tokens[16];
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
        const ULONG json_len = packet_ptr->nx_packet_length;

        jsmn_init(&parser);
        token_count = jsmn_parse(&parser, json_str, json_len, tokens, 16);

        findJsonInt(json_str, tokens, token_count, "telemetryInterval", &telemetry_interval);
        tx_thread_wait_abort(&azure_iot_nx_client.telemetry_thread);

        nx_packet_release(packet_ptr);

        // Wait for a desired property update
        if ((status = nx_azure_iot_hub_client_device_twin_desired_properties_receive(
                 &azure_iot_nx_client.iothub_client, &packet_ptr, NX_WAIT_FOREVER)))
        {
            printf("Receive desired property receive failed!: error code = "
                   "0x%08x\r\n",
                status);
            break;
        }
    }
}

static void direct_method_thread_entry(ULONG parameter)
{
    NX_PACKET* packet_ptr;
    UINT status;
    UCHAR* method_name_ptr;
    USHORT method_name_length;
    VOID* context_ptr;
    USHORT context_length;

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

        if (strncmp((CHAR*)method_name_ptr, "set_led_state", method_name_length) == 0)
        {
            // set_led_state command
            printf("received set_led_state\r\n");

            bool arg = strncmp((CHAR*)packet_ptr->nx_packet_prepend_ptr, "true", packet_ptr->nx_packet_length);
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

        nx_packet_release(packet_ptr);
    }
}

static void c2d_thread_entry(ULONG parameter)
{
    NX_PACKET* packet_ptr;
    UINT status = 0;
    USHORT property_buf_size;
    UCHAR* property_buf;
    CHAR* temperature_property = "temperature";

    NX_PARAMETER_NOT_USED(parameter);

    while (true)
    {
        if ((status = nx_azure_iot_hub_client_cloud_message_receive(
                 &azure_iot_nx_client.iothub_client, &packet_ptr, NX_WAIT_FOREVER)))
        {
            printf("C2D receive failed!: error code = 0x%08x\r\n", status);
            break;
        }

        if ((status = nx_azure_iot_hub_client_cloud_message_property_get(&azure_iot_nx_client.iothub_client,
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
        nx_packet_release(packet_ptr);
    }
}

UINT azure_iot_embedded_sdk_entry(
    NX_IP* ip_ptr, NX_PACKET_POOL* pool_ptr, NX_DNS* dns_ptr, UINT (*unix_time_callback)(ULONG* unix_time))
{
    UINT status;
    if ((status = azure_iot_nx_client_create(&azure_iot_nx_client,
             ip_ptr,
             pool_ptr,
             dns_ptr,
             unix_time_callback,
             IOT_HUB_HOSTNAME,
             IOT_DEVICE_ID,
             IOT_PRIMARY_KEY,
             IOT_MODEL_ID)))
    {
        printf("ERROR: failed to create iot client 0x%08x\r\n", status);
        return status;
    }

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
