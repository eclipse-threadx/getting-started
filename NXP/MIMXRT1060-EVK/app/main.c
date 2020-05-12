#include <stdio.h>
#include <time.h>

#include "tx_api.h"
#include "nx_driver_imxrt1062.h"

#include "azure_iothub.h"
#include "board_init.h"
#include "networking.h"
#include "sntp_client.h"

#define AZURE_THREAD_STACK_SIZE 2048
#define AZURE_THREAD_PRIORITY 4

void* __RAM_segment_used_end__ = 0;

TX_THREAD azure_thread;
UCHAR azure_thread_stack[AZURE_THREAD_STACK_SIZE];

void azure_thread_entry(ULONG parameter);
void tx_application_define(void* first_unused_memory);

void azure_thread_entry(ULONG parameter)
{
    printf("\r\nStarting Azure thread. Built %s, %s\r\n\r\n", __DATE__, __TIME__);
    printf("Floating point enabled %f\r\n\r\n", 3.1415);

    // Initialize the network
    if (!network_init(nx_driver_imx))
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
    if (!azure_iothub_start())
    {
        printf("Failed to start Azure Iot Hub thread\r\n");
        return;
    }

    while (true)
    {
        tx_thread_sleep(60 * TX_TIMER_TICKS_PER_SECOND);

        time_t current = time(NULL);
        printf("Time %ld\r\n", (long)current);
    }
}

void tx_application_define(void* first_unused_memory)
{
    // Initialise the board
    board_init();
        
    // Create Azure SDK thread.
    UINT status = tx_thread_create(
        &azure_thread, "Azure SDK Thread",
        azure_thread_entry, 0,
        azure_thread_stack, AZURE_THREAD_STACK_SIZE,
        AZURE_THREAD_PRIORITY, AZURE_THREAD_PRIORITY,
        TX_NO_TIME_SLICE, TX_AUTO_START);

    if (status != TX_SUCCESS)
    {
        printf("Azure MQTT application failed, please restart\r\n");
    }
}

int main(void)
{
    tx_kernel_enter();
    return 0;
}
