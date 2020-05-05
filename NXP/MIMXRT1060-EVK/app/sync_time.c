#include "sync_time.h"

#include <time.h>

#include "nx_api.h"
#include "nxd_dns.h"
#include "nxd_sntp_client.h"

#include "network.h"

//#define SNTP_SERVER "pool.ntp.org"
#define SNTP_SERVER "time.nist.gov"
#define SNTP_UPDATE_EVENT 1

// Seconds between Unix Epoch (1/1/1970) and NTP Epoch (1/1/1999) */
#define UNIX_TO_NTP_EPOCH_SECS 0x83AA7E80

static NX_SNTP_CLIENT sntp_client;

static ULONG sntp_base_time = 0;

static TX_EVENT_FLAGS_GROUP sntp_flags;

bool sntp_init()
{
	/* Create SNTP Client. */  
	UINT status = nx_sntp_client_create(&sntp_client, &ip_0, 0, &pool_0, NULL, NULL, NULL);
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
	
	return true;
}

void sntp_deinit()
{
	nx_sntp_client_delete(&sntp_client);
	tx_event_flags_delete(&sntp_flags);
}

bool sntp_sync()
{
	UINT status;
	UINT server_status;
	
	ULONG events = 0;
	ULONG sntp_address;

	printf("Run unicast SNTP client...\r\n");

	status = nx_dns_host_by_name_get(&dns_client, (UCHAR *)SNTP_SERVER, &sntp_address, 2000);
	if (status != NX_SUCCESS)
	{
		printf("Unable to resolve SNTP Server: %s\r\n", SNTP_SERVER);
		sntp_address = IP_ADDRESS(8, 8, 8, 8);
	}
	
	printf("SNTP IP address for %s : %d.%d.%d.%d\r\n", SNTP_SERVER,
		sntp_address >> 24,
		sntp_address >> 16 & 0xFF,
		sntp_address >> 8 & 0xFF,
		sntp_address & 0xFF);

	status = nx_sntp_client_initialize_unicast(&sntp_client, sntp_address);
	if (status != NX_SUCCESS)
	{
		printf("Unable to initialize unicast SNTP client\r\n");
		return false;
	}

	status = nx_sntp_client_set_local_time(&sntp_client, 0, 0);
	if (status != NX_SUCCESS) 
	{
		printf("Unable to set local time for SNTP client\r\n");
		return false;
	}

	// Run Unicast client
	status = nx_sntp_client_run_unicast(&sntp_client);
	if (status != NX_SUCCESS) 
	{
		printf("Unable to start unicast SNTP client\r\n");
		return false;
	}

	for (int i = 0; i < 100; i++)
	{
		tx_event_flags_get(&sntp_flags, SNTP_UPDATE_EVENT, TX_OR_CLEAR, &events, NX_IP_PERIODIC_RATE * 10);

		printf(".");

		if (events == SNTP_UPDATE_EVENT)
		{
			status = nx_sntp_client_receiving_updates(&sntp_client, &server_status);
			if (status != NX_SUCCESS)
			{
				printf("Unable to receive SNTP update\r\n");
				break;
			}

			if (server_status == NX_FALSE)
			{
				continue;
			}
		
			ULONG seconds;
			ULONG milliseconds;

			status = nx_sntp_client_get_local_time(&sntp_client, &seconds, &milliseconds, NX_NULL);
			if (status != NX_SUCCESS)
			{
				printf("Unable to get local time\r\n");
				break;
			}

			ULONG ticks = tx_time_get() / TX_TIMER_TICKS_PER_SECOND;

			sntp_base_time = seconds - ticks;
			// Convert to Unix epoch
			sntp_base_time -= UNIX_TO_NTP_EPOCH_SECS;

			nx_sntp_client_stop(&sntp_client);

			time_t rawtime = time(NULL);
			struct tm* time = gmtime(&rawtime);
			printf("SNTP Time sync successful\r\n");
			printf("\tDate: %0.4d/%0.2d/%0.2d\r\n", time->tm_year + 1900, time->tm_mon + 1, time->tm_mday);
			printf("\tTime: %0.2d:%0.2d.%0.2d\r\n", time->tm_hour, time->tm_min, time->tm_sec);
			return true;
		}
		
	}
	
	printf("No SNTP updates received\r\n");
	nx_sntp_client_stop(&sntp_client);

	return false;
}