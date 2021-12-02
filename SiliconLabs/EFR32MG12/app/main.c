/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include <stdio.h>

#include "tx_api.h"

#include "nx_sl_wfx_driver.h"

#include "sl_system_init.h"
#include "sl_system_kernel.h"

#include "networking.h"
#include "sntp_client.h"

#include "azure_config.h"
#include "nx_client.h"

#define AZURE_THREAD_STACK_SIZE 4096
#define AZURE_THREAD_PRIORITY   4

static TX_THREAD azure_thread;
static ULONG azure_thread_stack[AZURE_THREAD_STACK_SIZE / sizeof(ULONG)];

static nx_sl_wfx_wifi_info_t wifi_info = {.ssid = WIFI_SSID, .password = WIFI_PASSWORD};

UINT wifi_init()
{
    switch (WIFI_MODE)
    {
        case None:
            wifi_info.mode = WFM_SECURITY_MODE_OPEN;
            break;
        case WEP:
            wifi_info.mode = WFM_SECURITY_MODE_WEP;
            break;
        case WPA_PSK_TKIP:
            wifi_info.mode = WFM_SECURITY_MODE_WPA2_WPA1_PSK;
            break;
        case WPA2_PSK_AES:
        default:
            wifi_info.mode = WFM_SECURITY_MODE_WPA2_PSK;
            break;
    };

    nx_sl_wifi_info_set(&wifi_info);

    return NX_SUCCESS;
}

void azure_thread_entry(ULONG thread_input)
{
    UINT status;

    printf("Starting Azure thread\r\n\r\n");

    // Set wifi network information
    if ((status = wifi_init()))
    {
        printf("ERROR: wifi_init (0x%08x)\r\n", status);
    }

    // Initialize the network
    else if ((status = network_init(nx_sl_wfx_driver_entry)))
    {
        printf("ERROR: Failed to initialize the network (0x%08x)\r\n", status);
    }

    else if ((status = azure_iot_nx_client_entry(&nx_ip, &nx_pool, &nx_dns_client, sntp_time)))
    {
        printf("ERROR: Failed to run Azure IoT (0x%08x)\r\n", status);
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
        printf("ERROR: Azure IoT thread creation failed\r\n");
    }
}

int main(void)
{
    // Initialize the board
    sl_system_init();

    // Enter the ThreadX kernel. Task(s) created in tx_application_define() will start running
    sl_system_kernel_start();

    return 0;
}
