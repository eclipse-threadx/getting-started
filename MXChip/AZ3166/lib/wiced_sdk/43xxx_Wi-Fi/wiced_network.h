/*
 * Copyright 2019, Cypress Semiconductor Corporation or a subsidiary of 
 * Cypress Semiconductor Corporation. All Rights Reserved.
 * 
 * This software, associated documentation and materials ("Software"),
 * is owned by Cypress Semiconductor Corporation
 * or one of its subsidiaries ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products. Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 */
#pragma once

#include "nx_api.h"
#include "tx_port.h"
#include "wiced_result.h"
#ifndef DISABLE_WICED_TLS
#include "tls_types.h"
#endif
#include "linked_list.h"
#include "wwd_network_constants.h"
#include "wiced_rtos.h"
#ifndef DISABLE_WICED_TLS
#include "dtls_types.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************
 *                      Macros
 ******************************************************/

#define IP_HANDLE(interface)                       (*wiced_ip_handle[( interface )&3])
#define WICED_LINK_CHECK( interface )              { if ( !wiced_network_interface_is_up( &IP_HANDLE(interface) ) ){ return WICED_NOTUP; }}
#define WICED_LINK_CHECK_TCP_SOCKET( socket_in )   { if ( (socket_in)->socket.nx_tcp_socket_ip_ptr->nx_ip_driver_link_up == 0 ){ return WICED_NOTUP; }}
#define WICED_LINK_CHECK_UDP_SOCKET( socket_in )   { if ( (socket_in)->socket.nx_udp_socket_ip_ptr->nx_ip_driver_link_up == 0 ){ return WICED_NOTUP; }}

/******************************************************
 *                    Constants
 ******************************************************/

#define WICED_MAXIMUM_NUMBER_OF_SOCKETS_WITH_CALLBACKS    (NX_MAX_LISTEN_REQUESTS)
#define WICED_MAXIMUM_NUMBER_OF_SERVER_SOCKETS            (WICED_MAXIMUM_NUMBER_OF_SOCKETS_WITH_CALLBACKS)

#define SIZE_OF_ARP_ENTRY                   sizeof(NX_ARP)

#ifdef DEBUG
#define IP_STACK_SIZE                       (3*1024)
#else
#define IP_STACK_SIZE                       (2*1024)
#endif
#define ARP_CACHE_SIZE                      (6 * SIZE_OF_ARP_ENTRY)
#define DHCP_STACK_SIZE                     (1280)

#define WICED_ANY_PORT                      (0)
#define WICED_NETWORK_MTU_SIZE              (WICED_LINK_MTU)
#define WICED_SOCKET_MAGIC_NUMBER           (0xfeedbead)
#define WICED_MAXIMUM_SEGMENT_SIZE(socket)  MIN(socket->socket.nx_tcp_socket_mss, socket->socket.nx_tcp_socket_connect_mss)

#define WICED_NUM_PACKET_POOLS 2
/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

typedef NX_PACKET wiced_packet_t;

typedef enum
{
    WICED_SOCKET_CLOSED,
    WICED_SOCKET_CLOSING,
    WICED_SOCKET_CONNECTING,
    WICED_SOCKET_CONNECTED,
    WICED_SOCKET_DATA_PENDING,
    WICED_SOCKET_LISTEN,
    WICED_SOCKET_ERROR
} wiced_socket_state_t;

typedef struct wiced_packet_pool_s
{
    NX_PACKET_POOL pool;
} wiced_packet_pool_t;

/******************************************************
 *                    Structures
 ******************************************************/
/* These should be in wiced_tcpip.h but are needed by wiced_tcp_socket_t, which would cause a circular include chain */
typedef struct wiced_tcp_socket_struct wiced_tcp_socket_t;
typedef struct wiced_udp_socket_struct wiced_udp_socket_t;

typedef wiced_result_t (*wiced_tcp_socket_callback_t)( wiced_tcp_socket_t* socket, void* arg );
typedef wiced_result_t (*wiced_udp_socket_callback_t)( wiced_udp_socket_t* socket, void* arg );


/* NOTE: Don't change the order or the fields within this wiced_tcp_socket_t and wiced_udp_socket_t.
 * Socket must always be the first field.
 * WICED TCP/IP layer uses socket magic number to differentiate between a native NX socket or a WICED socket.
 * This allows access to WICED socket object from a NX callback without having to store its pointer globally.
 */

struct wiced_tcp_socket_struct
{
    NX_TCP_SOCKET        socket;
    uint32_t             socket_magic_number;
#ifndef DISABLE_WICED_TLS
    wiced_tls_context_t* tls_context;
#endif
    wiced_bool_t         context_malloced;
    struct
    {
        wiced_tcp_socket_callback_t disconnect;
        wiced_tcp_socket_callback_t receive;
        wiced_tcp_socket_callback_t connect;
    } callbacks;
    void*                callback_arg;
};

struct wiced_udp_socket_struct
{
    NX_UDP_SOCKET               socket;
    uint32_t                    socket_magic_number;
#ifndef DISABLE_WICED_TLS
    wiced_dtls_context_t*       dtls_context;
#endif
    wiced_bool_t                context_malloced;
    wiced_udp_socket_callback_t receive_callback;
    void*                       callback_arg;
};

typedef struct
{
    wiced_mutex_t         mutex;
    linked_list_t         socket_list;
    int                   interface;
    uint16_t              port;
#ifndef DISABLE_WICED_TLS
    wiced_tls_identity_t* tls_identity;
#endif
    int                   max_tcp_connections;
    int                   active_tcp_connections;
} wiced_tcp_server_t;

typedef struct
{
    linked_list_node_t socket_node;
    wiced_tcp_socket_t socket;
} wiced_tcp_server_socket_t;

/******************************************************
 *                 Global Variables
 ******************************************************/

/*
 * Note: These objects are for internal use only!
 */
extern NX_IP*         wiced_ip_handle   [4];
extern NX_PACKET_POOL wiced_packet_pools[WICED_NUM_PACKET_POOLS];

/******************************************************
 *               Function Declarations
 ******************************************************/

extern wiced_bool_t wiced_network_interface_is_up( NX_IP* ip_handle );
extern wiced_result_t wiced_init_autoipv6( wiced_interface_t interface );

#ifdef __cplusplus
} /*extern "C" */
#endif
