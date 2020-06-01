/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */
   
#include "azure_iothub.h"

#include <stdio.h>

#include "atmel_start.h"
#include "Bosch_BME280.h"

#include "azure_config.h"
#include "azure/azure_mqtt.h"
#include "networking.h"
#include "sntp_client.h"

static AZURE_MQTT azure_mqtt;

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

static void mqtt_direct_method(CHAR *direct_method_name, CHAR *message, MQTT_DIRECT_METHOD_RESPONSE *response)
{
    // Default response - 501 Not Implemented
    int status = 501;
    if (strcmp(direct_method_name, "set_led_state") == 0)
    {
        // Set LED state
        // '0' - turn LED off
        // '1' - turn LED on
        int arg = atoi(message);
        if (arg != 0 && arg != 1)
        {
            printf("Invalid LED state. Possible states are '0' - turn LED off or '1' - turn LED on\r\n");

            response->status = 500;
            strcpy(response->message, "{}");
            return;
        }
        bool new_state = !arg;
        set_led_state(new_state);
        
        // 204 No Content
        // The server successfully processed the request and is not returning any content.
        status = 204;

        // Update device twin property
        azure_mqtt_publish_bool_property(&azure_mqtt, "led0State", arg);

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

static void mqtt_c2d_message(CHAR *key, CHAR *value)
{
    if (strcmp(key, "led0State") == 0)
    {
        // Set LED state
        // '0' - turn LED off
        // '1' - turn LED on
        int arg = atoi(value);
        if (arg != 0 && arg != 1)
        {
            printf("Invalid LED state. Possible states are '0' - turn LED off or '1' - turn LED on\r\n");
            return;
        }
        bool new_state = !arg;
        set_led_state(new_state);

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
    float tempDegC;

    // Create Azure MQTT
    status = azure_mqtt_create(&azure_mqtt, 
        &nx_ip, &nx_pool, &nx_dns_client,
        sntp_time_get,
        iot_hub_hostname, iot_device_id, iot_sas_key);    
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

    printf("Starting MQTT loop\r\n");

    while (true)
    {
#if __SENSOR_BME280__ == 1
        // Print the compensated temperature readings
        WeatherClick_waitforRead();
        tempDegC = Weather_getTemperatureDegC();
#else
        tempDegC = 23.5;
#endif

        // Send the compensated temperature as a telemetry event
        azure_mqtt_publish_float_telemetry(&azure_mqtt, "temperature(c)", tempDegC);

        // Send the compensated temperature as a device twin update
        azure_mqtt_publish_float_property(&azure_mqtt, "temperature(c)", tempDegC);

        // Sleep for 10 seconds
        tx_thread_sleep(10 * TX_TIMER_TICKS_PER_SECOND);
    }
    
    return NXD_MQTT_SUCCESS;
}