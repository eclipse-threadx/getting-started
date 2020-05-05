#include <stdio.h>
#include <time.h>

#include "tx_api.h"

#include "networking.h"
#include "board_init.h"
#include "sntp_client.h"

#include "azure/azure_mqtt.h"

// define the thread for running Azure SDK on Azure RTOS
#define AZURE_THREAD_STACK_SIZE 4096
#define AZURE_THREAD_PRIORITY 4

// define the memory area for the SDK thread
UCHAR azure_thread_stack[AZURE_THREAD_STACK_SIZE];

TX_THREAD azure_thread;

// Azure thread
void azure_thread_entry(ULONG parameter);
void azure_thread_entry(ULONG parameter)
{
    // initialise the network
    threadx_net_init();

    // Start the SNTP client
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
        tx_thread_sleep(10 * TX_TIMER_TICKS_PER_SECOND);
    }
}

// threadx entry point
void tx_application_define(void *first_unused_memory)
{
    // Create Azure SDK thread.
    UINT status = tx_thread_create(
        &azure_thread, "Azure SDK Thread",
        azure_thread_entry, 0,
        azure_thread_stack, AZURE_THREAD_STACK_SIZE,
        AZURE_THREAD_PRIORITY, AZURE_THREAD_PRIORITY,
        TX_NO_TIME_SLICE, TX_AUTO_START);

    if (status)
    {
        printf("Azure SDK thread creation failed\r\n");
    }
}

int main(void)
{
    // initialise the board
    board_init();
    
    // enter the threadx kernel
    tx_kernel_enter();
}
