/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "nx_client.h"

#include <stdio.h>

#include "nx_api.h"
#include "nx_azure_iot_hub_client.h"
#include "nx_azure_iot_json_reader.h"
#include "nx_azure_iot_provisioning_client.h"

#include "azure_iot_nx_client.h"
#include "networking.h"

#include "azure_config.h"
#include "azure_device_x509_cert_config.h"
#include "azure_pnp_info.h"

#include "platform.h"

#define IOT_MODEL_ID "dtmi:azurertos:devkit:gsg;2"

#define TELEMETRY_TEMPERATURE       "temperature"
#define TELEMETRY_INTERVAL_PROPERTY "telemetryInterval"
#define LED_STATE_PROPERTY          "ledState"
#define SET_LED_STATE_COMMAND       "setLedState"

#define TELEMETRY_INTERVAL_EVENT 1

#define LED_ON  0
#define LED_OFF 1
#define LED0    PORT7.PODR.BIT.B3
#define LED1    PORTG.PODR.BIT.B7
#define LED2    PORTG.PODR.BIT.B6
#define LED3    PORTG.PODR.BIT.B5

static AZURE_IOT_NX_CONTEXT azure_iot_nx_client;

static int32_t telemetry_interval = 10;

static UINT append_device_info_properties(NX_AZURE_IOT_JSON_WRITER* json_writer)
{
    if (nx_azure_iot_json_writer_append_property_with_string_value(json_writer,
            (UCHAR*)DEVICE_INFO_MANUFACTURER_PROPERTY_NAME,
            sizeof(DEVICE_INFO_MANUFACTURER_PROPERTY_NAME) - 1,
            (UCHAR*)DEVICE_INFO_MANUFACTURER_PROPERTY_VALUE,
            sizeof(DEVICE_INFO_MANUFACTURER_PROPERTY_VALUE) - 1) ||
        nx_azure_iot_json_writer_append_property_with_string_value(json_writer,
            (UCHAR*)DEVICE_INFO_MODEL_PROPERTY_NAME,
            sizeof(DEVICE_INFO_MODEL_PROPERTY_NAME) - 1,
            (UCHAR*)DEVICE_INFO_MODEL_PROPERTY_VALUE,
            sizeof(DEVICE_INFO_MODEL_PROPERTY_VALUE) - 1) ||
        nx_azure_iot_json_writer_append_property_with_string_value(json_writer,
            (UCHAR*)DEVICE_INFO_SW_VERSION_PROPERTY_NAME,
            sizeof(DEVICE_INFO_SW_VERSION_PROPERTY_NAME) - 1,
            (UCHAR*)DEVICE_INFO_SW_VERSION_PROPERTY_VALUE,
            sizeof(DEVICE_INFO_SW_VERSION_PROPERTY_VALUE) - 1) ||
        nx_azure_iot_json_writer_append_property_with_string_value(json_writer,
            (UCHAR*)DEVICE_INFO_OS_NAME_PROPERTY_NAME,
            sizeof(DEVICE_INFO_OS_NAME_PROPERTY_NAME) - 1,
            (UCHAR*)DEVICE_INFO_OS_NAME_PROPERTY_VALUE,
            sizeof(DEVICE_INFO_OS_NAME_PROPERTY_VALUE) - 1) ||
        nx_azure_iot_json_writer_append_property_with_string_value(json_writer,
            (UCHAR*)DEVICE_INFO_PROCESSOR_ARCHITECTURE_PROPERTY_NAME,
            sizeof(DEVICE_INFO_PROCESSOR_ARCHITECTURE_PROPERTY_NAME) - 1,
            (UCHAR*)DEVICE_INFO_PROCESSOR_ARCHITECTURE_PROPERTY_VALUE,
            sizeof(DEVICE_INFO_PROCESSOR_ARCHITECTURE_PROPERTY_VALUE) - 1) ||
        nx_azure_iot_json_writer_append_property_with_string_value(json_writer,
            (UCHAR*)DEVICE_INFO_PROCESSOR_MANUFACTURER_PROPERTY_NAME,
            sizeof(DEVICE_INFO_PROCESSOR_MANUFACTURER_PROPERTY_NAME) - 1,
            (UCHAR*)DEVICE_INFO_PROCESSOR_MANUFACTURER_PROPERTY_VALUE,
            sizeof(DEVICE_INFO_PROCESSOR_MANUFACTURER_PROPERTY_VALUE) - 1) ||
        nx_azure_iot_json_writer_append_property_with_double_value(json_writer,
            (UCHAR*)DEVICE_INFO_TOTAL_STORAGE_PROPERTY_NAME,
            sizeof(DEVICE_INFO_TOTAL_STORAGE_PROPERTY_NAME) - 1,
            DEVICE_INFO_TOTAL_STORAGE_PROPERTY_VALUE,
            2) ||
        nx_azure_iot_json_writer_append_property_with_double_value(json_writer,
            (UCHAR*)DEVICE_INFO_TOTAL_MEMORY_PROPERTY_NAME,
            sizeof(DEVICE_INFO_TOTAL_MEMORY_PROPERTY_NAME) - 1,
            DEVICE_INFO_TOTAL_MEMORY_PROPERTY_VALUE,
            2))
    {
        return NX_NOT_SUCCESSFUL;
    }

    return NX_AZURE_IOT_SUCCESS;
}

static UINT append_device_telemetry(NX_AZURE_IOT_JSON_WRITER* json_writer)
{
    const float temperature = 28.5;

    if (nx_azure_iot_json_writer_append_property_with_double_value(
            json_writer, (UCHAR*)TELEMETRY_TEMPERATURE, sizeof(TELEMETRY_TEMPERATURE) - 1, temperature, 2))
    {
        return NX_NOT_SUCCESSFUL;
    }

    return NX_AZURE_IOT_SUCCESS;
}

static void set_led_state(bool level)
{
    if (level)
    {
        printf("LED is turned ON\r\n");
        LED0 = LED_ON;
        LED1 = LED_ON;
        LED2 = LED_ON;
        LED3 = LED_ON;
    }
    else
    {
        printf("LED is turned OFF\r\n");
        LED0 = LED_OFF;
        LED1 = LED_OFF;
        LED2 = LED_OFF;
        LED3 = LED_OFF;
    }
}

static void command_received_cb(AZURE_IOT_NX_CONTEXT* nx_context_ptr,
    const UCHAR* component,
    USHORT component_length,
    const UCHAR* method,
    USHORT method_length,
    UCHAR* payload,
    USHORT payload_length,
    VOID* context_ptr,
    USHORT context_length)
{
    UINT status;

    if (strncmp((CHAR*)method, SET_LED_STATE_COMMAND, method_length) == 0)
    {
        bool arg = (strncmp((CHAR*)payload, "true", payload_length) == 0);
        set_led_state(arg);

        if ((status = nx_azure_iot_hub_client_command_message_response(
                 &nx_context_ptr->iothub_client, 200, context_ptr, context_length, NULL, 0, NX_WAIT_FOREVER)))
        {
            printf("Direct method response failed! (0x%08x)\r\n", status);
            return;
        }

        azure_iot_nx_client_publish_bool_property(&azure_iot_nx_client, NULL, LED_STATE_PROPERTY, arg);
    }
    else
    {
        printf("Direct method is not for this device\r\n");

        if ((status = nx_azure_iot_hub_client_command_message_response(
                 &nx_context_ptr->iothub_client, 501, context_ptr, context_length, NULL, 0, NX_WAIT_FOREVER)))
        {
            printf("Direct method response failed! (0x%08x)\r\n", status);
            return;
        }
    }
}

static void writable_property_received_cb(AZURE_IOT_NX_CONTEXT* nx_context,
    const UCHAR* component_name,
    UINT component_name_len,
    UCHAR* property_name,
    UINT property_name_len,
    NX_AZURE_IOT_JSON_READER* json_reader_ptr,
    UINT version)
{
    UINT status;

    if (strncmp((CHAR*)property_name, TELEMETRY_INTERVAL_PROPERTY, property_name_len) == 0)
    {
        status = nx_azure_iot_json_reader_token_int32_get(json_reader_ptr, &telemetry_interval);
        if (status == NX_AZURE_IOT_SUCCESS)
        {
            printf("Updating %s to %ld\r\n", TELEMETRY_INTERVAL_PROPERTY, telemetry_interval);

            // Confirm reception back to hub
            azure_nx_client_respond_int_writable_property(
                nx_context, NULL, TELEMETRY_INTERVAL_PROPERTY, telemetry_interval, 200, version);

            azure_nx_client_periodic_interval_set(nx_context, telemetry_interval);
        }
    }
}

static void property_received_cb(AZURE_IOT_NX_CONTEXT* nx_context,
    const UCHAR* component_name,
    UINT component_name_len,
    UCHAR* property_name,
    UINT property_name_len,
    NX_AZURE_IOT_JSON_READER* json_reader_ptr,
    UINT version)
{
    UINT status;

    if (strncmp((CHAR*)property_name, TELEMETRY_INTERVAL_PROPERTY, property_name_len) == 0)
    {
        status = nx_azure_iot_json_reader_token_int32_get(json_reader_ptr, &telemetry_interval);
        if (status == NX_AZURE_IOT_SUCCESS)
        {
            printf("Updating %s to %ld\r\n", TELEMETRY_INTERVAL_PROPERTY, telemetry_interval);
            azure_nx_client_periodic_interval_set(nx_context, telemetry_interval);
        }
    }
}

static void properties_complete_cb(AZURE_IOT_NX_CONTEXT* nx_context)
{
    // Device twin processing is done, send out property updates
    azure_iot_nx_client_publish_properties(nx_context, DEVICE_INFO_COMPONENT_NAME, append_device_info_properties);
    azure_iot_nx_client_publish_bool_property(nx_context, NULL, LED_STATE_PROPERTY, false);
    azure_iot_nx_client_publish_int_writable_property(
        nx_context, NULL, TELEMETRY_INTERVAL_PROPERTY, telemetry_interval);

    printf("\r\nStarting Main loop\r\n");
}

static void telemetry_cb(AZURE_IOT_NX_CONTEXT* nx_context)
{
    // Send out telemetry
    azure_iot_nx_client_publish_telemetry(nx_context, NULL, append_device_telemetry);
}

UINT azure_iot_nx_client_entry(
    NX_IP* ip_ptr, NX_PACKET_POOL* pool_ptr, NX_DNS* dns_ptr, UINT (*unix_time_callback)(ULONG* unix_time))
{
    UINT status;

    if ((status = azure_iot_nx_client_create(&azure_iot_nx_client,
             ip_ptr,
             pool_ptr,
             dns_ptr,
             unix_time_callback,
             IOT_MODEL_ID,
             sizeof(IOT_MODEL_ID) - 1)))
    {
        printf("ERROR: azure_iot_nx_client_create failed (0x%08x)\r\n", status);
        return status;
    }

    // Register the callbacks
    azure_iot_nx_client_register_command_callback(&azure_iot_nx_client, command_received_cb);
    azure_iot_nx_client_register_writable_property_callback(&azure_iot_nx_client, writable_property_received_cb);
    azure_iot_nx_client_register_property_callback(&azure_iot_nx_client, property_received_cb);
    azure_iot_nx_client_register_properties_complete_callback(&azure_iot_nx_client, properties_complete_cb);
    azure_iot_nx_client_register_timer_callback(&azure_iot_nx_client, telemetry_cb, telemetry_interval);

    // Setup authentication
#ifdef ENABLE_X509
    if ((status = azure_iot_nx_client_cert_set(&azure_iot_nx_client,
             (UCHAR*)iot_x509_device_cert,
             iot_x509_device_cert_len,
             (UCHAR*)iot_x509_private_key,
             iot_x509_private_key_len)))
    {
        printf("ERROR: azure_iot_nx_client_cert_set (0x%08x)\r\n", status);
        return status;
    }
#else
    if ((status = azure_iot_nx_client_sas_set(&azure_iot_nx_client, IOT_DEVICE_SAS_KEY)))
    {
        printf("ERROR: azure_iot_nx_client_sas_set (0x%08x)\r\n", status);
        return status;
    }
#endif

    // Enter the main loop
#ifdef ENABLE_DPS
    azure_iot_nx_client_dps_run(&azure_iot_nx_client, IOT_DPS_ID_SCOPE, IOT_DPS_REGISTRATION_ID, network_connect);
#else
    azure_iot_nx_client_hub_run(&azure_iot_nx_client, IOT_HUB_HOSTNAME, IOT_HUB_DEVICE_ID, network_connect);
#endif

    return NX_SUCCESS;
}
