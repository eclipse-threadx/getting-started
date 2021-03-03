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
;
;
        EXTERN  _tx_execution_isr_enter
        EXTERN  _tx_execution_isr_exit
        EXTERN  nx_driver_imx_ethernet_isr
        SECTION `.text`:CODE:NOROOT(2)
        THUMB

        PUBLIC  ENET_IRQHandler
ENET_IRQHandler:
    PUSH    {r0, lr}
#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
    BL      _tx_execution_isr_enter             ; Call the ISR enter function
#endif   
    BL      nx_driver_imx_ethernet_isr
#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
    BL      _tx_execution_isr_exit              ; Call the ISR exit function
#endif
    POP     {r0, lr}
    BX      lr
    
    END
