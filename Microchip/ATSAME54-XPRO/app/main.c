#include <stdio.h>
#include <time.h>

#include "tx_api.h"

#include "azure/azure_mqtt.h"
#include "board_init.h"
#include "networking.h"
#include "sntp_client.h"
#include "azure_config.h"

#define AZURE_THREAD_STACK_SIZE 4096
#define AZURE_THREAD_PRIORITY   4

TX_THREAD azure_thread;
UCHAR azure_thread_stack[AZURE_THREAD_STACK_SIZE];

void* __RAM_segment_used_end__ = 0;

extern  VOID nx_driver_same54(NX_IP_DRIVER*);

void azure_thread_entry(ULONG parameter);
void mqtt_thread_entry(ULONG info);
void mqtt_direct_method_invoke(CHAR *topic, CHAR *request_id, CHAR *direct_method_name, CHAR *message);
void mqtt_c2d_message(CHAR *key, CHAR *value);
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

void mqtt_direct_method_invoke(CHAR *topic, CHAR *request_id, CHAR *direct_method_name, CHAR *message)
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

            status = 500;
            azure_mqtt_respond_direct_method(topic, request_id, "{}", status);

            return;
        }
        bool new_state = !arg;
        set_led_state(new_state);
        
        // 204 No Content
        // The server successfully processed the request and is not returning any content.
        status = 204;

        // Update device twin property
        azure_mqtt_publish_bool_twin("led0State", arg);

        printf("Direct method=%s invoked\r\n", direct_method_name);
    }
    else
    {
        printf("Received direct menthod=%s is unknown\r\n", direct_method_name);
    }

    azure_mqtt_respond_direct_method(topic, request_id, "{}", status);
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
        azure_mqtt_publish_bool_twin(key, arg);
    }
    else
    {
        // Update device twin property
        azure_mqtt_publish_string_twin(key, value);
    }
    
    printf("Propoerty=%s updated with value=%s\r\n", key, value);
}

void mqtt_thread_entry(ULONG info)
{
    printf("Starting MQTT thread\r\n");

    while (true)
    {
        float tempDegC;

        // Print the compensated temperature readings
        WeatherClick_waitforRead();
        tempDegC = Weather_getTemperatureDegC();

        // Send the compensated temperature as a telemetry event
        azure_mqtt_publish_float_telemetry("temperature(C)", tempDegC);

        // Send the compensated temperature as a device twin update
        azure_mqtt_publish_float_twin("temperature(C)", tempDegC);

        // Sleep for 1 minute
        tx_thread_sleep(60 * TX_TIMER_TICKS_PER_SECOND);
    }
}

void azure_thread_entry(ULONG parameter)
{
    // Sleep to allow time for user to connect serial terminal
    for(int i = 10 ; i > 0 ; --i)
    {
        printf("Starting application in %d\r\n", i);
        tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND);
    }

    printf("Starting Azure thread\r\n");
    
    // Initialise the network
    if(!network_init(nx_driver_same54))
    {
        printf("Failed to initialize the network\r\n");
        return;
    }

     // Start the SNTP client
     if (!sntp_start())
     {
         printf("Failed to start the SNTP client\r\n");
         return;
     }

     // Wait for an SNTP sync
     if (!sntp_wait_for_sync())
     {
         printf("Failed to start sync SNTP time\r\n");

     }
    
    if (!azure_mqtt_register_main_thread_callback(mqtt_thread_entry))
    {
        printf("Failed to register MQTT main thread callback\r\n");
        return;
    }
    
    if (!azure_mqtt_register_direct_method_invoke_callback(mqtt_direct_method_invoke))
    {
        printf("Failed to register MQTT direct method callback\r\n");
        return;
    }
    
    if (!azure_mqtt_register_c2d_message_callback(mqtt_c2d_message))
    {
        printf("Failed to register MQTT cloud to device callback callback\r\n");
        return;
    }
    
    // Start the Azure MQTT client
    if(!azure_mqtt_start())
    {
        printf("Failed to start Azure IoT thread\r\n");
        return;
    }
    
    while (true)
    {
        time_t current = time(NULL);
        printf("Time %ld\r\n", (long)current);
        tx_thread_sleep(60 * TX_TIMER_TICKS_PER_SECOND);
    }
}

// threadx entry point
void tx_application_define(void *first_unused_memory)
{
    // Initialise the board
    board_init();

    // Create Azure thread
    UINT status = tx_thread_create(
        &azure_thread,
        "Azure SDK Thread",
        azure_thread_entry,
        0,
        azure_thread_stack,
        AZURE_THREAD_STACK_SIZE,
        AZURE_THREAD_PRIORITY,
        AZURE_THREAD_PRIORITY,
        TX_NO_TIME_SLICE,
        TX_AUTO_START);

    if (status != TX_SUCCESS)
    {
        printf("Azure MQTT application failed, please restart\r\n");
    }
}

int main(void)
{
    // Enter the ThreadX kernel
    tx_kernel_enter();
    return 0;
}