/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "nx_client.h"

#include <stdio.h>

#include "platform.h"
#include "rx65n_cloud_kit_sensors.h"

#include "nx_api.h"
#include "nx_azure_iot_hub_client.h"
#include "nx_azure_iot_json_reader.h"
#include "nx_azure_iot_provisioning_client.h"

#include "azure_iot_nx_client.h"

#include "azure_config.h"
#include "azure_device_x509_cert_config.h"
#include "azure_pnp_info.h"
#include "rx_networking.h"

#define IOT_MODEL_ID "dtmi:azurertos:devkit:gsgrx65ncloud;1"

// Device telemetry names
#define TELEMETRY_HUMIDITY          "humidity"
#define TELEMETRY_TEMPERATURE       "temperature"
#define TELEMETRY_PRESSURE          "pressure"
#define TELEMETRY_GAS_RESISTANCE    "gasResistance"
#define TELEMETRY_ACCELEROMETERX    "accelerometerX"
#define TELEMETRY_ACCELEROMETERY    "accelerometerY"
#define TELEMETRY_ACCELEROMETERZ    "accelerometerZ"
#define TELEMETRY_GYROSCOPEX        "gyroscopeX"
#define TELEMETRY_GYROSCOPEY        "gyroscopeY"
#define TELEMETRY_GYROSCOPEZ        "gyroscopeZ"
#define TELEMETRY_LIGHT             "illuminance"
#define TELEMETRY_INTERVAL_PROPERTY "telemetryInterval"
#define LED_STATE_PROPERTY          "ledState"
#define SET_LED_STATE_COMMAND       "setLedState"

typedef enum TELEMETRY_STATE_ENUM
{
    TELEMETRY_STATE_DEFAULT,
    TELEMETRY_STATE_ACCELEROMETER,
    TELEMETRY_STATE_GYROSCOPE,
    TELEMETRY_STATE_LIGHT,
    TELEMETRY_STATE_END
} TELEMETRY_STATE;

#define LED_ON  0
#define LED_OFF 1
#define LED0    PORTB.PODR.BIT.B0
#define LED1    PORTB.PODR.BIT.B2

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
    struct bme68x_data data;
    read_bme680(&data);

    if (nx_azure_iot_json_writer_append_property_with_double_value(
            json_writer, (UCHAR*)TELEMETRY_HUMIDITY, sizeof(TELEMETRY_HUMIDITY) - 1, data.humidity, 2) ||

        nx_azure_iot_json_writer_append_property_with_double_value(
            json_writer, (UCHAR*)TELEMETRY_TEMPERATURE, sizeof(TELEMETRY_TEMPERATURE) - 1, data.temperature, 2) ||

        nx_azure_iot_json_writer_append_property_with_double_value(
            json_writer, (UCHAR*)TELEMETRY_PRESSURE, sizeof(TELEMETRY_PRESSURE) - 1, data.pressure, 2) ||

        nx_azure_iot_json_writer_append_property_with_double_value(json_writer,
            (UCHAR*)TELEMETRY_GAS_RESISTANCE,
            sizeof(TELEMETRY_GAS_RESISTANCE) - 1,
            data.gas_resistance,
            2))
    {
        return NX_NOT_SUCCESSFUL;
    }

    return NX_AZURE_IOT_SUCCESS;
}

static UINT append_device_accelerometer(NX_AZURE_IOT_JSON_WRITER* json_writer)
{
    struct bmi160_sensor_data data;
    read_bmi160_accel(&data);

    if (nx_azure_iot_json_writer_append_property_with_int32_value(
            json_writer, (UCHAR*)TELEMETRY_ACCELEROMETERX, sizeof(TELEMETRY_ACCELEROMETERX) - 1, data.x) ||

        nx_azure_iot_json_writer_append_property_with_int32_value(
            json_writer, (UCHAR*)TELEMETRY_ACCELEROMETERY, sizeof(TELEMETRY_ACCELEROMETERY) - 1, data.y) ||

        nx_azure_iot_json_writer_append_property_with_int32_value(
            json_writer, (UCHAR*)TELEMETRY_ACCELEROMETERZ, sizeof(TELEMETRY_ACCELEROMETERZ) - 1, data.z))
    {
        return NX_NOT_SUCCESSFUL;
    }

    return NX_AZURE_IOT_SUCCESS;
}

static UINT append_device_gyroscope(NX_AZURE_IOT_JSON_WRITER* json_writer)
{
    struct bmi160_sensor_data data;
    read_bmi160_gyro(&data);

    if (nx_azure_iot_json_writer_append_property_with_int32_value(
            json_writer, (UCHAR*)TELEMETRY_GYROSCOPEX, sizeof(TELEMETRY_GYROSCOPEX) - 1, data.x) ||

        nx_azure_iot_json_writer_append_property_with_int32_value(
            json_writer, (UCHAR*)TELEMETRY_GYROSCOPEY, sizeof(TELEMETRY_GYROSCOPEY) - 1, data.y) ||

        nx_azure_iot_json_writer_append_property_with_int32_value(
            json_writer, (UCHAR*)TELEMETRY_GYROSCOPEZ, sizeof(TELEMETRY_GYROSCOPEZ) - 1, data.z))
    {
        return NX_NOT_SUCCESSFUL;
    }

    return NX_AZURE_IOT_SUCCESS;
}

static UINT append_device_light(NX_AZURE_IOT_JSON_WRITER* json_writer)
{
    double als;

    read_isl29035(&als);

    if (nx_azure_iot_json_writer_append_property_with_double_value(
            json_writer, (UCHAR*)TELEMETRY_LIGHT, sizeof(TELEMETRY_LIGHT) - 1, als, 2))
    {
        return NX_NOT_SUCCESSFUL;
    }

    return NX_AZURE_IOT_SUCCESS;
}

static void set_led_state(bool level)
{
    if (level)
    {
        printf("\tLED is turned ON\r\n");
        LED0 = LED_ON;
        LED1 = LED_ON;
    }
    else
    {
        printf("\tLED is turned OFF\r\n");
        LED0 = LED_OFF;
        LED1 = LED_OFF;
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
    static TELEMETRY_STATE telemetry_state = TELEMETRY_STATE_DEFAULT;

    switch (telemetry_state)
    {
        case TELEMETRY_STATE_DEFAULT:
            azure_iot_nx_client_publish_telemetry(&azure_iot_nx_client, NULL, append_device_telemetry);
            break;

        case TELEMETRY_STATE_ACCELEROMETER:
            azure_iot_nx_client_publish_telemetry(&azure_iot_nx_client, NULL, append_device_accelerometer);
            break;

        case TELEMETRY_STATE_GYROSCOPE:
            azure_iot_nx_client_publish_telemetry(&azure_iot_nx_client, NULL, append_device_gyroscope);
            break;

        case TELEMETRY_STATE_LIGHT:
            azure_iot_nx_client_publish_telemetry(&azure_iot_nx_client, NULL, append_device_light);
            break;

        default:
            break;
    }

    telemetry_state = (telemetry_state + 1) % TELEMETRY_STATE_END;
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
    azure_iot_nx_client_dps_run(&azure_iot_nx_client, IOT_DPS_ID_SCOPE, IOT_DPS_REGISTRATION_ID, rx_network_connect);
#else
    azure_iot_nx_client_hub_run(&azure_iot_nx_client, IOT_HUB_HOSTNAME, IOT_HUB_DEVICE_ID, rx_network_connect);
#endif

    return NX_SUCCESS;
}
