/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "networking.h"

#include <stdint.h>

#include "nx_api.h"
#include "nx_secure_tls_api.h"
#include "nxd_dhcp_client.h"
#include "nxd_dns.h"

#include "sntp_client.h"

#define NETX_IP_STACK_SIZE  2048
#define NETX_PACKET_COUNT   60
#define NETX_PACKET_SIZE    1536
#define NETX_POOL_SIZE      ((NETX_PACKET_SIZE + sizeof(NX_PACKET)) * NETX_PACKET_COUNT)
#define NETX_ARP_CACHE_SIZE 512
#define NETX_DNS_COUNT      6

#define NETX_IPV4_ADDRESS IP_ADDRESS(0, 0, 0, 0)
#define NETX_IPV4_MASK    IP_ADDRESS(255, 255, 255, 0)

#define DHCP_WAIT_TIME_TICKS (30 * TX_TIMER_TICKS_PER_SECOND)

static UCHAR netx_ip_stack[NETX_IP_STACK_SIZE];
static UCHAR netx_ip_pool[NETX_POOL_SIZE];
static UCHAR netx_arp_cache_area[NETX_ARP_CACHE_SIZE];

static NX_DHCP nx_dhcp_client;

NX_IP nx_ip;
NX_PACKET_POOL nx_pool;
NX_DNS nx_dns_client;

// Print IPv4 address
static void print_address(CHAR* preable, ULONG address)
{
    printf("\t%s: %d.%d.%d.%d\r\n",
        preable,
        (uint8_t)(address >> 24),
        (uint8_t)(address >> 16 & 0xFF),
        (uint8_t)(address >> 8 & 0xFF),
        (uint8_t)(address & 0xFF));
}

static void print_mac()
{
    const ULONG lsw = nx_ip.nx_ip_gateway_interface->nx_interface_physical_address_lsw;
    const ULONG msw = nx_ip.nx_ip_gateway_interface->nx_interface_physical_address_msw;

    printf("\tMAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
        (uint8_t)(msw >> 8 & 0xFF),
        (uint8_t)(msw & 0xFF),
        (uint8_t)(lsw >> 24 & 0xFF),
        (uint8_t)(lsw >> 16 & 0xFF),
        (uint8_t)(lsw >> 8 & 0xFF),
        (uint8_t)(lsw & 0xFF));
}

static UINT dhcp_connect()
{
    UINT status;
    ULONG actual_status;
    ULONG ip_address;
    ULONG network_mask;
    ULONG gateway_address;

    printf("\r\nInitializing DHCP\r\n");

    if ((status = nx_dhcp_force_renew(&nx_dhcp_client)))
    {
        printf("ERROR: nx_dhcp_force_renew (0x%08x\r\n", status);
        return status;
    }

    // Wait until address is solved
    if ((status = nx_ip_status_check(&nx_ip, NX_IP_ADDRESS_RESOLVED, &actual_status, DHCP_WAIT_TIME_TICKS)))
    {
        // DHCP Failed...  no IP address!
        printf("ERROR: Can't resolve DHCP address (0x%08x\r\n", status);
        return status;
    }

    // Get IP address and gateway address
    nx_ip_address_get(&nx_ip, &ip_address, &network_mask);
    nx_ip_gateway_address_get(&nx_ip, &gateway_address);

    // Output MAC, IP address and gateway address
    print_mac();
    print_address("IP address", ip_address);
    print_address("Mask", network_mask);
    print_address("Gateway", gateway_address);

    printf("SUCCESS: DHCP initialized\r\n");

    return NX_SUCCESS;
}

static UINT dns_connect()
{
    UINT status;
    ULONG dns_server_address[NETX_DNS_COUNT] = {0};
    UINT dns_server_address_size             = sizeof(UINT) * NETX_DNS_COUNT;

    printf("\r\nInitializing DNS client\r\n");

    // Retrieve DNS server address
    if ((status = nx_dhcp_interface_user_option_retrieve(
             &nx_dhcp_client, 0, NX_DHCP_OPTION_DNS_SVR, (UCHAR*)dns_server_address, &dns_server_address_size)))
    {
        printf("ERROR: nx_dhcp_interface_user_option_retrieve (0x%08x)\r\n", status);
        return status;
    }

    if ((status = nx_dns_server_remove_all(&nx_dns_client)))
    {
        printf("ERROR: nx_dns_server_remove_all (0x%08x)\r\n", status);
        return status;
    }

    for (int i = 0; i < dns_server_address_size / sizeof(UINT); ++i)
    {
        print_address("DNS address", dns_server_address[i]);

        // Add an IPv4 server address to the Client list
        if ((status = nx_dns_server_add(&nx_dns_client, dns_server_address[i])))
        {
            printf("ERROR: nx_dns_server_add (0x%08x)\r\n", status);
            return status;
        }
    }

    printf("SUCCESS: DNS client initialized\r\n");

    return NX_SUCCESS;
}

UINT network_init(VOID (*ip_link_driver)(struct NX_IP_DRIVER_STRUCT*))
{
    UINT status;

    // Initialize the NetX system.
    nx_system_initialize();

    // Create a packet pool.
    if ((status = nx_packet_pool_create(&nx_pool, "NetX Packet Pool", NETX_PACKET_SIZE, netx_ip_pool, NETX_POOL_SIZE)))
    {
        printf("ERROR: nx_packet_pool_create (0x%08x)\r\n", status);
    }

    // Create an IP instance
    else if ((status = nx_ip_create(&nx_ip,
                  "NetX IP Instance 0",
                  NETX_IPV4_ADDRESS,
                  NETX_IPV4_MASK,
                  &nx_pool,
                  ip_link_driver,
                  netx_ip_stack,
                  NETX_IP_STACK_SIZE,
                  1)))
    {
        nx_packet_pool_delete(&nx_pool);
        printf("ERROR: nx_ip_create (0x%08x)\r\n", status);
    }

    // Enable ARP and supply ARP cache memory
    else if ((status = nx_arp_enable(&nx_ip, (VOID*)netx_arp_cache_area, NETX_ARP_CACHE_SIZE)))
    {
        nx_ip_delete(&nx_ip);
        nx_packet_pool_delete(&nx_pool);
        printf("ERROR: nx_arp_enable (0x%08x)\r\n", status);
    }

    // Enable TCP traffic
    else if ((status = nx_tcp_enable(&nx_ip)))
    {
        nx_ip_delete(&nx_ip);
        nx_packet_pool_delete(&nx_pool);
        printf("ERROR: nx_tcp_enable (0x%08x)\r\n", status);
        return status;
    }

    // Enable UDP traffic
    else if ((status = nx_udp_enable(&nx_ip)))
    {
        nx_ip_delete(&nx_ip);
        nx_packet_pool_delete(&nx_pool);
        printf("ERROR: nx_udp_enable (0x%08x)\r\n", status);
    }

    // Enable ICMP traffic
    else if ((status = nx_icmp_enable(&nx_ip)))
    {
        nx_ip_delete(&nx_ip);
        nx_packet_pool_delete(&nx_pool);
        printf("ERROR: nx_icmp_enable (0x%08x)\r\n", status);
    }

    // Create the DHCP instance.
    else if ((status = nx_dhcp_create(&nx_dhcp_client, &nx_ip, "azure_iot")))
    {
        nx_ip_delete(&nx_ip);
        nx_packet_pool_delete(&nx_pool);
        printf("ERROR: nx_dhcp_create (0x%08x)\r\n", status);
    }

    // Start the DHCP Client
    if ((status = nx_dhcp_start(&nx_dhcp_client)))
    {
        nx_dhcp_delete(&nx_dhcp_client);
        nx_ip_delete(&nx_ip);
        nx_packet_pool_delete(&nx_pool);
        printf("ERROR: nx_dhcp_start (0x%08x)\r\n", status);
    }

    // Create DNS
    else if ((status = nx_dns_create(&nx_dns_client, &nx_ip, (UCHAR*)"DNS Client")))
    {
        nx_dhcp_delete(&nx_dhcp_client);
        nx_ip_delete(&nx_ip);
        nx_packet_pool_delete(&nx_pool);
        printf("ERROR: nx_dns_create (0x%08x)\r\n", status);
    }

    // Use the packet pool here
#ifdef NX_DNS_CLIENT_USER_CREATE_PACKET_POOL
    else if ((status = nx_dns_packet_pool_set(&nx_dns_client, nx_ip.nx_ip_default_packet_pool)))
    {
        nx_dns_delete(&nx_dns_client);
        nx_dhcp_delete(&nx_dhcp_client);
        nx_ip_delete(&nx_ip);
        nx_packet_pool_delete(&nx_pool);
        printf("ERROR: nx_dns_packet_pool_set (%0x08)\r\n", status);
    }
#endif

    // Initialize the SNTP client
    else if ((status = sntp_init()))
    {
        printf("ERROR: Failed to init the SNTP client (0x%08x)\r\n", status);
    }

    // Initialize TLS
    else
    {
        nx_secure_tls_initialize();
    }

    return status;
}

UINT network_connect()
{
    UINT status;

    // Fetch IP details
    if ((status = dhcp_connect()))
    {
        printf("ERROR: dhcp_connect\r\n");
    }

    // Create DNS
    else if ((status = dns_connect()))
    {
        printf("ERROR: dns_connect\r\n");
    }

    // Wait for an SNTP sync
    else if ((status = sntp_sync()))
    {
        printf("ERROR: Failed to sync SNTP time (0x%08x)\r\n", status);
    }

    return status;
}
