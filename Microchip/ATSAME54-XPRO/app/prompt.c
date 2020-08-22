/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */


#include "prompt.h"

void serial_setup()
{
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
}