/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include <stdio.h>

#include "tx_api.h"

#include "board_init.h"
#include "cmsis_utils.h"
#include "sntp_client.h"
#include "stm_networking.h"

#include "legacy/mqtt.h"
#include "nx_client.h"

#include "azure_config.h"

#define AZURE_THREAD_STACK_SIZE 4096
#define AZURE_THREAD_PRIORITY   4

TX_THREAD azure_thread;
ULONG azure_thread_stack[AZURE_THREAD_STACK_SIZE / sizeof(ULONG)];

void azure_thread_entry(ULONG parameter);
void tx_application_define(void* first_unused_memory);

void azure_thread_entry(ULONG parameter)
{
    UINT status;

    printf("Starting Azure thread\r\n\r\n");

    // Initialize the network
    if ((status = stm_network_init(WIFI_SSID, WIFI_PASSWORD, WIFI_MODE)))
    {
        printf("ERROR: Failed to initialize the network (0x%08x)\r\n", status);
    }

    // Connect the network
    else if ((status = stm_network_connect()))
    {
        printf("ERROR: Failed to connect the network (0x%08x)\r\n", status);
    }

    // Start the SNTP client
    else if ((status = sntp_start()))
    {
        printf("ERROR: Failed to start the SNTP client (0x%08x)\r\n", status);
    }

    // Wait for an SNTP sync
    else if ((status = sntp_sync_wait()))
    {
        printf("ERROR: Failed to start sync SNTP time (0x%02x)\r\n", status);
    }

#ifdef ENABLE_LEGACY_MQTT
    else if ((status = azure_iot_mqtt_entry(&nx_ip, &nx_pool, &nx_dns_client, sntp_time_get)))
#else
    else if ((status = azure_iot_nx_client_entry(&nx_ip, &nx_pool, &nx_dns_client, sntp_time)))
#endif
    {
        printf("ERROR: Failed to run Azure IoT (0x%04x)\r\n", status);
    }
}

void tx_application_define(void* first_unused_memory)
{
    systick_interval_set(TX_TIMER_TICKS_PER_SECOND);

    // Create Azure thread
    UINT status = tx_thread_create(&azure_thread,
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
        printf("ERROR: Azure IoT thread creation failed\r\n");
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
