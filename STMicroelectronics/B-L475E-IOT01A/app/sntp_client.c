#include "sntp_client.h"

#include <time.h>

#include "nx_api.h"
#include "nxd_dns.h"
#include "nxd_sntp_client.h"

#include "networking.h"

#define SNTP_THREAD_STACK_SIZE	2048
#define SNTP_THREAD_PRIORITY	9

#define SNTP_SERVER				"time.google.com"

#define SNTP_UPDATE_EVENT		1
#define SNTP_NEW_TIME			2

// Seconds between Unix Epoch (1/1/1970) and NTP Epoch (1/1/1999) */
#define UNIX_TO_NTP_EPOCH_SECS	0x83AA7E80

static UCHAR sntp_thread_stack[SNTP_THREAD_STACK_SIZE];
static TX_THREAD mqtt_client_thread;

static NX_SNTP_CLIENT sntp_client;
static TX_EVENT_FLAGS_GROUP sntp_flags;

// Variable to keep track of time
static TX_MUTEX time_mutex;
static ULONG sntp_last_time = 1590969600;
static ULONG tx_last_ticks = 0;

UINT kiss_of_death_handler(NX_SNTP_CLIENT* client_ptr, UINT KOD_code);
UINT leap_second_handler(NX_SNTP_CLIENT* client_ptr, UINT leap_indicator);
void sntp_thread_entry(ULONG info);

UINT kiss_of_death_handler(NX_SNTP_CLIENT* client_ptr, UINT KOD_code)
{
    // Nothing here yet
    return NX_SUCCESS;
}

UINT leap_second_handler(NX_SNTP_CLIENT* client_ptr, UINT leap_indicator)
{
    // Nothing here yet
    return NX_SUCCESS;
}

static VOID time_update_callback(NX_SNTP_TIME_MESSAGE* time_update_ptr, NX_SNTP_TIME* local_time)
{
    // Set the update flag so we pick up the new time in the SNTP thread
    tx_event_flags_set(&sntp_flags, SNTP_UPDATE_EVENT, TX_OR);
}

static void set_sntp_time()
{
    ULONG seconds;
    ULONG milliseconds;
    UINT status;
    CHAR time_buffer[64];
    
    status = nx_sntp_client_get_local_time(&sntp_client, &seconds, &milliseconds, NX_NULL);
    if (status != NX_SUCCESS)
    {
        printf("Internal error with getting local time 0x%x\n", status);
        return;
    }
    
    tx_mutex_get(&time_mutex, TX_WAIT_FOREVER);
    
    // Stash the Unix and ThreadX times
    sntp_last_time = seconds - UNIX_TO_NTP_EPOCH_SECS;
    tx_last_ticks = tx_time_get();
    
    tx_mutex_put(&time_mutex);

    // Flag the sync was successful
    tx_event_flags_set(&sntp_flags, SNTP_NEW_TIME, TX_OR);
    
    nx_sntp_client_utility_display_date_time(&sntp_client, time_buffer, sizeof(time_buffer));
    printf("SNTP time update:\r\n\t%s\r\n", time_buffer);
}

void sntp_thread_entry(ULONG info)
{
    UINT status;
    UINT server_status;
    
    ULONG events = 0;
    ULONG sntp_address;
    
    printf("Running unicast SNTP client\r\n");

    status = nx_dns_host_by_name_get(&dns_client, (UCHAR*)SNTP_SERVER, &sntp_address, 5 * NX_IP_PERIODIC_RATE);
    if (status != NX_SUCCESS)
    {
        printf("Unable to resolve DNS for SNTP Server: %s\r\n", SNTP_SERVER);
        return;
    }
    
    status = nx_sntp_client_initialize_unicast(&sntp_client, sntp_address);
    if (status != NX_SUCCESS)
    {
        printf("Unable to initialize unicast SNTP client\r\n");
        return;
    }

    status = nx_sntp_client_set_local_time(&sntp_client, 0, 0);
    if (status != NX_SUCCESS) 
    {
        printf("Unable to set local time for SNTP client\r\n");
        return;
    }

    // Run Unicast client
    status = nx_sntp_client_run_unicast(&sntp_client);
    if (status != NX_SUCCESS) 
    {
        printf("Unable to start unicast SNTP client\r\n");
        return;
    }
    
    // Run initial sync, try 5 times
    for (int i = 0 ; i < 5 ; ++i)
    {
        status = nx_sntp_client_request_unicast_time(&sntp_client, 5 * NX_IP_PERIODIC_RATE);
        if (status == NX_SUCCESS)
        {
            set_sntp_time();

            break;
        }

        printf("Unable to sync SNTP time, try %d\r\n", i);
    }

    // Setup time update callback function
    nx_sntp_client_set_time_update_notify(&sntp_client, time_update_callback);
    
    while (true)
    {
        // Wait for an incoming SNTP message
        tx_event_flags_get(&sntp_flags, SNTP_UPDATE_EVENT, TX_OR_CLEAR, &events, TX_WAIT_FOREVER);

        if (events == SNTP_UPDATE_EVENT)
        {
            // Clear event flags
            events = 0;
            
            status = nx_sntp_client_receiving_updates(&sntp_client, &server_status);
            if ((status != NX_SUCCESS) || (server_status == NX_FALSE))
            {
                printf("Error receiving SNTP updates\r\n");
                continue;
            }

            set_sntp_time();
        }
    }
    
    nx_sntp_client_stop(&sntp_client);
    nx_sntp_client_delete(&sntp_client);

    return;
}

unsigned long sntp_get_time(void)
{
    tx_mutex_get(&time_mutex, TX_WAIT_FOREVER);

    // Calculate how many seconds have elapsed since the last sync
    ULONG tx_time_delta = (tx_time_get() - tx_last_ticks) / TX_TIMER_TICKS_PER_SECOND;

    // Add this to the last sync time to get the current time
    ULONG current_time = sntp_last_time + tx_time_delta;

    tx_mutex_put(&time_mutex);

    return current_time;
}

bool sntp_wait_for_sync()
{
    ULONG events = 0;
    tx_event_flags_get(&sntp_flags, SNTP_NEW_TIME, TX_OR_CLEAR, &events, TX_WAIT_FOREVER);
    return true;
}

bool sntp_start()
{
    UINT status;
    
    status = nx_sntp_client_create(&sntp_client, &ip_0, 0, &main_pool, leap_second_handler, kiss_of_death_handler, NULL);
    if (status != NX_SUCCESS) 
    {
        printf("SNTP client create failed\r\n");
        return false;
    }

    status = tx_event_flags_create(&sntp_flags, "SNTP event flags");
    if (status != TX_SUCCESS)
    {
        printf("Unable to create sntp event flags\r\n");
        return false;
    }	
    
    status = tx_mutex_create(&time_mutex, "time mutex", TX_NO_INHERIT);
    if (status != TX_SUCCESS)
    {
        printf("Unable to create time mutex\r\n");
        return false;
    }

    status = tx_thread_create(&mqtt_client_thread,
        "SNTP client thread",
        sntp_thread_entry, 
        (ULONG)NULL,
        &sntp_thread_stack, SNTP_THREAD_STACK_SIZE, 
        SNTP_THREAD_PRIORITY, SNTP_THREAD_PRIORITY,
        TX_NO_TIME_SLICE, TX_AUTO_START);
    if (status != TX_SUCCESS)
    {
        printf("Unable to create SNTP thread\r\n");
        return false;
    }

    return true;
}
