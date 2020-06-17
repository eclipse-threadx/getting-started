/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */
   
#include <stdio.h>

#include "tx_api.h"

#include "board_init.h"
#include "networking.h"
#include "sntp_client.h"

#include "azure_iot_mqtt.h"
#include "azure_iot_embedded_sdk.h"
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
    UINT status;

    printf("Starting Azure thread\r\n\r\n");
    
    // Initialise the network
    if (!network_init(nx_driver_same54))
    {
        printf("Failed to initialize the network\r\n");
        return;
    }

   // Start the SNTP client
    status = sntp_start();
    if (status != NX_SUCCESS)
    {
        printf("Failed to start the SNTP client (0x%02x)\r\n", status);
        return;
    }

    // Wait for an SNTP sync
    status = sntp_sync_wait();
    if (status != NX_SUCCESS)
    {
        printf("Failed to start sync SNTP time (0x%02x)\r\n", status);
        return;
    }
    
 #ifdef AZURE_IOT_MQTT
    if (!azure_iot_mqtt_run(IOT_HUB_HOSTNAME, IOT_DEVICE_ID, IOT_PRIMARY_KEY))
    {
        printf("Failed to initialize Azure IoTHub\r\n");
        return;
    }
#else
    status = azure_iot_embedded_sdk_entry(&nx_ip, &nx_pool, &nx_dns_client, sntp_time);
    if (status != NX_SUCCESS)
    {
        printf("Failed to start Azure IoTHub\r\n");
        return;
    }    
#endif
}

void tx_application_define(void *first_unused_memory)
{
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
    // Initialize the board
    board_init();

    // Enter the ThreadX kernel
    tx_kernel_enter();
    return 0;
}
