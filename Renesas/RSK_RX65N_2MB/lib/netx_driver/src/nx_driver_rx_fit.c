/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** NetX Component                                                        */
/**                                                                       */
/**   Ethernet device driver for the Renesas RX FIT driver.    */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/* Indicate that driver source is being compiled.  */

#define NX_DRIVER_SOURCE

#include <stdint.h>

#include <tx_api.h>
#include <nx_api.h>

#include <r_ether_rx_if.h>

#include "nx_driver_rx_fit.h"

#define NX_DRIVER_ETHERNET_IP (0x0800U)
#define NX_DRIVER_ETHERNET_IPV6 (0x86ddU)
#define NX_DRIVER_ETHERNET_ARP (0x0806U)
#define NX_DRIVER_ETHERNET_RARP (0x8035U)

#define NX_DRIVER_ETHERNET_MTU (1514U)
#define NX_DRIVER_ETHERNET_FRAME_SIZE (14U)
#define NX_DRIVER_PHYSICAL_ADDRESS_SIZE (6U)

#define NX_DRIVER_STATE_NOT_INITIALIZED (1U)
#define NX_DRIVER_STATE_INITIALIZE_FAILED (2U)
#define NX_DRIVER_STATE_INITIALIZED (3U)
#define NX_DRIVER_STATE_LINK_ENABLED (4U)


#define NX_DRIVER_DEFERRED_PACKET_RECEIVED (1U)
#define NX_DRIVER_DEFERRED_PACKET_TRANSMITTED (2U)
#define NX_DRIVER_DEFERRED_LINK_STATE_CHANGE (4U)


/* Internal driver data. */
typedef struct netx_driver_rx_fit_data {
    NX_INTERFACE *netx_interface_ptr;
    NX_IP *netx_ip_ptr;
    NX_PACKET_POOL *netx_packet_pool_ptr;
    ULONG driver_state;
    ULONG deferred_events_flags;
    UINT rx_ether_chan;
} netx_driver_rx_fit_data_t;

static VOID _netx_driver_interface_attach(NX_IP_DRIVER *driver_req_ptr);
static VOID _netx_driver_initialize(NX_IP_DRIVER *driver_req_ptr);
static VOID _netx_driver_enable(NX_IP_DRIVER *driver_req_ptr);
static VOID _netx_driver_disable(NX_IP_DRIVER *driver_req_ptr);
static VOID _netx_driver_deferred_processing(NX_IP_DRIVER *driver_req_ptr);
static VOID _netx_driver_packet_send(NX_IP_DRIVER *driver_req_ptr);
static VOID _netx_driver_get_status(NX_IP_DRIVER *driver_req_ptr);

static VOID _rx_ether_cb(VOID *p_arg);
static VOID _rx_ether_int_cb(VOID *p_arg);

// TODO - Support two instances.
static netx_driver_rx_fit_data_t netx_driver_rx_fit_data[1];

/* Driver instance MAC address. MAC address should be set prior to
   initializing the driver by calling 'nx_ip_interface_physical_address_set()'
   otherwise the default address will be used.*/
static UCHAR _netx_driver_rx_fit_mac_address[] = {0x0,0x02,0x00,0x00,0x02,0x00};


VOID nx_driver_rx_fit(NX_IP_DRIVER *driver_req_ptr)
{

    /* Default to successful return.  */
    driver_req_ptr->nx_ip_driver_status = NX_SUCCESS;

    /* Process according to the driver request type in the IP control
           block.  */
    switch(driver_req_ptr->nx_ip_driver_command)
    {
    case NX_LINK_INTERFACE_ATTACH:
        _netx_driver_interface_attach(driver_req_ptr);
        break;

    case NX_LINK_INITIALIZE:
        _netx_driver_initialize(driver_req_ptr);
        break;

    case NX_LINK_ENABLE:
        _netx_driver_enable(driver_req_ptr);
        break;

    case NX_LINK_DISABLE:
        _netx_driver_disable(driver_req_ptr);
        break;

    case NX_LINK_DEFERRED_PROCESSING:
        _netx_driver_deferred_processing(driver_req_ptr);
        break;

    case NX_LINK_ARP_SEND:
    case NX_LINK_ARP_RESPONSE_SEND:
    case NX_LINK_PACKET_BROADCAST:
    case NX_LINK_RARP_SEND:
    case NX_LINK_PACKET_SEND:
        _netx_driver_packet_send(driver_req_ptr);
        break;

    case NX_LINK_GET_STATUS:
        _netx_driver_get_status(driver_req_ptr);
        break;

    case NX_LINK_MULTICAST_JOIN:
    case NX_LINK_MULTICAST_LEAVE:
        /* Nothing to do here as multicast hash filtering is not supported. */
        break;

    default:
        driver_req_ptr->nx_ip_driver_status = NX_UNHANDLED_COMMAND;
        break;
    }

}


static VOID _netx_driver_interface_attach(NX_IP_DRIVER *driver_req_ptr)
{
    // TODO - handle multiple instances.

    /* Save the NETX interface instance associated with this driver instance. */
    netx_driver_rx_fit_data[0].netx_interface_ptr = driver_req_ptr->nx_ip_driver_interface;
    netx_driver_rx_fit_data[0].rx_ether_chan = 0u;

    driver_req_ptr->nx_ip_driver_interface->nx_interface_additional_link_info = (void *)0u;

    /* Return success. */
    driver_req_ptr->nx_ip_driver_status = NX_SUCCESS;
}


static VOID _netx_driver_initialize(NX_IP_DRIVER *driver_req_ptr)
{
    NX_IP *ip_ptr;
    NX_INTERFACE *interface_ptr;
    UINT chan;

    /* Fetch the NETX IP instance and NETX interface. */
    ip_ptr = driver_req_ptr->nx_ip_driver_ptr;

    interface_ptr = driver_req_ptr->nx_ip_driver_interface;

    chan = (UINT)interface_ptr->nx_interface_additional_link_info;

    /* Save the packet pool pointer. */
    netx_driver_rx_fit_data[chan].netx_packet_pool_ptr = ip_ptr->nx_ip_default_packet_pool;

    /* Save the IP instance pointer. */
    netx_driver_rx_fit_data[chan].netx_ip_ptr = ip_ptr;

    /* Clear deferred event flags. */
    netx_driver_rx_fit_data[chan].deferred_events_flags = 0u;

    /* Save the MAC address. */
    interface_ptr->nx_interface_physical_address_msw =
            (ULONG)((_netx_driver_rx_fit_mac_address[0] << 8) | (_netx_driver_rx_fit_mac_address[1]));
    interface_ptr->nx_interface_physical_address_lsw =
            (ULONG)((_netx_driver_rx_fit_mac_address[2] << 24) | (_netx_driver_rx_fit_mac_address[3] << 16) |
                    (_netx_driver_rx_fit_mac_address[4] << 8) | (_netx_driver_rx_fit_mac_address[5]));

    /* Save the MTU size. */
    interface_ptr->nx_interface_ip_mtu_size = NX_DRIVER_ETHERNET_MTU;

    /* Indicate to the IP software that IP to physical mapping
       is required.  */
    interface_ptr -> nx_interface_address_mapping_needed =  NX_TRUE;

    /* Set initial state to not initialized. */
    netx_driver_rx_fit_data[chan].driver_state = NX_DRIVER_STATE_INITIALIZED;
}


static VOID _netx_driver_enable(NX_IP_DRIVER *driver_req_ptr)
{
    ether_return_t rx_ether_ret;
    UINT chan;
    ether_param_t eth_param = {0};

    chan = (UINT)driver_req_ptr->nx_ip_driver_interface->nx_interface_additional_link_info;

    /* Verify that initialization was done. */
    if(netx_driver_rx_fit_data[chan].driver_state < NX_DRIVER_STATE_INITIALIZED)
    {
        driver_req_ptr->nx_ip_driver_status = NX_DRIVER_ERROR;
        return;
    }

    /* Verify that it's not already enabled. */
    if(netx_driver_rx_fit_data[chan].driver_state >= NX_DRIVER_STATE_LINK_ENABLED)
    {
        driver_req_ptr->nx_ip_driver_status = NX_ALREADY_ENABLED;
        return;
    }

    eth_param.channel = chan;
    eth_param.ether_callback.pcb_func = _rx_ether_cb;
    eth_param.ether_int_hnd.pcb_int_hnd = _rx_ether_int_cb;

    rx_ether_ret = R_ETHER_Control(CONTROL_SET_CALLBACK, eth_param);
    if(rx_ether_ret != ETHER_SUCCESS) {
        driver_req_ptr->nx_ip_driver_status = NX_DRIVER_ERROR;
        return;
    }

    eth_param.channel = chan;
    eth_param.ether_callback.pcb_func = _rx_ether_cb;
    eth_param.ether_int_hnd.pcb_int_hnd = _rx_ether_int_cb;

    rx_ether_ret = R_ETHER_Control(CONTROL_SET_INT_HANDLER, eth_param);
    if(rx_ether_ret != ETHER_SUCCESS) {
        driver_req_ptr->nx_ip_driver_status = NX_DRIVER_ERROR;
        return;
    }

    rx_ether_ret = R_ETHER_Open_ZC2(chan, _netx_driver_rx_fit_mac_address, ETHER_FLAG_OFF);
    if(rx_ether_ret != ETHER_SUCCESS) {
        driver_req_ptr->nx_ip_driver_status = NX_DRIVER_ERROR;
        return;
    }

    /* Set state to driver enabled. */
    netx_driver_rx_fit_data[chan].driver_state = NX_DRIVER_STATE_LINK_ENABLED;

    netx_driver_rx_fit_data[chan].netx_interface_ptr->nx_interface_link_up = NX_TRUE;

    driver_req_ptr->nx_ip_driver_status = NX_SUCCESS;
}


static VOID _netx_driver_disable(NX_IP_DRIVER *driver_req_ptr)
{
    ether_return_t rx_ether_ret;
    UINT chan;

    chan = (UINT)driver_req_ptr->nx_ip_driver_interface->nx_interface_additional_link_info;

    /* Verify that initialization was done. */
    if(netx_driver_rx_fit_data[chan].driver_state < NX_DRIVER_STATE_INITIALIZED)
    {
        driver_req_ptr->nx_ip_driver_status = NX_DRIVER_ERROR;
        return;
    }

    /* Close the Ethernet channel. */
    rx_ether_ret = R_ETHER_Close_ZC2(chan);
    if(rx_ether_ret != ETHER_SUCCESS) {
        driver_req_ptr->nx_ip_driver_status = NX_DRIVER_ERROR;
        return;
    }

    netx_driver_rx_fit_data[chan].driver_state = 0u;

    driver_req_ptr->nx_ip_driver_status = NX_SUCCESS;
}


static VOID _netx_driver_deferred_processing(NX_IP_DRIVER *driver_req_ptr)
{
    UINT chan;
    ULONG flags;
    UINT res;
    NX_PACKET *packet_ptr;
    void *p_buf;
    int32_t read_res;
    USHORT packet_type;
    NX_IP *ip_ptr;
    uint32_t iter_cnt;
    TX_INTERRUPT_SAVE_AREA;


    TX_DISABLE;

    chan = (UINT)driver_req_ptr->nx_ip_driver_interface->nx_interface_additional_link_info;
    flags = netx_driver_rx_fit_data[chan].deferred_events_flags;
    netx_driver_rx_fit_data[chan].deferred_events_flags = 0u;

    /* Fetch the NETX IP instance and NETX interface. */
    ip_ptr = driver_req_ptr->nx_ip_driver_ptr;

    TX_RESTORE;

    if((flags & NX_DRIVER_DEFERRED_LINK_STATE_CHANGE) != 0u) {
        /* Link state change detected, process... */
        R_ETHER_LinkProcess(chan);
    }

    if((flags & NX_DRIVER_DEFERRED_PACKET_RECEIVED) != 0u) {
        /* Packet received. */


        /* Iterate over received packets with a safety maximum count just in case. */
        for(iter_cnt = 0u; iter_cnt < (ETHER_CFG_EMAC_RX_DESCRIPTORS * 2u); iter_cnt++) {

            /* Get the received frame from the Ethernet driver. */
            read_res = R_ETHER_Read_ZC2(chan, &p_buf);
            if(read_res <= 0) {
                /* Unexpected error, return. */
                driver_req_ptr->nx_ip_driver_status = NX_DRIVER_ERROR;

                return;
            }

            /* Get a new packet buffer. */
            res = nx_packet_allocate(netx_driver_rx_fit_data[chan].netx_packet_pool_ptr, &packet_ptr, NX_RECEIVE_PACKET, NX_NO_WAIT);
            if(res != NX_SUCCESS) {
                if(res == NX_NO_PACKET) {
                    /* No packet buffer available. Discard received data and exit. */
                    (void)R_ETHER_Read_ZC2_BufRelease(chan); /* Error ignored, already returning from one. */
                    driver_req_ptr->nx_ip_driver_status = NX_DRIVER_ERROR;

                    return;
                }
            }

            /* Copy packet data. */
            packet_ptr->nx_packet_append_ptr += 2u;
            packet_ptr->nx_packet_prepend_ptr += 2u;
            res = nx_packet_data_append(packet_ptr, p_buf, read_res, netx_driver_rx_fit_data[chan].netx_packet_pool_ptr, TX_NO_WAIT);
            if(res != NX_SUCCESS) {
                /* No packet buffer available. Discard received data and exit. */
                (void)R_ETHER_Read_ZC2_BufRelease(chan); /* Error ignored, already returning from one. */
                driver_req_ptr->nx_ip_driver_status = NX_DRIVER_ERROR;

                return;
            }

            /* Release the buffer. */
            read_res = R_ETHER_Read_ZC2_BufRelease(chan);
            if(read_res != ETHER_SUCCESS) {
                driver_req_ptr->nx_ip_driver_status = NX_DRIVER_ERROR;

                return;
            }

            /* Prepare the packet for NETX. */
            packet_ptr->nx_packet_ip_interface = netx_driver_rx_fit_data[chan].netx_interface_ptr;

            /* Pickup the packet header to determine where the packet needs to be sent. */
            packet_type = (USHORT)(((UINT) (*(packet_ptr->nx_packet_prepend_ptr + 12))) << 8) |
                ((UINT) (*(packet_ptr->nx_packet_prepend_ptr + 13)));

            /* Route the incoming packet according to its Ethernet type. */
            if(packet_type == NX_DRIVER_ETHERNET_IP || packet_type == NX_DRIVER_ETHERNET_IPV6)
            {
                /* Remove the Ethernet header. */
                packet_ptr->nx_packet_prepend_ptr = packet_ptr->nx_packet_prepend_ptr + NX_DRIVER_ETHERNET_FRAME_SIZE;

                packet_ptr->nx_packet_length = packet_ptr->nx_packet_length - NX_DRIVER_ETHERNET_FRAME_SIZE;

                /* Route to the ip receive function. */
                _nx_ip_packet_deferred_receive(ip_ptr, packet_ptr);
            }
            else if(packet_type == NX_DRIVER_ETHERNET_ARP)
            {

                /* Clean off the Ethernet header. */
                packet_ptr->nx_packet_prepend_ptr = packet_ptr->nx_packet_prepend_ptr + NX_DRIVER_ETHERNET_FRAME_SIZE;

                /* Adjust the packet length. */
                packet_ptr->nx_packet_length = packet_ptr->nx_packet_length - NX_DRIVER_ETHERNET_FRAME_SIZE;

                /* Route to the ARP receive function. */
                _nx_arp_packet_deferred_receive(ip_ptr, packet_ptr);
            }
            else if(packet_type == NX_DRIVER_ETHERNET_RARP)
            {

                /* Clean off the Ethernet header. */
                packet_ptr->nx_packet_prepend_ptr = packet_ptr->nx_packet_prepend_ptr + NX_DRIVER_ETHERNET_FRAME_SIZE;

                /* Adjust the packet length. */
                packet_ptr->nx_packet_length = packet_ptr->nx_packet_length - NX_DRIVER_ETHERNET_FRAME_SIZE;

                /* Route to the RARP receive function. */
                _nx_rarp_packet_deferred_receive(ip_ptr, packet_ptr);
            }
            else
            {
                /* Invalid Ethernet header, just release the packet. */
                nx_packet_release(packet_ptr);
            }

        }

    }

}


static VOID _netx_driver_packet_send(NX_IP_DRIVER *driver_req_ptr)
{
    NX_PACKET *packet_ptr;
    ULONG *frame_ptr;
    UINT chan;
    void *p_buf;
    uint16_t buf_size;
    UINT len;
    ether_return_t ether_ret;

    chan = (UINT)driver_req_ptr->nx_ip_driver_interface->nx_interface_additional_link_info;

    /* Verify that the link is up. */
    if(netx_driver_rx_fit_data[chan].driver_state < NX_DRIVER_STATE_LINK_ENABLED)
    {
        driver_req_ptr->nx_ip_driver_status = NX_DRIVER_ERROR;

        nx_packet_transmit_release(driver_req_ptr->nx_ip_driver_packet);

        return;
    }

    /* Place the Ethernet frame at the front of the packet. */
    packet_ptr = driver_req_ptr->nx_ip_driver_packet;

    /* Adjust the prepend pointer and packet length. */
    packet_ptr->nx_packet_prepend_ptr = packet_ptr->nx_packet_prepend_ptr - NX_DRIVER_ETHERNET_FRAME_SIZE;

    packet_ptr->nx_packet_length = packet_ptr->nx_packet_length + NX_DRIVER_ETHERNET_FRAME_SIZE;

    /* Setup the Ethernet frame pointer to build the Ethernet frame. */
    frame_ptr = (ULONG *) (packet_ptr->nx_packet_prepend_ptr - 2);

    /* Write the hardware addresses in the Ethernet header. */
    *frame_ptr = driver_req_ptr->nx_ip_driver_physical_address_msw;
    *(frame_ptr + 1) = driver_req_ptr->nx_ip_driver_physical_address_lsw;

    *(frame_ptr + 2) = (driver_req_ptr->nx_ip_driver_interface->nx_interface_physical_address_msw << 16) |
        (driver_req_ptr->nx_ip_driver_interface->nx_interface_physical_address_lsw >> 16);
    *(frame_ptr + 3) = (driver_req_ptr->nx_ip_driver_interface->nx_interface_physical_address_lsw << 16);

    /* Write the frame type field in the Ethernet harder. */
    if((driver_req_ptr->nx_ip_driver_command == NX_LINK_ARP_SEND) || (driver_req_ptr->nx_ip_driver_command == NX_LINK_ARP_RESPONSE_SEND))
    {
        *(frame_ptr + 3) |= NX_DRIVER_ETHERNET_ARP;
    }
    else if(driver_req_ptr->nx_ip_driver_command == NX_LINK_RARP_SEND)
    {
        *(frame_ptr + 3) |= NX_DRIVER_ETHERNET_RARP;
    }
#ifdef FEATURE_NX_IPV6
    else if(packet_ptr->nx_packet_ip_version == NX_IP_VERSION_V6)
    {
        *(frame_ptr + 3) |= NX_DRIVER_ETHERNET_IPV6;
    }
#endif
    else
    {
        *(frame_ptr + 3) |= NX_DRIVER_ETHERNET_IP;
    }

    /* Endian swapping if NX_LITTLE_ENDIAN is defined. */
    NX_CHANGE_ULONG_ENDIAN(*(frame_ptr));
    NX_CHANGE_ULONG_ENDIAN(*(frame_ptr + 1));
    NX_CHANGE_ULONG_ENDIAN(*(frame_ptr + 2));
    NX_CHANGE_ULONG_ENDIAN(*(frame_ptr + 3));

    /* Determine if the packet exceeds the driver's MTU. */
    if(packet_ptr->nx_packet_length > NX_DRIVER_ETHERNET_MTU)
    {
        /* Remove the Ethernet header. */
        packet_ptr->nx_packet_prepend_ptr += NX_DRIVER_ETHERNET_FRAME_SIZE;
        packet_ptr->nx_packet_length -= NX_DRIVER_ETHERNET_FRAME_SIZE;

        /* Indicate an unsuccessful packet send. */
        driver_req_ptr->nx_ip_driver_status =  NX_DRIVER_ERROR;

        nx_packet_transmit_release(packet_ptr);

        return;
    }

    /* Fetch a hardware buffer. */
    ether_ret = R_ETHER_Write_ZC2_GetBuf(chan, &p_buf, &buf_size);
    if(ether_ret != ETHER_SUCCESS) {
        /* Indicate an unsuccessful packet send. */
        driver_req_ptr->nx_ip_driver_status =  NX_DRIVER_ERROR;

        nx_packet_transmit_release(packet_ptr);
    }

    len = packet_ptr->nx_packet_length;

    /* Copy data. */
    memcpy(p_buf, packet_ptr->nx_packet_prepend_ptr, len);

    if(len < 60u) {
        len = 60u;
    }

    ether_ret = R_ETHER_Write_ZC2_SetBuf(chan, len);
    if(ether_ret != ETHER_SUCCESS) {
        /* Indicate an unsuccessful packet send. */
        driver_req_ptr->nx_ip_driver_status =  NX_DRIVER_ERROR;

        nx_packet_transmit_release(packet_ptr);
    }

    /* Release packet. */
    nx_packet_transmit_release(packet_ptr);
}


static VOID _netx_driver_get_status(NX_IP_DRIVER *driver_req_ptr)
{

    *(driver_req_ptr->nx_ip_driver_return_ptr) = driver_req_ptr->nx_ip_driver_interface->nx_interface_link_up;

    return;
}


static VOID _rx_ether_cb(VOID *p_arg)
{
}


static VOID _rx_ether_int_cb(VOID *p_arg)
{
    ether_cb_arg_t *p_cb_arg;
    UINT signal;
    UINT chan;

    p_cb_arg = (ether_cb_arg_t *)p_arg;

    chan = p_cb_arg->channel;

    signal = 0u;

    if((p_cb_arg->status_ecsr & (1u << 2)) != 0u) {
        /* Link change detected, signal Ethernet processing thread. */
        netx_driver_rx_fit_data[chan].deferred_events_flags |= NX_DRIVER_DEFERRED_LINK_STATE_CHANGE;
        signal = 1u;
    }

    if((p_cb_arg->status_eesr & (1u << 18)) != 0u) {
        netx_driver_rx_fit_data[chan].deferred_events_flags |= NX_DRIVER_DEFERRED_PACKET_RECEIVED;
        signal = 1u;
    }

    if(signal != 0u) {
        _nx_ip_driver_deferred_processing(netx_driver_rx_fit_data[chan].netx_ip_ptr);
    }
}
