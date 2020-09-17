/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "nx_client.h"

#include <stdio.h>

#include "stm32l475e_iot01.h"
#include "stm32l475e_iot01_tsensor.h"

#include "nx_api.h"
#include "nx_azure_iot_hub_client.h"
#include "nx_azure_iot_provisioning_client.h"

// These are sample files, user can build their own certificate and ciphersuites
#include "azure_iot_cert.h"
#include "azure_iot_ciphersuites.h"
#include "azure_iot_nx_client.h"

#include "azure_config.h"

#define IOT_MODEL_ID                "dtmi:com:examples:gsg;1"
#define TELEMETRY_INTERVAL_PROPERTY "telemetryInterval"
#define LED_STATE_PROPERTY          "ledState"
#define SET_LED_STATE_COMMAND       "setLedState"

#define TELEMETRY_INTERVAL_EVENT 1

static AZURE_IOT_NX_CONTEXT azure_iot_nx_client;
static TX_EVENT_FLAGS_GROUP azure_iot_flags;

static int32_t telemetry_interval = 10;

static void set_led_state(bool level)
{
    if (level)
    {
        printf("LED is turned ON\r\n");
        BSP_LED_On(LED_GREEN);
    }
    else
    {
        printf("LED is turned OFF\r\n");
        BSP_LED_Off(LED_GREEN);
    }
}

static void direct_method_cb(AZURE_IOT_NX_CONTEXT* nx_context,
    UCHAR* method,
    USHORT method_length,
    UCHAR* payload,
    USHORT payload_length,
    VOID* context,
    USHORT context_length)
{
    UINT status;
    UINT http_status    = 501;
    CHAR* http_response = "{}";

    if (strncmp((CHAR*)method, SET_LED_STATE_COMMAND, method_length) == 0)
    {
        bool arg = (strncmp((CHAR*)payload, "true", payload_length) == 0);
        set_led_state(arg);

        azure_iot_nx_client_publish_bool_property(&azure_iot_nx_client, LED_STATE_PROPERTY, arg);

        http_status = 200;
    }

    if ((status = nx_azure_iot_hub_client_direct_method_message_response(&nx_context->iothub_client,
             http_status,
             context,
             context_length,
             (UCHAR*)http_response,
             strlen(http_response),
             NX_WAIT_FOREVER)))
    {
        printf("Direct method response failed! (0x%08x)\r\n", status);
        return;
    }
}

static void device_twin_desired_property_cb(UCHAR* component_name,
    UINT component_name_len,
    UCHAR* property_name,
    UINT property_name_len,
    az_json_reader property_value_reader,
    UINT version,
    VOID* userContextCallback)
{
    AZURE_IOT_NX_CONTEXT* nx_context = (AZURE_IOT_NX_CONTEXT*)userContextCallback;

    if (strncmp((CHAR*)property_name, TELEMETRY_INTERVAL_PROPERTY, property_name_len) == 0)
    {
        if (az_succeeded(az_json_token_get_int32(&property_value_reader.token, &telemetry_interval)))
        {
            // Set a telemetry event so we pick up the change immediately
            tx_event_flags_set(&azure_iot_flags, TELEMETRY_INTERVAL_EVENT, TX_OR);

            // Confirm reception back to hub
            azure_nx_client_respond_int_writeable_property(
                nx_context, TELEMETRY_INTERVAL_PROPERTY, telemetry_interval, 200, version);
        }
    }
}

static void device_twin_property_cb(UCHAR* component_name,
    UINT component_name_len,
    UCHAR* property_name,
    UINT property_name_len,
    az_json_reader property_value_reader,
    UINT version,
    VOID* userContextCallback)
{
    AZURE_IOT_NX_CONTEXT* nx_context = (AZURE_IOT_NX_CONTEXT*)userContextCallback;

    if (strncmp((CHAR*)property_name, TELEMETRY_INTERVAL_PROPERTY, property_name_len) == 0)
    {
        if (az_succeeded(az_json_token_get_int32(&property_value_reader.token, &telemetry_interval)))
        {
            // Set a telemetry event so we pick up the change immediately
            tx_event_flags_set(&azure_iot_flags, TELEMETRY_INTERVAL_EVENT, TX_OR);
        }
    }

    // Confirm reception back to hub
    azure_nx_client_respond_int_writeable_property(
        nx_context, TELEMETRY_INTERVAL_PROPERTY, telemetry_interval, 200, version);
}

UINT azure_iot_nx_client_entry(
    NX_IP* ip_ptr, NX_PACKET_POOL* pool_ptr, NX_DNS* dns_ptr, UINT (*unix_time_callback)(ULONG* unix_time))
{
    UINT status;
    ULONG events;
    float temperature;

    if ((status = tx_event_flags_create(&azure_iot_flags, "Azure IoT flags")))
    {
        printf("FAIL: Unable to create nx_client event flags (0x%04x)\r\n", status);
        return status;
    }

#ifdef ENABLE_DPS
    status = azure_iot_nx_client_dps_create(&azure_iot_nx_client,
        ip_ptr,
        pool_ptr,
        dns_ptr,
        unix_time_callback,
        IOT_DPS_ENDPOINT,
        IOT_DPS_ID_SCOPE,
        IOT_DPS_REGISTRATION_ID,
        IOT_PRIMARY_KEY,
        IOT_MODEL_ID);
#else
    status = azure_iot_nx_client_create(&azure_iot_nx_client,
        ip_ptr,
        pool_ptr,
        dns_ptr,
        unix_time_callback,
        IOT_HUB_HOSTNAME,
        IOT_DEVICE_ID,
        IOT_PRIMARY_KEY,
        IOT_MODEL_ID);
#endif
    if (status != NX_SUCCESS)
    {
        printf("ERROR: failed to create iot client 0x%04x\r\n", status);
        return status;
    }

    // Register the callbacks
    azure_iot_nx_client_register_direct_method(&azure_iot_nx_client, direct_method_cb);
    azure_iot_nx_client_register_device_twin_desired_prop(&azure_iot_nx_client, device_twin_desired_property_cb);
    azure_iot_nx_client_register_device_twin_prop(&azure_iot_nx_client, device_twin_property_cb);

    if ((status = azure_iot_nx_client_connect(&azure_iot_nx_client)))
    {
        printf("ERROR: failed to connect nx client (0x%08x)\r\n", status);
        return status;
    }

    // Request the device twin for writeable property update
    if ((status = nx_azure_iot_hub_client_device_twin_properties_request(
             &azure_iot_nx_client.iothub_client, NX_WAIT_FOREVER)))
    {
        printf("ERROR: failed to request device twin (0x%08x)\r\n", status);
        return status;
    }

    // Send reported properties
    azure_iot_nx_client_publish_bool_property(&azure_iot_nx_client, LED_STATE_PROPERTY, false);

    printf("\r\nStarting Main loop\r\n");

    while (true)
    {
        tx_event_flags_get(
            &azure_iot_flags, TELEMETRY_INTERVAL_EVENT, TX_OR_CLEAR, &events, telemetry_interval * NX_IP_PERIODIC_RATE);

        temperature = BSP_TSENSOR_ReadTemp();

        azure_iot_nx_client_publish_float_telemetry(&azure_iot_nx_client, "temperature", temperature);
    }

    return NX_SUCCESS;
}
