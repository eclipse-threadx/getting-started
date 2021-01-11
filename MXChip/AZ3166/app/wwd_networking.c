/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "wwd_networking.h"

#include "nx_api.h"
#include "nx_secure_tls_api.h"
#include "nxd_dhcp_client.h"
#include "nxd_dns.h"

#include "stm32f4xx.h"

#include "wiced_sdk.h"

#define SAMPLE_IP_STACK_SIZE   2048
#define SAMPLE_TX_PACKET_COUNT 16
#define SAMPLE_RX_PACKET_COUNT 12
#define SAMPLE_PACKET_SIZE     (WICED_LINK_MTU)
#define SAMPLE_TX_POOL_SIZE    ((SAMPLE_PACKET_SIZE + sizeof(NX_PACKET)) * SAMPLE_TX_PACKET_COUNT)
#define SAMPLE_RX_POOL_SIZE    ((SAMPLE_PACKET_SIZE + sizeof(NX_PACKET)) * SAMPLE_RX_PACKET_COUNT)
#define SAMPLE_ARP_CACHE_SIZE  512

#define SAMPLE_IPV4_ADDRESS IP_ADDRESS(0, 0, 0, 0)
#define SAMPLE_IPV4_MASK    IP_ADDRESS(0, 0, 0, 0)

#define WIFI_COUNTRY WICED_COUNTRY_UNITED_STATES

/* Define the stack/cache for ThreadX.  */
static UCHAR sample_ip_stack[SAMPLE_IP_STACK_SIZE];
static UCHAR sample_tx_pool_stack[SAMPLE_TX_POOL_SIZE];
static UCHAR sample_rx_pool_stack[SAMPLE_RX_POOL_SIZE];
static UCHAR sample_arp_cache_area[SAMPLE_ARP_CACHE_SIZE];

/* Define the prototypes for ThreadX.  */
NX_PACKET_POOL nx_pool[2]; /* 0=TX, 1=RX. */
NX_IP nx_ip;
NX_DNS nx_dns_client;
NX_DHCP dhcp_client;

// Print IPv4 address
static void print_address(CHAR* preable, ULONG address)
{
    printf("\t%s: %d.%d.%d.%d\r\n",
        preable,
        (u_int8_t)(address >> 24),
        (u_int8_t)(address >> 16 & 0xFF),
        (u_int8_t)(address >> 8 & 0xFF),
        (u_int8_t)(address & 0xFF));
}

static void dhcp_wait(void)
{
    ULONG actual_status;
    ULONG ip_address;
    ULONG network_mask;
    ULONG gateway_address;

    printf("Initializing DHCP\r\n");

    /* Create the DHCP instance.  */
    nx_dhcp_create(&dhcp_client, &nx_ip, "MXChip_AZ3166");

    /* Start the DHCP Client.  */
    nx_dhcp_start(&dhcp_client);

    /* Wait until address is solved. */
    nx_ip_status_check(&nx_ip, NX_IP_ADDRESS_RESOLVED, &actual_status, NX_WAIT_FOREVER);

    /* Get IP address and gateway address. */
    nx_ip_address_get(&nx_ip, &ip_address, &network_mask);
    nx_ip_gateway_address_get(&nx_ip, &gateway_address);

    /* Output IP address and gateway address. */
    print_address("IP address", ip_address);
    print_address("Mask", network_mask);
    print_address("Gateway", gateway_address);

    printf("SUCCESS: DHCP initialized\r\n\r\n");
}

static UINT dns_create()
{
    UINT status;
    ULONG dns_server_address[3];
    UINT dns_server_address_size = 12;

    printf("Initializing DNS client\r\n");

    /* Create a DNS instance for the Client.  Note this function will create
         the DNS Client packet pool for creating DNS message packets intended
         for querying its DNS server. */
    status = nx_dns_create(&nx_dns_client, &nx_ip, (UCHAR*)"DNS Client");
    if (status)
    {
        return status;
    }

    /* Is the DNS client configured for the host application to create the packet pool? */
#ifdef NX_DNS_CLIENT_USER_CREATE_PACKET_POOL
    status = nx_dns_packet_pool_set(&nx_dns_client, nx_ip.nx_ip_default_packet_pool);
    if (status)
    {
        nx_dns_delete(&nx_dns_client);
        return status;
    }
#endif /* NX_DNS_CLIENT_USER_CREATE_PACKET_POOL */

    /* Retrieve DNS server address.  */
    nx_dhcp_interface_user_option_retrieve(
        &dhcp_client, 0, NX_DHCP_OPTION_DNS_SVR, (UCHAR*)(dns_server_address), &dns_server_address_size);

    /* Add an IPv4 server address to the Client list. */
    status = nx_dns_server_add(&nx_dns_client, dns_server_address[0]);
    if (status)
    {
        nx_dns_delete(&nx_dns_client);
        return status;
    }

    /* Output DNS Server address.  */
    print_address("DNS address", dns_server_address[0]);

    printf("SUCCESS: DNS client initialized\r\n\r\n");

    return NX_SUCCESS;
}

/* Join Network.  */
static UINT wifi_network_join(void* pools, CHAR* ssid, CHAR* password, WiFi_Mode mode)
{
    wiced_security_t wiced_security;

    printf("Initializing WiFi\r\n");

    if (ssid[0] == '\0')
    {
        printf("ERROR: wifi_ssid is empty\r\n");
        return NX_NOT_SUCCESSFUL;
    }

    switch (mode)
    {
        case None:
            wiced_security = WICED_SECURITY_OPEN;
            break;
        case WEP:
            wiced_security = WICED_SECURITY_WEP_SHARED;
            break;
        case WPA_PSK_TKIP:
            wiced_security = WICED_SECURITY_WPA_TKIP_PSK;
            break;
        case WPA2_PSK_AES:
            wiced_security = WICED_SECURITY_WPA2_AES_PSK;
            break;
    }

    /* Set pools for wifi. */
    wwd_buffer_init(pools);

    /* Set country. */
    if (wwd_management_wifi_on(WIFI_COUNTRY) != WWD_SUCCESS)
    {
        printf("Failed to set WiFi Country!\r\n");
        return NX_NOT_SUCCESSFUL;
    }

    wiced_ssid_t wiced_ssid = {0};
    wiced_ssid.length       = strlen(ssid);
    memcpy(wiced_ssid.value, ssid, wiced_ssid.length);

    /* Attempt to join the Wi-Fi network.  */
    printf("\tConnecting to SSID '%s'\r\n", ssid);
    while (wwd_wifi_join(&wiced_ssid, wiced_security, (uint8_t*)password, strlen(password), NULL, WWD_STA_INTERFACE) !=
           WWD_SUCCESS)
    {
        printf("Failed to join: %s ...retrying...\r\n", ssid);
        HAL_Delay(1000);
    }

    printf("SUCCESS: WiFi connected to %s\r\n\r\n", ssid);

    return NX_SUCCESS;
}

int platform_init(CHAR* ssid, CHAR* password, WiFi_Mode mode)
{
    UINT status;

    // Initialize the NetX system.
    nx_system_initialize();

    // Create a packet pool for TX.
    status = nx_packet_pool_create(
        &nx_pool[0], "NetX Main TX Packet Pool", SAMPLE_PACKET_SIZE, sample_tx_pool_stack, SAMPLE_TX_POOL_SIZE);
    if (status != NX_SUCCESS)
    {
        printf("Platform initialize fail: PACKET POOL CREATE FAIL.");
        return status;
    }

    // Create a packet pool for RX.
    status = nx_packet_pool_create(
        &nx_pool[1], "NetX Main RX Packet Pool", SAMPLE_PACKET_SIZE, sample_rx_pool_stack, SAMPLE_RX_POOL_SIZE);
    if (status != NX_SUCCESS)
    {
        nx_packet_pool_delete(&nx_pool[0]);
        printf("Platform initialize fail: PACKET POOL CREATE FAIL.\r\n");
        return status;
    }

    // Join Wifi network.
    status = wifi_network_join(&nx_pool, ssid, password, mode);
    if (status != NX_SUCCESS)
    {
        nx_packet_pool_delete(&nx_pool[0]);
        nx_packet_pool_delete(&nx_pool[1]);
        printf("Platform initialize fail: WiFi JOIN FAIL.\r\n");
        return status;
    }

    /* Create an IP instance for the DHCP Client. The rest of the DHCP Client set up is handled
         by the client thread entry function.  */
    status = nx_ip_create(&nx_ip,
        "NetX IP Instance 0",
        SAMPLE_IPV4_ADDRESS,
        SAMPLE_IPV4_MASK,
        &nx_pool[0],
        wiced_sta_netx_duo_driver_entry,
        (UCHAR*)sample_ip_stack,
        SAMPLE_IP_STACK_SIZE,
        1);
    if (status != NX_SUCCESS)
    {
        nx_packet_pool_delete(&nx_pool[0]);
        nx_packet_pool_delete(&nx_pool[1]);
        printf("Platform initialize fail: IP CREATE FAIL.\r\n");
        return status;
    }

    /* Enable ARP and supply ARP cache memory for IP Instance 0.  */
    status = nx_arp_enable(&nx_ip, (VOID*)sample_arp_cache_area, SAMPLE_ARP_CACHE_SIZE);
    if (status != NX_SUCCESS)
    {
        nx_ip_delete(&nx_ip);
        nx_packet_pool_delete(&nx_pool[0]);
        nx_packet_pool_delete(&nx_pool[1]);
        printf("Platform initialize fail: ARP ENABLE FAIL.\r\n");
        return status;
    }

    /* Enable ICMP traffic.  */
    status = nx_icmp_enable(&nx_ip);
    if (status != NX_SUCCESS)
    {
        nx_ip_delete(&nx_ip);
        nx_packet_pool_delete(&nx_pool[0]);
        nx_packet_pool_delete(&nx_pool[1]);
        printf("Platform initialize fail: ICMP ENABLE FAIL.\r\n");
        return status;
    }

    /* Enable TCP traffic.  */
    status = nx_tcp_enable(&nx_ip);
    if (status != NX_SUCCESS)
    {
        nx_ip_delete(&nx_ip);
        nx_packet_pool_delete(&nx_pool[0]);
        nx_packet_pool_delete(&nx_pool[1]);
        /* LogError */
        printf("Platform initialize fail: TCP ENABLE FAIL.\r\n");
        return status;
    }

    /* Enable UDP traffic.  */
    status = nx_udp_enable(&nx_ip);
    if (status != NX_SUCCESS)
    {
        nx_ip_delete(&nx_ip);
        nx_packet_pool_delete(&nx_pool[0]);
        nx_packet_pool_delete(&nx_pool[1]);
        printf("Platform initialize fail: UDP ENABLE FAIL.\r\n");
        return status;
    }

    dhcp_wait();

    /* Create DNS.  */
    status = dns_create();
    if (status != NX_SUCCESS)
    {
        nx_dhcp_delete(&dhcp_client);
        nx_ip_delete(&nx_ip);
        nx_packet_pool_delete(&nx_pool[0]);
        nx_packet_pool_delete(&nx_pool[1]);
        printf("Platform initialize fail: %u\r\n", status);
        return status;
    }

    /* Initialize TLS.  */
    nx_secure_tls_initialize();

    return NX_SUCCESS;
}
