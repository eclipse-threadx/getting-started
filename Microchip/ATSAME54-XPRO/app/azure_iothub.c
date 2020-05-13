#include <azure_iothub.h>

#include <stdio.h>
#include "azure/azure_mqtt.h"
#include "board_init.h"

void mqtt_thread_entry(ULONG info);
void mqtt_direct_method_invoke(CHAR *direct_method_name, CHAR *message, MQTT_DIRECT_METHOD_RESPONSE *response);
void mqtt_c2d_message(CHAR *key, CHAR *value);
void mqtt_device_twin_desired_prop_update(CHAR *message);
void set_led_state(bool level);

void set_led_state(bool level)
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

void mqtt_direct_method_invoke(CHAR *direct_method_name, CHAR *message, MQTT_DIRECT_METHOD_RESPONSE *response)
{
    // Default response - 501 Not Implemented
    int status = 501;
    if (strstr((CHAR *)direct_method_name, "set_led_state"))
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
            response->message[5] = 0;
            return;
        }
        bool new_state = !arg;
        set_led_state(new_state);
        
        // 204 No Content
        // The server successfully processed the request and is not returning any content.
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
    response->message[5] = 0;
    return;
}

void mqtt_c2d_message(CHAR *key, CHAR *value)
{
    if (strstr((CHAR *)key, "led0State"))
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
        azure_mqtt_publish_bool_property(key, arg);
    }
    else
    {
        // Update device twin property
        azure_mqtt_publish_string_property(key, value);
    }
    
    printf("Propoerty=%s updated with value=%s\r\n", key, value);
}

void mqtt_device_twin_desired_prop_update(CHAR *message)
{
    printf("Received device twin updated properties: %s\r\n", message);
}

void mqtt_thread_entry(ULONG info)
{
    printf("Starting MQTT thread\r\n");

    while (true)
    {
        float tempDegC;

#ifdef __SENSOR_BME280__
        // Print the compensated temperature readings
        WeatherClick_waitforRead();
        tempDegC = Weather_getTemperatureDegC();
#else
        tempDegC = 23.5;
#endif

        // Send the compensated temperature as a telemetry event
        azure_mqtt_publish_float_telemetry("temperature(c)", tempDegC);

        // Send the compensated temperature as a device twin update
        azure_mqtt_publish_float_property("temperature(c)", tempDegC);

        // Sleep for 1 minute
        tx_thread_sleep(5 * TX_TIMER_TICKS_PER_SECOND);
    }
}

bool azure_mqtt_init(CHAR *iot_hub_hostname, CHAR *iot_device_id, CHAR *iot_sas_key)
{
    bool status;
    status = azure_mqtt_register_main_thread_callback(mqtt_thread_entry);
    if(!status)
    {
        printf("Failed to register MQTT main thread callback\r\n");
        return status;
    }
    
    status = azure_mqtt_register_direct_method_invoke_callback(mqtt_direct_method_invoke);
    if(!status)
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
    
    status = azure_mqtt_register_device_twin_desired_prop_update(mqtt_device_twin_desired_prop_update);
    if (!status)
    {
        printf("Failed to register MQTT desired property update callback callback\r\n");
        return status;
    }
    
    // Start the Azure MQTT client
    status = azure_mqtt_start(iot_hub_hostname, iot_device_id, iot_sas_key);
    if(!status)
    {
        printf("Failed to start Azure IoT thread\r\n");
        return status;
    }
    
    return status;
}