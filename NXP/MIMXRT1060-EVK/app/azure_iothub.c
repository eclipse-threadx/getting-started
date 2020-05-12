#include "azure_iothub.h"

#include <stdio.h>

#include "tx_api.h"

#include "board.h"

#include "azure/azure_mqtt.h"

static void set_led_state(bool level);
static void mqtt_thread_entry(ULONG info);
static void mqtt_direct_method_invoke(CHAR *direct_method_name, CHAR *message, MQTT_DIRECT_METHOD_RESPONSE *response);
static void mqtt_c2d_message(CHAR *key, CHAR *value);

static void set_led_state(bool level)
{
    if (level)
    {
        // Pin level set to "low" state
        printf("LED is turned ON\r\n");
        USER_LED_ON();
    }
    else
    {
        // Pin level set to "high" state
        printf("LED is turned OFF\r\n");
        USER_LED_OFF();
    }
}

static void mqtt_direct_method_invoke(CHAR *direct_method_name, CHAR *message, MQTT_DIRECT_METHOD_RESPONSE *response)
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
            return;
        }

        set_led_state(arg);

        // 204 No Content, the server successfully processed the request and is not returning any content.
        status = 204;

        // Update device twin property
        azure_mqtt_publish_bool_twin("led0State", arg);

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
        azure_mqtt_publish_bool_twin(key, arg);
    }
    else
    {
        // Update device twin property
        azure_mqtt_publish_string_twin(key, value);
    }

    printf("Propoerty=%s updated with value=%s\r\n", key, value);
}

static void mqtt_thread_entry(ULONG info)
{
    while (true)
    {
        float temperature = 28.5;

        // Send the compensated temperature as a telemetry event
        azure_mqtt_publish_float_telemetry("temperature", temperature);

        // Send the compensated temperature as a device twin update
        azure_mqtt_publish_float_twin("temperature", temperature);

        // Sleep for 1 minute
        tx_thread_sleep(60 * TX_TIMER_TICKS_PER_SECOND);
    }
}

bool azure_iothub_start()
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
    status = azure_mqtt_start();
    if (!status)
    {
        printf("Failed to start Azure IoT thread\r\n");
        return status;
    }

    return true;
}
