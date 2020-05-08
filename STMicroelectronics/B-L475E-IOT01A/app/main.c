#include <stdio.h>
#include <time.h>

#include "tx_api.h"

#include "networking.h"
#include "board_init.h"
#include "sntp_client.h"

#include "azure/azure_mqtt.h"

#define AZURE_THREAD_STACK_SIZE 4096
#define AZURE_THREAD_PRIORITY 4

UCHAR azure_thread_stack[AZURE_THREAD_STACK_SIZE];

TX_THREAD azure_thread;

void mqtt_thread_entry(ULONG info);
void azure_thread_entry(ULONG parameter);

void mqtt_thread_entry(ULONG info)
{
    printf("Starting MQTT thread\r\n");

    while (true)
    {
        float tempDegC = 25.0;

        // Send the compensated temperature as a telemetry event
        azure_mqtt_publish_float_twin("temperature(C)", tempDegC);

        // Send the compensated temperature as a device twin update
        azure_mqtt_publish_float_telemetry("temperature(C)", tempDegC);

        // Sleep for 1 minute
        tx_thread_sleep(60 * TX_TIMER_TICKS_PER_SECOND);
    }
}

void azure_thread_entry(ULONG parameter)
{
    // Initialize the network
    threadx_net_init();

//    // Start the SNTP client
//    if (!sntp_start())
//    {
//        printf("Failed to start the SNTP client\r\n");
//        return;
//    }
//
//    // Wait for an SNTP sync
//    if (!sntp_wait_for_sync())
//    {
//        printf("Failed to start sync SNTP time\r\n");
//        return;
//    }

    if(!azure_mqtt_register_main_thread_callback(mqtt_thread_entry))
    {
        printf("Failed to register MQTT main thread callback\r\n");
        return;
    }

    // Start the Azure MQTT client
    if (!azure_mqtt_start())
    {
        printf("Failed to start MQTT client thread\r\n");
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
void tx_application_define(void* first_unused_memory)
{
    // Create Azure thread
    UINT status = tx_thread_create(
        &azure_thread, "Azure SDK Thread",
        azure_thread_entry, 0,
        azure_thread_stack, AZURE_THREAD_STACK_SIZE,
        AZURE_THREAD_PRIORITY, AZURE_THREAD_PRIORITY,
        TX_NO_TIME_SLICE, TX_AUTO_START);

    if (status != TX_SUCCESS)
    {
        printf("Azure SDK thread creation failed\r\n");
    }
}

int main(void)
{
    // Initialize the board
    board_init();

    // enter the threadx kernel
    tx_kernel_enter();
    
    return 0;
}
