/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/**************************************************************************/

#include "nx_api.h"
#include "nxd_dns.h"
#include "stm32f4xx_hal.h"

#include "azure_iothub.h"
#include "board_init.h"
#include "networking.h"
#include "sntp_client.h"

#include "azure_config.h"

/* Define the thread for running Azure demo on ThreadX (X-Ware IoT Platform).  */
#ifndef SAMPLE_THREAD_STACK_SIZE
#define SAMPLE_THREAD_STACK_SIZE (4096)
#endif /* SAMPLE_THREAD_STACK_SIZE  */

#ifndef SAMPLE_THREAD_PRIORITY
#define SAMPLE_THREAD_PRIORITY (4)
#endif /* SAMPLE_THREAD_PRIORITY  */

/* Define the memory area for sample thread.  */
UCHAR sample_thread_stack[SAMPLE_THREAD_STACK_SIZE];

/* Define the prototypes for sample thread.  */
TX_THREAD sample_thread;

void sample_thread_entry(ULONG parameter);

/* Define Sample thread entry.  */
void sample_thread_entry(ULONG parameter)
{
    UINT status;
    
    if (platform_init(WIFI_SSID, WIFI_PASSWORD, WIFI_SECURITY, WIFI_COUNTRY) != NX_SUCCESS)
    {
        printf("Failed to initialize platform.\r\n");
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

    // Enter the Azure MQTT loop
    if(!azure_iothub_run(IOT_HUB_HOSTNAME, IOT_DEVICE_ID, IOT_PRIMARY_KEY))
    {
        printf("Failed to start Azure IotHub\r\n");
        return;
    }
}

/* Define what the initial system looks like.  */
void tx_application_define(void *first_unused_memory)
{

    UINT status;

    /* Create Sample thread. */
    status = tx_thread_create(&sample_thread, "Sample Thread",
                              sample_thread_entry, 0,
                              sample_thread_stack, SAMPLE_THREAD_STACK_SIZE,
                              SAMPLE_THREAD_PRIORITY, SAMPLE_THREAD_PRIORITY,
                              TX_NO_TIME_SLICE, TX_AUTO_START);

    /* Check status.  */
    if (status)
    {
        printf("nx_packet_pool_create fail: %u\r\n", status);
        return;
    }
}

/* Define main entry point.  */
int main(void)
{
    /* Setup platform. */
    board_init();

    /* Enter the ThreadX kernel.  */
    tx_kernel_enter();

    return 0;
}
