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

#ifndef NX_DRIVER_RX_FIT_H
#define NX_DRIVER_RX_FIT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Include ThreadX header file, if not already.  */

#ifndef TX_API_H
#include "tx_api.h"
#endif


/* Include NetX header file, if not already.  */

#ifndef NX_API_H
#include "nx_api.h"
#endif


/* Define custom error code for unrecoverable driver errors. */
#define NX_DRIVER_ERROR (99U)
#define NX_DRIVER_BUSY (100U)

/* Define global driver entry function. */
VOID nx_driver_rx_fit(NX_IP_DRIVER *driver_req_ptr);

#ifdef __cplusplus
}
#endif

#endif // #ifndef NX_DRIVER_RX_FIT_H

