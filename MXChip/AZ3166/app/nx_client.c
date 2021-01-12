/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "nx_client.h"

#include <stdio.h>

#include "screen.h"
#include "sensor.h"
#include "stm32f4xx_hal.h"

#include "nx_api.h"
#include "nx_azure_iot_hub_client.h"
#include "nx_azure_iot_json_reader.h"
#include "nx_azure_iot_provisioning_client.h"

#include "azure_iot_nx_client.h"
#include "nx_azure_iot_pnp_helpers.h"

#include "azure_config.h"
#include "azure_device_x509_cert_config.h"
#include "azure_pnp_info.h"

#define IOT_MODEL_ID "dtmi:azurertos:devkit:gsgmxchip;1"

// Device telemetry names
#define TELEMETRY_HUMIDITY          "humidity"
#define TELEMETRY_TEMPERATURE       "temperature"
#define TELEMETRY_PRESSURE          "pressure"
#define TELEMETRY_MAGNETOMETERX     "magnetometerX"
#define TELEMETRY_MAGNETOMETERY     "magnetometerY"
#define TELEMETRY_MAGNETOMETERZ     "magnetometerZ"
#define TELEMETRY_ACCELEROMETERX    "accelerometerX"
#define TELEMETRY_ACCELEROMETERY    "accelerometerY"
#define TELEMETRY_ACCELEROMETERZ    "accelerometerZ"
#define TELEMETRY_GYROSCOPEX        "gyroscopeX"
#define TELEMETRY_GYROSCOPEY        "gyroscopeY"
#define TELEMETRY_GYROSCOPEZ        "gyroscopeZ"
#define TELEMETRY_INTERVAL_PROPERTY "telemetryInterval"
#define LED_STATE_PROPERTY          "ledState"
#define SET_LED_STATE_COMMAND       "setLedState"
#define SET_DISPLAY_TEXT_COMMAND    "setDisplayText"

#define TELEMETRY_INTERVAL_EVENT 1

typedef enum TELEMETRY_STATE_ENUM
{
    TELEMETRY_STATE_DEFAULT,
    TELEMETRY_STATE_MAGNETOMETER,
    TELEMETRY_STATE_ACCELEROMETER,
    TELEMETRY_STATE_GYROSCOPE
} TELEMETRY_STATE;

static AZURE_IOT_NX_CONTEXT azure_iot_nx_client;
static TX_EVENT_FLAGS_GROUP azure_iot_flags;

static int32_t telemetry_interval = 10;

static UINT append_device_info_properties(NX_AZURE_IOT_JSON_WRITER* json_writer, VOID* context)
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

static UINT append_device_telemetry(NX_AZURE_IOT_JSON_WRITER* json_writer, VOID* context)
{
    lps22hb_t lps22hb_data    = lps22hb_data_read();
    hts221_data_t hts221_data = hts221_data_read();

    if (nx_azure_iot_json_writer_append_property_with_double_value(
            json_writer, (UCHAR*)TELEMETRY_HUMIDITY, sizeof(TELEMETRY_HUMIDITY) - 1, hts221_data.humidity_perc, 2) ||
        nx_azure_iot_json_writer_append_property_with_double_value(json_writer,
            (UCHAR*)TELEMETRY_TEMPERATURE,
            sizeof(TELEMETRY_TEMPERATURE) - 1,
            lps22hb_data.temperature_degC,
            2) ||
        nx_azure_iot_json_writer_append_property_with_double_value(
            json_writer, (UCHAR*)TELEMETRY_PRESSURE, sizeof(TELEMETRY_PRESSURE) - 1, lps22hb_data.pressure_hPa, 2))
    {
        return NX_NOT_SUCCESSFUL;
    }

    return NX_AZURE_IOT_SUCCESS;
}

static UINT append_device_telemetry_magnetometer(NX_AZURE_IOT_JSON_WRITER* json_writer, VOID* context)
{
    lis2mdl_data_t lis2mdl_data = lis2mdl_data_read();

    if (nx_azure_iot_json_writer_append_property_with_double_value(json_writer,
            (UCHAR*)TELEMETRY_MAGNETOMETERX,
            sizeof(TELEMETRY_MAGNETOMETERX) - 1,
            lis2mdl_data.magnetic_mG[0],
            2) ||
        nx_azure_iot_json_writer_append_property_with_double_value(json_writer,
            (UCHAR*)TELEMETRY_MAGNETOMETERY,
            sizeof(TELEMETRY_MAGNETOMETERY) - 1,
            lis2mdl_data.magnetic_mG[1],
            2) ||
        nx_azure_iot_json_writer_append_property_with_double_value(json_writer,
            (UCHAR*)TELEMETRY_MAGNETOMETERZ,
            sizeof(TELEMETRY_MAGNETOMETERZ) - 1,
            lis2mdl_data.magnetic_mG[2],
            2))
    {
        return NX_NOT_SUCCESSFUL;
    }

    return NX_AZURE_IOT_SUCCESS;
}

static UINT append_device_telemetry_accelerometer(NX_AZURE_IOT_JSON_WRITER* json_writer, VOID* context)
{
    lsm6dsl_data_t lsm6dsl_data = lsm6dsl_data_read();

    if (nx_azure_iot_json_writer_append_property_with_double_value(json_writer,
            (UCHAR*)TELEMETRY_ACCELEROMETERX,
            sizeof(TELEMETRY_ACCELEROMETERX) - 1,
            lsm6dsl_data.acceleration_mg[0],
            2) ||
        nx_azure_iot_json_writer_append_property_with_double_value(json_writer,
            (UCHAR*)TELEMETRY_ACCELEROMETERY,
            sizeof(TELEMETRY_ACCELEROMETERY) - 1,
            lsm6dsl_data.acceleration_mg[1],
            2) ||
        nx_azure_iot_json_writer_append_property_with_double_value(json_writer,
            (UCHAR*)TELEMETRY_ACCELEROMETERZ,
            sizeof(TELEMETRY_ACCELEROMETERZ) - 1,
            lsm6dsl_data.acceleration_mg[2],
            2))
    {
        return NX_NOT_SUCCESSFUL;
    }

    return NX_AZURE_IOT_SUCCESS;
}

static UINT append_device_telemetry_gyroscope(NX_AZURE_IOT_JSON_WRITER* json_writer, VOID* context)
{
    lsm6dsl_data_t lsm6dsl_data = lsm6dsl_data_read();

    if (nx_azure_iot_json_writer_append_property_with_double_value(json_writer,
            (UCHAR*)TELEMETRY_GYROSCOPEX,
            sizeof(TELEMETRY_GYROSCOPEX) - 1,
            lsm6dsl_data.angular_rate_mdps[0],
            2) ||
        nx_azure_iot_json_writer_append_property_with_double_value(json_writer,
            (UCHAR*)TELEMETRY_GYROSCOPEY,
            sizeof(TELEMETRY_GYROSCOPEY) - 1,
            lsm6dsl_data.angular_rate_mdps[1],
            2) ||
        nx_azure_iot_json_writer_append_property_with_double_value(json_writer,
            (UCHAR*)TELEMETRY_GYROSCOPEZ,
            sizeof(TELEMETRY_GYROSCOPEZ) - 1,
            lsm6dsl_data.angular_rate_mdps[2],
            2))
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
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
    }
    else
    {
        printf("LED is turned OFF\r\n");
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
    }
}

static void direct_method_cb(AZURE_IOT_NX_CONTEXT* nx_context,
    const UCHAR* method,
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

    else if (strncmp((CHAR*)method, SET_DISPLAY_TEXT_COMMAND, method_length) == 0)
    {
        // drop the first and last character to remove the quotes
        screen_printn((CHAR*)payload + 1, payload_length - 2, L0);

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
    NX_AZURE_IOT_JSON_READER property_value_reader,
    UINT version,
    VOID* userContextCallback)
{
    UINT status;
    AZURE_IOT_NX_CONTEXT* nx_context = (AZURE_IOT_NX_CONTEXT*)userContextCallback;

    if (strncmp((CHAR*)property_name, TELEMETRY_INTERVAL_PROPERTY, property_name_len) == 0)
    {
        status = nx_azure_iot_json_reader_token_int32_get(&property_value_reader, &telemetry_interval);
        if (status == NX_AZURE_IOT_SUCCESS)
        {
            // Confirm reception back to hub
            azure_nx_client_respond_int_writeable_property(
                nx_context, TELEMETRY_INTERVAL_PROPERTY, telemetry_interval, 200, version);

            // Set a telemetry event so we pick up the change immediately
            tx_event_flags_set(&azure_iot_flags, TELEMETRY_INTERVAL_EVENT, TX_OR);
        }
    }
}

static void device_twin_property_cb(UCHAR* component_name,
    UINT component_name_len,
    UCHAR* property_name,
    UINT property_name_len,
    NX_AZURE_IOT_JSON_READER property_value_reader,
    UINT version,
    VOID* userContextCallback)
{
    if (strncmp((CHAR*)property_name, TELEMETRY_INTERVAL_PROPERTY, property_name_len) == 0)
    {
        nx_azure_iot_json_reader_token_int32_get(&property_value_reader, &telemetry_interval);
    }
}

static void device_twin_received_cb(AZURE_IOT_NX_CONTEXT* nx_context)
{
    azure_iot_nx_client_publish_int_writeable_property(nx_context, TELEMETRY_INTERVAL_PROPERTY, telemetry_interval);
    azure_iot_nx_client_publish_bool_property(&azure_iot_nx_client, LED_STATE_PROPERTY, false);
    azure_iot_nx_client_publish_properties(
        &azure_iot_nx_client, DEVICE_INFO_COMPONENT_NAME, append_device_info_properties);
}

UINT azure_iot_nx_client_entry(
    NX_IP* ip_ptr, NX_PACKET_POOL* pool_ptr, NX_DNS* dns_ptr, UINT (*unix_time_callback)(ULONG* unix_time))
{
    UINT status;
    ULONG events                    = 0;
    TELEMETRY_STATE telemetry_state = TELEMETRY_STATE_DEFAULT;

    if ((status = tx_event_flags_create(&azure_iot_flags, "Azure IoT flags")))
    {
        printf("FAIL: Unable to create nx_client event flags (0x%08x)\r\n", status);
        return status;
    }

    status =
        azure_iot_nx_client_create(&azure_iot_nx_client, ip_ptr, pool_ptr, dns_ptr, unix_time_callback, IOT_MODEL_ID);
    if (status != NX_SUCCESS)
    {
        printf("ERROR: azure_iot_nx_client_create failed (0x%08x)\r\n", status);
        return status;
    }

#ifdef ENABLE_X509
    status = azure_iot_nx_client_cert_set(&azure_iot_nx_client,
        (UCHAR*)iot_x509_device_cert,
        iot_x509_device_cert_len,
        (UCHAR*)iot_x509_private_key,
        iot_x509_private_key_len);
#else
    status = azure_iot_nx_client_sas_set(&azure_iot_nx_client, IOT_DEVICE_SAS_KEY);
#endif
    if (status != NX_SUCCESS)
    {
        printf("ERROR: azure_iot_nx_client_[sas|cert]_set failed (0x%08x)\r\n", status);
        return status;
    }

#ifdef ENABLE_DPS
    azure_iot_nx_client_dps_create(&azure_iot_nx_client, IOT_DPS_ID_SCOPE, IOT_DPS_REGISTRATION_ID);
#else
    azure_iot_nx_client_hub_create(&azure_iot_nx_client, IOT_HUB_HOSTNAME, IOT_HUB_DEVICE_ID);
#endif
    if (status != NX_SUCCESS)
    {
        printf("ERROR: azure_iot_nx_client_[hub|dps]_create failed (0x%08x)\r\n", status);
        return status;
    }

    // Register the callbacks
    azure_iot_nx_client_register_direct_method(&azure_iot_nx_client, direct_method_cb);
    azure_iot_nx_client_register_device_twin_desired_prop(&azure_iot_nx_client, device_twin_desired_property_cb);
    azure_iot_nx_client_register_device_twin_prop(&azure_iot_nx_client, device_twin_property_cb);
    azure_iot_nx_client_register_device_twin_received(&azure_iot_nx_client, device_twin_received_cb);

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

    printf("\r\nStarting Main loop\r\n");
    screen_print("Azure IoT", L0);

    while (true)
    {
        tx_event_flags_get(
            &azure_iot_flags, TELEMETRY_INTERVAL_EVENT, TX_OR_CLEAR, &events, telemetry_interval * NX_IP_PERIODIC_RATE);

        switch (telemetry_state)
        {
            case TELEMETRY_STATE_DEFAULT:
                azure_iot_nx_client_publish_telemetry(&azure_iot_nx_client, append_device_telemetry);
                break;

            case TELEMETRY_STATE_MAGNETOMETER:
                azure_iot_nx_client_publish_telemetry(&azure_iot_nx_client, append_device_telemetry_magnetometer);
                break;

            case TELEMETRY_STATE_ACCELEROMETER:
                azure_iot_nx_client_publish_telemetry(&azure_iot_nx_client, append_device_telemetry_accelerometer);
                break;

            case TELEMETRY_STATE_GYROSCOPE:
                azure_iot_nx_client_publish_telemetry(&azure_iot_nx_client, append_device_telemetry_gyroscope);
                break;
        }

        telemetry_state = (telemetry_state + 1) % 4;
    }

    return NX_SUCCESS;
}
