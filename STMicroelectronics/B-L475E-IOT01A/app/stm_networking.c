/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "stm_networking.h"

#include "nx_api.h"
#include "nx_driver_stm32l4.h"
#include "nx_secure_tls_api.h"
#include "nxd_dns.h"

#include "wifi.h"

#include "sntp_client.h"

#define NETX_IP_STACK_SIZE 2048
#define NETX_PACKET_COUNT  20
#define NETX_PACKET_SIZE   1200 // Set the default value to 1200 since WIFI payload size (ES_WIFI_PAYLOAD_SIZE) is 1200
#define NETX_POOL_SIZE     ((NETX_PACKET_SIZE + sizeof(NX_PACKET)) * NETX_PACKET_COUNT)

#define NETX_IPV4_ADDRESS IP_ADDRESS(0, 0, 0, 0)
#define NETX_IPV4_MASK    IP_ADDRESS(255, 255, 255, 0)

static UCHAR netx_ip_stack[NETX_IP_STACK_SIZE];
static UCHAR netx_ip_pool[NETX_POOL_SIZE];

static CHAR* netx_ssid;
static CHAR* netx_password;
static WIFI_Ecn_t netx_mode;

NX_IP nx_ip;
NX_PACKET_POOL nx_pool;
NX_DNS nx_dns_client;

// WiFi firmware version required
static const UINT wifi_required_version[] = {3, 5, 2, 7};

static void print_address(CHAR* preable, uint8_t address[4])
{
    printf("\t%s: %d.%d.%d.%d\r\n", preable, address[0], address[1], address[2], address[3]);
}

static bool check_firmware_version(CHAR* data)
{
    UINT status = 0;
    UINT version[4];

    status = sscanf(data, "C%d.%d.%d.%d.STM", &version[0], &version[1], &version[2], &version[3]);

    if (status <= 0)
    {
        printf("ERROR: Unable to decode WiFi firmware\r\n");
        return false;
    }

    for (int i = 0; i < 4; ++i)
    {
        if (version[i] > wifi_required_version[i])
        {
            return true;
        }
        else if (version[i] < wifi_required_version[i])
        {
            printf("ERROR: WIFI FIRMWARE IS OUTDATED, PLEASE UPDATE TO AVOID CONNECTION ISSUES\r\n");
            return false;
        }
    }

    return true;
}

static UINT wifi_init()
{
    CHAR data[32];
    uint8_t mac[6];

    printf("\r\nInitializing WiFi\r\n");

    if (netx_ssid[0] == 0)
    {
        printf("ERROR: wifi_ssid is empty\r\n");
        return NX_NOT_SUCCESSFUL;
    }

    if (WIFI_Init() != WIFI_STATUS_OK)
    {
        printf("ERROR: WIFI_Init\r\n");
        return NX_NOT_SUCCESSFUL;
    }

    WIFI_GetModuleID(data);
    printf("\tModule: %s\r\n", data);

    WIFI_GetMAC_Address(mac);
    printf("\tMAC address: %02X:%02X:%02X:%02X:%02X:%02X\r\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    WIFI_GetModuleFwRevision(data);
    printf("\tFirmware revision: %s\r\n", data);

    if (check_firmware_version(data))
    {
        printf("SUCCESS: WiFi initialized\r\n");
    }

    return NX_SUCCESS;
}

static UINT dhcp_connect()
{
    UINT status;
    UCHAR ip_address[4];
    UCHAR ip_mask[4];
    UCHAR gateway_address[4];

    printf("\r\nInitializing DHCP\r\n");

    // Get WIFI information
    WIFI_GetIP_Address(ip_address);
    WIFI_GetIP_Mask(ip_mask);
    WIFI_GetGateway_Address(gateway_address);

    // Output IP address and gateway address
    print_address("IP address", ip_address);
    print_address("Mask", ip_mask);
    print_address("Gateway", gateway_address);

    // Set IP address
    if ((status = nx_ip_address_set(&nx_ip,
             IP_ADDRESS(ip_address[0], ip_address[1], ip_address[2], ip_address[3]),
             IP_ADDRESS(ip_mask[0], ip_mask[1], ip_mask[2], ip_mask[3]))))
    {
        printf("ERROR: nx_ip_address_set (0x%08x)\r\n", status);
        return status;
    }

    // Set gateway address
    if ((status = nx_ip_gateway_address_set(
             &nx_ip, IP_ADDRESS(gateway_address[0], gateway_address[1], gateway_address[2], gateway_address[3]))))
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
    UCHAR dns_address_1[4] = {0};
    UCHAR dns_address_2[4] = {0};

    printf("\r\nInitializing DNS client\r\n");

    if (WIFI_GetDNS_Address(dns_address_1, dns_address_2) != WIFI_STATUS_OK)
    {
        printf("ERROR: WIFI_GetDNS_Address\r\n");
        return NX_NOT_SUCCESSFUL;
    }

    // Output DNS Server address
    print_address("DNS address 1", dns_address_1);
    print_address("DNS address 2", dns_address_2);

    if ((status = nx_dns_server_remove_all(&nx_dns_client)))
    {
        printf("ERROR: nx_dns_server_remove_all (0x%08x)\r\n", status);
    }

    else if ((status = nx_dns_server_add(
                  &nx_dns_client, IP_ADDRESS(dns_address_1[0], dns_address_1[1], dns_address_1[2], dns_address_1[3]))))
    {
        printf("ERROR: nx_dns_server_add (0x%08x)\r\n", status);
    }

    else if ((status = nx_dns_server_add(
                  &nx_dns_client, IP_ADDRESS(dns_address_2[0], dns_address_2[1], dns_address_2[2], dns_address_2[3]))))
    {
        printf("ERROR: nx_dns_server_add (0x%08x)\r\n", status);
    }
    else
    {
        printf("SUCCESS: DNS client initialized\r\n");
    }

    return status;
}

UINT stm_network_init(CHAR* ssid, CHAR* password, WiFi_Mode mode)
{
    UINT status;

    // Stash WiFi credentials
    netx_ssid     = ssid;
    netx_password = password;

    switch (mode)
    {
        case None:
            netx_mode = WIFI_ECN_OPEN;
            break;
        case WEP:
            netx_mode = WIFI_ECN_WEP;
            break;
        case WPA_PSK_TKIP:
            netx_mode = WIFI_ECN_WPA_PSK;
            break;
        case WPA2_PSK_AES:
        default:
            netx_mode = WIFI_ECN_WPA2_PSK;
            break;
    };

    // Initialize the NetX system
    nx_system_initialize();

    // Initialize Wifi
    if ((status = wifi_init()))
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
                  nx_driver_stm32l4,
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
        printf("ERROR: nx_dns_create (0x%08x)\r\n", status);
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

UINT stm_network_connect()
{
    UINT status;
    int32_t wifiConnectCounter = 1;
    WIFI_Status_t join_result;

    // Check if Wifi is already connected
    if (WIFI_IsConnected() != WIFI_STATUS_OK)
    {
        printf("\r\nConnecting WiFi\r\n");

        // Connect to the specified SSID
        printf("\tConnecting to SSID '%s'\r\n", netx_ssid);
        do
        {
            printf("\tAttempt %ld...\r\n", wifiConnectCounter++);

            // Obtain the IP internal mutex before reconnecting WiFi
            tx_mutex_get(&(nx_ip.nx_ip_protection), TX_WAIT_FOREVER);
            join_result = WIFI_Connect(netx_ssid, netx_password, netx_mode);
            tx_mutex_put(&(nx_ip.nx_ip_protection));

            tx_thread_sleep(5 * TX_TIMER_TICKS_PER_SECOND);
        } while (join_result != NX_SUCCESS);

        printf("SUCCESS: WiFi connected\r\n");
    }

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
