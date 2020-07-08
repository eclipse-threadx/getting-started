/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "mqtt.h"

#include <stdio.h>

#include "sensor.h"
#include "stm32f4xx_hal.h"

#include "jsmn.h"

#include "azure_iot_mqtt.h"
#include "json_utils.h"
#include "sntp_client.h"

#include "azure_config.h"

#define IOT_MODEL_ID "dtmi:microsoft:gsg;1"

#define TELEMETRY_INTERVAL_EVENT 1

static AZURE_IOT_MQTT azure_iot_mqtt;
static TX_EVENT_FLAGS_GROUP azure_iot_flags;

static INT telemetry_interval = 10;

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

static void mqtt_direct_method(CHAR* direct_method_name, CHAR* message, MQTT_DIRECT_METHOD_RESPONSE* response)
{
    // Default response - 501 Not Implemented
    int status = 501;
    if (strcmp(direct_method_name, "setLedState") == 0)
    {
        // 'false' - turn LED off
        // 'true'  - turn LED on
        bool arg = (strcmp(message, "true") == 0);

        set_led_state(arg);

        // 204 No Content, the server successfully processed the request and is not returning any content.
        status = 204;

        // Update device twin property
//        azure_iot_mqtt_publish_bool_property(&azure_iot_mqtt, "ledState", arg);

        printf("Direct method=%s invoked\r\n", direct_method_name);
    }
    else
    {
        printf("Received direct method=%s is unknown\r\n", direct_method_name);
    }

    response->status = status;
    strcpy(response->message, "{}");
    return;
}

static void mqtt_c2d_message(CHAR* key, CHAR* value)
{
    printf("C2D received Property=%s updated with value=%s\r\n", key, value);
}

static void mqtt_device_twin_desired_prop(CHAR* message)
{
    jsmn_parser parser;
    jsmntok_t tokens[16];
    INT token_count;

    jsmn_init(&parser);
    token_count = jsmn_parse(&parser, message, strlen(message), tokens, 16);

    if (findJsonInt(message, tokens, token_count, "telemetryInterval", &telemetry_interval))
    {
        // Set a telemetry event so we pick up the change immediately
        tx_event_flags_set(&azure_iot_flags, TELEMETRY_INTERVAL_EVENT, TX_OR);
    }
}

static void mqtt_device_twin_prop(CHAR* message)
{
    jsmn_parser parser;
    jsmntok_t tokens[64];
    INT token_count;

    jsmn_init(&parser);
    token_count = jsmn_parse(&parser, message, strlen(message), tokens, 64);

    if (findJsonInt(message, tokens, token_count, "telemetryInterval", &telemetry_interval))
    {
        // Set a telemetry event so we pick up the change immediately
        tx_event_flags_set(&azure_iot_flags, TELEMETRY_INTERVAL_EVENT, TX_OR);
    }
}

UINT azure_iot_mqtt_entry(NX_IP* ip_ptr, NX_PACKET_POOL* pool_ptr, NX_DNS* dns_ptr, ULONG (*sntp_time_get)(VOID))
{
    UINT status;
    ULONG events;
    lps22hb_t lps22hb_data;
    hts221_data_t hts221_data;
    lsm6dsl_data_t lsm6dsl_data;
    lis2mdl_data_t lis2mdl_data;

    if ((status = tx_event_flags_create(&azure_iot_flags, "Azure IoT flags")))
    {
        printf("FAIL: Unable to create nx_client event flags (0x%02x)\r\n", status);
        return status;
    }

    // Create Azure MQTT
    status = azure_iot_mqtt_create(&azure_iot_mqtt,
        ip_ptr,
        pool_ptr,
        dns_ptr,
        sntp_time_get,
        IOT_HUB_HOSTNAME,
        IOT_DEVICE_ID,
        IOT_PRIMARY_KEY,
        IOT_MODEL_ID);
    if (status != NXD_MQTT_SUCCESS)
    {
        printf("Error: Failed to create Azure MQTT (0x%02x)\r\n", status);
        return status;
    }

    // Register callbacks
    azure_iot_mqtt_register_direct_method_callback(&azure_iot_mqtt, mqtt_direct_method);
    azure_iot_mqtt_register_c2d_message_callback(&azure_iot_mqtt, mqtt_c2d_message);
    azure_iot_mqtt_register_device_twin_desired_prop_callback(&azure_iot_mqtt, mqtt_device_twin_desired_prop);
    azure_iot_mqtt_register_device_twin_prop_callback(&azure_iot_mqtt, mqtt_device_twin_prop);

    // Connect the Azure MQTT client
    status = azure_iot_mqtt_connect(&azure_iot_mqtt);
    if (status != NXD_MQTT_SUCCESS)
    {
        printf("Error: Failed to create Azure MQTT (0x%02x)\r\n", status);
        return status;
    }

    // Request the device twin
//    azure_iot_mqtt_device_twin_request(&azure_iot_mqtt);

    printf("Starting MQTT loop\r\n");
    while (true)
    {
        // Read data from sensors
        lps22hb_data = lps22hb_data_read();
        hts221_data  = hts221_data_read();
        lsm6dsl_data = lsm6dsl_data_read();
        lis2mdl_data = lis2mdl_data_read();

        // Send the compensated temperature
        azure_iot_mqtt_publish_float_telemetry(&azure_iot_mqtt, "temperature", lps22hb_data.temperature_degC);

        // Send the compensated pressure
        azure_iot_mqtt_publish_float_telemetry(&azure_iot_mqtt, "pressure", lps22hb_data.pressure_hPa);

        // Send the compensated humidity
        azure_iot_mqtt_publish_float_telemetry(&azure_iot_mqtt, "humidityPercentage", hts221_data.humidity_perc);

        // Send the compensated acceleration
        azure_iot_mqtt_publish_float_telemetry(&azure_iot_mqtt, "acceleration", lsm6dsl_data.acceleration_mg[0]);

        // Send the compensated magnetic
        azure_iot_mqtt_publish_float_telemetry(&azure_iot_mqtt, "magnetic", lis2mdl_data.magnetic_mG[0]);

        // Sleep
        tx_event_flags_get(
            &azure_iot_flags, TELEMETRY_INTERVAL_EVENT, TX_OR_CLEAR, &events, telemetry_interval * NX_IP_PERIODIC_RATE);
    }

    return NXD_MQTT_SUCCESS;
}