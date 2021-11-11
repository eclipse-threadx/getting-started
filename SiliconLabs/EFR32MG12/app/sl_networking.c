/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "sl_networking.h"

#include "nx_api.h"
#include "nx_secure_tls_api.h"
#include "nxd_dhcp_client.h"
#include "nxd_dns.h"

#include "nx_sl_wfx_driver.h"

//#include "azure_config.h"

// Define the priority of the threads and stack size
#define NETX_IP_THREAD_PRIORITY 3
#define NETX_IP_STACK_SIZE      4096

#define NETX_PACKET_COUNT   60
#define NETX_PACKET_SIZE    1536
#define NETX_POOL_SIZE      ((NETX_PACKET_SIZE + sizeof(NX_PACKET)) * NETX_PACKET_COUNT)
#define NETX_ARP_CACHE_SIZE 512

#define NETX_IPV4_ADDRESS IP_ADDRESS(0, 0, 0, 0)
#define NETX_IPV4_MASK    IP_ADDRESS(255, 255, 255, 0)

static UCHAR netx_ip_stack[NETX_IP_STACK_SIZE];
static UCHAR netx_ip_pool[NETX_POOL_SIZE];
static UCHAR netx_arp_cache_area[NETX_ARP_CACHE_SIZE];
static NX_DHCP nx_dhcp_client;

// Declare network control blocks for network initialization
NX_IP nx_ip;
NX_PACKET_POOL nx_pool;
NX_DNS nx_dns_client;

// Declare wifi information
// static nx_sl_wfx_wifi_info_t wifi_info = {.ssid = WIFI_SSID, .password = WIFI_PASSWORD};
static nx_sl_wfx_wifi_info_t wifi_info;

static void print_address(CHAR* preable, ULONG address)
{
    printf("\t%s: %d.%d.%d.%d\r\n",
        preable,
        (uint8_t)(address >> 24),
        (uint8_t)(address >> 16 & 0xFF),
        (uint8_t)(address >> 8 & 0xFF),
        (uint8_t)(address & 0xFF));
}

static UINT dhcp_wait()
{
    UINT status;
    ULONG actual_status;
    ULONG ip_address;
    ULONG network_mask;
    ULONG gateway_address;

    // Wait until WF200 connected to access point
    while ((sl_wfx_context->state & SL_WFX_STA_INTERFACE_CONNECTED) == 0)
    {
        tx_thread_sleep(50);
    }
    printf("Initializing DHCP\r\n");

    // Create the DHCP instance
    status = nx_dhcp_create(&nx_dhcp_client, &nx_ip, "azure_iot");
    if (status != NX_SUCCESS)
    {
        printf("Error in creating DHCP instance\r\n");
        return status;
    }

    // Start the DHCP Client
    status = nx_dhcp_start(&nx_dhcp_client);
    if (status != NX_SUCCESS)
    {
        printf("Error in starting DHCP Client\r\n");
        return status;
    }

    // Wait until address is solved
    status = nx_ip_status_check(&nx_ip, NX_IP_ADDRESS_RESOLVED, &actual_status, NX_WAIT_FOREVER);
    if (status != NX_SUCCESS)
    {
        // DHCP Failed...  no IP address!
        printf("Can't resolve address\r\n");
        return status;
    }

    // Get IP address and gateway address
    nx_ip_address_get(&nx_ip, &ip_address, &network_mask);
    nx_ip_gateway_address_get(&nx_ip, &gateway_address);

    // Output IP address and gateway address
    print_address("IP address", ip_address);
    print_address("Mask", network_mask);
    print_address("Gateway", gateway_address);
    printf("SUCCESS: DHCP initialized\r\n\r\n");

    return status;
}

/******************************************************************************
 * Create DNS client.
 ******************************************************************************/
static UINT dns_create()
{
    UINT status;
    ULONG dns_server_address[3]  = {0};
    UINT dns_server_address_size = 12;

    printf("Initializing DNS client\r\n");

    // Create a DNS instance for the Client. Note this function will create
    // the DNS Client packet pool for creating DNS message packets intended
    // for querying its DNS server.
    status = nx_dns_create(&nx_dns_client, &nx_ip, (UCHAR*)"DNS Client");
    if (status != NX_SUCCESS)
    {
        return status;
    }

#ifdef NX_DNS_CLIENT_USER_CREATE_PACKET_POOL
    // Use the packet pool here
    status = nx_dns_packet_pool_set(&nx_dns_client, nx_ip.nx_ip_default_packet_pool);
    if (status != NX_SUCCESS)
    {
        nx_dns_delete(&nx_dns_client);
        return status;
    }
#endif

    // Retrieve DNS server address
    nx_dhcp_interface_user_option_retrieve(
        &nx_dhcp_client, 0, NX_DHCP_OPTION_DNS_SVR, (UCHAR*)dns_server_address, &dns_server_address_size);

    // Add an IPv4 server address to the Client list
    status = nx_dns_server_add(&nx_dns_client, dns_server_address[0]);
    if (status != NX_SUCCESS)
    {
        nx_dns_delete(&nx_dns_client);
        return status;
    }

    // Output DNS Server address
    print_address("DNS address", dns_server_address[0]);
    printf("SUCCESS: DNS client initialized\r\n\r\n");

    return NX_SUCCESS;
}

UINT network_init(CHAR* ssid, CHAR* password, WiFi_Mode mode)
{
    UINT status;

    // Stash WiFi credentials
    strncpy(wifi_info.ssid, ssid, sizeof(wifi_info.ssid));
    strncpy(wifi_info.password, password, sizeof(wifi_info.password));
    
    switch (mode)
    {
        case None:
            wifi_info.mode = WFM_SECURITY_MODE_OPEN;
            break;
        case WEP:
            wifi_info.mode = WFM_SECURITY_MODE_WEP;
            break;
        case WPA_PSK_TKIP:
            wifi_info.mode = WFM_SECURITY_MODE_WPA2_WPA1_PSK;
            break;
        case WPA2_PSK_AES:
        default:
            wifi_info.mode = WFM_SECURITY_MODE_WPA2_PSK;
            break;
    };

    // Initialize the NetX system
    nx_system_initialize();

    // Create a packet pool
    if ((status = nx_packet_pool_create(
                  &nx_pool, "NetX Packet Pool", NETX_PACKET_SIZE, netx_ip_pool, NETX_POOL_SIZE)))
    {
        printf("ERROR: nx_packet_pool_create (0x%08x)\r\n", status);
    }

    // Create an IP instance
    else if ((status = nx_ip_create(&nx_ip,
        "NetX IP Instance",
        IP_ADDRESS(0, 0, 0, 0),
        IP_ADDRESS(0, 0, 0, 0),
        &nx_pool,
        nx_sl_wfx_driver_entry,
        netx_ip_stack,
        NETX_IP_STACK_SIZE,
        NETX_IP_THREAD_PRIORITY)))
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




    // Set wifi network information
    nx_ip.nx_ip_interface[0].nx_interface_additional_link_info = (void*)&wifi_info;

    // Enable ARP and supply ARP cache memory
/*    status = nx_arp_enable(&nx_ip, (VOID*)netx_arp_cache_area, NETX_ARP_CACHE_SIZE);
    if (status != NX_SUCCESS)
    {
        nx_ip_delete(&nx_ip);
        nx_packet_pool_delete(&nx_pool);
        printf("THREADX platform initialize fail: ARP ENABLE FAIL.\r\n");
        return false;
    }

    // Enable TCP traffic
    status = nx_tcp_enable(&nx_ip);
    if (status != NX_SUCCESS)
    {
        nx_ip_delete(&nx_ip);
        nx_packet_pool_delete(&nx_pool);
        printf("THREADX platform initialize fail: TCP ENABLE FAIL.\r\n");
        return false;
    }

    // Enable UDP traffic
    status = nx_udp_enable(&nx_ip);
    if (status != NX_SUCCESS)
    {
        nx_ip_delete(&nx_ip);
        nx_packet_pool_delete(&nx_pool);
        printf("THREADX platform initialize fail: UDP ENABLE FAIL.\r\n");
        return false;
    }

    // Enable ICMP traffic
    status = nx_icmp_enable(&nx_ip);
    if (status != NX_SUCCESS)
    {
        nx_ip_delete(&nx_ip);
        nx_packet_pool_delete(&nx_pool);
        printf("THREADX platform initialize fail: ICMP ENABLE FAIL.\r\n");
        return false;
    }*/

    // Start DHCP to obtain IP address
    status = dhcp_wait();
    if (status != NX_SUCCESS)
    {
        nx_ip_delete(&nx_ip);
        nx_packet_pool_delete(&nx_pool);
        printf("THREADX platform initialize fail: DHCP FAIL.\r\n");
        return false;
    }

    // Create DNS
    status = dns_create();
    if (status != NX_SUCCESS)
    {
        nx_dhcp_delete(&nx_dhcp_client);
        nx_ip_delete(&nx_ip);
        nx_packet_pool_delete(&nx_pool);
        printf("THREADX platform initialize fail: DNS CREATE FAIL.\r\n");
        return false;
    }

    // Initialize TLS
    nx_secure_tls_initialize();

    return true;
}

UINT network_connect()
{
    UINT status;

    // :TODO: wifi connection stuff here, if any?

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

    return status;
}
