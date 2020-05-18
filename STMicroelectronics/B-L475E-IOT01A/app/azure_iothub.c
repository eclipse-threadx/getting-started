/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "azure_iothub.h"

#include <stdio.h>

#include "stm32l475e_iot01.h"
#include "stm32l475e_iot01_tsensor.h"

#include "azure/azure_mqtt.h"

void set_led_state(bool level);
void mqtt_direct_method_invoke(CHAR *direct_method_name, CHAR *message, MQTT_DIRECT_METHOD_RESPONSE *response);
void mqtt_c2d_message(CHAR *key, CHAR *value);
void mqtt_thread_entry(ULONG info);

void set_led_state(bool level)
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

void mqtt_direct_method_invoke(CHAR *direct_method_name, CHAR *message, MQTT_DIRECT_METHOD_RESPONSE *response)
{
    // Default response - 501 Not Implemented
    int status = 501;
    if (strcmp((CHAR *)direct_method_name, "set_led_state") == 0)
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

        set_led_state(arg);
        
        // 204 No Content, the server successfully processed the request and is not returning any content.
        status = 204;

        // Update device twin property
        azure_mqtt_publish_bool_property("led0State", arg);

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

void mqtt_c2d_message(CHAR *key, CHAR *value)
{
    if (strcmp((CHAR *)key, "led0State") == 0)
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
        set_led_state(arg);

        // Update device twin property
        azure_mqtt_publish_bool_property(key, arg);
    }
    else
    {
        // Update device twin property
        azure_mqtt_publish_string_property(key, value);
    }
    
    printf("Property=%s updated with value=%s\r\n", key, value);
}

void mqtt_thread_entry(ULONG info)
{
    printf("Starting MQTT thread\r\n");

    float temperature;

    while (true)
    {
        temperature = BSP_TSENSOR_ReadTemp();
        
        // Send the compensated temperature as a telemetry event
        azure_mqtt_publish_float_property("temperature", temperature);

        // Send the compensated temperature as a device twin update
        azure_mqtt_publish_float_telemetry("temperature", temperature);

        // Sleep for 10 seconds
        tx_thread_sleep(10 * TX_TIMER_TICKS_PER_SECOND);
    }
}

bool azure_mqtt_init(CHAR *iot_hub_hostname, CHAR *iot_device_id, CHAR *iot_sas_key)
{
    bool status;
    status = azure_mqtt_register_main_thread_callback(mqtt_thread_entry);
    if (!status)
    {
        printf("Failed to register MQTT main thread callback\r\n");
        return status;
    }
    
    status = azure_mqtt_register_direct_method_invoke_callback(mqtt_direct_method_invoke);
    if (!status)
    {
        printf("Failed to register MQTT direct method callback\r\n");
        return status;
    }
    
    status = azure_mqtt_register_c2d_message_callback(mqtt_c2d_message);
    if (!status)
    {
        printf("Failed to register MQTT cloud to device callback callback\r\n");
        return status;
    }
    
    // Start the Azure MQTT client
    status = azure_mqtt_start(iot_hub_hostname, iot_device_id, iot_sas_key);
    if (!status)
    {
        printf("Failed to start Azure IoT thread\r\n");
        return status;
    }

    return true;
}
