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
/**   Ethernet device driver for ATMEL SAME54 family micro processors     */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/* Indicate that driver source is being compiled.  */

#define NX_DRIVER_SOURCE


/****** DRIVER SPECIFIC ****** Start of part/vendor specific include area.  Include driver-specific include file here!  */

/* Determine if the driver uses IP deferred processing or direct ISR processing.  */

#define NX_DRIVER_ENABLE_DEFERRED                /* Define this to enable deferred ISR processing.  */

/* Include driver specific include file.  */
#include "nx_driver_same54.h"


/****** DRIVER SPECIFIC ****** End of part/vendor specific include file area!  */


/* Define the driver information structure that is only available within this file.  */

static NX_DRIVER_INFORMATION   nx_driver_information;


/****** DRIVER SPECIFIC ****** Start of part/vendor specific data area.  Include hardware-specific data here!  */

/* Define driver specific ethernet hardware address.  */

#ifndef NX_DRIVER_ETHERNET_MAC
UCHAR   _nx_driver_hardware_address[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x56};  
#else
UCHAR   _nx_driver_hardware_address[] = NX_DRIVER_ETHERNET_MAC;  
#endif

sGmacRxDescriptor nx_driver_information_dma_rx_descriptors_area[NX_DRIVER_RX_DESCRIPTORS];
sGmacTxDescriptor nx_driver_information_dma_tx_descriptors_area[NX_DRIVER_TX_DESCRIPTORS]; 
sGmacTxDescriptor nx_driver_information_dummy_descriptor_area;

/****** DRIVER SPECIFIC ****** End of part/vendor specific data area!  */


/* Define the routines for processing each driver entry request.  The contents of these routines will change with
   each driver. However, the main driver entry function will not change, except for the entry function name.  */
   
static VOID         _nx_driver_interface_attach(NX_IP_DRIVER *driver_req_ptr);
static VOID         _nx_driver_initialize(NX_IP_DRIVER *driver_req_ptr);
static VOID         _nx_driver_enable(NX_IP_DRIVER *driver_req_ptr);
static VOID         _nx_driver_disable(NX_IP_DRIVER *driver_req_ptr);
static VOID         _nx_driver_packet_send(NX_IP_DRIVER *driver_req_ptr);
static VOID         _nx_driver_multicast_join(NX_IP_DRIVER *driver_req_ptr);
static VOID         _nx_driver_multicast_leave(NX_IP_DRIVER *driver_req_ptr);
static VOID         _nx_driver_get_status(NX_IP_DRIVER *driver_req_ptr);
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
static VOID         _nx_driver_capability_get(NX_IP_DRIVER *driver_req_ptr);
static VOID         _nx_driver_capability_set(NX_IP_DRIVER *driver_req_ptr);
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */
#ifdef NX_DRIVER_ENABLE_DEFERRED
static VOID         _nx_driver_deferred_processing(NX_IP_DRIVER *driver_req_ptr);
#endif /* NX_DRIVER_ENABLE_DEFERRED */
static VOID         _nx_driver_transfer_to_netx(NX_IP *ip_ptr, NX_PACKET *packet_ptr);


/* Define the prototypes for the hardware implementation of this driver. The contents of these routines are
   driver-specific.  */

static UINT         _nx_driver_hardware_initialize(NX_IP_DRIVER *driver_req_ptr); 
static UINT         _nx_driver_hardware_enable(NX_IP_DRIVER *driver_req_ptr); 
static UINT         _nx_driver_hardware_disable(NX_IP_DRIVER *driver_req_ptr); 
static UINT         _nx_driver_hardware_packet_send(NX_PACKET *packet_ptr); 
static UINT         _nx_driver_hardware_multicast_join(NX_IP_DRIVER *driver_req_ptr);
static UINT         _nx_driver_hardware_multicast_leave(NX_IP_DRIVER *driver_req_ptr);
static UINT         _nx_driver_hardware_get_status(NX_IP_DRIVER *driver_req_ptr);
static VOID         _nx_driver_hardware_packet_transmitted(VOID);
static VOID         _nx_driver_hardware_packet_received(VOID);
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
static UINT         _nx_driver_hardware_capability_set(NX_IP_DRIVER *driver_req_ptr);
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    nx_driver_same54                                    SAME54/IAR      */ 
/*                                                           5.0          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    Yuxin Zhou, Microsoft Corporation                                   */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This is the entry point of the NetX Ethernet Driver. This driver    */ 
/*    function is responsible for initializing the Ethernet controller,   */ 
/*    enabling or disabling the controller as need, preparing             */ 
/*    a packet for transmission, and getting status information.          */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    driver_req_ptr                        The driver request from the   */ 
/*                                            IP layer.                   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_driver_interface_attach           Process attach request        */ 
/*    _nx_driver_initialize                 Process initialize request    */ 
/*    _nx_driver_enable                     Process link enable request   */ 
/*    _nx_driver_disable                    Process link disable request  */ 
/*    _nx_driver_packet_send                Process send packet requests  */ 
/*    _nx_driver_multicast_join             Process multicast join request*/ 
/*    _nx_driver_multicast_leave            Process multicast leave req   */ 
/*    _nx_driver_get_status                 Process get status request    */ 
/*    _nx_driver_deferred_processing        Drive deferred processing     */ 
/*                                                                        */
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    IP layer                                                            */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-07-2015        Yuxin Zhou            Initial Version 5.0           */ 
/*                                                                        */ 
/**************************************************************************/ 
/****** DRIVER SPECIFIC ****** Start of part/vendor specific global driver entry function name.  */
VOID  nx_driver_same54(NX_IP_DRIVER *driver_req_ptr)
/****** DRIVER SPECIFIC ****** End of part/vendor specific global driver entry function name.  */
{
    
    /* Default to successful return.  */
    driver_req_ptr -> nx_ip_driver_status =  NX_SUCCESS;
    
    /* Process according to the driver request type in the IP control 
       block.  */
    switch (driver_req_ptr -> nx_ip_driver_command)
    {
        
    case NX_LINK_INTERFACE_ATTACH:
       
        /* Process link interface attach requests.  */
        _nx_driver_interface_attach(driver_req_ptr);
        break;
        
    case NX_LINK_INITIALIZE:
    {

        /* Process link initialize requests.  */
        _nx_driver_initialize(driver_req_ptr);
        break;
    }
        
    case NX_LINK_ENABLE:
    {
    
        /* Process link enable requests.  */
        _nx_driver_enable(driver_req_ptr);
        break;
    }
        
    case NX_LINK_DISABLE:
    {
    
        /* Process link disable requests.  */
        _nx_driver_disable(driver_req_ptr);
        break;
    } 
        
        
    case NX_LINK_ARP_SEND:
    case NX_LINK_ARP_RESPONSE_SEND:
    case NX_LINK_PACKET_BROADCAST:
    case NX_LINK_RARP_SEND:
    case NX_LINK_PACKET_SEND:
    {
            
        /* Process packet send requests.  */
        _nx_driver_packet_send(driver_req_ptr);
        break;
    }
        
        
    case NX_LINK_MULTICAST_JOIN:
    {
            
        /* Process multicast join requests.  */
        _nx_driver_multicast_join(driver_req_ptr);
        break;
    }
    
        
    case NX_LINK_MULTICAST_LEAVE:
    {
            
        /* Process multicast leave requests.  */
        _nx_driver_multicast_leave(driver_req_ptr);
        break;
    }
        
    case NX_LINK_GET_STATUS:
    {
            
        /* Process get status requests.  */
        _nx_driver_get_status(driver_req_ptr);
        break;
    }
        
#ifdef NX_DRIVER_ENABLE_DEFERRED
    case NX_LINK_DEFERRED_PROCESSING:
    { 

        /* Process driver deferred requests.  */
        
        /* Process a device driver function on behave of the IP thread. */
        _nx_driver_deferred_processing(driver_req_ptr);

        break;
    }

#endif

#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    case NX_INTERFACE_CAPABILITY_GET:
    {

        /* Process get capability requests.  */
        _nx_driver_capability_get(driver_req_ptr);
        break;
    }
    
    case NX_INTERFACE_CAPABILITY_SET:
    {

        /* Process set capability requests.  */
        _nx_driver_capability_set(driver_req_ptr);
        break;
    }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

    default:

            
        /* Invalid driver request.  */
        
        /* Return the unhandled command status.  */
        driver_req_ptr -> nx_ip_driver_status =  NX_UNHANDLED_COMMAND;

        /* Default to successful return.  */
        driver_req_ptr -> nx_ip_driver_status =  NX_DRIVER_ERROR;
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_driver_interface_attach                         SAME54/IAR      */ 
/*                                                           5.0          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    Yuxin Zhou, Microsoft Corporation                                   */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processing the interface attach request.              */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    driver_req_ptr                        Driver command from the IP    */ 
/*                                            thread                      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Driver entry function                                               */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-07-2015        Yuxin Zhou            Initial Version 5.0           */ 
/*                                                                        */ 
/**************************************************************************/ 
static VOID  _nx_driver_interface_attach(NX_IP_DRIVER *driver_req_ptr)
{


    /* Setup the driver's interface.  This example is for a simple one-interface
       Ethernet driver. Additional logic is necessary for multiple port devices.  */
    nx_driver_information.nx_driver_information_interface =  driver_req_ptr -> nx_ip_driver_interface;

#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    driver_req_ptr -> nx_ip_driver_interface -> nx_interface_capability_flag = NX_DRIVER_CAPABILITY;
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

    /* Return successful status.  */
    driver_req_ptr -> nx_ip_driver_status =  NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_driver_initialize                               SAME54/IAR      */ 
/*                                                           5.0          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    Yuxin Zhou, Microsoft Corporation                                   */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processing the initialize request.  The processing    */
/*    in this function is generic. All ethernet controller logic is to    */ 
/*    be placed in _nx_driver_hardware_initialize.                        */  
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    driver_req_ptr                        Driver command from the IP    */ 
/*                                            thread                      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_driver_hardware_initialize        Process initialize request    */ 
/*                                                                        */
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Driver entry function                                               */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-07-2015        Yuxin Zhou            Initial Version 5.0           */ 
/*                                                                        */ 
/**************************************************************************/ 
static VOID  _nx_driver_initialize(NX_IP_DRIVER *driver_req_ptr)
{

NX_IP           *ip_ptr;
NX_INTERFACE    *interface_ptr;
UINT            status;
        
        
    /* Setup the IP pointer from the driver request.  */
    ip_ptr =  driver_req_ptr -> nx_ip_driver_ptr;
    
    /* Setup interface pointer.  */
    interface_ptr = driver_req_ptr -> nx_ip_driver_interface;

    /* Initialize the driver's information structure.  */

    /* Default IP pointer to NULL.  */
    nx_driver_information.nx_driver_information_ip_ptr =               NX_NULL;

    /* Setup the driver state to not initialized.  */
    nx_driver_information.nx_driver_information_state =                NX_DRIVER_STATE_NOT_INITIALIZED;

    /* Setup the default packet pool for the driver's received packets.  */
    nx_driver_information.nx_driver_information_packet_pool_ptr = ip_ptr -> nx_ip_default_packet_pool;

    /* Clear the deferred events for the driver.  */
    nx_driver_information.nx_driver_information_deferred_events =       0;

    /* Call the hardware-specific ethernet controller initialization.  */
    status =  _nx_driver_hardware_initialize(driver_req_ptr);

    /* Determine if the request was successful.  */
    if (status == NX_SUCCESS)
    {

        /* Successful hardware initialization.  */

        /* Setup driver information to point to IP pointer.  */
        nx_driver_information.nx_driver_information_ip_ptr = driver_req_ptr -> nx_ip_driver_ptr;
            
        /* Setup the link maximum transfer unit. */
        interface_ptr -> nx_interface_ip_mtu_size =  NX_DRIVER_ETHERNET_MTU - NX_DRIVER_ETHERNET_FRAME_SIZE;
            
        /* Setup the physical address of this IP instance.  Increment the 
           physical address lsw to simulate multiple nodes hanging on the
           ethernet.  */
        interface_ptr -> nx_interface_physical_address_msw =  
                (ULONG)((_nx_driver_hardware_address[0] << 8) | (_nx_driver_hardware_address[1]));
        interface_ptr -> nx_interface_physical_address_lsw =  
                (ULONG)((_nx_driver_hardware_address[2] << 24) | (_nx_driver_hardware_address[3] << 16) | 
                        (_nx_driver_hardware_address[4] << 8) | (_nx_driver_hardware_address[5]));
                
        /* Indicate to the IP software that IP to physical mapping
           is required.  */
        interface_ptr -> nx_interface_address_mapping_needed =  NX_TRUE;
            
        /* Move the driver's state to initialized.  */
        nx_driver_information.nx_driver_information_state = NX_DRIVER_STATE_INITIALIZED;

        /* Indicate successful initialize.  */
        driver_req_ptr -> nx_ip_driver_status =  NX_SUCCESS;
    }
    else
    {
        
        /* Initialization failed.  Indicate that the request failed.  */
        driver_req_ptr -> nx_ip_driver_status =   NX_DRIVER_ERROR;
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_driver_enable                                   SAME54/IAR      */ 
/*                                                           5.0          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    Yuxin Zhou, Microsoft Corporation                                   */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processing the initialize request. The processing     */
/*    in this function is generic. All ethernet controller logic is to    */ 
/*    be placed in _nx_driver_hardware_enable.                            */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    driver_req_ptr                        Driver command from the IP    */ 
/*                                            thread                      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_driver_hardware_enable            Process enable request        */ 
/*                                                                        */
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Driver entry function                                               */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-07-2015        Yuxin Zhou            Initial Version 5.0           */ 
/*                                                                        */ 
/**************************************************************************/ 
static VOID  _nx_driver_enable(NX_IP_DRIVER *driver_req_ptr)
{

UINT            status;


    /* See if we can honor the NX_LINK_ENABLE request.  */
    if (nx_driver_information.nx_driver_information_state < NX_DRIVER_STATE_INITIALIZED)
    {

        /* Mark the request as not successful.  */
        driver_req_ptr -> nx_ip_driver_status =  NX_DRIVER_ERROR;                      
        return;
    }                
        
    /* Check if it is enabled by someone already */
    if (nx_driver_information.nx_driver_information_state >=  NX_DRIVER_STATE_LINK_ENABLED)
    {

        /* Yes, the request has already been made.  */
        driver_req_ptr -> nx_ip_driver_status =  NX_ALREADY_ENABLED;
        return;
    }

    /* Call hardware specific enable.  */
    status =  _nx_driver_hardware_enable(driver_req_ptr);

    /* Was the hardware enable successful?  */
    if (status == NX_SUCCESS)
    {

        /* Update the driver state to link enabled.  */
        nx_driver_information.nx_driver_information_state = NX_DRIVER_STATE_LINK_ENABLED;

        /* Mark request as successful.  */
        driver_req_ptr -> nx_ip_driver_status =  NX_SUCCESS;;
        
        /* Mark the IP interface as link up.  */
        driver_req_ptr -> nx_ip_driver_interface -> nx_interface_link_up = NX_TRUE;
    }
    else
    {
        
        /* Enable failed.  Indicate that the request failed.  */
        driver_req_ptr -> nx_ip_driver_status =   NX_DRIVER_ERROR;
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_driver_disable                                  SAME54/IAR      */ 
/*                                                           5.0          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    Yuxin Zhou, Microsoft Corporation                                   */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processing the disable request. The processing        */
/*    in this function is generic. All ethernet controller logic is to    */ 
/*    be placed in _nx_driver_hardware_disable.                           */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    driver_req_ptr                        Driver command from the IP    */ 
/*                                            thread                      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_driver_hardware_disable           Process disable request       */ 
/*                                                                        */
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Driver entry function                                               */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-07-2015        Yuxin Zhou            Initial Version 5.0           */ 
/*                                                                        */ 
/**************************************************************************/ 
static VOID  _nx_driver_disable(NX_IP_DRIVER *driver_req_ptr)
{

UINT            status;


    /* Check if the link is enabled.  */
    if (nx_driver_information.nx_driver_information_state !=  NX_DRIVER_STATE_LINK_ENABLED)
    {

        /* The link is not enabled, so just return an error.  */
        driver_req_ptr -> nx_ip_driver_status =  NX_DRIVER_ERROR;
        return;
    }

    /* Call hardware specific disable.  */
    status =  _nx_driver_hardware_disable(driver_req_ptr);

    /* Was the hardware disable successful?  */
    if (status == NX_SUCCESS)
    {
    
        /* Mark the IP interface as link down.  */
        driver_req_ptr -> nx_ip_driver_interface -> nx_interface_link_up = NX_FALSE;

        /* Update the driver state back to initialized.  */
        nx_driver_information.nx_driver_information_state =  NX_DRIVER_STATE_INITIALIZED;
        
        /* Mark request as successful.  */
        driver_req_ptr -> nx_ip_driver_status =  NX_SUCCESS;
    }
    else
    {

        /* Disable failed, return an error.  */    
        driver_req_ptr -> nx_ip_driver_status =  NX_DRIVER_ERROR;
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_driver_packet_send                              SAME54/IAR      */ 
/*                                                           5.0          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    Yuxin Zhou, Microsoft Corporation                                   */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processing the packet send request. The processing    */
/*    in this function is generic. All ethernet controller packet send    */ 
/*    logic is to be placed in _nx_driver_hardware_packet_send.           */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    driver_req_ptr                        Driver command from the IP    */ 
/*                                            thread                      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_driver_hardware_packet_send       Process packet send request   */ 
/*                                                                        */
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Driver entry function                                               */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-07-2015        Yuxin Zhou            Initial Version 5.0           */ 
/*                                                                        */ 
/**************************************************************************/ 
static VOID  _nx_driver_packet_send(NX_IP_DRIVER *driver_req_ptr)
{

NX_PACKET       *packet_ptr;
ULONG           *ethernet_frame_ptr;
UINT            status;


    /* Check to make sure the link is up.  */
    if (nx_driver_information.nx_driver_information_state != NX_DRIVER_STATE_LINK_ENABLED)
    {

        /* Inidate an unsuccessful packet send.  */
        driver_req_ptr -> nx_ip_driver_status =  NX_DRIVER_ERROR;

        /* Link is not up, simply free the packet.  */
        nx_packet_transmit_release(driver_req_ptr -> nx_ip_driver_packet);
        return;
    }                
    
    /* Process driver send packet.  */
    
    /* Place the ethernet frame at the front of the packet.  */
    packet_ptr =  driver_req_ptr -> nx_ip_driver_packet;
        
    /* Adjust the prepend pointer.  */
    packet_ptr -> nx_packet_prepend_ptr =  
            packet_ptr -> nx_packet_prepend_ptr - NX_DRIVER_ETHERNET_FRAME_SIZE;
        
    /* Adjust the packet length.  */
    packet_ptr -> nx_packet_length = packet_ptr -> nx_packet_length + NX_DRIVER_ETHERNET_FRAME_SIZE;
            
    /* Setup the ethernet frame pointer to build the ethernet frame.  Backup another 2
      * bytes to get 32-bit word alignment.  */
    ethernet_frame_ptr =  (ULONG *) (packet_ptr -> nx_packet_prepend_ptr - 2);
        
    /* Set up the hardware addresses in the Ethernet header. */
    *ethernet_frame_ptr       =  driver_req_ptr -> nx_ip_driver_physical_address_msw;
    *(ethernet_frame_ptr + 1) =  driver_req_ptr -> nx_ip_driver_physical_address_lsw;
        
    *(ethernet_frame_ptr + 2) =  (driver_req_ptr -> nx_ip_driver_interface -> nx_interface_physical_address_msw << 16) |
        (driver_req_ptr -> nx_ip_driver_interface -> nx_interface_physical_address_lsw >> 16);
    *(ethernet_frame_ptr + 3) =  (driver_req_ptr -> nx_ip_driver_interface -> nx_interface_physical_address_lsw << 16);

    /* Set up the frame type field in the Ethernet harder. */
    if ((driver_req_ptr -> nx_ip_driver_command == NX_LINK_ARP_SEND)||
        (driver_req_ptr -> nx_ip_driver_command == NX_LINK_ARP_RESPONSE_SEND))
    {

        *(ethernet_frame_ptr + 3) |= NX_DRIVER_ETHERNET_ARP;
    }
    else if(driver_req_ptr -> nx_ip_driver_command == NX_LINK_RARP_SEND)
    {

        *(ethernet_frame_ptr + 3) |= NX_DRIVER_ETHERNET_RARP;        
    }

#ifdef FEATURE_NX_IPV6
    else if(packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V6)
    {

        *(ethernet_frame_ptr + 3) |= NX_DRIVER_ETHERNET_IPV6;
    }
#endif /* FEATURE_NX_IPV6 */

    else
    {

        *(ethernet_frame_ptr + 3) |= NX_DRIVER_ETHERNET_IP;
    }

    /* Endian swapping if NX_LITTLE_ENDIAN is defined.  */
    NX_CHANGE_ULONG_ENDIAN(*(ethernet_frame_ptr));
    NX_CHANGE_ULONG_ENDIAN(*(ethernet_frame_ptr + 1));
    NX_CHANGE_ULONG_ENDIAN(*(ethernet_frame_ptr + 2));
    NX_CHANGE_ULONG_ENDIAN(*(ethernet_frame_ptr + 3));
        
    /* Determine if the packet exceeds the driver's MTU.  */
    if (packet_ptr -> nx_packet_length > NX_DRIVER_ETHERNET_MTU)
    {
    
        /* This packet exceeds the size of the driver's MTU. Simply throw it away! */

        /* Remove the Ethernet header.  */
        NX_DRIVER_ETHERNET_HEADER_REMOVE(packet_ptr);
        
        /* Indicate an unsuccessful packet send.  */
        driver_req_ptr -> nx_ip_driver_status =  NX_DRIVER_ERROR;

        /* Link is not up, simply free the packet.  */
        nx_packet_transmit_release(packet_ptr);
        return;
    }

    /* Transmit the packet through the Ethernet controller low level access routine. */
    status = _nx_driver_hardware_packet_send(packet_ptr);

    /* Determine if there was an error.  */
    if (status != NX_SUCCESS)
    {

        /* Driver's hardware send packet routine failed to send the packet.  */

        /* Remove the Ethernet header.  */
        NX_DRIVER_ETHERNET_HEADER_REMOVE(packet_ptr);
        
        /* Indicate an unsuccessful packet send.  */
        driver_req_ptr -> nx_ip_driver_status =  NX_DRIVER_ERROR;

        /* Link is not up, simply free the packet.  */
        nx_packet_transmit_release(packet_ptr);
    }
    else
    {

        /* Set the status of the request.  */    
        driver_req_ptr -> nx_ip_driver_status =  NX_SUCCESS;
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_driver_multicast_join                           SAME54/IAR      */ 
/*                                                           5.0          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    Yuxin Zhou, Microsoft Corporation                                   */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processing the multicast join request. The processing */
/*    in this function is generic. All ethernet controller multicast join */ 
/*    logic is to be placed in _nx_driver_hardware_multicast_join.        */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    driver_req_ptr                        Driver command from the IP    */ 
/*                                            thread                      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_driver_hardware_multicast_join    Process multicast join request*/ 
/*                                                                        */
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Driver entry function                                               */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-07-2015        Yuxin Zhou            Initial Version 5.0           */ 
/*                                                                        */ 
/**************************************************************************/ 
static VOID  _nx_driver_multicast_join(NX_IP_DRIVER *driver_req_ptr)
{

UINT        status;


    /* Call hardware specific multicast join function. */
    status =  _nx_driver_hardware_multicast_join(driver_req_ptr);
    
    /* Determine if there was an error.  */
    if (status != NX_SUCCESS)
    {

        /* Indicate an unsuccessful request.  */
        driver_req_ptr -> nx_ip_driver_status =  NX_DRIVER_ERROR;
    }
    else
    {

        /* Indicate the request was successful.   */    
        driver_req_ptr -> nx_ip_driver_status =  NX_SUCCESS;
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_driver_multicast_leave                          SAME54/IAR      */ 
/*                                                           5.0          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    Yuxin Zhou, Microsoft Corporation                                   */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processing the multicast leave request. The           */ 
/*    processing in this function is generic. All ethernet controller     */ 
/*    multicast leave logic is to be placed in                            */ 
/*    _nx_driver_hardware_multicast_leave.                                */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    driver_req_ptr                        Driver command from the IP    */ 
/*                                            thread                      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_driver_hardware_multicast_leave   Process multicast leave req   */ 
/*                                                                        */
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Driver entry function                                               */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-07-2015        Yuxin Zhou            Initial Version 5.0           */ 
/*                                                                        */ 
/**************************************************************************/ 
static VOID  _nx_driver_multicast_leave(NX_IP_DRIVER *driver_req_ptr)
{

UINT        status;


    /* Call hardware specific multicast leave function. */
    status =  _nx_driver_hardware_multicast_leave(driver_req_ptr);
    
    /* Determine if there was an error.  */
    if (status != NX_SUCCESS)
    {

        /* Indicate an unsuccessful request.  */
        driver_req_ptr -> nx_ip_driver_status =  NX_DRIVER_ERROR;
    }
    else
    {

        /* Indicate the request was successful.   */    
        driver_req_ptr -> nx_ip_driver_status =  NX_SUCCESS;
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_driver_get_status                               SAME54/IAR      */ 
/*                                                           5.0          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    Yuxin Zhou, Microsoft Corporation                                   */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processing the get status request. The processing     */
/*    in this function is generic. All ethernet controller get status     */ 
/*    logic is to be placed in _nx_driver_hardware_get_status.            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    driver_req_ptr                        Driver command from the IP    */ 
/*                                            thread                      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_driver_hardware_get_status        Process get status request    */ 
/*                                                                        */
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Driver entry function                                               */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-07-2015        Yuxin Zhou            Initial Version 5.0           */ 
/*                                                                        */ 
/**************************************************************************/ 
static VOID  _nx_driver_get_status(NX_IP_DRIVER *driver_req_ptr)
{

UINT        status;


    /* Call hardware specific get status function. */
    status =  _nx_driver_hardware_get_status(driver_req_ptr);
    
    /* Determine if there was an error.  */
    if (status != NX_SUCCESS)
    {

        /* Indicate an unsuccessful request.  */
        driver_req_ptr -> nx_ip_driver_status =  NX_DRIVER_ERROR;
    }
    else
    {

        /* Indicate the request was successful.   */    
        driver_req_ptr -> nx_ip_driver_status =  NX_SUCCESS;
    }
}


#ifdef NX_ENABLE_INTERFACE_CAPABILITY
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_driver_capability_get                           SAME54/IAR      */ 
/*                                                           5.0          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    Yuxin Zhou, Microsoft Corporation                                   */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processing the get capability request.                */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    driver_req_ptr                        Driver command from the IP    */ 
/*                                            thread                      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Driver entry function                                               */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-07-2015        Yuxin Zhou            Initial Version 5.0           */ 
/*                                                                        */ 
/**************************************************************************/ 
static VOID  _nx_driver_capability_get(NX_IP_DRIVER *driver_req_ptr)
{
    
    /* Return the capability of the Ethernet controller.  */
    *(driver_req_ptr -> nx_ip_driver_return_ptr) = NX_DRIVER_CAPABILITY;
    
    /* Return the success status.  */
    driver_req_ptr -> nx_ip_driver_status =  NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_driver_capability_set                           SAME54/IAR      */ 
/*                                                           5.0          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    Yuxin Zhou, Microsoft Corporation                                   */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processing the set capability request.                */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    driver_req_ptr                        Driver command from the IP    */ 
/*                                            thread                      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Driver entry function                                               */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-07-2015        Yuxin Zhou            Initial Version 5.0           */ 
/*                                                                        */ 
/**************************************************************************/ 
static VOID  _nx_driver_capability_set(NX_IP_DRIVER *driver_req_ptr)
{

UINT        status;


    /* Call hardware specific get status function. */
    status =  _nx_driver_hardware_capability_set(driver_req_ptr);
    
    /* Determine if there was an error.  */
    if (status != NX_SUCCESS)
    {

        /* Indicate an unsuccessful request.  */
        driver_req_ptr -> nx_ip_driver_status =  NX_DRIVER_ERROR;
    }
    else
    {

        /* Indicate the request was successful.   */    
        driver_req_ptr -> nx_ip_driver_status =  NX_SUCCESS;
    }
}
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */


#ifdef NX_DRIVER_ENABLE_DEFERRED
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_driver_deferred_processing                      SAME54/IAR      */ 
/*                                                           5.0          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    XC, Microsoft Corporation                                           */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processing the deferred ISR action within the context */ 
/*    of the IP thread.                                                   */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    driver_req_ptr                        Driver command from the IP    */ 
/*                                            thread                      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_driver_packet_transmitted        Clean up after transmission    */
/*    _nx_driver_packet_received           Process a received packet      */
/*                                                                        */
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Driver entry function                                               */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-07-2015        Yuxin Zhou            Initial Version 5.0           */ 
/*                                                                        */ 
/**************************************************************************/ 
static VOID  _nx_driver_deferred_processing(NX_IP_DRIVER *driver_req_ptr)
{

TX_INTERRUPT_SAVE_AREA

ULONG       deferred_events;

    
    /* Disable interrupts.  */
    TX_DISABLE

    /* Pickup deferred events.  */
    deferred_events =  nx_driver_information.nx_driver_information_deferred_events;
    nx_driver_information.nx_driver_information_deferred_events =  0;

    /* Restore interrupts.  */
    TX_RESTORE
        
    /* Check for a transmit complete event.  */
    if(deferred_events & NX_DRIVER_DEFERRED_PACKET_TRANSMITTED)
    {
        
        /* Process transmitted packet(s).  */
        _nx_driver_hardware_packet_transmitted();
    }    
    
    /* Check for recevied packet.  */
    if(deferred_events & NX_DRIVER_DEFERRED_PACKET_RECEIVED)
    {

        /* Process received packet(s).  */
        _nx_driver_hardware_packet_received();
    }

    /* Mark request as successful.  */    
    driver_req_ptr->nx_ip_driver_status =  NX_SUCCESS;
}
#endif /* NX_DRIVER_ENABLE_DEFERRED */


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_driver_transfer_to_netx                         SAME54/IAR      */ 
/*                                                           5.0          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    Yuxin Zhou, Microsoft Corporation                                   */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processing incoming packets.  This routine would      */
/*    be called from the driver-specific receive packet processing        */ 
/*    function _nx_driver_hardware.                                       */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ip_ptr                                Pointer to IP protocol block  */ 
/*    packet_ptr                            Packet pointer                */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Error indication                                                    */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ip_packet_receive                 NetX IP packet receive        */ 
/*    _nx_ip_packet_deferred_receive        NetX IP packet receive        */ 
/*    _nx_arp_packet_deferred_receive       NetX ARP packet receive       */ 
/*    _nx_rarp_packet_deferred_receive      NetX RARP packet receive      */ 
/*    _nx_packet_release                    Release packet                */ 
/*                                                                        */
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_driver_hardware_packet_received   Driver packet receive function*/ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-07-2015        Yuxin Zhou            Initial Version 5.0           */ 
/*                                                                        */ 
/**************************************************************************/ 
static VOID _nx_driver_transfer_to_netx(NX_IP *ip_ptr, NX_PACKET *packet_ptr)
{

USHORT    packet_type;


    /* Set the interface for the incoming packet.  */
    packet_ptr -> nx_packet_ip_interface = nx_driver_information.nx_driver_information_interface;

    /* Pickup the packet header to determine where the packet needs to be
       sent.  */
    packet_type =  (USHORT)(((UINT) (*(packet_ptr -> nx_packet_prepend_ptr+12))) << 8) | 
        ((UINT) (*(packet_ptr -> nx_packet_prepend_ptr+13)));

    /* Route the incoming packet according to its ethernet type.  */
    if (packet_type == NX_DRIVER_ETHERNET_IP || packet_type == NX_DRIVER_ETHERNET_IPV6)
    {
        /* Note:  The length reported by some Ethernet hardware includes 
           bytes after the packet as well as the Ethernet header.  In some 
           cases, the actual packet length after the Ethernet header should 
           be derived from the length in the IP header (lower 16 bits of
           the first 32-bit word).  */

        /* Clean off the Ethernet header.  */
        packet_ptr -> nx_packet_prepend_ptr =  
            packet_ptr -> nx_packet_prepend_ptr + NX_DRIVER_ETHERNET_FRAME_SIZE;
 
        /* Adjust the packet length.  */
        packet_ptr -> nx_packet_length =  
            packet_ptr -> nx_packet_length - NX_DRIVER_ETHERNET_FRAME_SIZE;

        /* Route to the ip receive function.  */
#ifdef NX_DRIVER_ENABLE_DEFERRED
        _nx_ip_packet_deferred_receive(ip_ptr, packet_ptr);
#else
        _nx_ip_packet_receive(ip_ptr, packet_ptr);
#endif /* NX_DRIVER_ENABLE_DEFERRED */
    }
    else if (packet_type == NX_DRIVER_ETHERNET_ARP)
    {

        /* Clean off the Ethernet header.  */
        packet_ptr -> nx_packet_prepend_ptr =  
            packet_ptr -> nx_packet_prepend_ptr + NX_DRIVER_ETHERNET_FRAME_SIZE;

        /* Adjust the packet length.  */
        packet_ptr -> nx_packet_length =  
            packet_ptr -> nx_packet_length - NX_DRIVER_ETHERNET_FRAME_SIZE;

        /* Route to the ARP receive function.  */
        _nx_arp_packet_deferred_receive(ip_ptr, packet_ptr);
    }
    else if (packet_type == NX_DRIVER_ETHERNET_RARP)
    {

        /* Clean off the Ethernet header.  */
        packet_ptr -> nx_packet_prepend_ptr =  
            packet_ptr -> nx_packet_prepend_ptr + NX_DRIVER_ETHERNET_FRAME_SIZE;

        /* Adjust the packet length.  */
        packet_ptr -> nx_packet_length =  
            packet_ptr -> nx_packet_length - NX_DRIVER_ETHERNET_FRAME_SIZE;

        /* Route to the RARP receive function.  */
        _nx_rarp_packet_deferred_receive(ip_ptr, packet_ptr);
    }
    else
    {
        /* Invalid ethernet header... release the packet.  */
        nx_packet_release(packet_ptr);
    }
}            


/****** DRIVER SPECIFIC ****** Start of part/vendor specific internal driver functions.  */


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_driver_hardware_initialize                      SAME54/IAR      */ 
/*                                                           5.0          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    Yuxin Zhou, Microsoft Corporation                                   */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes hardware-specific initialization.           */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    driver_req_ptr                        Driver request pointer        */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                [NX_SUCCESS|NX_DRIVER_ERROR]  */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    GMACD_Init                            Configure Ethernet            */ 
/*    GMAC_SetAddress                       Setup MAC address             */ 
/*    ETH_DMARxDescReceiveITConfig          Enable receive descriptors    */ 
/*    GMACB_Init                            Configure PHY instance        */ 
/*    GMACB_ResetPhy                        Reset PHY                     */ 
/*    GMACB_InitPhy                         Init PHY                      */ 
/*    NVIC_ClearPendingIRQ                  Clear interrupt               */ 
/*    NVIC_EnableIRQ                        Enable interrupt              */ 
/*    GMAC_TransmitEnable                   Transmit enable/disable       */ 
/*    GMAC_ReceiveEnable                    Receive enable/disable        */ 
/*    GMAC_SetTxQueue                       Configure TX Queue            */ 
/*    GMAC_SetRxQueue                       Configure RX Queue            */ 
/*    GMAC_SetDMAConfig                     Configure DMA                 */ 
/*    nx_packet_allocate                    Allocate receive packet(s)    */ 
/*                                                                        */
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_driver_initialize                 Driver initialize processing  */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-07-2015        Yuxin Zhou            Initial Version 5.0           */ 
/*                                                                        */ 
/**************************************************************************/ 
static UINT  _nx_driver_hardware_initialize(NX_IP_DRIVER *driver_req_ptr)
{

NX_PACKET              *packet_ptr;
UINT                    i;
struct mac_async_filter filter;


    //sGmacd *pGmacd = &nx_driver_information.gGmacd;
    //GMacb  *pGmacb = &nx_driver_information.gGmacb;


    /* Default to successful return.  */
    driver_req_ptr -> nx_ip_driver_status =  NX_SUCCESS;

    /* Setup indices.  */
    nx_driver_information.nx_driver_information_receive_current_index = 0; 
    nx_driver_information.nx_driver_information_transmit_current_index = 0; 
    nx_driver_information.nx_driver_information_transmit_release_index = 0;
    
    nx_driver_information.nx_driver_information_dma_rx_descriptors = nx_driver_information_dma_rx_descriptors_area;
    nx_driver_information.nx_driver_information_dma_tx_descriptors = nx_driver_information_dma_tx_descriptors_area;
    nx_driver_information.nx_driver_information_dummy_descriptor = &nx_driver_information_dummy_descriptor_area;

    /* Clear the number of buffers in use counter.  */
    nx_driver_information.nx_driver_information_number_of_transmit_buffers_in_use = 0;

    /* Make sure there are receive packets... otherwise, return an error.  */
    if (nx_driver_information.nx_driver_information_packet_pool_ptr == NULL)
    {
    
        /* There must be receive packets. If not, return an error!  */
        return(NX_DRIVER_ERROR);
    }

    //GMACD_Init(pGmacd, GMAC, ID_GMAC, GMAC_CAF_ENABLE, GMAC_NBC_DISABLE);
    memcpy(filter.mac, _nx_driver_hardware_address, sizeof(_nx_driver_hardware_address));
    filter.tid_enable = false;
    mac_async_set_filter(&MACIF, 0, &filter);

     /* Setup interrupts */  
    //NVIC_ClearPendingIRQ(GMAC_IRQn);
    //NVIC_EnableIRQ(GMAC_IRQn);

    /* Init GMACB driver */
    //GMACB_Init(pGmacb, pGmacd, BOARD_GMAC_PHY_ADDR);
    //GMACB_ResetPhy(pGmacb);

    /* PHY initialize */
    //if (!GMACB_InitPhy(pGmacb, BOARD_MCK,  0, 0,  gmacPins, PIO_LISTSIZE(gmacPins)))
    {
        //return(NX_DRIVER_ERROR);
    }
   
    /* Auto Negotiate, work in RMII mode */
    //if (!GMACB_AutoNegotiate(pGmacb))
    {
        //return(NX_DRIVER_ERROR);
    }

    /* Initialize TX Descriptors list: Ring Mode.  */
    mac_async_disable(&MACIF);
    
    /* Fill each DMATxDesc descriptor with the right values.  */   
    for(i = 0; i < NX_DRIVER_TX_DESCRIPTORS - 1; i++)
    {

        /* Initialize the descriptor.  */
        nx_driver_information.nx_driver_information_dma_tx_descriptors[i].status.val = GMAC_TX_USED_BIT;
    }
    
    nx_driver_information.nx_driver_information_dma_tx_descriptors[NX_DRIVER_TX_DESCRIPTORS - 1].status.val = GMAC_TX_USED_BIT | GMAC_TX_WRAP_BIT;
    
    nx_driver_information.nx_driver_information_dummy_descriptor -> status.val = GMAC_TX_USED_BIT | GMAC_TX_WRAP_BIT;

    /* Set Transmit Buffer Queue Pointer Register.  */
    hri_gmac_write_TBQB_reg(MACIF.dev.hw, (uint32_t) nx_driver_information.nx_driver_information_dma_tx_descriptors);
    
    /* Initialize RX Descriptors list: Ring Mode  */

    /* Fill each DMARxDesc descriptor with the right values */  
    for(i = 0; i < NX_DRIVER_RX_DESCRIPTORS; i++)
    {

        /* Allocate a packet for the receive buffers.  */
        if (nx_packet_allocate(nx_driver_information.nx_driver_information_packet_pool_ptr, &packet_ptr, 
                               NX_RECEIVE_PACKET, NX_NO_WAIT) == NX_SUCCESS)
        {

            nx_driver_information.nx_driver_information_dma_rx_descriptors[i].addr.val = 
                                        (ULONG)packet_ptr -> nx_packet_prepend_ptr & GMAC_ADDRESS_MASK;

            /* Adjust the packet.  */
            packet_ptr -> nx_packet_prepend_ptr += 2;

            /* Remember the receive packet poitner.  */
            nx_driver_information.nx_driver_information_receive_packets[i] =  packet_ptr;

            /* Set Own bit of the RX descriptor Status.  */
            nx_driver_information.nx_driver_information_dma_rx_descriptors[i].status.val = 0;       
        }
        else
        {

            /* Cannot allocate packets from the packet pool. */
            return(NX_DRIVER_ERROR);
        }

    }
    nx_driver_information.nx_driver_information_dma_rx_descriptors[NX_DRIVER_RX_DESCRIPTORS - 1].addr.val |= GMAC_RX_WRAP_BIT;

    /* Save the size of one rx buffer.  */
    nx_driver_information.nx_driver_information_rx_buffer_size = packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_data_start;
        
    /* Clear the number of buffers in use counter.  */
    nx_driver_information.nx_driver_information_multicast_count = 0;

    /* Set Receive Buffer Queue Pointer Register.  */
    hri_gmac_write_RBQB_reg(MACIF.dev.hw, (uint32_t)nx_driver_information.nx_driver_information_dma_rx_descriptors);

    /* Configure DMA options.  */
    hri_gmac_write_DCFGR_reg(MACIF.dev.hw, GMAC_DCFGR_FBLDO(16) | 
                                                        GMAC_DCFGR_RXBMS(3) | 
                                                        GMAC_DCFGR_TXPBMS |
                                                        GMAC_DCFGR_TXCOEN |
                                                        GMAC_DCFGR_DRBS(nx_driver_information.nx_driver_information_rx_buffer_size / 64));

    /* Return success!  */
    return(NX_SUCCESS);
} 


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_driver_hardware_enable                          SAME54/IAR      */ 
/*                                                           5.0          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    Yuxin Zhou, Microsoft Corporation                                   */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes hardware-specific link enable requests.     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    driver_req_ptr                        Driver request pointer        */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                [NX_SUCCESS|NX_DRIVER_ERROR]  */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    GMAC_TransmitEnable                   Start Ethernet transmition    */ 
/*    GMAC_ReceiveEnable                    Start Ethernet reception      */ 
/*    GMAC_EnableIt                         Enable Ethernet interrupt     */ 
/*                                                                        */
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_driver_enable                     Driver link enable processing */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-07-2015        Yuxin Zhou            Initial Version 5.0           */ 
/*                                                                        */ 
/**************************************************************************/ 
static UINT  _nx_driver_hardware_enable(NX_IP_DRIVER *driver_req_ptr)
{

    /* Enable transmit and receive.  */
    mac_async_enable(&MACIF);
//    /* Enable interrupt.  */

    hri_gmac_set_IMR_RCOMP_bit(MACIF.dev.hw);
    hri_gmac_set_IMR_TCOMP_bit(MACIF.dev.hw);
    /* Return success!  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_driver_hardware_disable                         SAME54/IAR      */ 
/*                                                           5.0          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    Yuxin Zhou, Microsoft Corporation                                   */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes hardware-specific link disable requests.    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    driver_req_ptr                        Driver request pointer        */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                [NX_SUCCESS|NX_DRIVER_ERROR]  */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_driver_disable                    Driver link disable processing*/ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-07-2015        Yuxin Zhou            Initial Version 5.0           */ 
/*                                                                        */ 
/**************************************************************************/ 
static UINT  _nx_driver_hardware_disable(NX_IP_DRIVER *driver_req_ptr)
{


    /* Return success!  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_driver_hardware_packet_send                     SAME54/IAR      */ 
/*                                                           5.0          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    Yuxin Zhou, Microsoft Corporation                                   */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes hardware-specific packet send requests.     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    packet_ptr                            Pointer to packet to send     */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                [NX_SUCCESS|NX_DRIVER_ERROR]  */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    [_nx_driver_transmit_packet_enqueue]  Optional internal transmit    */ 
/*                                            packet queue routine        */ 
/*                                                                        */
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_driver_packet_send                Driver packet send processing */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-07-2015        Yuxin Zhou            Initial Version 5.0           */ 
/*                                                                        */ 
/**************************************************************************/ 
static UINT  _nx_driver_hardware_packet_send(NX_PACKET *packet_ptr)
{

ULONG           curIdx;
#ifndef NX_DISABLE_PACKET_CHAIN
NX_PACKET       *pktIdx;
#endif /* NX_DISABLE_PACKET_CHAIN */
ULONG            bd_count = 0;


    /* Pick up the first BD. */
    curIdx = nx_driver_information.nx_driver_information_transmit_current_index;

    /* Check if it is a free descriptor.  */
    if ((nx_driver_information.nx_driver_information_dma_tx_descriptors[curIdx].status.bm.bUsed == 0) || nx_driver_information.nx_driver_information_transmit_packets[curIdx])
    {
        
        /* Buffer is still owned by device.  */
        return(NX_DRIVER_ERROR);
    }
    
    /* Find the Buffer, set the Buffer pointer.  */
    nx_driver_information.nx_driver_information_dma_tx_descriptors[curIdx].addr = (ULONG)packet_ptr->nx_packet_prepend_ptr;
    
    /* Set the buffer size.  */
    nx_driver_information.nx_driver_information_dma_tx_descriptors[curIdx].status.bm.len = packet_ptr -> nx_packet_append_ptr - packet_ptr->nx_packet_prepend_ptr;
    
    /* Clear the first Descriptor's LS bit.  */
    nx_driver_information.nx_driver_information_dma_tx_descriptors[curIdx].status.bm.bLastBuffer = 0;
    
    /* Save the packet pointer to release.  */
    nx_driver_information.nx_driver_information_transmit_packets[curIdx] = packet_ptr;


#ifndef NX_DISABLE_PACKET_CHAIN
    /* Find next packet.  */
    for (pktIdx = packet_ptr -> nx_packet_next;
         pktIdx != NX_NULL;
         pktIdx = pktIdx -> nx_packet_next)
    {
        
        /* Move to next descriptor.  */
        curIdx = (curIdx + 1) & (NX_DRIVER_TX_DESCRIPTORS - 1);
    
        /* Check if it is a free descriptor.  */
        if ((nx_driver_information.nx_driver_information_dma_tx_descriptors[curIdx].status.bm.bUsed == 0) || nx_driver_information.nx_driver_information_transmit_packets[curIdx])
        {
            
            /* No more descriptor available, return driver error status.  */
            return(NX_DRIVER_ERROR);
        }
        
        /* Set the buffer pointer.  */
        nx_driver_information.nx_driver_information_dma_tx_descriptors[curIdx].addr = (ULONG)pktIdx -> nx_packet_prepend_ptr;
        
        /* Set the buffer size.  */
        nx_driver_information.nx_driver_information_dma_tx_descriptors[curIdx].status.bm.len = pktIdx -> nx_packet_append_ptr - pktIdx -> nx_packet_prepend_ptr; 
        
        /* Clear the descriptor's FS & LS bit.  */
        nx_driver_information.nx_driver_information_dma_tx_descriptors[curIdx].status.bm.bLastBuffer = 0;

        /* Increment the BD count.  */
        bd_count++;

    }
#endif /* NX_DISABLE_PACKET_CHAIN */
    
    /* Set the last Descriptor's LS & IC & OWN bit.  */
    nx_driver_information.nx_driver_information_dma_tx_descriptors[curIdx].status.bm.bLastBuffer = 1;
    nx_driver_information.nx_driver_information_dma_tx_descriptors[curIdx].status.bm.bUsed = 0;
    
    /* Set the current index to the next descriptor.  */
    nx_driver_information.nx_driver_information_transmit_current_index = (curIdx + 1) & (NX_DRIVER_TX_DESCRIPTORS - 1);

    /* Increment the transmit buffers in use count.  */
    nx_driver_information.nx_driver_information_number_of_transmit_buffers_in_use += bd_count + 1;
    
    /* Set OWN bit to indicate BDs are ready.  */
    for (; bd_count > 0; bd_count--)
    {
        
        /* Set OWN bit in reverse order, move to prevous BD.  */
        curIdx = (curIdx - 1) & (NX_DRIVER_TX_DESCRIPTORS - 1);
        
        /* Set this BD's OWN bit.  */
        nx_driver_information.nx_driver_information_dma_tx_descriptors[curIdx].status.bm.bUsed = 0;
    }

    __DSB();
    
    hri_gmac_set_NCR_reg(MACIF.dev.hw, GMAC_NCR_TSTART);
    
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_driver_hardware_multicast_join                  SAME54/IAR      */ 
/*                                                           5.0          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    Yuxin Zhou, Microsoft Corporation                                   */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes hardware-specific multicast join requests.  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    driver_req_ptr                        Driver request pointer        */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                [NX_SUCCESS|NX_DRIVER_ERROR]  */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_driver_multicast_join             Driver multicast join         */ 
/*                                            processing                  */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-07-2015        Yuxin Zhou            Initial Version 5.0           */ 
/*                                                                        */ 
/**************************************************************************/ 
static UINT  _nx_driver_hardware_multicast_join(NX_IP_DRIVER *driver_req_ptr)
{


    /* Increase the multicast count.  */
    nx_driver_information.nx_driver_information_multicast_count++;
    
    /* Return success.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_driver_hardware_multicast_leave                 SAME54/IAR      */ 
/*                                                           5.0          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    Yuxin Zhou, Microsoft Corporation                                   */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes hardware-specific multicast leave requests. */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    driver_req_ptr                        Driver request pointer        */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                [NX_SUCCESS|NX_DRIVER_ERROR]  */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_driver_multicast_leave            Driver multicast leave        */ 
/*                                            processing                  */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-07-2015        Yuxin Zhou            Initial Version 5.0           */ 
/*                                                                        */ 
/**************************************************************************/ 
static UINT  _nx_driver_hardware_multicast_leave(NX_IP_DRIVER *driver_req_ptr)
{
    
    /* Decrease the multicast count.  */
    nx_driver_information.nx_driver_information_multicast_count--;
    
    /* If multicast count reachs zero, disable multicast frame reception.  */
    if (nx_driver_information.nx_driver_information_multicast_count == 0)
    {

    }
    
    /* Return success.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_driver_hardware_get_status                      SAME54/IAR      */ 
/*                                                           5.0          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    Yuxin Zhou, Microsoft Corporation                                   */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes hardware-specific get status requests.      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    driver_req_ptr                        Driver request pointer        */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                [NX_SUCCESS|NX_DRIVER_ERROR]  */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_driver_get_status                 Driver get status processing  */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-07-2015        Yuxin Zhou            Initial Version 5.0           */ 
/*                                                                        */ 
/**************************************************************************/ 
static UINT  _nx_driver_hardware_get_status(NX_IP_DRIVER *driver_req_ptr)
{

    /* Return success.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_driver_hardware_packet_transmitted              SAME54/IAR      */ 
/*                                                           5.0          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    Yuxin Zhou, Microsoft Corporation                                   */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes packets transmitted by the ethernet         */ 
/*    controller.                                                         */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    None                                                                */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_transmit_release            Release transmitted packet    */ 
/*    [_nx_driver_transmit_packet_dequeue]  Optional transmit packet      */ 
/*                                            dequeue                     */ 
/*                                                                        */
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_driver_deferred_processing        Deferred driver processing    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-07-2015        Yuxin Zhou            Initial Version 5.0           */ 
/*                                                                        */ 
/**************************************************************************/ 
static VOID  _nx_driver_hardware_packet_transmitted(VOID)
{

ULONG numOfBuf =  nx_driver_information.nx_driver_information_number_of_transmit_buffers_in_use;
ULONG idx =       nx_driver_information.nx_driver_information_transmit_release_index;
    
    
    /* Loop through buffers in use.  */
    while (numOfBuf--)
    {

        /* If no packet, just examine the next packet.  */
        if (nx_driver_information.nx_driver_information_transmit_packets[idx] == NX_NULL) 
        {

            /* Set this BD's Used bit.  */
            nx_driver_information.nx_driver_information_dma_tx_descriptors[idx].status.bm.bUsed = 1;

            /* No packet in use, skip to next.  */
            idx = (idx + 1) & (NX_DRIVER_TX_DESCRIPTORS - 1);
            continue;
        }

        /* Determine if the packet has been transmitted.  */
        if (nx_driver_information.nx_driver_information_dma_tx_descriptors[idx].status.bm.bUsed == 1)
        {

            /* Yes, packet has been transmitted.  */
            
            /* Remove the Ethernet header and release the packet.  */
            NX_DRIVER_ETHERNET_HEADER_REMOVE(nx_driver_information.nx_driver_information_transmit_packets[idx]);

            /* Release the packet.  */
            nx_packet_transmit_release(nx_driver_information.nx_driver_information_transmit_packets[idx]);

            /* Clear the entry in the in-use array.  */
            nx_driver_information.nx_driver_information_transmit_packets[idx] = NX_NULL;

            /* Update the transmit relesae index and number of buffers in use.  */
            idx = (idx + 1) & (NX_DRIVER_TX_DESCRIPTORS - 1);
            nx_driver_information.nx_driver_information_number_of_transmit_buffers_in_use = numOfBuf;
            nx_driver_information.nx_driver_information_transmit_release_index = idx;
        }
        else
        {

            /* Get out of the loop!  */
            break;
        }
    }

}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_driver_hardware_packet_received                 SAME54/IAR      */ 
/*                                                           5.0          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    Yuxin Zhou, Microsoft Corporation                                   */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes packets received by the ethernet            */ 
/*    controller.                                                         */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    None                                                                */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_driver_transfer_to_netx           Transfer packet to NetX       */ 
/*    nx_packet_allocate                    Allocate receive packets      */ 
/*    _nx_packet_release                    Release receive packets       */
/*                                                                        */
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_driver_deferred_processing        Deferred driver processing    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-07-2015        Yuxin Zhou            Initial Version 5.0           */ 
/*                                                                        */ 
/**************************************************************************/ 
static VOID  _nx_driver_hardware_packet_received(VOID)
{

NX_PACKET     *packet_ptr;
ULONG          bd_count = 0;
INT            i;
ULONG          idx;
ULONG          temp_idx;
ULONG          first_idx = nx_driver_information.nx_driver_information_receive_current_index;
NX_PACKET     *received_packet_ptr = nx_driver_information.nx_driver_information_receive_packets[first_idx];


    /* Find out the BDs that owned by CPU.  */
    for (first_idx = idx = nx_driver_information.nx_driver_information_receive_current_index;
         nx_driver_information.nx_driver_information_dma_rx_descriptors[idx].addr.bm.bOwnership;
         idx = (idx + 1) & (NX_DRIVER_RX_DESCRIPTORS - 1))
    {
        
        /* Is the BD marked as the end of a frame?  */
        if (nx_driver_information.nx_driver_information_dma_rx_descriptors[idx].status.bm.bEof)
        {

#ifndef NX_DISABLE_PACKET_CHAIN
            /* Yes, this BD is the last BD in the frame, set the last NX_PACKET's nx_packet_next to NULL.  */
            nx_driver_information.nx_driver_information_receive_packets[idx] -> nx_packet_next = NX_NULL;
#endif /* NX_DISABLE_PACKET_CHAIN */
            
            /* Store the length of the packet in the first NX_PACKET.  */
            nx_driver_information.nx_driver_information_receive_packets[first_idx] -> nx_packet_length = nx_driver_information.nx_driver_information_dma_rx_descriptors[idx].status.bm.len;

            /* Adjust nx_packet_append_ptr with the size of the data in this buffer.  */
            nx_driver_information.nx_driver_information_receive_packets[idx] -> nx_packet_append_ptr = nx_driver_information.nx_driver_information_receive_packets[idx] -> nx_packet_prepend_ptr
                                                                                                     + nx_driver_information.nx_driver_information_receive_packets[first_idx] -> nx_packet_length
                                                                                                     - bd_count * nx_driver_information.nx_driver_information_rx_buffer_size;
            /* Is there only one BD for the current frame?  */
            if (idx != first_idx)
            {
                
                /* No, this BD is not the first BD of the frame, frame data starts at the aligned address.  */
                nx_driver_information.nx_driver_information_receive_packets[idx] -> nx_packet_prepend_ptr -= 2;
            }

#ifdef NX_DISABLE_PACKET_CHAIN
            if (bd_count)
            {

                /* Free up the BD since chained packet is disabled. */
                for (i = bd_count; i >= 0; i--)
                {
                    
                    /* Free up the BD to ready state. */
                    temp_idx = (first_idx + i) & (NX_DRIVER_RX_DESCRIPTORS - 1);
                    nx_driver_information.nx_driver_information_dma_rx_descriptors[temp_idx].addr.bm.bOwnership = 0;
                    nx_driver_information.nx_driver_information_receive_packets[temp_idx] -> nx_packet_prepend_ptr = nx_driver_information.nx_driver_information_receive_packets[temp_idx] -> nx_packet_data_start + 2;
                } 
            }
            else
#endif /* NX_DISABLE_PACKET_CHAIN */
            {

                /* Allocate new NX_PACKETs for BDs.  */
                for (i = bd_count; i >= 0; i--)
                {

                    temp_idx = (first_idx + i) & (NX_DRIVER_RX_DESCRIPTORS - 1);

                    /* Allocate a new packet from the packet pool.  */
                    if (nx_packet_allocate(nx_driver_information.nx_driver_information_packet_pool_ptr, &packet_ptr, 
                                              NX_RECEIVE_PACKET, NX_NO_WAIT) == NX_SUCCESS)
                    {

                        nx_driver_information.nx_driver_information_dma_rx_descriptors[temp_idx].addr.bm.addrDW = (ULONG)packet_ptr->nx_packet_prepend_ptr >> 2;
                        nx_driver_information.nx_driver_information_dma_rx_descriptors[temp_idx].addr.bm.bOwnership = 0;
                        /* Adjust the new packet and assign it to the BD.  */
                        packet_ptr -> nx_packet_prepend_ptr += 2;
                        nx_driver_information.nx_driver_information_receive_packets[temp_idx] = packet_ptr;
                    }
                    else
                    {
                        
                        /* Allocation failed, get out of the loop.  */
                        break;
                    }
                }
                
                if (i >= 0)
                {
                    
#ifndef NX_DISABLE_PACKET_CHAIN
                    /* At least one packet allocation was failed, release the received packet.  */
                    nx_packet_release(nx_driver_information.nx_driver_information_receive_packets[temp_idx] -> nx_packet_next);
#endif /* NX_DISABLE_PACKET_CHAIN */
                    
                    for (; i >= 0; i--)
                    {
                        
                        /* Free up the BD to ready state. */
                        temp_idx = (first_idx + i) & (NX_DRIVER_RX_DESCRIPTORS - 1);
                        nx_driver_information.nx_driver_information_dma_rx_descriptors[temp_idx].addr.bm.bOwnership = 0;
                        nx_driver_information.nx_driver_information_receive_packets[temp_idx] -> nx_packet_prepend_ptr = nx_driver_information.nx_driver_information_receive_packets[temp_idx] -> nx_packet_data_start + 2;
                    }
                }
                else
                {
                    
                    /* Everything is OK, transfer the packet to NetX.  */
                    _nx_driver_transfer_to_netx(nx_driver_information.nx_driver_information_ip_ptr, received_packet_ptr);
                }
            }
            
            /* Set the first BD index for the next packet.  */
            first_idx = (idx + 1) & (NX_DRIVER_RX_DESCRIPTORS - 1);
            
            /* Update the current receive index.  */
            nx_driver_information.nx_driver_information_receive_current_index = first_idx;
            
            received_packet_ptr = nx_driver_information.nx_driver_information_receive_packets[first_idx];
            
            bd_count = 0;

        }
        else
        {
            
#ifndef NX_DISABLE_PACKET_CHAIN
            /* This BD is not the last BD of a frame. It is a intermediate descriptor.  */
            
            nx_driver_information.nx_driver_information_receive_packets[idx] -> nx_packet_next = nx_driver_information.nx_driver_information_receive_packets[(idx + 1) & (NX_DRIVER_RX_DESCRIPTORS - 1)];
            
            nx_driver_information.nx_driver_information_receive_packets[idx] -> nx_packet_append_ptr = nx_driver_information.nx_driver_information_receive_packets[idx] -> nx_packet_data_end;
            
            if (idx != first_idx)
            {
                
                nx_driver_information.nx_driver_information_receive_packets[idx] -> nx_packet_prepend_ptr = nx_driver_information.nx_driver_information_receive_packets[idx] -> nx_packet_data_start;
            }
#endif /* NX_DISABLE_PACKET_CHAIN */
            
            bd_count++;
        }
    }
}


#ifdef NX_ENABLE_INTERFACE_CAPABILITY
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_driver_hardware_capability_set                  SAME54/IAR      */  
/*                                                           5.0          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    Yuxin Zhou, Microsoft Corporation                                   */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes hardware-specific capability set requests.  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    driver_req_ptr                         Driver request pointer       */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                [NX_SUCCESS|NX_DRIVER_ERROR]  */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_driver_capability_set             Capability set processing     */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-07-2015        Yuxin Zhou            Initial Version 5.0           */ 
/*                                                                        */ 
/**************************************************************************/ 
static UINT _nx_driver_hardware_capability_set(NX_IP_DRIVER *driver_req_ptr)
{
    
    return NX_SUCCESS;
}
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    nx_driver_same54_ethernet_isr                       SAME54/IAR      */ 
/*                                                           5.0          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    Yuxin Zhou, Microsoft Corporation                                   */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processing incoming packets.  This routine is         */
/*    be called from the receive packet ISR and assumes that the          */ 
/*    interrupt is saved/restored around the call by ThreadX.             */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    None                                                                */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ip_driver_deferred_processing     IP receive packet processing  */ 
/*    GMAC_GetItStatus                      Get and clear MAC Interrupt   */ 
/*                                                                        */
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    ISR                                                                 */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-07-2015        Yuxin Zhou            Initial Version 5.0           */ 
/*                                                                        */ 
/**************************************************************************/ 
VOID  nx_driver_same54_ethernet_isr(VOID)
{
ULONG status;
#ifdef NX_DRIVER_ENABLE_DEFERRED
ULONG deffered_events;
#endif /* NX_DRIVER_ENABLE_DEFERRED */


    status = hri_gmac_read_ISR_reg(MACIF.dev.hw);
    
#ifdef NX_DRIVER_ENABLE_DEFERRED
    deffered_events = nx_driver_information.nx_driver_information_deferred_events;
#endif /* NX_DRIVER_ENABLE_DEFERRED */

    /* Interrupt on transmit completion.  */
    if(status & GMAC_ISR_TCOMP)
    {
#ifdef NX_DRIVER_ENABLE_DEFERRED
        /* Set the transmit complete bit.  */
        nx_driver_information.nx_driver_information_deferred_events |= NX_DRIVER_DEFERRED_PACKET_TRANSMITTED;
#else
        /* Process transmitted packet(s).  */
        _nx_driver_hardware_packet_transmitted();
#endif /* NX_DRIVER_ENABLE_DEFERRED */
    }
    /* Receive packet interrupt.  */
    if(status & GMAC_ISR_RCOMP)
    {
#ifdef NX_DRIVER_ENABLE_DEFERRED

        /* Set the receive packet interrupt.  */
        nx_driver_information.nx_driver_information_deferred_events |= NX_DRIVER_DEFERRED_PACKET_RECEIVED;
#else
        /* Process received packet(s).  */
        _nx_driver_hardware_packet_received();
#endif /* NX_DRIVER_ENABLE_DEFERRED */
    }


#ifdef NX_DRIVER_ENABLE_DEFERRED
    if (!deffered_events)
    {
        
        /* Call NetX deferred driver processing.  */        
        _nx_ip_driver_deferred_processing(nx_driver_information.nx_driver_information_ip_ptr);
    }
#endif /* NX_DRIVER_ENABLE_DEFERRED */

}


/****** DRIVER SPECIFIC ****** Start of part/vendor specific internal driver functions.  */

