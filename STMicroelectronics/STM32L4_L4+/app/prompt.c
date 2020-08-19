#include "prompt.h"

bool serial_setup()
{
    DevConfig_IoT_Info_t device_info;
    bool first_init = false;

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
            first_init = true;
            break;
        }
    }
    return first_init;
}