/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "rx_networking.h"

#include "nx_api.h"
#include "nx_secure_tls_api.h"
#include "nxd_dns.h"

#include "sntp_client.h"

#include "nx_driver_rx65n_cloud_kit.h"

#include "r_wifi_sx_ulpgn_if.h"

#define NETX_IP_STACK_SIZE 2048
#define NETX_PACKET_COUNT  60
#define NETX_PACKET_SIZE   1500
#define NETX_POOL_SIZE     ((NETX_PACKET_SIZE + sizeof(NX_PACKET)) * NETX_PACKET_COUNT)
#define NETX_DNS_COUNT     3

#define NETX_IPV4_ADDRESS IP_ADDRESS(0, 0, 0, 0)
#define NETX_IPV4_MASK    IP_ADDRESS(255, 255, 255, 0)

static UCHAR netx_ip_stack[NETX_IP_STACK_SIZE];
static UCHAR netx_ip_pool[NETX_POOL_SIZE];

static CHAR* netx_ssid;
static CHAR* netx_password;
static wifi_security_t netx_mode;

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

static UINT wifi_init(CHAR* ssid, CHAR* password, WiFi_Mode mode)
{
    uint8_t mac[6];
    uint8_t ver;

    printf("\r\nInitializing WiFi\r\n");

    if (netx_ssid[0] == 0)
    {
        printf("ERROR: wifi_ssid is empty\r\n");
        return NX_NOT_SUCCESSFUL;
    }

    if (R_WIFI_SX_ULPGN_Open() != WIFI_SUCCESS)
    {
        printf("ERROR: Failed to initialize WIFI module\r\n");
        return NX_NOT_SUCCESSFUL;
    }

    R_WIFI_SX_ULPGN_GetMacAddress(mac);
    printf("\tMAC address: %02X:%02X:%02X:%02X:%02X:%02X\r\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    ver = R_WIFI_SX_ULPGN_GetVersion();
    printf("\tFirmware version %d.%2d\n", ((ver >> 16) & 0x0000FFFF), (ver & 0x0000FFFF));

    printf("SUCCESS: WiFi initialized\r\n");

    return NX_SUCCESS;
}

static UINT dhcp_connect()
{
    UINT status;

    printf("\r\nInitializing DHCP\r\n");

    R_WIFI_SX_ULPGN_GetIpAddress(&ip_cfg);

    // Output IP address and gateway address
    print_address("IP address", ip_cfg.ipaddress);
    print_address("Mask", ip_cfg.subnetmask);
    print_address("Gateway", ip_cfg.gateway);

    // Set IP address
    if ((status = nx_ip_address_set(&nx_ip, ip_cfg.ipaddress, ip_cfg.subnetmask)))
    {
        printf("ERROR: nx_ip_address_set (0x%08x)\r\n", status);
        return status;
    }

    // Set gateway address
    if ((status = nx_ip_gateway_address_set(&nx_ip, ip_cfg.gateway)))
    {
        printf("ERROR: nx_ip_gateway_address_set (0x%08x)\r\n", status);
        return status;
    }

    printf("SUCCESS: DHCP initialized\r\n");

    return NX_SUCCESS;
}

static UINT dns_connect()
{
    UINT status;
    uint32_t dns_address[NETX_DNS_COUNT];
    uint32_t dns_address_count = NETX_DNS_COUNT;

    printf("\r\nInitializing DNS client\r\n");

    if (R_WIFI_SX_ULPGN_GetDnsServerAddress(&dns_address, &dns_address_count) != WIFI_SUCCESS)
    {
        printf("ERROR: Failed to fetch Wifi DNS\r\n");
        return NX_NOT_SUCCESSFUL;
    }

    if ((status = nx_dns_server_remove_all(&nx_dns_client)))
    {
        printf("ERROR: nx_dns_server_remove_all (0x%08x)\r\n", status);
        return status;
    }

    for (int i = 0; i < dns_address_count; ++i)
    {
        print_address("DNS address", dns_address[i]);

        if ((status = nx_dns_server_add(&nx_dns_client, dns_address[i])))
        {
            printf("ERROR: nx_dns_server_add (0x%08x)\r\n", status);
            return status;
        }
    }

    printf("SUCCESS: DNS client initialized\r\n");

    return NX_SUCCESS;
}

UINT rx_network_init(CHAR* ssid, CHAR* password, WiFi_Mode mode)
{
    UINT status;

    // Stash WiFi credentials
    netx_ssid     = ssid;
    netx_password = password;

    switch (mode)
    {
        case None:
            netx_mode = WIFI_SECURITY_OPEN;
            break;
        case WEP:
            netx_mode = WIFI_SECURITY_WEP;
            break;
        case WPA_PSK_TKIP:
            netx_mode = WIFI_SECURITY_WPA;
            break;
        case WPA2_PSK_AES:
        default:
            netx_mode = WIFI_SECURITY_WPA2;
            break;
    };

    // Initialize the NetX system
    nx_system_initialize();

    // Intialize Wifi
    if (status = wifi_init(ssid, password, mode))
    {
        printf("ERROR: wifi_init (0x%08x)\r\n", status);
    }

    // Create a packet pool
    else if ((status = nx_packet_pool_create(
                  &nx_pool, "NetX Packet Pool", NETX_PACKET_SIZE, netx_ip_pool, NETX_POOL_SIZE)))
    {
        printf("ERROR: nx_packet_pool_create (0x%08x)\r\n", status);
    }

    // Create an IP instance
    else if ((status = nx_ip_create(&nx_ip,
                  "NetX IP Instance 0",
                  NETX_IPV4_ADDRESS,
                  NETX_IPV4_MASK,
                  &nx_pool,
                  nx_driver_rx65n_cloud_kit,
                  (UCHAR*)netx_ip_stack,
                  NETX_IP_STACK_SIZE,
                  1)))
    {
        nx_packet_pool_delete(&nx_pool);
        printf("ERROR: nx_ip_create (0x%08x)\r\n", status);
    }

    // Enable TCP traffic
    else if ((status = nx_tcp_enable(&nx_ip)))
    {
        nx_ip_delete(&nx_ip);
        nx_packet_pool_delete(&nx_pool);
        printf("ERROR: nx_tcp_enable (0x%08x)\r\n", status);
    }

    // Enable UDP traffic
    else if ((status = nx_udp_enable(&nx_ip)))
    {
        nx_ip_delete(&nx_ip);
        nx_packet_pool_delete(&nx_pool);
        printf("ERROR: nx_udp_enable (0x%08x)\r\n", status);
    }

    else if ((status = nx_dns_create(&nx_dns_client, &nx_ip, (UCHAR*)"DNS Client")))
    {
        nx_ip_delete(&nx_ip);
        nx_packet_pool_delete(&nx_pool);
        printf("ERROR: nx_dns_create (0x%04x)\r\n", status);
    }

    // Use the packet pool here
#ifdef NX_DNS_CLIENT_USER_CREATE_PACKET_POOL
    else if ((status = nx_dns_packet_pool_set(&nx_dns_client, nx_ip.nx_ip_default_packet_pool)))
    {
        nx_dns_delete(&nx_dns_client);
        nx_ip_delete(&nx_ip);
        nx_packet_pool_delete(&nx_pool);
        printf("ERROR: nx_dns_packet_pool_set (%0x08)\r\n", status);
    }
#endif

    // Initialize the SNTP client
    else if ((status = sntp_init()))
    {
        nx_dns_delete(&nx_dns_client);
        nx_ip_delete(&nx_ip);
        nx_packet_pool_delete(&nx_pool);
        printf("ERROR: Failed to init the SNTP client (0x%08x)\r\n", status);
    }

    // Initialize TLS
    else
    {
        nx_secure_tls_initialize();
    }

    return status;
}

UINT rx_network_connect()
{
    UINT status;
    int32_t wifiConnectCounter = 1;
    wifi_err_t join_result;

    // Check if Wifi is already connected
    printf("\r\nConnecting WiFi\r\n");

    // Connect to the specified SSID
    printf("\tConnecting to SSID '%s'\r\n", netx_ssid);
    do
    {
        printf("\tAttempt %ld...\r\n", wifiConnectCounter++);

        // Force a disconnect
        R_WIFI_SX_ULPGN_Disconnect();

        // Obtain the IP internal mutex before reconnecting WiFi
        join_result = R_WIFI_SX_ULPGN_Connect(netx_ssid, netx_password, netx_mode, 1, &ip_cfg);

        tx_thread_sleep(5 * TX_TIMER_TICKS_PER_SECOND);
    } while (join_result != WIFI_SUCCESS);

    printf("SUCCESS: WiFi connected\r\n");

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
