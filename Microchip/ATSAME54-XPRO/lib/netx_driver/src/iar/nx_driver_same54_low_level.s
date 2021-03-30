;/**************************************************************************/
;/*                                                                        */
;/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
;/*                                                                        */
;/*       This software is licensed under the Microsoft Software License   */
;/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
;/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
;/*       and in the root directory of this software.                      */
;/*                                                                        */
;/**************************************************************************/
;
;
;/**************************************************************************/
;/**************************************************************************/
;/**                                                                       */
;/** ThreadX Component                                                     */
;/**                                                                       */
;/**   Initialize                                                          */
;/**                                                                       */
;/**************************************************************************/
;/**************************************************************************/
;
;#define TX_SOURCE_CODE
;
;
        EXTERN  _tx_thread_context_save
        EXTERN  _tx_thread_context_restore
        EXTERN  nx_driver_same54_ethernet_isr

        SECTION `.text`:CODE:NOROOT(2)
        THUMB
;/**************************************************************************/
;/*                                                                        */
;/*  FUNCTION                                               RELEASE        */
;/*                                                                        */
;/*    __nx_driver_same54_ethernet_isr                     SAME54/IAR      */
;/*                                                           6.x          */
;/*  AUTHOR                                                                */
;/*                                                                        */
;/*    Yuxin Zhou, Microsoft Corporation                                   */
;/*                                                                        */
;/*  DESCRIPTION                                                           */
;/*                                                                        */
;/*    This function is responsible for fielding the etherent interrupts   */
;/*    of the ATMEL SAME54.                                                */
;/*                                                                        */
;/*  INPUT                                                                 */
;/*                                                                        */
;/*    None                                                                */
;/*                                                                        */
;/*  OUTPUT                                                                */
;/*                                                                        */
;/*    None                                                                */
;/*                                                                        */
;/*  CALLS                                                                 */
;/*                                                                        */
;/*    nx_driver_same54_ethernet_isr         NetX driver ethernet ISR      */
;/*                                                                        */
;/*  CALLED BY                                                             */
;/*                                                                        */
;/*    _tx_initialize_kernel_enter           ThreadX entry function        */
;/*                                                                        */
;/*  RELEASE HISTORY                                                       */
;/*                                                                        */
;/*    DATE              NAME                      DESCRIPTION             */
;/*                                                                        */
;/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
;/*  xx-xx-xxxx     Scott Larson             Update comments and push r0   */
;/*                                            for 8-byte stack alignment. */
;/*                                            resulting in version 6.x.   */
;/*                                                                        */
;/**************************************************************************/
    PUBLIC  GMAC_Handler
    PUBLIC  __nx_driver_same54_ethernet_isr 
GMAC_Handler:
__nx_driver_same54_ethernet_isr:
    PUSH    {r0, lr}
#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
    BL      _tx_execution_isr_enter             ; Call the ISR enter function
#endif
    BL      nx_driver_same54_ethernet_isr
#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
    BL      _tx_execution_isr_exit              ; Call the ISR exit function
#endif
    POP     {r0, lr}
    BX      lr

    END
        
