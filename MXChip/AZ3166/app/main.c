/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/**************************************************************************/

#include <nx_api.h>
#include <nxd_dns.h>
#include <board_init.h>
#include <wwd_management.h>
#include <wwd_wifi.h>
#include <wwd_buffer_interface.h>
#include <wwd_network.h>
#include <stm32f4xx_hal.h>

#define STR_EXPAND(x) #x
#define STR(x) STR_EXPAND(x)

/* Define the default wifi ssid and password. The user can override this 
   via -D command line option or via project settings.  */
#ifndef WIFI_SSID
#error "Symbol WIFI_SSID must be defined."
#endif /* WIFI_SSID  */

#ifndef WIFI_PASSWORD
#error "Symbol WIFI_PASSWORD must be defined."
#endif /* WIFI_PASSWORD  */

/* WIFI Security, the security types are defined in wwwd_constants.h.  */
#ifndef WIFI_SECURITY
#error "Symbol WIFI_SECURITY must be defined."
#endif /* WIFI_SECURITY  */

/* Country codes are defined in wwwd_constants.h.  */
#ifndef WIFI_COUNTRY
#error "Symbol WIFI_COUNTRY must be defined."
#endif /* WIFI_COUNTRY  */

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

/* Initalize the WiFi and application */
extern int platform_init(void);

/******** Optionally substitute your Ethernet driver here. ***********/
void (*platform_driver_get())(NX_IP_DRIVER *);

/* Define Sample thread entry.  */
void sample_thread_entry(ULONG parameter)
{
    if (platform_init() != NX_SUCCESS)
    {
        printf("Failed to initialize platform.\r\n");
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


/* Get the network driver.  */
VOID(*platform_driver_get())
(NX_IP_DRIVER *)
{
    return (wiced_sta_netx_duo_driver_entry);
}

static const wiced_ssid_t wifi_ssid =
    {
        .length = sizeof(STR(WIFI_SSID)) - 1,
        .value = STR(WIFI_SSID),
};

/* Join Network.  */
UINT wifi_network_join(void *pools)
{

    /* Set pools for wifi.   */
    wwd_buffer_init(pools);

    /* Set country.  */
    if (wwd_management_wifi_on(WIFI_COUNTRY) != WWD_SUCCESS)
    {
        printf("Failed to set WiFi Country!\r\n");
        return (NX_NOT_SUCCESSFUL);
    }

    /* Attempt to join the Wi-Fi network.  */
    printf("Joining: %s\r\n", STR(WIFI_SSID));
    while (wwd_wifi_join(&wifi_ssid, WIFI_SECURITY, (uint8_t *)STR(WIFI_PASSWORD), sizeof(STR(WIFI_PASSWORD)) - 1, NULL, WWD_STA_INTERFACE) != WWD_SUCCESS)
    {
        printf("Failed to join: %s ... retrying...\r\n", STR(WIFI_SSID));
    }

    printf("Successfully joined: %s.\r\n", STR(WIFI_SSID));

    return NX_SUCCESS;
}