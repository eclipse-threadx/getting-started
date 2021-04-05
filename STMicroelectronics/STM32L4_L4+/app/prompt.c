/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "prompt.h"

#define ENABLE_DPS
#define STM32L4

bool serial_setup()
{
    DevConfig_IoT_Info_t device_info;
    bool first_init = false;
   
    TX_TIMER my_timer;
    UINT status;
    CHAR *name;
    UINT active;
    ULONG remaining_ticks;
    ULONG reschedule_ticks;

    if (has_credentials())
    {
        if(read_flash(&device_info) == STATUS_OK)
        {
            printf("Currently device %s is connected to %s. \n",
            device_info.deviceid, device_info.hostname);
            printf("Press blue button in the next 5 seconds to change configuration\n");
        }
        else
        {
            printf("\nError reading from flash\n");
        }

        // Start menu option
        int menu_option, rc;

        // test timer and timeout
        status = tx_timer_create(&my_timer,"my_timer_name",
                                NULL, 0, 500, 0,
                                TX_AUTO_ACTIVATE);
        if(status != TX_SUCCESS)
        {
            printf("Error creating application timer.\n");
            return first_init;
        }
        
        while((tx_timer_info_get(&my_timer, &name, &active,&remaining_ticks, &reschedule_ticks, NULL) == TX_SUCCESS) 
                && remaining_ticks > 0)
        {
            if(get_user_button() == 1)
            {
                printf("Enter 0 - Continue\nEnter 1 - Erase/Reset flash\n");

                // call device config
                while (1)
                {
                    if (((rc = scanf("%d", &menu_option)) == 0))
                    {
                        scanf("%*[^\n]");
                        printf("Invalid input.\nEnter 0 - Continue\nEnter 1 - Erase/Reset flash\n"); 
                        fflush(stdin);
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
            // check if anything came into the register
        }
        printf("Continuing\n");

    }

    char hostname[MAX_HUB_HOSTNAME_SIZE] = "";
	char deviceid[MAX_HUB_DEVICE_ID_SIZE] = "";
	char sas[MAX_DEVICE_SAS_KEY_SIZE] = "";
#ifdef ENABLE_DPS
    char idscope[MAX_DPS_ID_SCOPE_SIZE] = "";
	char registrationid[MAX_DPS_REGISTRATION_ID_SIZE] = "";
#endif
#ifdef STM32L4
    char ssid[MAX_WIFI_SSID_SIZE] = "";
	char pswd[MAX_WIFI_PSWD_SIZE] = "";
#endif

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
        if (scanf("%s", deviceid) == 0) {
            /* Not valid input, flush stdin */
            fflush(stdin);
            continue;
        }

        printf("Please enter your IoTHub primary key: \n");
        if (scanf("%s", sas) == 0) {
            /* Not valid input, flush stdin */
            fflush(stdin);
            continue;
        }
#ifdef ENABLE_DPS
        printf("Please enter your DPS ID scope: \n");
        if (scanf("%s", idscope) == 0) {
            /* Not valid input, flush stdin */
            fflush(stdin);
            continue;
        }

        printf("Please enter your DPS registration ID: \n");
        if (scanf("%s", registrationid) == 0) {
            /* Not valid input, flush stdin */
            fflush(stdin);
            continue;
        }
#endif
#ifdef STM32L4
        printf("Please enter your WiFi SSID: \n");
        if (scanf("%s", ssid) == 0) {
            /* Not valid input, flush stdin */
            fflush(stdin);
            continue;
        }

        printf("Please enter your WiFi password: \n");
        if (scanf("%s", pswd) == 0) {
            /* Not valid input, flush stdin */
            fflush(stdin);
            continue;
        }
#endif

        printf("Please verify you have entered the correct configuration: \n\n"); 
        printf("hostname: %s\n", hostname); 
        printf("deviceid: %s\n", deviceid); 
        printf("sas: %s\n\n", sas);
#ifdef ENABLE_DPS
        printf("dps_id_scope: %s\n", idscope); 
        printf("registration id: %s\n\n", registrationid);
#endif

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
        strcpy(device_info.deviceid, deviceid);
        strcpy(device_info.sas, sas);
#ifdef ENABLE_DPS
        strcpy(device_info.idscope, idscope);
        strcpy(device_info.registrationid, registrationid);
#endif
#ifdef STM32L4
        strcpy(device_info.ssid, ssid);
        strcpy(device_info.pswd, pswd);
#endif

        if (save_to_flash(&device_info) == STATUS_OK)
        {
            printf("Successfully saved credentials to flash. \n\n");
            first_init = true;
            break;
        }
    }
    return first_init;
}