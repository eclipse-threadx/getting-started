/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include <stdio.h>

#include "tx_api.h"

#include "log/gsg_log.h"

#include "board_init.h"
#include "sntp_client.h"
#include "stm_networking.h"

#include "nx_client.h"

#include "azure_config.h"

#define LOG_COMPONENT_NAME "main"

#define AZURE_THREAD_STACK_SIZE 4096
#define AZURE_THREAD_PRIORITY   4

TX_THREAD azure_thread;
ULONG azure_thread_stack[AZURE_THREAD_STACK_SIZE / sizeof(ULONG)];

static void azure_thread_entry(ULONG parameter)
{
    UINT status;

    GsgLogInfo("Starting Azure thread");

    // Initialize the network
    if ((status = stm_network_init(WIFI_SSID, WIFI_PASSWORD, WIFI_MODE)))
    {
        GsgLogError("Failed to initialize the network (0x%08x)", status);
    }

    else if ((status = azure_iot_nx_client_entry(&nx_ip, &nx_pool, &nx_dns_client, sntp_time)))
    {
        GsgLogError("Failed to run Azure IoT (0x%08x)", status);
    }
}

void tx_application_define(void* first_unused_memory)
{
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
        GsgLogError("Azure IoT thread creation failed");
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
