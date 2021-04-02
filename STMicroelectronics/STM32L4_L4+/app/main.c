/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include <inttypes.h>
#include <stdio.h>

#include "tx_api.h"

#include "board_init.h"
#include "cmsis_utils.h"
#include "sntp_client.h"
#include "stm_networking.h"

#include "legacy/mqtt.h"
#include "nx_client.h"

#include "azure_config.h"

#include "az_ulib_ipc_api.h"
#include "cipher_v1i1.h"

#define AZURE_THREAD_STACK_SIZE 4096
#define AZURE_THREAD_PRIORITY   4

TX_THREAD azure_thread;
ULONG azure_thread_stack[AZURE_THREAD_STACK_SIZE / sizeof(ULONG)];
static az_ulib_ipc _az_ipc_handle;


void azure_thread_entry(ULONG parameter);
void tx_application_define(void* first_unused_memory);

void azure_thread_entry(ULONG parameter)
{
    UINT status;
    az_result result;

    printf("\r\nStarting Azure thread\r\n\r\n");

    //Start DCF.
    if((result = az_ulib_ipc_init(&_az_ipc_handle)) != AZ_OK)
    {
        (void)printf("Initialize IPC failed with code %" PRIi32 ".\r\n", result);
        return;
    }

    cipher_v1i1_create();

    // Initialize the network
    if (stm32_network_init(WIFI_SSID, WIFI_PASSWORD, WIFI_MODE) != NX_SUCCESS)
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

#ifdef ENABLE_LEGACY_MQTT
    if ((status = azure_iot_mqtt_entry(&nx_ip, &nx_pool, &nx_dns_client, sntp_time_get)))
#else
    if ((status = azure_iot_nx_client_entry(&nx_ip, &nx_pool, &nx_dns_client, sntp_time)))
#endif
    {
        printf("Failed to run Azure IoT (0x%04x)\r\n", status);
        return;
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
        printf("Azure IoT thread creation failed\r\n");
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
