/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "mqtt.h"

#include <stdio.h>

#include "screen.h"
#include "sensor.h"
#include "pnp_device_info.h"
#include "stm32f4xx_hal.h"

#include "azure_iot_mqtt.h"
#include "json_utils.h"
#include "sntp_client.h"

#include "azure_config.h"

#define IOT_MODEL_ID "dtmi:com:mxchip:mxchip_iot_devkit:example:RTOSGetStarted;1"

#define IOT_MODEL_COMPONENT_NAME    "deviceInformation"

#define TELEMETRY_INTERVAL_PROPERTY "telemetryInterval"
#define LED_STATE_PROPERTY          "ledState"

#define TELEMETRY_INTERVAL_EVENT 1

#define MAX_MESSAGE_SIZE 128

static AZURE_IOT_MQTT azure_iot_mqtt;
static TX_EVENT_FLAGS_GROUP azure_iot_flags;


static INT properties_interval = 3;
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

static void mqtt_device_method(AZURE_IOT_MQTT* azure_iot_mqtt, CHAR* method_name, CHAR* message)
{
    if (strcmp(method_name, "turnOnLed") == 0)
    {
        printf("Method=%s invoked\r\n", method_name);
        set_led_state(true);

        // Return success
        azure_iot_mqtt_respond_direct_method(azure_iot_mqtt, 200);
        // Update device twin property
        azure_iot_mqtt_publish_bool_property(azure_iot_mqtt, LED_STATE_PROPERTY, true);
    }
    else if (strcmp(method_name, "turnOffLed") == 0)
    {
        printf("Method=%s invoked\r\n", method_name);
        set_led_state(false);

        // Return success
        azure_iot_mqtt_respond_direct_method(azure_iot_mqtt, 200);
        // Update device twin property
        azure_iot_mqtt_publish_bool_property(azure_iot_mqtt, LED_STATE_PROPERTY, false);
    }
    else if (strcmp(method_name, "displayText") == 0)
    {
        printf("Method=%s invoked\r\n", method_name);
        printf("Screen : %s\r\n", message);
        screen_print(message, L0);

        // Return success
        azure_iot_mqtt_respond_direct_method(azure_iot_mqtt, 200);
    }
    else
    {
        printf("Received method=%s is unknown\r\n", method_name);
        azure_iot_mqtt_respond_direct_method(azure_iot_mqtt, 501);
    }
}

static void mqtt_c2d_message(AZURE_IOT_MQTT* azure_iot_mqtt, CHAR* key, CHAR* value)
{
    printf("Property=%s updated with value=%s\r\n", key, value);
}

static void azure_iot_mqtt_publish_device_property_all(AZURE_IOT_MQTT* azure_iot_mqtt)
{
    ULONG pro_events;

    azure_iot_mqtt_publish_property(azure_iot_mqtt, IOT_MODEL_COMPONENT_NAME, DEVICE_INFO_MANUfACTURER_PROPERTY, MANUfACTURER_PROPERTY);
    azure_iot_mqtt_publish_property(azure_iot_mqtt, IOT_MODEL_COMPONENT_NAME, DEVICE_INFO_MODEL_PROPERTY, MODEL_PROPERTY);
    tx_event_flags_get(&azure_iot_flags, TELEMETRY_INTERVAL_EVENT, TX_OR_CLEAR, &pro_events, properties_interval * NX_IP_PERIODIC_RATE);

    azure_iot_mqtt_publish_property(azure_iot_mqtt, IOT_MODEL_COMPONENT_NAME, DEVICE_INFO_SWVERSION_PROPERTY, SWVERSION_PROPERTY);
    azure_iot_mqtt_publish_property(azure_iot_mqtt, IOT_MODEL_COMPONENT_NAME, DEVICE_INFO_OSNAME_PROPERTY, OSNAME_PROPERTY);
    azure_iot_mqtt_publish_property(azure_iot_mqtt, IOT_MODEL_COMPONENT_NAME, DEVICE_INFO_PROCESSORARCHITECTURE_PROPERTY, PROCESSORARCHITECTURE_PROPERTY);
    tx_event_flags_get(&azure_iot_flags, TELEMETRY_INTERVAL_EVENT, TX_OR_CLEAR, &pro_events, properties_interval * NX_IP_PERIODIC_RATE);
    
    azure_iot_mqtt_publish_property(azure_iot_mqtt, IOT_MODEL_COMPONENT_NAME, DEVICE_INFO_PROCESSORMANUfACTURER_PROPERTY, PROCESSORMANUfACTURER_PROPERTY);
    azure_iot_mqtt_publish_int_property(azure_iot_mqtt, IOT_MODEL_COMPONENT_NAME, DEVICE_INFO_TOTALSTORAGE_PROPERTY, TOTALSTORAGE_PROPERTY);
    azure_iot_mqtt_publish_int_property(azure_iot_mqtt, IOT_MODEL_COMPONENT_NAME, DEVICE_INFO_TOTAKMEMORY_PROPERTY, TOTAKMEMORY_PROPERTY);
    tx_event_flags_get(&azure_iot_flags, TELEMETRY_INTERVAL_EVENT, TX_OR_CLEAR, &pro_events, properties_interval * NX_IP_PERIODIC_RATE);
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

static void azure_iot_mqtt_publish_device_telemetry(AZURE_IOT_MQTT* azure_iot_mqtt, lps22hb_t lps22hb_data, hts221_data_t hts221_data, lis2mdl_data_t lis2mdl_data)
{
    CHAR combined_message[MAX_MESSAGE_SIZE];

    // Send multiple telemetries in a single message
    snprintf(combined_message, sizeof(combined_message), "{");
    snprintf(combined_message + strlen(combined_message), sizeof(combined_message), "\"%s\":%3.2f,", HUMIDITY, hts221_data.humidity_perc);
    snprintf(combined_message + strlen(combined_message), sizeof(combined_message), "\"%s\":%3.2f,", TEMPERATURE, lps22hb_data.temperature_degC);
    snprintf(combined_message + strlen(combined_message), sizeof(combined_message), "\"%s\":%3.2f,", PRESSURE, lps22hb_data.pressure_hPa);
    snprintf(combined_message + strlen(combined_message), sizeof(combined_message), "\"%s\":%3.2f,", MAGNETOMETERX, lis2mdl_data.magnetic_mG[0]);
    snprintf(combined_message + strlen(combined_message), sizeof(combined_message), "\"%s\":%3.2f,", MAGNETOMETERY, lis2mdl_data.magnetic_mG[1]);
    snprintf(combined_message + strlen(combined_message), sizeof(combined_message), "\"%s\":%3.2f", MAGNETOMETERZ, lis2mdl_data.magnetic_mG[2]);
    snprintf(combined_message + strlen(combined_message), sizeof(combined_message), "}");

    azure_iot_mqtt_publish_telemetry(azure_iot_mqtt, combined_message);
}

static void azure_iot_mqtt_publish_device_telemetry_acceleration(AZURE_IOT_MQTT* azure_iot_mqtt, lsm6dsl_data_t lsm6dsl_data)
{
    CHAR combined_message[MAX_MESSAGE_SIZE];

    // Send multiple telemetries in a single message
    snprintf(combined_message, sizeof(combined_message), "{");
    snprintf(combined_message + strlen(combined_message), sizeof(combined_message), "\"%s\":%3.2f,", ACCELEROMETERX, lsm6dsl_data.acceleration_mg[0]);
    snprintf(combined_message + strlen(combined_message), sizeof(combined_message), "\"%s\":%3.2f,", ACCELEROMETERY, lsm6dsl_data.acceleration_mg[1]);
    snprintf(combined_message + strlen(combined_message), sizeof(combined_message), "\"%s\":%3.2f", ACCELEROMETERZ, lsm6dsl_data.acceleration_mg[2]);
    snprintf(combined_message + strlen(combined_message), sizeof(combined_message), "}");

    azure_iot_mqtt_publish_telemetry(azure_iot_mqtt, combined_message);
}

static void azure_iot_mqtt_publish_device_telemetry_angular_rate(AZURE_IOT_MQTT* azure_iot_mqtt, lsm6dsl_data_t lsm6dsl_data)
{
    CHAR combined_message[MAX_MESSAGE_SIZE];

    // Send multiple telemetries in a single message
    snprintf(combined_message, sizeof(combined_message), "{");
    snprintf(combined_message + strlen(combined_message), sizeof(combined_message), "\"%s\":%3.2f,", GYROSCOPEX, lsm6dsl_data.angular_rate_mdps[0]);
    snprintf(combined_message + strlen(combined_message), sizeof(combined_message), "\"%s\":%3.2f,", GYROSCOPEY, lsm6dsl_data.angular_rate_mdps[1]);
    snprintf(combined_message + strlen(combined_message), sizeof(combined_message), "\"%s\":%3.2f", GYROSCOPEZ, lsm6dsl_data.angular_rate_mdps[2]);
    snprintf(combined_message + strlen(combined_message), sizeof(combined_message), "}");

    azure_iot_mqtt_publish_telemetry(azure_iot_mqtt, combined_message);
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
    azure_iot_mqtt_register_direct_method_callback(&azure_iot_mqtt, mqtt_device_method);
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

    // Update telemetryInterval property
    azure_iot_mqtt_publish_int_writeable_property(&azure_iot_mqtt, TELEMETRY_INTERVAL_PROPERTY, telemetry_interval);
    // Report device properties 
    azure_iot_mqtt_publish_device_property_all(&azure_iot_mqtt);

    printf("Starting MQTT loop\r\n");
    screen_print("Azure IoT", L0);
    while (true)
    {
        switch (telemetry_state)
        {
            case 0:
                // Send the compensated temperature, pressure, humidity and magnetic
                lps22hb_data = lps22hb_data_read();
                hts221_data = hts221_data_read();
                lis2mdl_data = lis2mdl_data_read();
                azure_iot_mqtt_publish_device_telemetry(&azure_iot_mqtt, lps22hb_data, hts221_data, lis2mdl_data);
                break;

            case 1:
                // Send the compensated acceleration
                lsm6dsl_data = lsm6dsl_data_read();
                azure_iot_mqtt_publish_device_telemetry_acceleration(&azure_iot_mqtt, lsm6dsl_data);
                break;

            case 2:
                // Send the compensated gyroscope
                lsm6dsl_data = lsm6dsl_data_read();
                azure_iot_mqtt_publish_device_telemetry_angular_rate(&azure_iot_mqtt, lsm6dsl_data);
                break;
        }

        telemetry_state = (telemetry_state + 1) % 3;

        // Sleep
        tx_event_flags_get(&azure_iot_flags, TELEMETRY_INTERVAL_EVENT, TX_OR_CLEAR, &events, telemetry_interval * NX_IP_PERIODIC_RATE);
    }

    return NXD_MQTT_SUCCESS;
}