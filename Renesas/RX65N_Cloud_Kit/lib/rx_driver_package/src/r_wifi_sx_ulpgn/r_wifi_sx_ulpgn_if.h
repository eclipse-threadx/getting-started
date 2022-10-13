/**********************************************************************************************************************
 * DISCLAIMER
 * This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
 * other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
 * applicable laws, including copyright laws.
 * THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
 * THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
 * EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
 * SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO
 * THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
 * this software. By using this software, you agree to the additional terms and conditions found by accessing the
 * following link:
 * http://www.renesas.com/disclaimer
 *
 * Copyright (C) 2021 Renesas Electronics Corporation. All rights reserved.
 *********************************************************************************************************************/
/**********************************************************************************************************************
 * File Name    : r_wifi_sx_ulpgn_if.h
 * Version      : 1.0
 * Description  : API functions definition for SX ULPGN of RX65N.
 *********************************************************************************************************************/
/**********************************************************************************************************************
 * History : DD.MM.YYYY Version  Description
 *         : DD.MM.2021 1.00     First Release
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Includes   <System Includes> , "Project Includes"
 *********************************************************************************************************************/
#include "platform.h"
#include "r_wifi_sx_ulpgn_config.h"

/**********************************************************************************************************************
 Macro definitions
 *********************************************************************************************************************/
#ifndef R_WIFI_SX_ULPGN_CFG_IF_H
#define R_WIFI_SX_ULPGN_CFG_IF_H

/* Driver Version Number. */
#define WIFI_SX_ULPGN_CFG_VERSION_MAJOR           (1)
#define WIFI_SX_ULPGN_CFG_VERSION_MINOR           (14)

/* Configuration */
#define WIFI_SOCKET_IP_PROTOCOL_TCP               (0)       // Socket type = TCP
#define WIFI_SOCKET_IP_PROTOCOL_UDP               (1)       // Socket type = UDP
#define WIFI_SOCKET_IP_PROTOCOL_SSL               (2)       // Socket type = UDP
#define WIFI_SOCKET_IP_VERSION_4                  (4)       // IP version = IPv4

/**********************************************************************************************************************
 Global Typedef definitions
 *********************************************************************************************************************/
/*  WiFi API error code */
typedef enum
{
    WIFI_SUCCESS            = 0,      // success
    WIFI_ERR_PARAMETER      = -1,     // invalid parameter
    WIFI_ERR_ALREADY_OPEN   = -2,     // already WIFI module opened
    WIFI_ERR_NOT_OPEN       = -3,     // WIFI module is not opened
    WIFI_ERR_SERIAL_OPEN    = -4,     // serial open failed
    WIFI_ERR_MODULE_COM     = -5,     // cannot communicate WiFi module
    WIFI_ERR_NOT_CONNECT    = -6,     // not connect to access point
    WIFI_ERR_SOCKET_NUM     = -7,     // no available sockets
    WIFI_ERR_SOCKET_CREATE  = -8,     // create socket failed
    WIFI_ERR_CHANGE_SOCKET  = -9,     // cannot change socket
    WIFI_ERR_SOCKET_CONNECT = -10,    // cannot connect socket
    WIFI_ERR_BYTEQ_OPEN     = -11,    // cannot assigned BYTEQ
    WIFI_ERR_SOCKET_TIMEOUT = -12,    // socket timeout
    WIFI_ERR_TAKE_MUTEX     = -13     // cannot take mutex
} wifi_err_t;

/* Security type */
typedef enum
{
    WIFI_SECURITY_OPEN = 0,           // Open
    WIFI_SECURITY_WEP,                // WEP
    WIFI_SECURITY_WPA,                // WPA
    WIFI_SECURITY_WPA2,               // WPA2
    WIFI_SECURITY_UNDEFINED           // Undefined
} wifi_security_t;

/* Query current socket status */
typedef enum
{
    ULPGN_SOCKET_STATUS_CLOSED = 0,   // "CLOSED"
    ULPGN_SOCKET_STATUS_SOCKET,       // "SOCKET"
    ULPGN_SOCKET_STATUS_BOUND,        // "BOUND"
    ULPGN_SOCKET_STATUS_LISTEN,       // "LISTEN"
    ULPGN_SOCKET_STATUS_CONNECTED,    // "CONNECTED"
    ULPGN_SOCKET_STATUS_BROKEN,       // "BROKEN"
    ULPGN_SOCKET_STATUS_MAX           // Stopper
} sx_ulpgn_socket_status_t;

/* Error event for user callback */
typedef enum
{
    WIFI_EVENT_WIFI_REBOOT = 0,       // reboot WIFI
    WIFI_EVENT_WIFI_DISCONNECT,       // disconnected WIFI
    WIFI_EVENT_SERIAL_OVF_ERR,        // serial : overflow error
    WIFI_EVENT_SERIAL_FLM_ERR,        // serial : flaming error
    WIFI_EVENT_SERIAL_RXQ_OVF_ERR,    // serial : receiving queue overflow
    WIFI_EVENT_RCV_TASK_RXB_OVF_ERR,  // receiving task : receive buffer overflow
    WIFI_EVENT_SOCKET_CLOSED,         // socket is closed
    WIFI_EVENT_SOCKET_RXQ_OVF_ERR     // socket : receiving queue overflow
} wifi_err_event_enum_t;

typedef struct
{
    wifi_err_event_enum_t event;      // Error event
    uint8_t socket_number;            // Socket number
} wifi_err_event_t;

/* AP scan result */
typedef struct
{
    uint8_t  ssid[33];                // SSID
    uint8_t  bssid[6];                // BSSID
    wifi_security_t security;         // kinds of security
    int8_t   channel;                 // Channel
    int8_t   rssi;                    // RSSI
    uint8_t  hidden;                  // Hidden channel
} wifi_scan_result_t;

/* IP configurations */
typedef struct
{
    uint32_t ipaddress;               // IP address
    uint32_t subnetmask;              // subnet mask
    uint32_t gateway;                 // gateway
} wifi_ip_configuration_t;

/* Certificate information */
typedef struct {
    uint8_t  num_of_files;            // certificate number
    struct {
        uint8_t  file_name[20];       // certificate file name
    } cert[10];
} wifi_certificate_infomation_t;

/**********************************************************************************************************************
 External global variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Exported global functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_Open
 * Description  : Open WIFI Module.
 * Arguments    : none.
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_TAKE_MUTEX
 *                WIFI_ERR_SERIAL_OPEN
 *                WIFI_ERR_SOCKET_BYTEQ
 *                WIFI_ERR_ALREADY_OPEN
 *                WIFI_ERR_MODULE_COM
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_Open (void);

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_Close
 * Description  : Close WIFI Module.
 * Arguments    : none.
 * Return Value : WIFI_SUCCESS
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_Close (void);

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_SetDnsServerAddress
 * Description  : Set DNS Server Address.
 * Arguments    : dns_address1 - First DNS address
 *                dns_address2 - Second DNS address
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_NOT_OPEN
 *                WIFI_ERR_TAKE_MUTEX
 *                WIFI_ERR_MODULE_COM
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_SetDnsServerAddress (uint32_t dnsaddress1, uint32_t dnsaddress2);

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_Scan
 * Description  : Scan Access points.
 * Arguments    : ap_results
 *                max_networks
 *                exist_ap_list
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_NOT_OPEN
 *                WIFI_ERR_PARAMETER
 *                WIFI_ERR_MODULE_COM
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_Scan (wifi_scan_result_t * ap_results, uint32_t max_networks, uint32_t * exist_ap_count);

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_Connect
 * Description  : Connect to Access Point.
 * Arguments    : ssid
 *                pass
 *                security
 *                dhcp_enable
 *                ip_config
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_NOT_OPEN
 *                WIFI_ERR_PARAMETER
 *                WIFI_ERR_TAKE_MUTEX
 *                WIFI_ERR_MODULE_COM
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_Connect (const uint8_t * ssid, const uint8_t * pass, uint32_t security,
        uint8_t dhcp_enable, wifi_ip_configuration_t * ipconfig);

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_Disconnect
 * Description  : Disconnect from Access Point.
 * Arguments    : none.
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_NOT_OPEN
 *                WIFI_ERR_TAKE_MUTEX
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_Disconnect (void);

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_IsConnected
 * Description  : Check connected access point.
 * Arguments    : none.
 * Return Value : 0  - connected
 *                -1 - not connected
 *********************************************************************************************************************/
int32_t    R_WIFI_SX_ULPGN_IsConnected (void);

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_GetMacAddress
 * Description  : Get WIFI module MAC Address.
 * Arguments    : mac_address.
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_NOT_OPEN
 *                WIFI_ERR_TAKE_MUTEX
 *                WIFI_ERR_PARAMETER
 *                WIFI_ERR_MODULE_COM
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_GetMacAddress (uint8_t * mac_address);

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_GetIpAddress
 * Description  : Get IP Address.
 * Arguments    : ip_config.
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_NOT_OPEN
 *                WIFI_ERR_TAKE_MUTEX
 *                WIFI_ERR_PARAMETER
 *                WIFI_ERR_MODULE_COM
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_GetIpAddress (wifi_ip_configuration_t * ip_config);

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_CreateSocket
 * Description  : Create TCP socket.
 * Arguments    : type
 *                ip_version
 * Return Value : Positive number - created socket number
 *                WIFI_ERR_PARAMETER
 *                WIFI_ERR_NOT_CONNECT
 *                WIFI_ERR_SOCKET_CREATE
 *********************************************************************************************************************/
int32_t R_WIFI_SX_ULPGN_CreateSocket (uint32_t type, uint32_t ip_version);

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_ConnectSocket
 * Description  : Open client mode TCP/UDP socket.
 * Arguments    : socket_number
 *                ip_address
 *                port
 *                destination
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_PARAMETER
 *                WIFI_ERR_SOCKET_NUM
 *                WIFI_ERR_TAKE_MUTEX
 *                WIFI_ERR_MODULE_COM
 *                WIFI_ERR_NOT_CONNECT
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_ConnectSocket (uint8_t socket_number, uint32_t ip_address, uint16_t port, char * destination);

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_SendSocket
 * Description  : Send data on connecting socket.
 * Arguments    : socket_number
 *                data
 *                length
 *                timeout_ms
 * Return Value : Positive number - number of sent data
 *                WIFI_ERR_PARAMETER
 *                WIFI_ERR_SOCKET_NUM
 *                WIFI_ERR_NOT_CONNECT
 *                WIFI_ERR_TAKE_MUTEX
 *                WIFI_ERR_MODULE_COM
 *********************************************************************************************************************/
int32_t R_WIFI_SX_ULPGN_SendSocket (uint8_t socket_number, uint8_t * data, uint32_t length, uint32_t timeout_ms);

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_ReceiveSocket
 * Description  : Receive data on connecting socket.
 * Arguments    : socket_number
 *                data
 *                length
 *                timeout_ms
 * Return Value : Positive number - number of received data
 *                WIFI_ERR_PARAMETER
 *                WIFI_ERR_SOCKET_NUM
 *                WIFI_ERR_NOT_CONNECT
 *                WIFI_ERR_TAKE_MUTEX
 *                WIFI_ERR_CHANGE_SOCKET
 *                WIFI_ERR_MODULE_COM
 *********************************************************************************************************************/
int32_t R_WIFI_SX_ULPGN_ReceiveSocket (uint8_t socket_number, uint8_t * data, uint32_t length, uint32_t timeout_ms);

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_ShutdownSocket
 * Description  : Shutdown connecting socket.
 * Arguments    : socket_number
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_NOT_CONNECT
 *                WIFI_ERR_SOCKET_NUM
 *                WIFI_ERR_CHANGE_SOCKET
 *                WIFI_ERR_MODULE_COM
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_ShutdownSocket (uint8_t socket_number);

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_CloseSocket
 * Description  : Disconnect connecting socket.
 * Arguments    : socket_number
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_NOT_CONNECT
 *                WIFI_ERR_SOCKET_NUM
 *                WIFI_ERR_TAKE_MUTEX
 *                WIFI_ERR_CHANGE_SOCKET
 *                WIFI_ERR_MODULE_COM
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_CloseSocket (uint8_t socket_number);

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_DnsQuery
 * Description  : Execute DNS query.
 * Arguments    : domain_name
 *                ip_address
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_NOT_CONNECT
 *                WIFI_ERR_PARAMETER
 *                WIFI_ERR_MODULE_COM
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_DnsQuery (uint8_t * domain_name, uint32_t * ip_address);

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_Ping
 * Description  : Execute Ping command.
 * Arguments    : ip_address
 *                count
 *                interval_ms
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_PARAMETER
 *                WIFI_ERR_NOT_CONNECT
 *                WIFI_ERR_TAKE_MUTEX
 *                WIFI_ERR_MODULE_COM
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_Ping (uint32_t ip_address, uint16_t count, uint32_t interval_ms);

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_GetVersion
 * Description  : Get FIT module version.
 * Arguments    : none.
 * Return Value : FIT module version
 *********************************************************************************************************************/
uint32_t R_WIFI_SX_ULPGN_GetVersion (void);

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_GetTcpSocketStatus
 * Description  : Get TCP socket status.
 * Arguments    : socket_number
 * Return Value : -1    - not exist
 *                other - socket table pointer
 *********************************************************************************************************************/
int32_t R_WIFI_SX_ULPGN_GetTcpSocketStatus (uint8_t socket_number);

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_RequestTlsSocket
 * Description  : Request TLS socket communication.
 * Arguments    : socket_number
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_SOCKET_NUM
 *                WIFI_ERR_NOT_CONNECT
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_RequestTlsSocket (uint8_t socket_number);

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_WriteServerCertificate
 * Description  : Write server certificate to WiFi module.
 * Arguments    : data_id
 *                data_type
 *                certificate
 *                certificate_length
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_PARAMETER
 *                WIFI_ERR_NOT_OPEN
 *                WIFI_ERR_TAKE_MUTEX
 *                WIFI_ERR_MODULE_COM
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_WriteServerCertificate (uint32_t data_id, uint32_t data_type,
        const uint8_t * certificate, uint32_t certificate_length);

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_EraseServerCertificate
 * Description  : Erase server certificate on WiFi module.
 * Arguments    : certificate_name
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_PARAMETER
 *                WIFI_ERR_TAKE_MUTEX
 *                WIFI_ERR_NOT_OPEN
 *                WIFI_ERR_MODULE_COM
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_EraseServerCertificate (uint8_t *certificate_name);

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_GetServerCertificate
 * Description  : Get stored server certificate on WiFi module.
 * Arguments    : wifi_certificate_information
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_PARAMETER
 *                WIFI_ERR_TAKE_MUTEX
 *                WIFI_ERR_MODULE_COM
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_GetServerCertificate (wifi_certificate_infomation_t * wifi_certificate_information);

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_EraseAllServerCertificate
 * Description  : Erase all stored server certificate on WiFi module.
 * Arguments    : none
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_NOT_OPEN
 *                WIFI_ERR_TAKE_MUTEX
 *                WIFI_ERR_MODULE_COM
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_EraseAllServerCertificate (void);

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_SetCertificateProfile
 * Description  : Associate server information to certificate.
 * Arguments    : certificate_id
 *                ip_address
 *                server_name
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_PARAMETER
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_SetCertificateProfile (uint8_t certificate_id, uint32_t ipaddress, char * servername);

wifi_err_t R_WIFI_SX_ULPGN_GetDnsServerAddress(uint32_t *dns_address, uint32_t *dns_address_count);

#endif /* R_WIFI_SX_ULPGN_CFG_IF_H */
