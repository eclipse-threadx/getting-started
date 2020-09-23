@/**************************************************************************/
@/*                                                                        */
@/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
@/*                                                                        */
@/*       This software is licensed under the Microsoft Software License   */
@/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
@/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
@/*       and in the root directory of this software.                      */
@/*                                                                        */
@/**************************************************************************/
@
@
@/**************************************************************************/
@/**************************************************************************/
@/**                                                                       */ 
@/** ThreadX Component                                                     */ 
@/**                                                                       */
@/**   Initialize                                                          */
@/**                                                                       */
@/**************************************************************************/
@/**************************************************************************/
@
@#define TX_SOURCE_CODE
@
@
    .global  nx_driver_imx_ethernet_isr
        
    .text 32
    .align 4
    .syntax unified
@/**************************************************************************/ 
@/*                                                                        */ 
@/*  FUNCTION                                               RELEASE        */ 
@/*                                                                        */ 
@/*    ENET_Transmit_IRQHandler                          MIMXRT1060/GCC    */ 
@/*                                                           5.0          */ 
@/*  AUTHOR                                                                */ 
@/*                                                                        */ 
@/*    William E. Lamie, Microsoft Corporation                             */ 
@/*                                                                        */ 
@/*  DESCRIPTION                                                           */ 
@/*                                                                        */ 
@/*    This function is responsible for fielding the ethernet interrupts   */ 
@/*    of the MIMXRT1060.                                                  */ 
@/*                                                                        */ 
@/*  INPUT                                                                 */ 
@/*                                                                        */ 
@/*    None                                                                */ 
@/*                                                                        */ 
@/*  OUTPUT                                                                */ 
@/*                                                                        */ 
@/*    None                                                                */ 
@/*                                                                        */ 
@/*  CALLS                                                                 */ 
@/*                                                                        */ 
@/*    nx_driver_imx_ethernet_isr            NetX driver ethernet ISR      */ 
@/*                                                                        */ 
@/*  CALLED BY                                                             */ 
@/*                                                                        */ 
@/*    _tx_initialize_kernel_enter           ThreadX entry function        */ 
@/*                                                                        */ 
@/*  RELEASE HISTORY                                                       */ 
@/*                                                                        */ 
@/*    DATE              NAME                      DESCRIPTION             */ 
@/*                                                                        */ 
@/*  02-01-2018     William E. Lamie         Initial Version 5.0           */ 
@/*                                                                        */ 
@/**************************************************************************/
    .global __nx_ENET_IRQHandler
    .global ENET_IRQHandler
    .thumb_func
__nx_ENET_IRQHandler:
    .thumb_func
ENET_IRQHandler:
    PUSH    {r0, lr}
#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
    BL      _tx_execution_isr_enter             @ Call the ISR enter function
#endif   
    BL      nx_driver_imx_ethernet_isr
#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
    BL      _tx_execution_isr_exit              @ Call the ISR exit function
#endif
    POP     {r0, lr}
    BX      lr
