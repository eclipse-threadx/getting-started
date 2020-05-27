/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */
   
#include <stdio.h>

#include "tx_api.h"

#include "board_init.h"
#include "networking.h"
#include "sntp_client.h"
#include "azure_iothub.h"

#include "azure_config.h"

#define AZURE_THREAD_STACK_SIZE 4096
#define AZURE_THREAD_PRIORITY   4

TX_THREAD azure_thread;
UCHAR azure_thread_stack[AZURE_THREAD_STACK_SIZE];

extern VOID nx_driver_same54(NX_IP_DRIVER*);

void azure_thread_entry(ULONG parameter);
void tx_application_define(void *first_unused_memory);

void azure_thread_entry(ULONG parameter)
{
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
    if (!sntp_sync_wait())
    {
        printf("Failed to start sync SNTP time\r\n");
        return;
    }
    
    if (!azure_iothub_run(IOT_HUB_HOSTNAME, IOT_DEVICE_ID, IOT_PRIMARY_KEY))
    {
        printf("Failed to initialize Azure IoTHub\r\n");
        return;
    }
}

void tx_application_define(void *first_unused_memory)
{
    // Initialise the board
    board_init();

    // Create Azure thread
    UINT status = tx_thread_create(
        &azure_thread,
        "Azure Thread",
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
        printf("Azure IoT application failed, please restart\r\n");
    }
}

int main(void)
{
    // Enter the ThreadX kernel
    tx_kernel_enter();
    return 0;
}