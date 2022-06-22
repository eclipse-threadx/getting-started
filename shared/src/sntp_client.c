/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "sntp_client.h"

#include <stdbool.h>
#include <stdint.h>

#include "nx_api.h"
#include "nxd_dns.h"
#include "nxd_sntp_client.h"

#include "networking.h"

#define SNTP_UPDATE_EVENT 1

// Time to wait for each server poll
#define SNTP_WAIT_TIME (10 * NX_IP_PERIODIC_RATE)

// Seconds between Unix Epoch (1/1/1970) and NTP Epoch (1/1/1999)
#define UNIX_TO_NTP_EPOCH_SECS 0x83AA7E80

static const char* SNTP_SERVER[] = {
    "0.pool.ntp.org",
    "1.pool.ntp.org",
    "2.pool.ntp.org",
    "3.pool.ntp.org",
};
static UINT sntp_server_count = 0;

static NX_SNTP_CLIENT sntp_client;

static TX_EVENT_FLAGS_GROUP sntp_flags;

// Variables to keep track of time
static ULONG sntp_last_time = 0;
static ULONG tx_last_ticks  = 0;

static VOID time_update_callback(NX_SNTP_TIME_MESSAGE* time_update_ptr, NX_SNTP_TIME* local_time)
{
    // Set the update flag so we pick up the new time in the SNTP thread
    tx_event_flags_set(&sntp_flags, SNTP_UPDATE_EVENT, TX_OR);
}

static void set_sntp_time()
{
    UINT status;
    ULONG seconds;
    ULONG milliseconds;
    CHAR time_buffer[64];

    status = nx_sntp_client_get_local_time(&sntp_client, &seconds, &milliseconds, NX_NULL);
    if (status != NX_SUCCESS)
    {
        printf("ERROR: Internal error with getting local time (0x%08x)\n", status);
        return;
    }

    // Stash the Unix and ThreadX times
    sntp_last_time = seconds - UNIX_TO_NTP_EPOCH_SECS;
    tx_last_ticks  = tx_time_get();

    nx_sntp_client_utility_display_date_time(&sntp_client, time_buffer, sizeof(time_buffer));

    printf("\tSNTP time update: %s\r\n", time_buffer);
    printf("SUCCESS: SNTP initialized\r\n");
}

static UINT sntp_client_run()
{
    UINT status;
    NXD_ADDRESS sntp_address;

    if (sntp_server_count > (sizeof(SNTP_SERVER) / sizeof(&SNTP_SERVER)))
    {
        // We rotated through all the servers, fail out.
        return NX_SNTP_SERVER_NOT_AVAILABLE;
    }

    printf("\tSNTP server %s\r\n", SNTP_SERVER[sntp_server_count]);

    // Stop the server in case it's already running
    nx_sntp_client_stop(&sntp_client);

    // Resolve DNS
    if ((status = nxd_dns_host_by_name_get(&nx_dns_client,
             (UCHAR*)SNTP_SERVER[sntp_server_count],
             &sntp_address,
             5 * NX_IP_PERIODIC_RATE,
             NX_IP_VERSION_V4)))
    {
        printf("ERROR: Unable to resolve SNTP IP %s (0x%08x)\r\n", SNTP_SERVER[sntp_server_count], status);
    }

    // Initialize the service
    else if ((status = nxd_sntp_client_initialize_unicast(&sntp_client, &sntp_address)))
    {
        printf("ERROR: Unable to initialize unicast SNTP client (0x%08x)\r\n", status);
    }

    // Run Unicast client
    else if ((status = nx_sntp_client_run_unicast(&sntp_client)))
    {
        printf("ERROR: Unable to start unicast SNTP client (0x%08x)\r\n", status);
    }

    // rotate to the next SNTP service
    sntp_server_count++;

    return status;
}

ULONG sntp_time_get()
{
    // Calculate how many seconds have elapsed since the last sync
    ULONG tx_time_delta = (tx_time_get() - tx_last_ticks) / TX_TIMER_TICKS_PER_SECOND;

    // Add this to the last sync time to get the current time
    ULONG sntp_time = sntp_last_time + tx_time_delta;

    return sntp_time;
}

UINT sntp_time(ULONG* unix_time)
{
    *unix_time = sntp_time_get();

    return NX_SUCCESS;
}

UINT sntp_init()
{
    UINT status;

    if ((status = tx_event_flags_create(&sntp_flags, "SNTP")))
    {
        printf("ERROR: Create SNTP event flags (0x%08x)\r\n", status);
    }

    else if ((status = nx_sntp_client_create(
                  &sntp_client, &nx_ip, 0, nx_ip.nx_ip_default_packet_pool, NX_NULL, NX_NULL, NULL)))
    {
        printf("ERROR: SNTP client create failed (0x%08x)\r\n", status);
    }

    else if ((status = nx_sntp_client_set_local_time(&sntp_client, 0, 0)))
    {
        printf("ERROR: Unable to set local time for SNTP client (0x%08x)\r\n", status);
        nx_sntp_client_delete(&sntp_client);
    }

    // Setup time update callback function
    else if ((status = nx_sntp_client_set_time_update_notify(&sntp_client, time_update_callback)))
    {
        printf("ERROR: nx_sntp_client_set_time_update_notify (0x%08x)\r\n", status);
        nx_sntp_client_delete(&sntp_client);
    }

    return status;
}

UINT sntp_sync()
{
    UINT status;
    UINT server_status;
    ULONG events = 0;

    printf("\r\nInitializing SNTP time sync\r\n");

    // Reset the server index so we start from the beginning
    sntp_server_count = 0;

    while (NX_TRUE)
    {
        // Run the client
        sntp_client_run();

        // Wait for new events
        events = 0;
        tx_event_flags_get(&sntp_flags, SNTP_UPDATE_EVENT, TX_OR_CLEAR, &events, SNTP_WAIT_TIME);

        if (events & SNTP_UPDATE_EVENT)
        {
            // Get time updates
            nx_sntp_client_receiving_updates(&sntp_client, &server_status);

            // New time, update our local time and we are done
            set_sntp_time();
            status = NX_SUCCESS;
            break;
        }

        // Otherwise we rotate around to the next server
    }

    nx_sntp_client_stop(&sntp_client);

    return status;
}
