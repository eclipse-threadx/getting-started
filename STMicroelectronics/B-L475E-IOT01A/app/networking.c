/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "networking.h"

#include "tx_api.h"
#include "nx_api.h"
#include "nx_secure_tls_api.h"
#include "nx_wifi.h"

#include "nxd_dns.h"

#include "wifi.h"

#define THREADX_PACKET_COUNT (20)
#define THREADX_PACKET_SIZE  (1200) // Set the default value to 1200 since WIFI payload size (ES_WIFI_PAYLOAD_SIZE) is 1200
#define THREADX_POOL_SIZE    ((THREADX_PACKET_SIZE + sizeof(NX_PACKET)) * THREADX_PACKET_COUNT)

#define WIFI_CONNECT_MAX_ATTEMPT_COUNT 5

static UCHAR threadx_ip_pool[THREADX_POOL_SIZE];

NX_IP ip_0;
NX_PACKET_POOL main_pool;
NX_DNS dns_client;

static void print_address(CHAR* preable, uint8_t address[4]);
static bool wifi_init(CHAR* ssid, CHAR* password, WiFi_Mode mode);
static UINT dns_create();

// Print IPv4 address
static void print_address(CHAR* preable, uint8_t address[4])
{
    printf("\t%s: %d.%d.%d.%d\r\n", preable, address[0], address[1], address[2], address[3]);
}

static bool wifi_init(CHAR* ssid, CHAR* password, WiFi_Mode mode)
{
    WIFI_Ecn_t security_mode;
    char moduleinfo[ES_WIFI_MAX_SSID_NAME_SIZE];

    switch (mode)
    {
        case None:
            security_mode = WIFI_ECN_OPEN;
            break;
        case WEP:
            security_mode = WIFI_ECN_WEP;
            break;
        case WPA2_Personal:
        default:
            security_mode = WIFI_ECN_WPA2_PSK;
            break;
    };

    printf("Initializing WiFi\r\n");

    if (ssid[0] == '\0')
    {
        printf("ERROR: wifi_ssid is empty\r\n");
        return false;
    }


    if (WIFI_Init() != WIFI_STATUS_OK)
    {
        printf("ERROR: Failed to initialize WIFI module\r\n");
        return false;
    }

    WIFI_GetModuleID(moduleinfo);
    printf("\tModule: %s\r\n", moduleinfo);

    WIFI_GetModuleFwRevision(moduleinfo);
    printf("\tFW Revision: %s\r\n", moduleinfo);
    
    // Connect to the specified SSID
    int32_t wifiConnectCounter = 1;
    printf("\tConnecting to SSID '%s'\r\n", ssid);
    while (WIFI_Connect(ssid, password, security_mode) != WIFI_STATUS_OK)
    {
        printf("\tWiFi is unable connect to '%s', attempt = %ld\r\n", ssid, wifiConnectCounter++);
        HAL_Delay(1000);

        // Max number of attempts
        if (wifiConnectCounter == WIFI_CONNECT_MAX_ATTEMPT_COUNT)
        {
            printf("ERROR: WiFi is unable to connected to the '%s'\r\n", ssid);
            return false;
        }
    }

    printf("SUCCESS: WiFi connected to %s\r\n\r\n", ssid);

    printf("Initializing DHCP\r\n");

    uint8_t ip_address[4];
    if (WIFI_GetIP_Address(ip_address) != WIFI_STATUS_OK)
    {
        return false;
    }

    uint8_t gateway_address[4];
    if (WIFI_GetGateway_Address(gateway_address) != WIFI_STATUS_OK)
    {
        return false;
    }

    // Output IP address and gateway address
    print_address("IP address", ip_address);
    print_address("Gateway", gateway_address);

    printf("SUCCESS: DHCP initialized\r\n\r\n");

    return true;
}

static UINT dns_create()
{
    UINT status;
    UCHAR dns_address_1[4];
    UCHAR dns_address_2[4];
 
    printf("Initializing DNS client\r\n");

    status = nx_dns_create(&dns_client, &ip_0, (UCHAR *)"DNS Client");
    if (status != NX_SUCCESS)
    {
        printf("ERROR: Failed to create DNS (%0x02)\r\n", status);
        return status;
    }

    // Use the packet pool here
#ifdef NX_DNS_CLIENT_USER_CREATE_PACKET_POOL 
    status = nx_dns_packet_pool_set(&dns_client, ip_0.nx_ip_default_packet_pool);
    if (status != NX_SUCCESS)
    {
        printf("ERROR: Failed to create DNS packet pool (%0x02)\r\n", status);
        nx_dns_delete(&dns_client);
        return status;
    }
#endif

    if (WIFI_GetDNS_Address(dns_address_1, dns_address_2) != WIFI_STATUS_OK)
    {
        printf("ERROR: Failed to fetch Wifi DNS\r\n");
        nx_dns_delete(&dns_client);
        return NX_NOT_SUCCESSFUL;
    }

    // Add an IPv4 server address to the Client list.
    status = nx_dns_server_add(&dns_client, IP_ADDRESS(dns_address_1[0], dns_address_1[1], dns_address_1[2], dns_address_1[3]));
    if (status != NX_SUCCESS)
    {
        printf("ERROR: Failed to add dns server (%0x02)\r\n", status);
        nx_dns_delete(&dns_client);
        return status;
    }
    
    // Output DNS Server address
    print_address("DNS address", dns_address_1);

    printf("SUCCESS: DNS client initialized\r\n\r\n");

    return NX_SUCCESS;
}

int stm32_network_init(CHAR* ssid, CHAR* password, WiFi_Mode mode)
{
    UINT status;

    // Intialize Wifi
    if (!wifi_init(ssid, password, mode))
    {
        return NX_NOT_SUCCESSFUL;
    }

    // Initialize the NetX system.
    nx_system_initialize();

    // Create a packet pool
    status = nx_packet_pool_create(&main_pool, "NetX Packet Pool",
        THREADX_PACKET_SIZE,
        threadx_ip_pool, THREADX_POOL_SIZE);
    if (status != NX_SUCCESS)
    {
        printf("ERROR: Packet pool create fail.\r\n");
        return status;
    }

    // Create an IP instance
     status = nx_ip_create(&ip_0, "NetX IP Instance 0",
        0, 0,
        &main_pool, NULL, 
        NULL, 0, 
        0);
    if (status != NX_SUCCESS)
    {
        nx_packet_pool_delete(&main_pool);
        printf("ERROR: IP create fail.\r\n");
        return status;
    }
    
    // Initialize NetX WiFi
    status = nx_wifi_initialize(&ip_0, &main_pool);
    if (status != NX_SUCCESS)
    {
        nx_ip_delete(&ip_0);
        nx_packet_pool_delete(&main_pool);
        printf("ERROR: WiFi initialize fail.\r\n");
        return status;
    }

    // Initialize TLS
    nx_secure_tls_initialize();

    // Create DNS.
    status = dns_create();
    if (status != NX_SUCCESS)
    {
        nx_ip_delete(&ip_0);
        nx_packet_pool_delete(&main_pool);
        printf("ERROR: DNS create fail.\r\n");
        return status;
    }

    return NX_SUCCESS;
}
