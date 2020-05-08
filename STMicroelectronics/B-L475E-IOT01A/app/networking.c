#include "networking.h"

#include "tx_api.h"
#include "nx_api.h"
#include "nx_secure_tls_api.h"
#include "nx_wifi.h"

#include "nxd_dns.h"

#include "wifi.h"

#define THREADX_PACKET_COUNT (10)
#define THREADX_PACKET_SIZE  (1200)  /* Set the default value to 1200 since WIFI payload size (ES_WIFI_PAYLOAD_SIZE) is 1200.  */
#define THREADX_POOL_SIZE    ((THREADX_PACKET_SIZE + sizeof(NX_PACKET)) * THREADX_PACKET_COUNT)

static UCHAR threadx_pool_stack[THREADX_POOL_SIZE];

NX_IP ip_0;
NX_PACKET_POOL main_pool;
NX_DNS dns_client;

static UINT dns_create();

int threadx_net_init(void)
{
    UINT status;

    // Initialize the NetX system.
    nx_system_initialize();

    // Create a packet pool
    status = nx_packet_pool_create(&main_pool, "NetX Main Packet Pool", THREADX_PACKET_SIZE,
                                   threadx_pool_stack , THREADX_POOL_SIZE);
    if (status != NX_SUCCESS)
    {
        printf("THREADX platform initialize fail: PACKET POOL CREATE FAIL.\r\n");
        return status;
    }
    
    // Create an IP instance
     status = nx_ip_create(&ip_0, "NetX IP Instance 0", 0, 0,
                           &main_pool, NULL, NULL, 0, 0);
    if (status != NX_SUCCESS)
    {
        printf("THREADX platform initialize fail: IP CREATE FAIL.\r\n");
        return status;
    }
    
    status = nx_udp_enable(&ip_0);
    if (status != NX_SUCCESS)
    {
        nx_ip_delete(&ip_0);
        nx_packet_pool_delete(&main_pool);
        printf("THREADX platform initialize fail: UDP ENABLE FAIL.\r\n");
        return false;
    }

    // Initialize NetX WiFi
    status = nx_wifi_initialize(&ip_0, &main_pool);
    if (status != NX_SUCCESS)
    {
        printf("THREADX platform initialize fail: WIFI INITIALIZE FAIL.\r\n");
        return status;
    }

    // Get the DNS address.
    status = dns_create();
    if (status != NX_SUCCESS)
    {
        return status;
    }

    // Initialize TLS
    nx_secure_tls_initialize();

    return 0;
}

static UINT dns_create()
{
    UINT status;
    UCHAR dns_address_1[4];
    UCHAR dns_address_2[4];
 
    if (WIFI_GetDNS_Address(dns_address_1, dns_address_2) != WIFI_STATUS_OK)
    {
        printf("Failed to create DNS\r\n");
        return 1;
    }
    
    status = nx_dns_create(&dns_client, &ip_0, (UCHAR *)"DNS Client");
    if (status)
    {
        return(status);
    }

    // Use the packet pool here
    status = nx_dns_packet_pool_set(&dns_client, ip_0.nx_ip_default_packet_pool);
    if (status != NX_SUCCESS)
    {
        nx_dns_delete(&dns_client);
        return (status);
    }
    
    // Add an IPv4 server address to the Client list.
    status = nx_dns_server_add(&dns_client, IP_ADDRESS(dns_address_1[0], dns_address_1[1], dns_address_1[2], dns_address_1[3]));
    if (status)
    {
        return(status);
    }
    
    return(0);
}

static ULONG tx_last_ticks = 0;
unsigned long sntp_get_time(void)
{
    // Calculate how many seconds have elapsed since the last sync
    ULONG tx_time_delta = (tx_time_get() - tx_last_ticks) / TX_TIMER_TICKS_PER_SECOND;

    // Add this to the last sync time to get the current time
    ULONG current_time = 1590969600 + tx_time_delta;

    return current_time;
}
