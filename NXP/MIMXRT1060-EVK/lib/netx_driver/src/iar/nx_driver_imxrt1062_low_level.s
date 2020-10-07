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
        
        
;/**************************************************************************/
;/*                                                                        */
;/*  FUNCTION                                               RELEASE        */
;/*                                                                        */
;/*    ENET_Transmit_IRQHandler                           IMX/IAR          */
;/*                                                           6.x          */
;/*  AUTHOR                                                                */
;/*                                                                        */
;/*    Yuxin Zhou, Microsoft Corporation                                   */
;/*                                                                        */
;/*  DESCRIPTION                                                           */
;/*                                                                        */
;/*    This function is responsible for fielding the etherent interrupts   */
;/*    of the IMX.                                                         */
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
;/*    nx_driver_imx_ethernet_isr      NetX driver ethernet ISR            */
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
        PUBLIC  ENET_IRQHandler
ENET_IRQHandler:
#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
        PUSH    {r0, lr}
        BL      _tx_execution_isr_enter         ; Call the ISR enter function
        BL      nx_driver_imx_ethernet_isr
        BL      _tx_execution_isr_exit          ; Call the ISR exit function
        POP     {r0, pc}
#else
        B       nx_driver_imx_ethernet_isr
#endif
 
        END
        
