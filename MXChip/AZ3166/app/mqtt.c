/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "mqtt.h"

#include <stdio.h>

#include "screen.h"
#include "sensor.h"
#include "stm32f4xx_hal.h"

#include "azure_iot_mqtt.h"
#include "json_utils.h"
#include "sntp_client.h"

#include "azure_config.h"

#define IOT_MODEL_ID "dtmi:com:examples:gsg;1"

#define TELEMETRY_INTERVAL_PROPERTY "telemetryInterval"
#define LED_STATE_PROPERTY          "ledState"

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

static void mqtt_direct_method(AZURE_IOT_MQTT* azure_iot_mqtt, CHAR* direct_method_name, CHAR* message)
{
    if (strcmp(direct_method_name, "setLedState") == 0)
    {
        printf("Direct method=%s invoked\r\n", direct_method_name);

        // 'false' - turn LED off
        // 'true'  - turn LED on
        bool arg = (strcmp(message, "true") == 0);

        set_led_state(arg);

        // Return success
        azure_iot_mqtt_respond_direct_method(azure_iot_mqtt, 200);

        // Update device twin property
        azure_iot_mqtt_publish_bool_property(azure_iot_mqtt, LED_STATE_PROPERTY, arg);
    }
    else
    {
        printf("Received direct method=%s is unknown\r\n", direct_method_name);
        azure_iot_mqtt_respond_direct_method(azure_iot_mqtt, 501);
    }
}

static void mqtt_c2d_message(AZURE_IOT_MQTT* azure_iot_mqtt, CHAR* properties, CHAR* message)
{
    printf("Received C2D message, properties='%s', message='%s'\r\n", properties, message);
}

static void mqtt_device_twin_desired_prop(AZURE_IOT_MQTT* azure_iot_mqtt, CHAR* message)
{
    jsmn_parser parser;
    jsmntok_t tokens[64];
    INT token_count;

    jsmn_init(&parser);
    token_count = jsmn_parse(&parser, message, strlen(message), tokens, 64);

    if (findJsonInt(message, tokens, token_count, TELEMETRY_INTERVAL_PROPERTY, &telemetry_interval))
    {
        // Set a telemetry event so we pick up the change immediately
        tx_event_flags_set(&azure_iot_flags, TELEMETRY_INTERVAL_EVENT, TX_OR);

        // Confirm reception back to hub
        azure_iot_mqtt_respond_int_writeable_property(
            azure_iot_mqtt, TELEMETRY_INTERVAL_PROPERTY, telemetry_interval, 200);
    }
}

static void mqtt_device_twin_prop(AZURE_IOT_MQTT* azure_iot_mqtt, CHAR* message)
{
    jsmn_parser parser;
    jsmntok_t tokens[64];
    INT token_count;

    jsmn_init(&parser);
    token_count = jsmn_parse(&parser, message, strlen(message), tokens, 64);

    if (findJsonInt(message, tokens, token_count, TELEMETRY_INTERVAL_PROPERTY, &telemetry_interval))
    {
        // Set a telemetry event so we pick up the change immediately
        tx_event_flags_set(&azure_iot_flags, TELEMETRY_INTERVAL_EVENT, TX_OR);
    }

    // Report writeable properties to the Hub
    azure_iot_mqtt_publish_int_writeable_property(azure_iot_mqtt, TELEMETRY_INTERVAL_PROPERTY, telemetry_interval);
}

UINT azure_iot_mqtt_entry(NX_IP* ip_ptr, NX_PACKET_POOL* pool_ptr, NX_DNS* dns_ptr, ULONG (*sntp_time_get)(VOID))
{
    UINT status;
    ULONG events;
    lps22hb_t lps22hb_data;
    hts221_data_t hts221_data;
    lsm6dsl_data_t lsm6dsl_data;
    lis2mdl_data_t lis2mdl_data;

    int telemetry_state = 0;

    if ((status = tx_event_flags_create(&azure_iot_flags, "Azure IoT flags")))
    {
        printf("FAIL: Unable to create nx_client event flags (0x%02x)\r\n", status);
        return status;
    }

#ifdef ENABLE_DPS
    // Create Azure MQTT for Hub via DPS
    status = azure_iot_mqtt_create_with_dps(&azure_iot_mqtt,
        ip_ptr,
        pool_ptr,
        dns_ptr,
        sntp_time_get,
        IOT_DPS_ENDPOINT,
        IOT_DPS_ID_SCOPE,
        IOT_DPS_REGISTRATION_ID,
        IOT_PRIMARY_KEY,
        IOT_MODEL_ID);
#else
    // Create Azure MQTT for Hub
    status = azure_iot_mqtt_create(&azure_iot_mqtt,
        ip_ptr,
        pool_ptr,
        dns_ptr,
        sntp_time_get,
        IOT_HUB_HOSTNAME,
        IOT_DEVICE_ID,
        IOT_PRIMARY_KEY,
        IOT_MODEL_ID);
#endif

    if (status != NXD_MQTT_SUCCESS)
    {
        printf("Error: Failed to create Azure IoT MQTT (0x%04x)\r\n", status);
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

    // Update ledState property
    azure_iot_mqtt_publish_bool_property(&azure_iot_mqtt, LED_STATE_PROPERTY, false);

    // Request the device twin
    azure_iot_mqtt_device_twin_request(&azure_iot_mqtt);

    printf("Starting MQTT loop\r\n");
    screen_print("Azure IoT", L0);
    while (true)
    {
        // Sleep
        tx_event_flags_get(
            &azure_iot_flags, TELEMETRY_INTERVAL_EVENT, TX_OR_CLEAR, &events, telemetry_interval * NX_IP_PERIODIC_RATE);        

        switch (telemetry_state)
        {
            case 0:
                // Send the compensated temperature
                lps22hb_data = lps22hb_data_read();
                azure_iot_mqtt_publish_float_telemetry(&azure_iot_mqtt, "temperature", lps22hb_data.temperature_degC);
                break;

            case 1:
                // Send the compensated pressure
                lps22hb_data = lps22hb_data_read();
                azure_iot_mqtt_publish_float_telemetry(&azure_iot_mqtt, "pressure", lps22hb_data.pressure_hPa);
                break;

            case 2:
                // Send the compensated humidity
                hts221_data = hts221_data_read();
                azure_iot_mqtt_publish_float_telemetry(&azure_iot_mqtt, "humidity", hts221_data.humidity_perc);
                break;

            case 3:
                // Send the compensated acceleration
                lsm6dsl_data = lsm6dsl_data_read();
                azure_iot_mqtt_publish_float_telemetry(
                    &azure_iot_mqtt, "acceleration", lsm6dsl_data.acceleration_mg[0]);
                break;

            case 4:
                // Send the compensated magnetic
                lis2mdl_data = lis2mdl_data_read();
                azure_iot_mqtt_publish_float_telemetry(&azure_iot_mqtt, "magnetic", lis2mdl_data.magnetic_mG[0]);
                break;
        }

        telemetry_state = (telemetry_state + 1) % 5;
    }

    return NXD_MQTT_SUCCESS;
}