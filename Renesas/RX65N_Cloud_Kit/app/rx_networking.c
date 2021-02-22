/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "rx_networking.h"

#include "nx_api.h"
#include "nx_secure_tls_api.h"
#include "nx_wifi.h"
#include "nxd_dns.h"

#include <r_wifi_sx_ulpgn_if.h>

#define THREADX_PACKET_COUNT 60
#define THREADX_PACKET_SIZE  1536 // Set the default value to 1200 since WIFI payload size (ES_WIFI_PAYLOAD_SIZE) is 1200
#define THREADX_POOL_SIZE    ((THREADX_PACKET_SIZE + sizeof(NX_PACKET)) * THREADX_PACKET_COUNT)

static UCHAR threadx_ip_pool[THREADX_POOL_SIZE];

NX_IP nx_ip;
NX_PACKET_POOL nx_pool;
NX_DNS nx_dns_client;

// WiFi firmware version required
// static const UINT wifi_required_version[] = {3, 5, 2, 5};

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

/*static void checkWifiVersion()
{
    UINT status = 0;
    UINT version[4];
    CHAR moduleinfo[32];

    WIFI_GetModuleID(moduleinfo);
    printf("\tModule: %s\r\n", moduleinfo);

    WIFI_GetModuleFwRevision(moduleinfo);
    printf("\tFirmware revision: %s\r\n", moduleinfo);

    status = sscanf(moduleinfo, "C%d.%d.%d.%d.STM", &version[0], &version[1], &version[2], &version[3]);

    if (status <= 0)
    {
        printf("WARNING: Unable to decode WiFi firmware\r\n");
        return;
    }

    for (int i = 0; i < 4; ++i)
    {
        if (version[i] > wifi_required_version[i])
        {
            break;
        }
        else if (version[i] < wifi_required_version[i])
        {
            printf("WARNING: The WiFi firmware is out of date\r\n");
            break;
        }
    }
}*/

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

/*static UINT dns_create()
{
    UINT status;
    UCHAR dns_address_1[4];
    UCHAR dns_address_2[4];

    printf("Initializing DNS client\r\n");

    status = nx_dns_create(&nx_dns_client, &nx_ip, (UCHAR*)"DNS Client");
    if (status != NX_SUCCESS)
    {
        printf("ERROR: Failed to create DNS (%0x02)\r\n", status);
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

    if (WIFI_GetDNS_Address(dns_address_1, dns_address_2) != WIFI_STATUS_OK)
    {
        printf("ERROR: Failed to fetch Wifi DNS\r\n");
        nx_dns_delete(&nx_dns_client);
        return NX_NOT_SUCCESSFUL;dns
    }

    // Add an IPv4 server address to the Client list.
    status = nx_dns_server_add(
        &nx_dns_client, IP_ADDRESS(dns_address_1[0], dns_address_1[1], dns_address_1[2], dns_address_1[3]));
    if (status != NX_SUCCESS)
    {
        printf("ERROR: Failed to add dns server (%0x02)\r\n", status);
        nx_dns_delete(&nx_dns_client);
        return status;
    }

    // Output DNS Server address
    print_address("DNS address", dns_address_1);

    printf("SUCCESS: DNS client initialized\r\n\r\n");

    return NX_SUCCESS;
}*/

// the RX driver doesn't surface a way to retreive the DNS servers from the DHCP connection
UINT _nxde_dns_host_by_name_get(
    NX_DNS* dns_ptr, UCHAR* host_name, NXD_ADDRESS* host_address_ptr, ULONG wait_option, UINT lookup_type)
{
    wifi_err_t wifi_err;
    uint32_t ip_addr;

    if (R_WIFI_SX_ULPGN_DnsQuery(host_name, &ip_addr) != WIFI_SUCCESS)
    {
        return NX_DNS_QUERY_FAILED;
    }

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

    // Create DNS
    /*    status = dns_create();
        if (status != NX_SUCCESS)
        {
            nx_ip_delete(&nx_ip);
            nx_packet_pool_delete(&nx_pool);
            printf("ERROR: DNS create fail.\r\n");
            return status;
        }*/

    return NX_SUCCESS;
}
