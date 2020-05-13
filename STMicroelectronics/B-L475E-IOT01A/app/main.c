#include <stdio.h>
#include <time.h>

#include "tx_api.h"

#include "azure_iothub.h"
#include "networking.h"
#include "board_init.h"
#include "sntp_client.h"

#define AZURE_THREAD_STACK_SIZE 4096
#define AZURE_THREAD_PRIORITY 4

UCHAR azure_thread_stack[AZURE_THREAD_STACK_SIZE];

TX_THREAD azure_thread;

void azure_thread_entry(ULONG parameter);

void azure_thread_entry(ULONG parameter)
{
    // Initialize the network
    if (stm32_network_init() != NX_SUCCESS)
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
        return;
    }

    // Start the Azure IoT hub thread
    if (!azure_mqtt_init())
    {
        printf("Failed to start Azure Iot Hub thread\r\n");
        return;
    }

    while (true)
    {
        time_t current = time(NULL);
        printf("Time %ld\r\n", (long)current);
        tx_thread_sleep(60 * TX_TIMER_TICKS_PER_SECOND);
    }
}

// Threadx entry point
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

    // Enter the ThreadX kernel
    tx_kernel_enter();
    return 0;
}
