/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */
   
#include "sntp_client.h"

#include <time.h>

#include "nx_api.h"
#include "nxd_dns.h"
#include "nxd_sntp_client.h"

#include "networking.h"

#define SNTP_THREAD_STACK_SIZE  2048
#define SNTP_THREAD_PRIORITY    9

#define SNTP_SERVER             "pool.ntp.org"

#define SNTP_UPDATE_EVENT       1
#define SNTP_NEW_TIME           2

// Seconds between Unix Epoch (1/1/1970) and NTP Epoch (1/1/1999)
#define UNIX_TO_NTP_EPOCH_SECS  0x83AA7E80

static ULONG sntp_thread_stack[SNTP_THREAD_STACK_SIZE / sizeof(ULONG)];
static TX_THREAD mqtt_client_thread;

static NX_SNTP_CLIENT sntp_client;
static TX_EVENT_FLAGS_GROUP sntp_flags;

// Variables to keep track of time
static TX_MUTEX time_mutex;
static ULONG sntp_last_time = 0;
static ULONG tx_last_ticks = 0;
static bool first_sync = false;

int _gettimeofday(struct timeval *tp, void *tzvp);

static void print_address(CHAR* preable, NXD_ADDRESS address)
{
    if (address.nxd_ip_version == NX_IP_VERSION_V4)
    {
        ULONG ipv4 = address.nxd_ip_address.v4;
    
        printf("\t%s: %d.%d.%d.%d\r\n",
            preable,
            (u_int8_t)(ipv4 >> 24),
            (u_int8_t)(ipv4 >> 16 & 0xFF),
            (u_int8_t)(ipv4 >> 8 & 0xFF),
            (u_int8_t)(ipv4 & 0xFF));
    }
    else
    {
        printf("\tUnsupported address format\r\n");
    }
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
        printf("FAIL: Internal error with getting local time (0x%02x)\n", status);
        return;
    }

    tx_mutex_get(&time_mutex, TX_WAIT_FOREVER);

    // Stash the Unix and ThreadX times
    sntp_last_time = seconds - UNIX_TO_NTP_EPOCH_SECS;
    tx_last_ticks = tx_time_get();

    tx_mutex_put(&time_mutex);

    nx_sntp_client_utility_display_date_time(&sntp_client, time_buffer, sizeof(time_buffer));

    if (first_sync == false)
    {
        printf("\tSNTP time update: %s\r\n", time_buffer);
        printf("SUCCESS: SNTP initialized\r\n\r\n");
        first_sync = true;
    }
    else
    {
        printf("SNTP time update: %s\r\n", time_buffer);
    }

    // Flag the sync was successful
    tx_event_flags_set(&sntp_flags, SNTP_NEW_TIME, TX_OR);
}

static UINT sntp_client_run()
{
    UINT status;
    NXD_ADDRESS sntp_address;

    status = nxd_dns_host_by_name_get(&nx_dns_client, (UCHAR *)SNTP_SERVER, &sntp_address, 5 * NX_IP_PERIODIC_RATE, NX_IP_VERSION_V4);
    if (status != NX_SUCCESS)
    {
        printf("\tFAIL: Unable to resolve DNS for SNTP Server %s (0x%02x)\r\n", SNTP_SERVER, status);
        return status;
    }

    print_address("SNTP IP address", sntp_address);
    status = nxd_sntp_client_initialize_unicast(&sntp_client, &sntp_address);
    if (status != NX_SUCCESS)
    {
        printf("\tFAIL: Unable to initialize unicast SNTP client (0x%02x)\r\n", status);
        nx_sntp_client_delete(&sntp_client);
        return status;
    }

    // Run Unicast client
    status = nx_sntp_client_run_unicast(&sntp_client);
    if (status != NX_SUCCESS) 
    {
        printf("\tFAIL: Unable to start unicast SNTP client (0x%02x)\r\n", status);
        nx_sntp_client_stop(&sntp_client);
        nx_sntp_client_delete(&sntp_client);
        return status;
    }

    return NX_SUCCESS;
}

static void sntp_thread_entry(ULONG info)
{
    UINT status;
    UINT server_status;
    ULONG events = 0;

    printf("Initializing SNTP client\r\n");

    // status = nx_sntp_client_create(&sntp_client, &nx_ip, 0, &nx_pool, NX_NULL, NX_NULL, NULL);
    status = nx_sntp_client_create(&sntp_client, &nx_ip, 0, nx_ip.nx_ip_default_packet_pool, NX_NULL, NX_NULL, NULL);
    if (status != NX_SUCCESS) 
    {
        printf("\tFAIL: SNTP client create failed (0x%02x)\r\n", status);
        return;
    }

    status = nx_sntp_client_set_local_time(&sntp_client, 0, 0);
    if (status != NX_SUCCESS) 
    {
        printf("\tFAIL: Unable to set local time for SNTP client (0x%02x)\r\n", status);
        nx_sntp_client_delete(&sntp_client);
        return;
    }

    // Setup time update callback function
    status = nx_sntp_client_set_time_update_notify(&sntp_client, time_update_callback);
    if (status != NX_SUCCESS) 
    {
        printf("\tFAIL: Unable to set time update notify CB (0x%02x)\r\n", status);
        nx_sntp_client_delete(&sntp_client);
        return;
    }

    status = sntp_client_run();
    if (status != NX_SUCCESS)
    {
        printf("ERROR: Failed to run the sntp client\r\n");
        return;
    }

    while (true)
    {
        // Wait for an incoming SNTP message
        tx_event_flags_get(&sntp_flags, SNTP_UPDATE_EVENT, TX_OR_CLEAR, &events, 5 * NX_IP_PERIODIC_RATE);

        status = nx_sntp_client_receiving_updates(&sntp_client, &server_status);
        if (status != NX_SUCCESS)
        {
            printf("FAIL: SNTP receiving updates call failed (0x%02x)\r\n", status);
            continue;
        }

        if (server_status == NX_FALSE)
        {
            // Failed to read from server, restart the client
            printf("SNTP server timeout, restarting client\r\n");
            nx_sntp_client_stop(&sntp_client);
            sntp_client_run();
            continue;
        }

        if (events == SNTP_UPDATE_EVENT)
        {
            // New time, update our local time
            set_sntp_time();
            events = 0;
        }
    }

    nx_sntp_client_stop(&sntp_client);
    nx_sntp_client_delete(&sntp_client);

    return;
}

ULONG sntp_time_get()
{
    tx_mutex_get(&time_mutex, TX_WAIT_FOREVER);

    // Calculate how many seconds have elapsed since the last sync
    ULONG tx_time_delta = (tx_time_get() - tx_last_ticks) / TX_TIMER_TICKS_PER_SECOND;

    // Add this to the last sync time to get the current time
    ULONG sntp_time = sntp_last_time + tx_time_delta;

    tx_mutex_put(&time_mutex);

    return sntp_time;
}

UINT sntp_time(ULONG* unix_time)
{
    *unix_time = sntp_time_get();
    return NX_SUCCESS;
}

UINT sntp_sync_wait()
{
    ULONG events = 0;
    return tx_event_flags_get(&sntp_flags, SNTP_NEW_TIME, TX_OR_CLEAR, &events, TX_WAIT_FOREVER);
}

UINT sntp_start()
{
    UINT status;

    status = tx_event_flags_create(&sntp_flags, "SNTP event flags");
    if (status != TX_SUCCESS)
    {
        printf("FAIL: Unable to create SNTP event flags (0x%02x)\r\n", status);
        return false;
    }

    status = tx_mutex_create(&time_mutex, "time mutex", TX_NO_INHERIT);
    if (status != TX_SUCCESS)
    {
        printf("FAIL: Unable to create SNTP time mutex (0x%02x)\r\n", status);
        return status;
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
        printf("Unable to create SNTP thread (0x%02x)\r\n", status);
        return status;
    }

    return NX_SUCCESS;
}

// newlibc-nano stub
int _gettimeofday(struct timeval *tp, void *tzvp)
{
    tp->tv_sec = sntp_time_get();
    tp->tv_usec = 0;
    
    return 0;
}
