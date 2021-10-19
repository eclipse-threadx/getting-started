/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "rx_networking.h"

#include "nx_api.h"
#include "nx_driver_rx65n_cloud_kit.h"
#include "nx_secure_tls_api.h"
#include "nxd_dns.h"

#include <r_wifi_sx_ulpgn_if.h>

#define THREADX_IP_STACK_SIZE 2048
#define THREADX_PACKET_COUNT  60
#define THREADX_PACKET_SIZE   1500
#define THREADX_POOL_SIZE     ((THREADX_PACKET_SIZE + sizeof(NX_PACKET)) * THREADX_PACKET_COUNT)

static UCHAR threadx_ip_stack[THREADX_IP_STACK_SIZE];
static UCHAR threadx_ip_pool[THREADX_POOL_SIZE];

NX_IP nx_ip;
NX_PACKET_POOL nx_pool;
NX_DNS nx_dns_client;

wifi_ip_configuration_t ip_cfg = {0};

// Print IPv4 address
static void print_address(CHAR* preable, uint32_t address)
{
    printf("\t%s: %d.%d.%d.%d\r\n",
        preable,
        address >> 24 & 0xFF,
        address >> 16 & 0xFF,
        address >> 8 & 0xFF,
        address & 0xFF);
}

static bool wifi_init(CHAR* ssid, CHAR* password, WiFi_Mode mode)
{
    wifi_security_t security_mode;

    switch (mode)
    {
        case None:
            security_mode = WIFI_SECURITY_OPEN;
            break;
        case WEP:
            security_mode = WIFI_SECURITY_WEP;
            break;
        case WPA_PSK_TKIP:
            security_mode = WIFI_SECURITY_WPA;
            break;
        case WPA2_PSK_AES:
        default:
            security_mode = WIFI_SECURITY_WPA2;
            break;
    };

    printf("Initializing WiFi\r\n");

    if (ssid[0] == '\0')
    {
        printf("ERROR: wifi_ssid is empty\r\n");
        return false;
    }

    if (R_WIFI_SX_ULPGN_Open() != WIFI_SUCCESS)
    {
        printf("ERROR: Failed to initialize WIFI module\r\n");
        return false;
    }

    // Connect to the specified SSID
    int32_t wifiConnectCounter = 1;
    printf("\tConnecting to SSID '%s'\r\n", ssid);
    while (R_WIFI_SX_ULPGN_Connect(ssid, password, security_mode, 1, &ip_cfg))
    {
        printf("\tWiFi is unable connect to '%s', attempt = %ld\r\n", ssid, wifiConnectCounter++);
        tx_thread_sleep(NX_IP_PERIODIC_RATE);
    }

    printf("SUCCESS: WiFi connected to %s\r\n\r\n", ssid);

    return true;
}

static UINT dns_create()
{
    UINT status;
    uint32_t dns_address_1;

    printf("Initializing DNS client\r\n");

    status = nx_dns_create(&nx_dns_client, &nx_ip, (UCHAR*)"DNS Client");
    if (status != NX_SUCCESS)
    {
        printf("ERROR: Failed to create DNS (0x%04x)\r\n", status);
        return status;
    }

    // Use the packet pool here
#ifdef NX_DNS_CLIENT_USER_CREATE_PACKET_POOL
    status = nx_dns_packet_pool_set(&nx_dns_client, nx_ip.nx_ip_default_packet_pool);
    if (status != NX_SUCCESS)
    {
        printf("ERROR: Failed to create DNS packet pool (%0x02)\r\n", status);
        nx_dns_delete(&nx_dns_client);
        return status;
    }
#endif

    if (R_WIFI_SX_ULPGN_GetDnsServerAddress(&dns_address_1) != WIFI_SUCCESS)
    {
        printf("ERROR: Failed to fetch Wifi DNS\r\n");
        nx_dns_delete(&nx_dns_client);
        return NX_NOT_SUCCESSFUL;
    }

    // Output DNS Server address
    print_address("DNS address", dns_address_1);

    // Add an IPv4 server address to the Client list.
    status = nx_dns_server_add(&nx_dns_client, dns_address_1);
    if (status != NX_SUCCESS)
    {
        printf("ERROR: Failed to add DNS server (0x%04x)\r\n", status);
        nx_dns_delete(&nx_dns_client);
        return status;
    }

    printf("SUCCESS: DNS client initialized\r\n\r\n");

    return NX_SUCCESS;
}

int rx_network_init(CHAR* ssid, CHAR* password, WiFi_Mode mode)
{
    UINT status;
    UCHAR ip_address[4];
    UCHAR ip_mask[4];
    UCHAR gateway_address[4];

    // Intialize Wifi
    if (!wifi_init(ssid, password, mode))
    {
        return NX_NOT_SUCCESSFUL;
    }

    printf("Initializing DHCP\r\n");

    R_WIFI_SX_ULPGN_GetIpAddress(&ip_cfg);

    // Output IP address and gateway address
    print_address("IP address", ip_cfg.ipaddress);
    print_address("Gateway", ip_cfg.gateway);

    printf("SUCCESS: DHCP initialized\r\n\r\n");

    // Initialize the NetX system
    nx_system_initialize();

    // Create a packet pool
    status =
        nx_packet_pool_create(&nx_pool, "NetX Packet Pool", THREADX_PACKET_SIZE, threadx_ip_pool, THREADX_POOL_SIZE);
    if (status != NX_SUCCESS)
    {
        printf("ERROR: Packet pool create fail.\r\n");
        return status;
    }

    // Create an IP instance
    status = nx_ip_create(&nx_ip,
        "NetX IP Instance 0",
        ip_cfg.ipaddress,
        ip_cfg.subnetmask,
        &nx_pool,
        nx_driver_rx65n_cloud_kit,
        (UCHAR*)threadx_ip_stack,
        THREADX_IP_STACK_SIZE,
        1);
    if (status != NX_SUCCESS)
    {
        nx_packet_pool_delete(&nx_pool);
        printf("ERROR: IP create fail.\r\n");
        return status;
    }

    // Set gateway address
    status = nx_ip_gateway_address_set(&nx_ip, ip_cfg.gateway);
    if (status != NX_SUCCESS)
    {
        nx_ip_delete(&nx_ip);
        nx_packet_pool_delete(&nx_pool);
        printf("THREADX platform initialize fail: Gateway set FAIL.\r\n");
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

    // Initialize TLS
    nx_secure_tls_initialize();

    // Create DNS
    status = dns_create();
    if (status != NX_SUCCESS)
    {
        nx_ip_delete(&nx_ip);
        nx_packet_pool_delete(&nx_pool);
        printf("ERROR: DNS create fail.\r\n");
        return status;
    }

    return NX_SUCCESS;
}
