#include "azure_iothub.h"

#include <stdio.h>

#include "board/board.h"

#include "tx_api.h"

#include "azure/azure_mqtt.h"

void mqtt_thread_entry(ULONG info);
void mqtt_direct_method_invoke(CHAR *direct_method_name, CHAR *message, MQTT_DIRECT_METHOD_RESPONSE *response);
void mqtt_c2d_message(CHAR *key, CHAR *value);

void mqtt_thread_entry(ULONG info)
{
    printf("Starting MQTT thread\r\n");

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

        if (arg == 0)
        {
            USER_LED_OFF();
        }
        else if (arg == 1)
        {
            USER_LED_ON();
        }
        else
        {
            printf("Invalid LED state. Possible states are '0' - turn LED off or '1' - turn LED on\r\n");

            response->status = 500;
            strcpy(response->message, "{}");
            return;
        }
        
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
    
//    status = azure_mqtt_register_c2d_message_callback(mqtt_c2d_message);
//    if (!status)
//    {
//        printf("Failed to register MQTT cloud to device callback callback\r\n");
//        return status;
//    }
    
    // Start the Azure MQTT client
    status = azure_mqtt_start();
    if (!status)
    {
        printf("Failed to start Azure IoT thread\r\n");
        return status;
    }

    return true;
}
