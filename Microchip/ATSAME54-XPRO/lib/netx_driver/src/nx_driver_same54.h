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
/**   Ethernet driver for ATMEL SAME54 family of microprocessors          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#ifndef NX_DRIVER_SAME54_H
#define NX_DRIVER_SAME54_H


#ifdef   __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {
#endif


/* Include ThreadX header file, if not already.  */  
  
#ifndef TX_API_H
#include "tx_api.h"
#endif


/* Include NetX header file, if not already.  */

#ifndef NX_API_H
#include "nx_api.h"
#endif


/* Determine if the driver's source file is being compiled. The constants and typdefs are only valid within
   the driver's source file compilation.  */

#ifdef NX_DRIVER_SOURCE


/****** DRIVER SPECIFIC ****** Start of part/vendor specific include area.  Include any such files here!  */

#include <same54.h>
#include "hal_mac_async.h"
    
/****** DRIVER SPECIFIC ****** End of part/vendor specific include file area!  */


/* Define generic constants and macros for all NetX Ethernet drivers.  */

#define NX_DRIVER_ETHERNET_IP                   0x0800
#define NX_DRIVER_ETHERNET_IPV6                 0x86dd
#define NX_DRIVER_ETHERNET_ARP                  0x0806
#define NX_DRIVER_ETHERNET_RARP                 0x8035

#define NX_DRIVER_ETHERNET_MTU                  1514
#define NX_DRIVER_ETHERNET_FRAME_SIZE           14

#define NX_DRIVER_DEFERRED_PACKET_RECEIVED      1
#define NX_DRIVER_DEFERRED_DEVICE_RESET         2
#define NX_DRIVER_DEFERRED_PACKET_TRANSMITTED   4

#define NX_DRIVER_STATE_NOT_INITIALIZED         1
#define NX_DRIVER_STATE_INITIALIZE_FAILED       2
#define NX_DRIVER_STATE_INITIALIZED             3    
#define NX_DRIVER_STATE_LINK_ENABLED            4    

#ifdef NX_DRIVER_INTERNAL_TRANSMIT_QUEUE
#define NX_DRIVER_MAX_TRANSMIT_QUEUE_DEPTH      10
#endif

#define NX_DRIVER_ERROR                         90


#define NX_DRIVER_ETHERNET_HEADER_REMOVE(p)   \
{   \
    p -> nx_packet_prepend_ptr =  p -> nx_packet_prepend_ptr + NX_DRIVER_ETHERNET_FRAME_SIZE;  \
    p -> nx_packet_length =  p -> nx_packet_length - NX_DRIVER_ETHERNET_FRAME_SIZE;    \
}    


/****** DRIVER SPECIFIC ****** Start of part/vendor specific constants area.  Include any such constants here!  */
#define GMAC_CAF_DISABLE  0
#define GMAC_CAF_ENABLE   1
#define GMAC_NBC_DISABLE  0
#define GMAC_NBC_ENABLE   1

/// The buffer addresses written into the descriptors must be aligned so the
/// last few bits are zero.  These bits have special meaning for the GMAC
/// peripheral and cannot be used as part of the address.
#define GMAC_ADDRESS_MASK   ((unsigned int)0xFFFFFFFC)
#define GMAC_LENGTH_FRAME   ((unsigned int)0x3FFF)    /// Length of frame mask

// receive buffer descriptor bits
#define GMAC_RX_OWNERSHIP_BIT   (1u <<  0)
#define GMAC_RX_WRAP_BIT        (1u <<  1)
#define GMAC_RX_SOF_BIT         (1u << 14)
#define GMAC_RX_EOF_BIT         (1u << 15)

// Transmit buffer descriptor bits
#define GMAC_TX_LAST_BUFFER_BIT (1u << 15)
#define GMAC_TX_WRAP_BIT        (1u << 30)
#define GMAC_TX_USED_BIT        (1u << 31)
#define GMAC_TX_RLE_BIT         (1u << 29) /// Retry Limit Exceeded
#define GMAC_TX_UND_BIT         (1u << 28) /// Tx Buffer Under-run
#define GMAC_TX_ERR_BIT         (1u << 27) /// Exhausted in mid-frame
#define GMAC_TX_ERR_BITS  \
	(GMAC_TX_RLE_BIT | GMAC_TX_UND_BIT | GMAC_TX_ERR_BIT)

// Interrupt bits
#define GMAC_INT_RX_BITS  \
	(GMAC_IER_RCOMP | GMAC_IER_RXUBR | GMAC_IER_ROVR)
#define GMAC_INT_TX_ERR_BITS  \
	(GMAC_IER_TUR | GMAC_IER_RLEX | GMAC_IER_TFC | GMAC_IER_HRESP)
#define GMAC_INT_TX_BITS  \
	(GMAC_INT_TX_ERR_BITS | GMAC_IER_TCOMP)
// Interrupt Status bits
#define GMAC_INT_RX_STATUS_BITS  \
	(GMAC_ISR_RCOMP | GMAC_ISR_RXUBR | GMAC_ISR_ROVR)
#define GMAC_INT_TX_STATUS_ERR_BITS  \
	(GMAC_ISR_TUR | GMAC_ISR_RLEX | GMAC_ISR_TFC | GMAC_ISR_HRESP)
        
        
/** Receive buffer descriptor struct */
typedef struct _GmacRxDescriptor {
	union _GmacRxAddr {
		uint32_t val;
		struct _GmacRxAddrBM {
			uint32_t bOwnership:1,  /**< User clear, GMAC set this to one once
										 it has successfully written a frame to
										 memory */
					 bWrap:1,       /**< Marks last descriptor in receive buffer */
					 addrDW:30;     /**< Address in number of DW */
		} bm;
	} addr;                    /**< Address, Wrap & Ownership */
	union _GmacRxStatus {
		uint32_t val;
		struct _GmacRxStatusBM {
			uint32_t len:12,                /** Length of frame including FCS */
					 offset:2,              /** Receive buffer offset,
												bits 13:12 of frame length for jumbo
												frame */
					 bSof:1,                /** Start of frame */
					 bEof:1,                /** End of frame */
					 bCFI:1,                /** Concatenation Format Indicator */
					 vlanPriority:3,        /** VLAN priority (if VLAN detected) */
					 bPriorityDetected:1,   /** Priority tag detected */
					 bVlanDetected:1,       /**< VLAN tag detected */
					 bTypeIDMatch:1,        /**< Type ID match */
					 bAddr4Match:1,         /**< Address register 4 match */
					 bAddr3Match:1,         /**< Address register 3 match */
					 bAddr2Match:1,         /**< Address register 2 match */
					 bAddr1Match:1,         /**< Address register 1 match */
					 reserved:1,
					 bExtAddrMatch:1,       /**< External address match */
					 bUniHashMatch:1,       /**< Unicast hash match */
					 bMultiHashMatch:1,     /**< Multicast hash match */
					 bBroadcastDetected:1;  /**< Global all ones broadcast
												 address detected */
		} bm;
	} status;
} sGmacRxDescriptor ;    /* GCC */

/** Transmit buffer descriptor struct */
typedef struct _GmacTxDescriptor {
	uint32_t addr;
	union _GmacTxStatus {
		uint32_t val;
		struct _GmacTxStatusBM {
			uint32_t len:11,        /**< Length of buffer */
					 reserved:4,
					 bLastBuffer:1, /**< Last buffer (in the current frame) */
					 bNoCRC:1,      /**< No CRC */
					 reserved1:10,
					 bExhausted:1,  /**< Buffer exhausted in mid frame */
					 bUnderrun:1,   /**< Transmit under run */
					 bError:1,      /**< Retry limit exceeded, error detected */
					 bWrap:1,       /**< Marks last descriptor in TD list */
					 bUsed:1;       /**< User clear, GMAC sets this once a frame
										 has been successfully transmitted */
		} bm;
	} status;
} sGmacTxDescriptor;     /* GCC */


extern struct mac_async_descriptor MACIF;

/* Define the number of descriptors and attached packets for transmit and receive operations. */

#ifndef NX_DRIVER_TX_DESCRIPTORS   
#define NX_DRIVER_TX_DESCRIPTORS   64   
#endif

#ifndef NX_DRIVER_RX_DESCRIPTORS
#define NX_DRIVER_RX_DESCRIPTORS   32
#endif

/* ETHERNET DMA Rx descriptors Frame Length Shift */

#define ETH_DMARXDESC_FRAME_LENGTHSHIFT           16
    

    
#define NX_DRIVER_CAPABILITY ( NX_INTERFACE_CAPABILITY_IPV4_TX_CHECKSUM   | \
                               NX_INTERFACE_CAPABILITY_IPV4_RX_CHECKSUM   | \
                               NX_INTERFACE_CAPABILITY_TCP_TX_CHECKSUM    | \
                               NX_INTERFACE_CAPABILITY_TCP_RX_CHECKSUM    | \
                               NX_INTERFACE_CAPABILITY_UDP_TX_CHECKSUM    | \
                               NX_INTERFACE_CAPABILITY_UDP_RX_CHECKSUM)    

/****** DRIVER SPECIFIC ****** End of part/vendor specific constant area!  */


/* Define basic Ethernet driver information typedef. Note that this typedefs is designed to be used only
   in the driver's C file. */

typedef struct NX_DRIVER_INFORMATION_STRUCT
{
    /* NetX IP instance that this driver is attached to.  */
    NX_IP               *nx_driver_information_ip_ptr;            

    /* Driver's current state.  */
    ULONG               nx_driver_information_state ;  

    /* Packet pool used for receiving packets. */
    NX_PACKET_POOL      *nx_driver_information_packet_pool_ptr;        

    /* Define the driver interface association.  */
    NX_INTERFACE        *nx_driver_information_interface;

    /* Define the deferred event field. This will contain bits representing events 
       deferred from the ISR for processing in the thread context.  */
    ULONG               nx_driver_information_deferred_events;


    /****** DRIVER SPECIFIC ****** Start of part/vendor specific driver information area.  Include any such constants here!  */

    /* The GMAC driver instance */
    //sGmacd gGmacd;
    
    /* The GMACB driver instance */
    //GMacb gGmacb;

    /* Indices to current receive/transmit descriptors.  */
    UINT                nx_driver_information_receive_current_index;          
    UINT                nx_driver_information_transmit_current_index; 
    
    /* Transmit release index.  */
    UINT                nx_driver_information_transmit_release_index;
    
    /* Define the number of transmit buffers in use.  */
    UINT                nx_driver_information_number_of_transmit_buffers_in_use;

    /* Define the Ethernet RX & TX DMA Descriptors.  */
    sGmacRxDescriptor  *nx_driver_information_dma_rx_descriptors;
    sGmacTxDescriptor  *nx_driver_information_dma_tx_descriptors; 
    sGmacTxDescriptor  *nx_driver_information_dummy_descriptor;
    
    /* Define the association between buffer descriptors and NetX packets.  */
    NX_PACKET           *nx_driver_information_transmit_packets[NX_DRIVER_TX_DESCRIPTORS];
    NX_PACKET           *nx_driver_information_receive_packets[NX_DRIVER_RX_DESCRIPTORS];
    
    /* Define the size of a rx buffer size.  */
    ULONG               nx_driver_information_rx_buffer_size;

    ULONG               nx_driver_information_multicast_count;
    
#ifdef NX_DRIVER_INTERNAL_TRANSMIT_QUEUE

    /* Define the parameters in the internal driver transmit queue.  The queue is maintained as a singularly
       linked-list with head and tail pointers.  The maximum number of packets on the queue is regulated by 
       the constant NX_DRIVER_MAX_TRANSMIT_QUEUE_DEPTH, which is defined above. When this number is reached,
       the oldest packet is discarded after the new packet is queued.  */
    ULONG               nx_driver_transmit_packets_queued;
    NX_PACKET           nx_driver_transmit_queue_head;
    NX_PACKET           nx_driver_transmit_queue_tail;
#endif /* NX_DRIVER_INTERNAL_TRANSMIT_QUEUE */

    /****** DRIVER SPECIFIC ****** End of part/vendor specific driver information area.  */

}   NX_DRIVER_INFORMATION;

#endif


/****** DRIVER SPECIFIC ****** Start of part/vendor specific external function prototypes.  A typical NetX Ethernet driver
                               should expose its entry function as well as its interrupt handling function(s) here. All other
                               functions in the driver should have local scope, i.e., defined as static.  */
    
/* Define global driver entry function. */

VOID  nx_driver_same54(NX_IP_DRIVER *driver_req_ptr);

/* Define global driver interrupt dispatch function.  */

VOID  nx_driver_same54_ethernet_isr(void);

/****** DRIVER SPECIFIC ****** End of part/vendor specific external function prototypes.  */


#ifdef   __cplusplus
/* Yes, C++ compiler is present.  Use standard C.  */
    }
#endif
#endif

