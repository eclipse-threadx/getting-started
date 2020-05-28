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
@/** NetX Component                                                        */ 
@/**                                                                       */
@/**   Ethernet device driver for ATMEL SAME54 family micro processors     */
@/**                                                                       */
@/**************************************************************************/
@/**************************************************************************/
@
@
@
@
    .global  _tx_thread_context_save
    .global  _tx_thread_context_restore
    .global  nx_driver_same54_ethernet_isr

    .text
    .align 4
    .syntax unified
@/**************************************************************************/ 
@/*                                                                        */ 
@/*  FUNCTION                                               RELEASE        */ 
@/*                                                                        */ 
@/*    __nx_driver_same54_ethernet_isr                     SAME54/GNU      */ 
@/*                                                           6.0          */
@/*  AUTHOR                                                                */
@/*                                                                        */
@/*    Yuxin Zhou, Microsoft Corporation                                   */
@/*                                                                        */
@/*  DESCRIPTION                                                           */ 
@/*                                                                        */ 
@/*    This function is responsible for fielding the Etherent interrupts   */ 
@/*    of the ATMEL SAME54.                                                */ 
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
@/*    nx_driver_same54_ethernet_isr         NetX driver ethernet ISR      */ 
@/*                                                                        */ 
@/*  CALLED BY                                                             */ 
@/*                                                                        */ 
@/*    Interrupt                             Ethernet peripheral interrupt */ 
@/*                                                                        */ 
@/*  RELEASE HISTORY                                                       */ 
@/*                                                                        */ 
@/*    DATE              NAME                      DESCRIPTION             */
@/*                                                                        */
@/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
@/*                                                                        */
@/**************************************************************************/
    .global  GMAC_Handler
    .global   __nx_driver_same54_ethernet_isr 
    .thumb_func
GMAC_Handler: 
    .thumb_func
__nx_driver_same54_ethernet_isr:
    PUSH    {lr}
    BL      _tx_thread_context_save
    BL      nx_driver_same54_ethernet_isr
    BL      _tx_thread_context_restore
    POP     {lr}
    BX      lr


