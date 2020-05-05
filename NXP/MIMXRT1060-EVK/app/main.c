#include <stdio.h>

#include "tx_api.h"

#include "board.h"

#include "board_init.h"
#include "network.h"
#include "sync_time.h"

#include "azure_central.h"

// define the thread for running Azure SDK on Azure RTOS
#define AZURE_THREAD_STACK_SIZE 4096
#define AZURE_THREAD_PRIORITY 4

// define the memory area for the SDK thread
UCHAR azure_thread_stack[AZURE_THREAD_STACK_SIZE];

TX_THREAD azure_thread;

#define EXAMPLE_LED_GPIO BOARD_USER_LED_GPIO
#define EXAMPLE_LED_GPIO_PIN BOARD_USER_LED_GPIO_PIN

// Azure thread
void azure_thread_entry(ULONG parameter)
{
	printf("Starting Azure thread\r\n");
    
	// initialise the network
	network_init();
//    sntp_init();

	// synchonise time via SNTP
//	sntp_sync();
	
	// run the sample
	azure_central_application();

    bool g_pinSet = false;
    while (true)
    {
        printf("hello world\r\n");
        if (g_pinSet)
        {
            GPIO_PinWrite(BOARD_USER_LED_GPIO, BOARD_USER_LED_GPIO_PIN, 0U);
            g_pinSet = false;
        }
        else
        {
            GPIO_PinWrite(BOARD_USER_LED_GPIO, BOARD_USER_LED_GPIO_PIN, 1U);
            g_pinSet = true;
        }
        
        // Sleep 2 second
	    tx_thread_sleep(2 * TX_TIMER_TICKS_PER_SECOND);
    }

    sntp_deinit();
}

// threadx entry point
void tx_application_define(void* first_unused_memory)
{
    // create Azure SDK thread.
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
