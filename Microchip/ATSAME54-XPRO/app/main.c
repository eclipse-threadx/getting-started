/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include <stdio.h>

#include "tx_api.h"

#include "board_init.h"
#include "networking.h"
#include "sntp_client.h"

#include "mqtt.h"
#include "nx_client.h"

#include "azure_config.h"
#include "device_config.h"

#define AZURE_THREAD_STACK_SIZE 4096
#define AZURE_THREAD_PRIORITY   4

TX_THREAD azure_thread;
UCHAR azure_thread_stack[AZURE_THREAD_STACK_SIZE];

extern VOID nx_driver_same54(NX_IP_DRIVER*);

void azure_thread_entry(ULONG parameter);
void tx_application_define(void* first_unused_memory);

void azure_thread_entry(ULONG parameter)
{
    UINT status;

    printf("Starting Azure thread\r\n\r\n");
    DevConfig_IoT_Info_t device_info;
	
	if (has_credentials())
	{
		if(read_flash(&device_info) == STATUS_OK)
		{
			printf("\nCurrently device %s is connected to %s. \n", device_info.device_id, device_info.hostname);
		}
		else
		{
			printf("\nError reading from flash\n");
		}
		
		// Start menu option
		int menu_option;
		printf("Press 0: Continue .\r\nPress 1: Erase credentials/Reset Flash .\r\n");

		if (scanf("%d", &menu_option) == 0)
		{
			/* Not valid input, flush stdin */
			fflush(stdin);
		}
		if (menu_option == 1)
		{
			erase_flash();
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
		printf(hostname);
		
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
		
		printf("Please verify you have entered the correct configuration: \n\n");
		printf("hostname: %s\n", hostname);
		printf("device_id: %s\n", device_id);
		printf("primary_key: %s\n\n", primary_key);
		
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
		
		if (save_to_flash(hostname, device_id, primary_key) == STATUS_OK)
		{
			printf("Successfully saved credentials to flash. \n\n");
		}
	} 
    

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
