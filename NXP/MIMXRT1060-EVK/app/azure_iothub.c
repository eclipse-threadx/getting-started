/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */
   
#include "azure_iothub.h"

#include <stdio.h>

#include "tx_api.h"

#include "board.h"

#include "azure/azure_mqtt.h"
#include "networking.h"
#include "sntp_client.h"

#define IOT_MODEL_ID "dtmi:microsoft:gsg;1"

static AZURE_MQTT azure_mqtt;

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
}

UINT azure_iothub_run(CHAR *iot_hub_hostname, CHAR *iot_device_id, CHAR *iot_sas_key)
{
    UINT status;
    float temperature;

    // Create Azure MQTT
    status = azure_mqtt_create(&azure_mqtt, 
        &nx_ip, &nx_pool, &nx_dns_client,
        sntp_time_get,
        iot_hub_hostname, iot_device_id, iot_sas_key,
        IOT_MODEL_ID);
    if (status != NXD_MQTT_SUCCESS)
    {
        printf("Error: Failed to create Azure MQTT (0x%02x)\r\n", status);
        return status;
    }

    // Register callbacks
    azure_mqtt_register_direct_method_callback(&azure_mqtt, mqtt_direct_method);
    azure_mqtt_register_c2d_message_callback(&azure_mqtt, mqtt_c2d_message);
    azure_mqtt_register_device_twin_desired_prop_callback(&azure_mqtt, mqtt_device_twin_desired_prop);

    // Connect the Azure MQTT client
    status = azure_mqtt_connect(&azure_mqtt);
    if (status != NXD_MQTT_SUCCESS)
    {
        printf("Error: Failed to create Azure MQTT (0x%02x)\r\n", status);
        return status;
    }
    
    while (true)
    {
        temperature = 28.5;

        // Send the temperature as a telemetry event
        azure_mqtt_publish_float_telemetry(&azure_mqtt, "temperature", temperature);

        // Send the temperature as a device twin update
        azure_mqtt_publish_float_property(&azure_mqtt, "currentTemperature", temperature);

        // Sleep for 10 seconds
        tx_thread_sleep(10 * TX_TIMER_TICKS_PER_SECOND);
    }

    return NXD_MQTT_SUCCESS;
}
