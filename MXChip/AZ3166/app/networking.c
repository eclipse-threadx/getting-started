/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "tx_api.h"
#include "nx_api.h"
#include "nxd_dns.h"
#include "nx_secure_tls_api.h"
#include "wwd_management.h"
#include "wwd_wifi.h"
#include "wwd_buffer_interface.h"
#include "wwd_network.h"
#include "wwd_network_constants.h"

#include "networking.h"

/* Define the default thread priority, stack size, etc. The user can override this 
   via -D command line option or via project settings.  */

#ifndef SAMPLE_IP_STACK_SIZE
#define SAMPLE_IP_STACK_SIZE (2048)
#endif /* SAMPLE_IP_STACK_SIZE  */

#ifndef SAMPLE_TX_PACKET_COUNT
#define SAMPLE_TX_PACKET_COUNT (16)
#endif /* SAMPLE_TX_PACKET_COUNT  */

#ifndef SAMPLE_RX_PACKET_COUNT
#define SAMPLE_RX_PACKET_COUNT (8)
#endif /* SAMPLE_RX_PACKET_COUNT  */

#ifndef SAMPLE_PACKET_SIZE
#define SAMPLE_PACKET_SIZE (WICED_LINK_MTU)
#endif /* SAMPLE_PACKET_SIZE  */

#define SAMPLE_TX_POOL_SIZE ((SAMPLE_PACKET_SIZE + sizeof(NX_PACKET)) * SAMPLE_TX_PACKET_COUNT)
#define SAMPLE_RX_POOL_SIZE ((SAMPLE_PACKET_SIZE + sizeof(NX_PACKET)) * SAMPLE_RX_PACKET_COUNT)

#ifndef SAMPLE_ARP_CACHE_SIZE
#define SAMPLE_ARP_CACHE_SIZE 512
#endif /* SAMPLE_ARP_CACHE_SIZE  */

/* Define the address of SNTP Server. If not defined, use DNS module to resolve the host name THREADX_SNTP_SERVER_NAME.  */
/*
#define THREADX_SNTP_SERVER_ADDRESS     IP_ADDRESS(118, 190, 21, 209)
*/

#ifndef THREADX_SNTP_SERVER_NAME
#define THREADX_SNTP_SERVER_NAME "0.pool.ntp.org" /* SNTP Server.  */
#endif                                            /* THREADX_SNTP_SERVER_NAME */

#ifndef THREADX_SNTP_SYNC_MAX
#define THREADX_SNTP_SYNC_MAX 3
#endif /* THREADX_SNTP_SYNC_MAX */

#ifndef THREADX_SNTP_UPDATE_MAX
#define THREADX_SNTP_UPDATE_MAX 10
#endif /* THREADX_SNTP_UPDATE_MAX */

/* Default time. GMT: Monday, June 1, 2020 12:00:00 AM. Epoch timestamp: 1590969601.  */
#ifndef THREADX_SYSTEM_TIME
#define THREADX_SYSTEM_TIME 1590969601
#endif /* THREADX_SYSTEM_TIME  */

/* Seconds between Unix Epoch (1/1/1970) and NTP Epoch (1/1/1999) */
#define THREADX_UNIX_TO_NTP_EPOCH_SECOND 0x83AA7E80

/* Define the stack/cache for ThreadX.  */
static UCHAR sample_ip_stack[SAMPLE_IP_STACK_SIZE];
static UCHAR sample_tx_pool_stack[SAMPLE_TX_POOL_SIZE];
static UCHAR sample_rx_pool_stack[SAMPLE_RX_POOL_SIZE];
static UCHAR sample_arp_cache_area[SAMPLE_ARP_CACHE_SIZE];

/* Define the prototypes for ThreadX.  */
NX_PACKET_POOL                   nx_pool[2]; /* 0=TX, 1=RX. */
NX_IP                            nx_ip;
NX_DNS                           nx_dns_client;

/* System clock time for UTC.  */
ULONG                                   unix_time_base;

#ifndef SAMPLE_DHCP_DISABLE

#include <nxd_dhcp_client.h>
static NX_DHCP dhcp_client;
static void wait_dhcp(void);

#define SAMPLE_IPV4_ADDRESS IP_ADDRESS(0, 0, 0, 0)
#define SAMPLE_IPV4_MASK IP_ADDRESS(0, 0, 0, 0)

#else

#ifndef SAMPLE_IPV4_ADDRESS
//#define SAMPLE_IPV4_ADDRESS          IP_ADDRESS(192, 168, 100, 33)
#error "SYMBOL SAMPLE_IPV4_ADDRESS must be defined. This symbol specifies the IP address of device. "

#endif /* SAMPLE_IPV4_ADDRESS */
#ifndef SAMPLE_IPV4_MASK
//#define SAMPLE_IPV4_MASK             0xFFFFFF00UL
#error "SYMBOL SAMPLE_IPV4_MASK must be defined. This symbol specifies the IP address mask of device. "
#endif /* IPV4_MASK */

#ifndef SAMPLE_GATEWAY_ADDRESS
//#define SAMPLE_GATEWAY_ADDRESS       IP_ADDRESS(192, 168, 100, 1)
#error "SYMBOL SAMPLE_GATEWAY_ADDRESS must be defined. This symbol specifies the gateway address for routing. "
#endif /* SAMPLE_GATEWAY_ADDRESS */

#ifndef SAMPLE_DNS_SERVER_ADDRESS
//#define SAMPLE_DNS_SERVER_ADDRESS      IP_ADDRESS(192, 168, 100, 1)
#error "SYMBOL SAMPLE_DNS_SERVER_ADDRESS must be defined. This symbol specifies the dns server address for routing. "
#endif /* SAMPLE_DNS_SERVER_ADDRESS */

#endif /* SAMPLE_DHCP_DISABLE  */

static UINT dns_create();

extern void (*platform_driver_get())(NX_IP_DRIVER *);
UINT wifi_network_join(void *pools, CHAR *ssid, CHAR *password, wiced_security_t security, wiced_country_code_t country);

/* Include the demo. */
// extern VOID demo_entry(NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr, NX_DNS *dns_ptr, UINT (*unix_time_callback)(ULONG *unix_time));

int platform_init(CHAR *ssid, CHAR *password, wiced_security_t security, wiced_country_code_t country)

{

  UINT status;
  ULONG ip_address;
  ULONG network_mask;
  ULONG gateway_address;

  /* Initialize the NetX system.  */
  nx_system_initialize();

  // Create a packet pool for TX.
  status = nx_packet_pool_create(&nx_pool[0], "NetX Main TX Packet Pool", SAMPLE_PACKET_SIZE,
                                 sample_tx_pool_stack, SAMPLE_TX_POOL_SIZE);
  if (status != NX_SUCCESS)
  {
    printf("Sample platform initialize fail: PACKET POOL CREATE FAIL.");
    return status;
  }

  // Create a packet pool for RX.
  status = nx_packet_pool_create(&nx_pool[1], "NetX Main RX Packet Pool", SAMPLE_PACKET_SIZE,
                                 sample_rx_pool_stack, SAMPLE_RX_POOL_SIZE);
  if (status != NX_SUCCESS)
  {
    nx_packet_pool_delete(&nx_pool[0]);
    printf("Sample platform initialize fail: PACKET POOL CREATE FAIL.\r\n");
    return status;
  }

  // Join Wifi network.
  status = wifi_network_join(&nx_pool, ssid, password, security, country);
  if (status != NX_SUCCESS)
  {
    nx_packet_pool_delete(&nx_pool[0]);
    nx_packet_pool_delete(&nx_pool[1]);
    printf("Sample platform initialize fail: WIFI JOIN FAIL.\r\n");
    return status;
  }

  /* Create an IP instance for the DHCP Client. The rest of the DHCP Client set up is handled
       by the client thread entry function.  */
  status = nx_ip_create(&nx_ip, "NetX IP Instance 0", SAMPLE_IPV4_ADDRESS, SAMPLE_IPV4_MASK,
                        &nx_pool[0], platform_driver_get(), (UCHAR *)sample_ip_stack, SAMPLE_IP_STACK_SIZE, 1);
  if (status != NX_SUCCESS)
  {
    nx_packet_pool_delete(&nx_pool[0]);
    nx_packet_pool_delete(&nx_pool[1]);
    printf("Sample platform initialize fail: IP CREATE FAIL.\r\n");
    return status;
  }

  /* Enable ARP and supply ARP cache memory for IP Instance 0.  */
  status = nx_arp_enable(&nx_ip, (VOID *)sample_arp_cache_area, SAMPLE_ARP_CACHE_SIZE);
  if (status != NX_SUCCESS)
  {
    nx_ip_delete(&nx_ip);
    nx_packet_pool_delete(&nx_pool[0]);
    nx_packet_pool_delete(&nx_pool[1]);
    printf("Sample platform initialize fail: ARP ENABLE FAIL.\r\n");
    return status;
  }

  /* Enable ICMP traffic.  */
  status = nx_icmp_enable(&nx_ip);
  if (status != NX_SUCCESS)
  {
    nx_ip_delete(&nx_ip);
    nx_packet_pool_delete(&nx_pool[0]);
    nx_packet_pool_delete(&nx_pool[1]);
    printf("Sample platform initialize fail: ICMP ENABLE FAIL.\r\n");
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
    printf("Sample platform initialize fail: TCP ENABLE FAIL.\r\n");
    return status;
  }

  /* Enable UDP traffic.  */
  status = nx_udp_enable(&nx_ip);
  if (status != NX_SUCCESS)
  {
    nx_ip_delete(&nx_ip);
    nx_packet_pool_delete(&nx_pool[0]);
    nx_packet_pool_delete(&nx_pool[1]);
    printf("Sample platform initialize fail: UDP ENABLE FAIL.\r\n");
    return status;
  }

#ifndef SAMPLE_DHCP_DISABLE
  wait_dhcp();
#else
  nx_ip_gateway_address_set(&nx_ip, SAMPLE_GATEWAY_ADDRESS);
#endif /* SAMPLE_DHCP_DISABLE  */

  /* Get IP address and gateway address. */
  nx_ip_address_get(&nx_ip, &ip_address, &network_mask);
  nx_ip_gateway_address_get(&nx_ip, &gateway_address);

  /* Output IP address and gateway address. */
  /* LogInfo */
  printf("IP address: %lu.%lu.%lu.%lu\r\n",
         (ip_address >> 24),
         (ip_address >> 16 & 0xFF),
         (ip_address >> 8 & 0xFF),
         (ip_address & 0xFF));
  printf("Mask: %lu.%lu.%lu.%lu\r\n",
         (network_mask >> 24),
         (network_mask >> 16 & 0xFF),
         (network_mask >> 8 & 0xFF),
         (network_mask & 0xFF));
  printf("Gateway: %lu.%lu.%lu.%lu\r\n",
         (gateway_address >> 24),
         (gateway_address >> 16 & 0xFF),
         (gateway_address >> 8 & 0xFF),
         (gateway_address & 0xFF));

  /* Ceate dns.  */
  status = dns_create();
  if (status != NX_SUCCESS)
  {
    nx_dhcp_delete(&dhcp_client);
    nx_ip_delete(&nx_ip);
    nx_packet_pool_delete(&nx_pool[0]);
    nx_packet_pool_delete(&nx_pool[1]);
    printf("Sample platform initialize fail: %u\r\n", status);
    return status;
  }

  /* Initialize TLS.  */
  nx_secure_tls_initialize();

  /* Start demo.  */
  // demo_entry(&nx_ip, &nx_pool[0], &nx_dns_client, unix_time_get);

  return NX_SUCCESS;
}

void platform_deinit(void)
{

  /* Cleanup the resource.  */
  // threadx_azure_sdk_deinitialize();

  /* Cleanup TLS.  */
  tx_mutex_delete(&_nx_secure_tls_protection);

  /* Cleanup DNS.  */
  nx_dns_delete(&nx_dns_client);

  /* Cleanup DHCP.  */
  nx_dhcp_delete(&dhcp_client);

  /* Cleanup IP and packet pool.  */
  nx_ip_delete(&nx_ip);
  nx_packet_pool_delete(&nx_pool[0]);
  nx_packet_pool_delete(&nx_pool[1]);
}

#ifndef SAMPLE_DHCP_DISABLE
static void wait_dhcp(void)
{

  ULONG actual_status;

  /* LogInfo */
  printf("DHCP In Progress...\r\n");

  /* Create the DHCP instance.  */
  nx_dhcp_create(&dhcp_client, &nx_ip, "MXChip_AZ3166");

  /* Start the DHCP Client.  */
  nx_dhcp_start(&dhcp_client);

  /* Wait util address is solved. */
  nx_ip_status_check(&nx_ip, NX_IP_ADDRESS_RESOLVED, &actual_status, NX_WAIT_FOREVER);
}
#endif /* SAMPLE_DHCP_DISABLE  */

static UINT dns_create()
{

  UINT status;
  ULONG dns_server_address[3];
  UINT dns_server_address_size = 12;

  /* Create a DNS instance for the Client.  Note this function will create
       the DNS Client packet pool for creating DNS message packets intended
       for querying its DNS server. */
  status = nx_dns_create(&nx_dns_client, &nx_ip, (UCHAR *)"DNS Client");
  if (status)
  {
    return (status);
  }

  /* Is the DNS client configured for the host application to create the pecket pool? */
#ifdef NX_DNS_CLIENT_USER_CREATE_PACKET_POOL

  /* Yes, use the packet pool created above which has appropriate payload size
       for DNS messages. */
  status = nx_dns_packet_pool_set(&nx_dns_client, nx_ip.nx_ip_default_packet_pool);
  if (status)
  {
    nx_dns_delete(&nx_dns_client);
    return (status);
  }
#endif /* NX_DNS_CLIENT_USER_CREATE_PACKET_POOL */

#ifndef SAMPLE_DHCP_DISABLE
  /* Retrieve DNS server address.  */
  nx_dhcp_interface_user_option_retrieve(&dhcp_client, 0, NX_DHCP_OPTION_DNS_SVR, (UCHAR *)(dns_server_address), &dns_server_address_size);
#else
  dns_server_address[0] = SAMPLE_DNS_SERVER_ADDRESS;
#endif

  /* Add an IPv4 server address to the Client list. */
  status = nx_dns_server_add(&nx_dns_client, dns_server_address[0]);
  if (status)
  {
    nx_dns_delete(&nx_dns_client);
    return (status);
  }

  /* Output DNS Server address.  */
  printf("DNS Server address: %lu.%lu.%lu.%lu\r\n",
         (dns_server_address[0] >> 24),
         (dns_server_address[0] >> 16 & 0xFF),
         (dns_server_address[0] >> 8 & 0xFF),
         (dns_server_address[0] & 0xFF));

  return 0;
}

/* Get the network driver.  */
VOID(*platform_driver_get())
(NX_IP_DRIVER *)
{
	return (wiced_sta_netx_duo_driver_entry);
}

/* Join Network.  */
UINT wifi_network_join(void *pools, CHAR *ssid, CHAR *password, wiced_security_t security, wiced_country_code_t country)
{
	/* Set pools for wifi.   */
	wwd_buffer_init(pools);

	/* Set country.  */
	if (wwd_management_wifi_on(country) != WWD_SUCCESS)
	{
		printf("Failed to set WiFi Country!\r\n");
		return (NX_NOT_SUCCESSFUL);
	}

	wiced_ssid_t wiced_ssid = {0};
	wiced_ssid.length = strlen(ssid);
	memcpy(wiced_ssid.value, ssid, wiced_ssid.length);

	/* Attempt to join the Wi-Fi network.  */
	printf("Joining WiFi SSID: %s\r\n", ssid);
	while (wwd_wifi_join(&wiced_ssid, security, (uint8_t *)password, strlen(password), NULL, WWD_STA_INTERFACE) != WWD_SUCCESS)
	{
		printf("Failed to join: %s ... retrying...\r\n", ssid);
	}

	printf("Successfully joined WiFi SSID: %s.\r\n", ssid);

	return NX_SUCCESS;
}