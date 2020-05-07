#include "networking.h"

#include "tx_api.h"
#include "nx_api.h"
#include "nx_secure_tls_api.h"
#include "nx_wifi.h"
#include "nxd_dns.h"

#include "wifi.h"

/* Define the default thread priority, stack size, etc. */
#define THREADX_PACKET_COUNT            (10)
#define THREADX_PACKET_SIZE             (1200)  /* Set the default value to 1200 since WIFI payload size (ES_WIFI_PAYLOAD_SIZE) is 1200.  */
#define THREADX_POOL_SIZE               ((THREADX_PACKET_SIZE + sizeof(NX_PACKET)) * THREADX_PACKET_COUNT)

static UCHAR threadx_pool_stack[THREADX_POOL_SIZE];

/* Define the prototypes for ThreadX.  */
NX_IP ip_0;
NX_PACKET_POOL main_pool;
NX_DNS dns_client;

//extern void platform_set_dns(NX_DNS *dns_client);
static UINT	dns_create(ULONG dns_server_address);

int threadx_net_init(void)
{
    UINT    status;
    UCHAR   dns_address_1[4]; 
    UCHAR   dns_address_2[4];
    
    /* Initialize the NetX system.  */
    nx_system_initialize();

    /* Create a packet pool.  */
    status = nx_packet_pool_create(&main_pool, "NetX Main Packet Pool", THREADX_PACKET_SIZE,
                                   threadx_pool_stack , THREADX_POOL_SIZE);
    
    /* Check for pool creation error.  */
    if (status)
    {
        printf("THREADX platform initialize fail: PACKET POOL CREATE FAIL.\r\n");
        return(status);
    }
    
    /* Create an IP instance.  */
     status = nx_ip_create(&ip_0, "NetX IP Instance 0", 0, 0,
                           &main_pool, NULL, NULL, 0, 0);

    /* Check for IP create errors.  */
    if (status)
    {
	    printf("THREADX platform initialize fail: IP CREATE FAIL.\r\n");
        return(status);
    }
    
    /* Initialize THREADX Wifi.  */
    status = nx_wifi_initialize(&ip_0, &main_pool);
    
    /* Check status.  */
    if (status)
    {
        printf("THREADX platform initialize fail: WIFI INITIALIZE FAIL.\r\n");
        return (status);
    }    	

    /* Get the DNS address.  */
    if (WIFI_GetDNS_Address(dns_address_1, dns_address_2) == WIFI_STATUS_OK)
    {
        /* Create DNS */
        status = dns_create(IP_ADDRESS(dns_address_1[0], dns_address_1[1], dns_address_1[2], dns_address_1[3]));
        if (status)
        {
            printf("THREADX platform initialize fail: DNS CREATE FAIL.\r\n");
            return(status);
        }
    }
    else
    {
        printf("THREADX platform initialize fail: DNS CREATE FAIL.\r\n");
        return(1);
    }
    
    /* Initialize TLS.  */
    nx_secure_tls_initialize();
    
    return 0;
}

static UINT	dns_create(ULONG dns_server_address)
{
    UINT    status; 
 
    /* Create a DNS instance for the Client.  Note this function will create
       the DNS Client packet pool for creating DNS message packets intended
       for querying its DNS server. */
    status = nx_dns_create(&dns_client, &ip_0, (UCHAR *)"DNS Client");
    if (status)
    {
        return(status);
    }

    /* Add an IPv4 server address to the Client list. */
    status = nx_dns_server_add(&dns_client, dns_server_address);
    if (status)
    {
        return(status);
    }
    
    return(0);
}
