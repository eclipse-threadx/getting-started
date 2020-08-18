/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include <stdio.h>

#include "tx_api.h"

#include "board_init.h"
#include "cmsis_utils.h"
#include "sntp_client.h"
#include "stm_networking.h"

#include "mqtt.h"
#include "nx_client.h"

#include "azure_config.h"

#define AZURE_THREAD_STACK_SIZE 4096
#define AZURE_THREAD_PRIORITY   4

TX_THREAD azure_thread;
UCHAR azure_thread_stack[AZURE_THREAD_STACK_SIZE];

void azure_thread_entry(ULONG parameter);
void tx_application_define(void* first_unused_memory);

void azure_thread_entry(ULONG parameter)
{
    UINT status;

    printf("\r\nStarting Azure thread\r\n\r\n");
    // DevConfig_IoT_Info_t device_info;
    // read_flash(&device_info);

    DevConfig_IoT_Info_t device_info;

    if (has_credentials())
    {
        if(read_flash(&device_info) == STATUS_OK)
        {
            printf("Currently device %s is connected to %s. \n",
            device_info.device_id, device_info.hostname);
        }
        else
        {
            printf("\nError reading from flash\n");
        }

        // Start menu option
        int menu_option, rc;
        printf("Enter 0 - Continue\nEnter 1 - Erase/Reset flash\n");
        while (1)
        {
            while ((rc = scanf("%d", &menu_option)) == 0)
            {
                scanf("%*[^\n]");
                printf("Invalid input.\nEnter 0 - Continue\nEnter 1 - Erase/Reset flash\n"); fflush(stdin);
            }
            if (menu_option == 1) {
                if (erase_flash() == STATUS_OK) {
                printf("Successfully erased flash.\n");
                break;
                }
            } else if (menu_option == 0) {
                break;
            }
            printf("Invalid input.\nEnter 0 - Continue\nEnter 1 - Erase/Reset flash\n");
        }

    }

    char hostname[MAX_HOSTNAME_LEN] = "";
    char device_id[MAX_DEVICEID_LEN] = "";
    char primary_key[MAX_KEY_LEN] = "";

    while (!has_credentials())
    {
        printf("No Azure IoT credentials stored in device. Please enter credentials into serial terminal. \n\n");

        printf("Please enter your IoTHub hostname: \n");
        if (scanf("%s", hostname) == 0) {
            /* Not valid input, flush stdin */
            fflush(stdin);
            continue;
        }

        printf("Please enter your IoTHub device ID: \n");
        if (scanf("%s", device_id) == 0) {
            /* Not valid input, flush stdin */
            fflush(stdin);
            continue;
        }

        printf("Please enter your IoTHub primary key: \n");
        if (scanf("%s", primary_key) == 0) {
            /* Not valid input, flush stdin */
            fflush(stdin);
            continue;
        }

        printf("Please verify you have entered the correct configuration: \n\n"); printf("hostname: %s\n", hostname); 
        printf("device_id: %s\n", device_id); printf("primary_key: %s\n\n", primary_key);

        // Logic about going back and changing if things are wrong
        printf("Press 0: YES, proceed \nPress 1: NO, re-enter credentials \n");

        int user_selection = 0;
        if (scanf("%d", &user_selection) == 0)
        {
            /* Not valid input, flush stdin */
            fflush(stdin);
            continue;
        }
        if (user_selection == 1)
        {
            // loop again
            continue;
        }

        strcpy(device_info.hostname, hostname);
        strcpy(device_info.device_id, device_id);
        strcpy(device_info.primary_key, primary_key);


        if (save_to_flash(&device_info) == STATUS_OK)
        {
            printf("Successfully saved credentials to flash. \n\n");
        }
    }

    //printf("%x", device_info.ssid[0]); 0xEF

    // WiFi Initialization
    if(device_info.ssid[0] != 0xEF)
    {
        //Initialize the network
        if (!wifi_init(device_info.ssid, device_info.pswd,
        device_info.security))
        {
            printf("Error initializing wifi from stored info");
        }
    }
    else
    {
        SoftAP_WiFi_Info_t softAP_wifi;

        if (wifi_softAP_init(&softAP_wifi))
        {
            printf("Initialized wifi connection with SoftAP. SSID: %s\n\n", softAP_wifi.SSID);
        }

        strcpy(device_info.ssid, softAP_wifi.SSID);
        strcpy(device_info.pswd, softAP_wifi.PSWD);
        device_info.security = softAP_wifi.Security;

        if (save_to_flash(&device_info) == STATUS_OK) {
            printf("Successfully saved WiFi credentials to flash. \n\n");
        }
    }

    

    if (stm32_network_init() != NX_SUCCESS)
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

#ifdef USE_NX_CLIENT_PREVIEW
    if ((status = azure_iot_nx_client_entry(&nx_ip, &nx_pool, &nx_dns_client, sntp_time)))
#else
    if ((status = azure_iot_mqtt_entry(&nx_ip, &nx_pool, &nx_dns_client, sntp_time_get)))
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
    UINT status = tx_thread_create(
        &azure_thread, "Azure Thread",
        azure_thread_entry, 0,
        azure_thread_stack, AZURE_THREAD_STACK_SIZE,
        AZURE_THREAD_PRIORITY, AZURE_THREAD_PRIORITY,
        TX_NO_TIME_SLICE, TX_AUTO_START);

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
