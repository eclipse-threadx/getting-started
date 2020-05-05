#include "network.h"

#include "nx_api.h"
#include "nx_secure_tls_api.h"
#include "nx_driver_imxrt1062.h"
#include "nxd_dhcp_client.h"
#include "nxd_dns.h"

#define THREADX_IP_STACK_SIZE 2048
#define THREADX_PACKET_COUNT 20
#define THREADX_PACKET_SIZE 1536
#define THREADX_POOL_SIZE ((THREADX_PACKET_SIZE + sizeof(NX_PACKET)) * THREADX_PACKET_COUNT)
#define THREADX_ARP_CACHE_SIZE 512

#define THREADX_IPV4_ADDRESS IP_ADDRESS(0, 0, 0, 0)
#define THREADX_IPV4_MASK IP_ADDRESS(0, 0, 0, 0)

/* Define the stack/cache for ThreadX.  */ 
static UCHAR threadx_ip_stack[THREADX_IP_STACK_SIZE];
static UCHAR threadx_pool_stack[THREADX_POOL_SIZE];
static UCHAR threadx_arp_cache_area[THREADX_ARP_CACHE_SIZE];

static NX_DHCP dhcp_client;

NX_PACKET_POOL pool_0;
NX_IP ip_0;
NX_DNS dns_client;

static UINT dhcp_wait()
{
	UINT status;
	ULONG actual_status;

	printf("DHCP In Progress...\r\n");

	// Create the DHCP instance.
	status = nx_dhcp_create(&dhcp_client, &ip_0, "dhcp_client");

	// Start the DHCP Client.
	status = nx_dhcp_start(&dhcp_client);

	// Wait until address is solved.
	status = nx_ip_status_check(&ip_0, NX_IP_ADDRESS_RESOLVED, &actual_status, NX_WAIT_FOREVER);
    
	if (status != NX_SUCCESS)
	{
		// DHCP Failed...  no IP address!
		printf("Can't resolve address\r\n");
	}
    
	return status;
}

static UINT dns_create()
{
	UINT status;
	ULONG dns_server_address[3];
	UINT dns_server_address_size = 12;

	/* Create a DNS instance for the Client.  Note this function will create
	   the DNS Client packet pool for creating DNS message packets intended
	   for querying its DNS server. */
	status = nx_dns_create(&dns_client, &ip_0, "DNS Client");
	if (status != NX_SUCCESS)
	{
		return status;
	}
	
	/* Is the DNS client configured for the host application to create the packet pool? */
#ifdef NX_DNS_CLIENT_USER_CREATE_PACKET_POOL   

	    /* Yes, use the packet pool created above which has appropriate payload size
	       for DNS messages. */
	status = nx_dns_packet_pool_set(&dns_client, ip_0.nx_ip_default_packet_pool);
	if (status != NX_SUCCESS)
	{
		nx_dns_delete(&dns_client);
		return (status);
	}
#endif /* NX_DNS_CLIENT_USER_CREATE_PACKET_POOL */  

	/* Retrieve DNS server address.  */
	nx_dhcp_interface_user_option_retrieve(&dhcp_client, 0, NX_DHCP_OPTION_DNS_SVR, (UCHAR *)(dns_server_address), &dns_server_address_size); 
    
	/* Add an IPv4 server address to the Client list. */
	status = nx_dns_server_add(&dns_client, dns_server_address[0]);
	if (status != NX_SUCCESS)
	{
		nx_dns_delete(&dns_client);
		return status;
	}
    
	/* Output DNS Server address.  */
    printf("DNS address: %d.%d.%d.%d\r\n",
		(dns_server_address[0] >> 24),
		(dns_server_address[0] >> 16 & 0xFF),
		(dns_server_address[0] >> 8 & 0xFF),
		(dns_server_address[0] & 0xFF));
   
	return NX_SUCCESS;
}

bool network_init()
{
    UINT status;
    ULONG ip_address;
    ULONG network_mask;
    ULONG gateway_address;

    /* Initialize the NetX system.  */
    nx_system_initialize();

    /* Create a packet pool.  */
    status = nx_packet_pool_create(&pool_0, "NetX Main Packet Pool", THREADX_PACKET_SIZE,
                                   threadx_pool_stack , THREADX_POOL_SIZE);
    
    /* Check for pool creation error.  */
    if (status != NX_SUCCESS)
    {
        printf("THREADX platform initialize fail: PACKET POOL CREATE FAIL.\r\n");
        return false;
    }

    /* Create an IP instance for the DHCP Client. The rest of the DHCP Client set up is handled
       by the client thread entry function.  */
    status = nx_ip_create(&ip_0, "NetX IP Instance 0", THREADX_IPV4_ADDRESS, THREADX_IPV4_MASK,
                           &pool_0, nx_driver_imx, (UCHAR*)threadx_ip_stack, THREADX_IP_STACK_SIZE, 1);

    /* Check for IP create errors.  */
    if (status != NX_SUCCESS)
    {
        nx_packet_pool_delete(&pool_0);
        printf("THREADX platform initialize fail: IP CREATE FAIL.\r\n");
        return false;
    }

    /* Enable ARP and supply ARP cache memory for IP Instance 0.  */
    status = nx_arp_enable(&ip_0, (VOID *)threadx_arp_cache_area, THREADX_ARP_CACHE_SIZE);

    /* Check for ARP enable errors.  */
    if (status != NX_SUCCESS)
    {
        nx_ip_delete(&ip_0);
        nx_packet_pool_delete(&pool_0);
        printf("THREADX platform initialize fail: ARP ENABLE FAIL.\r\n");
        return false;
    }

    /* Enable ICMP traffic.  */
    status = nx_icmp_enable(&ip_0);

    /* Check for ICMP enable errors.  */
    if (status != NX_SUCCESS)
    {
        nx_ip_delete(&ip_0);
        nx_packet_pool_delete(&pool_0);
        printf("THREADX platform initialize fail: ICMP ENABLE FAIL.\r\n");
        return false;
    }

    /* Enable TCP traffic.  */
    status = nx_tcp_enable(&ip_0);

    /* Check for TCP enable errors.  */
    if (status != NX_SUCCESS)
    {
        nx_ip_delete(&ip_0);
        nx_packet_pool_delete(&pool_0);
        printf("THREADX platform initialize fail: TCP ENABLE FAIL.\r\n");
        return false;
    }

    /* Enable UDP traffic.  */
    status = nx_udp_enable(&ip_0);

    /* Check for UDP enable errors.  */
    if (status != NX_SUCCESS)
    {
        nx_ip_delete(&ip_0);
        nx_packet_pool_delete(&pool_0);
        printf("THREADX platform initialize fail: UDP ENABLE FAIL.\r\n");
        return false;
    }

    status = dhcp_wait();

    /* Get IP address and gateway address. */
    nx_ip_address_get(&ip_0, &ip_address, &network_mask);
    nx_ip_gateway_address_get(&ip_0, &gateway_address);

    /* Output IP address and gateway address. */
    printf("IP address: %d.%d.%d.%d\r\n",
           (ip_address >> 24),
           (ip_address >> 16 & 0xFF),
           (ip_address >> 8 & 0xFF),
           (ip_address & 0xFF));
    printf("Mask: %d.%d.%d.%d\r\n",
           (network_mask >> 24),
           (network_mask >> 16 & 0xFF),
           (network_mask >> 8 & 0xFF),
           (network_mask & 0xFF));
    printf("Gateway: %d.%d.%d.%d\r\n",
           (gateway_address >> 24),
           (gateway_address >> 16 & 0xFF),
           (gateway_address >> 8 & 0xFF),
           (gateway_address & 0xFF));

    /* Create dns. */
    status = dns_create();

    // Check for DNS create errors.
    if (status != NX_SUCCESS)
    {
        nx_dhcp_delete(&dhcp_client);
        nx_ip_delete(&ip_0);
        nx_packet_pool_delete(&pool_0);
        printf("THREADX platform initialize fail: DNS CREATE FAIL.\r\n");
        return false;
    }
    
    /* Initialize TLS.  */
    nx_secure_tls_initialize();  
    
    return true;
}
