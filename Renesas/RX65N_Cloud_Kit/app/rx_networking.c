/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "rx_networking.h"

#include "nx_api.h"
#include "nx_secure_tls_api.h"
#include "nx_wifi.h"
#include "nxd_dns.h"

#include <r_wifi_sx_ulpgn_if.h>

#define THREADX_PACKET_COUNT 60
#define THREADX_PACKET_SIZE  1536
#define THREADX_POOL_SIZE    ((THREADX_PACKET_SIZE + sizeof(NX_PACKET)) * THREADX_PACKET_COUNT)

static UCHAR threadx_ip_pool[THREADX_POOL_SIZE];

NX_IP nx_ip;
NX_PACKET_POOL nx_pool;
NX_DNS nx_dns_client;

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

    // checkWifiVersion();

    // Connect to the specified SSID
    int32_t wifiConnectCounter     = 1;
    wifi_ip_configuration_t ip_cfg = {0};
    printf("\tConnecting to SSID '%s'\r\n", ssid);
    while (R_WIFI_SX_ULPGN_Connect(ssid, password, security_mode, 1, &ip_cfg))
    {
        printf("\tWiFi is unable connect to '%s', attempt = %ld\r\n", ssid, wifiConnectCounter++);
        tx_thread_sleep(NX_IP_PERIODIC_RATE);
    }

    printf("SUCCESS: WiFi connected to %s\r\n\r\n", ssid);

    printf("Initializing DHCP\r\n");

    if (R_WIFI_SX_ULPGN_GetIpAddress(&ip_cfg) != WIFI_SUCCESS)
    {
        return false;
    }

    // Output IP address and gateway address
    print_address("IP address", ip_cfg.ipaddress);
    print_address("Gateway", ip_cfg.gateway);

    printf("SUCCESS: DHCP initialized\r\n\r\n");

    return true;
}

// the RX driver doesn't surface a way to retreive the DNS servers from the DHCP connection
UINT _nxde_dns_host_by_name_get(
    NX_DNS* dns_ptr, UCHAR* host_name, NXD_ADDRESS* host_address_ptr, ULONG wait_option, UINT lookup_type)
{
    wifi_err_t wifi_err;
    uint32_t ip_addr;
    int status;

    status = R_WIFI_SX_ULPGN_DnsQuery(host_name, &ip_addr);
    if (status != WIFI_SUCCESS)
    {
        return NX_DNS_QUERY_FAILED;
    }

    host_address_ptr->nxd_ip_version    = NX_IP_VERSION_V4;
    host_address_ptr->nxd_ip_address.v4 = ip_addr;

    return NX_SUCCESS;
}

int rx_network_init(CHAR* ssid, CHAR* password, WiFi_Mode mode)
{
    UINT status;

    // Intialize Wifi
    if (!wifi_init(ssid, password, mode))
    {
        return NX_NOT_SUCCESSFUL;
    }

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
    status = nx_ip_create(&nx_ip, "NetX IP Instance 0", 0, 0, &nx_pool, NULL, NULL, 0, 0);
    if (status != NX_SUCCESS)
    {
        nx_packet_pool_delete(&nx_pool);
        printf("ERROR: IP create fail.\r\n");
        return status;
    }

    // Initialize NetX WiFi
    status = nx_wifi_initialize(&nx_ip, &nx_pool);
    if (status != NX_SUCCESS)
    {
        nx_ip_delete(&nx_ip);
        nx_packet_pool_delete(&nx_pool);
        printf("ERROR: WiFi initialize fail.\r\n");
        return status;
    }

    // Initialize TLS
    nx_secure_tls_initialize();

    return NX_SUCCESS;
}
