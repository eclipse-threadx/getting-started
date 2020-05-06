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

#define PUBLISH_TOPIC "devices/%s/messages/events/"

TX_THREAD azure_thread;
UCHAR azure_thread_stack[AZURE_THREAD_STACK_SIZE];

void* __RAM_segment_used_end__ = 0;

extern  VOID nx_driver_same54(NX_IP_DRIVER*);

void azure_thread_entry(ULONG parameter);

void mqtt_thread_entry(ULONG info);
extern UINT mqtt_publish(CHAR *topic, CHAR *message);

void mqtt_thread_entry(ULONG info)
{
    CHAR mqtt_message[200];
    CHAR mqtt_publish_topic[100];

    printf("Starting MQTT thread\r\n");

    snprintf(mqtt_publish_topic, sizeof(mqtt_publish_topic), PUBLISH_TOPIC, iot_device_id);

    while (true)
    {
        float tempDegC;
        // Print the compensated temperature readings
        WeatherClick_waitforRead();
        tempDegC = Weather_getTemperatureDegC();
        
        snprintf(mqtt_message, sizeof(mqtt_message), "{\"temperature\": %3.2f C}", tempDegC);
        printf("Sending %s\r\n", mqtt_message);

        mqtt_publish(mqtt_publish_topic, mqtt_message);

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
        tx_thread_sleep(5 * TX_TIMER_TICKS_PER_SECOND);
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