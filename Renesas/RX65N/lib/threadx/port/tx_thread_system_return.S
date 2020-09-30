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
;/**   Thread                                                              */
;/**                                                                       */
;/**************************************************************************/
;/**************************************************************************/
;
;#define TX_SOURCE_CODE
;
;
;/* Include necessary system files.  */
;
;#include "tx_api.h"
;#include "tx_thread.h"
;#include "tx_timer.h"
;
;
.extern __tx_thread_current_ptr
.extern __tx_timer_time_slice
.extern __tx_thread_schedule
#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY	
.extern __tx_execution_thread_exit
#endif   
.text
;/**************************************************************************/ 
;/*                                                                        */ 
;/*  FUNCTION                                               RELEASE        */ 
;/*                                                                        */ 
;/*    _tx_thread_system_return                            RX6xx/GNU       */ 
;/*                                                           6.0          */
;/*  AUTHOR                                                                */
;/*                                                                        */
;/*    William E. Lamie, Microsoft Corporation                             */
;/*                                                                        */
;/*  DESCRIPTION                                                           */ 
;/*                                                                        */ 
;/*    This function is target processor specific.  It is used to transfer */ 
;/*    control from a thread back to the system.  Only a minimal context   */ 
;/*    is saved since the compiler assumes temp registers are going to get */ 
;/*    slicked by a function call anyway.                                  */ 
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
;/*    _tx_thread_schedule                   Thread scheduling loop        */ 
;/*                                                                        */ 
;/*  CALLED BY                                                             */ 
;/*                                                                        */ 
;/*    ThreadX components                                                  */ 
;/*                                                                        */ 
;/*  RELEASE HISTORY                                                       */ 
;/*                                                                        */ 
;/*    DATE              NAME                      DESCRIPTION             */
;/*                                                                        */
;/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
;/*                                                                        */
;/**************************************************************************/
;VOID   _tx_thread_system_return(VOID)
;{
.global __tx_thread_system_return
__tx_thread_system_return:
;
;    /* Save call save registers on the stack.  */
;
      PUSHM    R6-R13
      PUSHC    PSW

#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
;    /* Call the thread exit function to indicate the thread is no longer executing.  */
;
     CLRPSW   I      
     BSR      __tx_execution_thread_exit           ; Call the thread exit function
#endif   

      MOV.L    #0, R1
      PUSH.L   R1                   ; solicited stack frame    
;   
;   /* Lockout interrupts.  */
;
      CLRPSW   I                    ; Lockout interrupts
;
;    /* Save current stack in current Thread controle block.  */
;    _tx_thread_current_ptr -> tx_thread_stack_ptr =  SP;
     MOV.L   #__tx_thread_current_ptr, R2
     MOV.L   [R2], R3
     MOV.L   R0, 8[R3]
    
     MOV.L   #__tx_timer_time_slice, R4
     MOV.L   [R4], R5

;    /* Determine if the time-slice is active.  */
;    if (_tx_timer_time_slice)
;    {
;
     CMP     #0,R5                       ; Is a time-slice present?
     BEQ     __tx_thread_dont_save_ts    ; No, don't save the time-slice
;
;   /* Save time-slice for the thread and clear the current time-slice.  */
;   _tx_thread_current_ptr -> tx_thread_time_slice =  _tx_timer_time_slice;
     MOV.L   R5,24[R3]  
;   _tx_timer_time_slice =  0;
;
     MOV.L   R1, [R4] ; Disable the time-slice         
;
;    }
__tx_thread_dont_save_ts:
;
;    /* Clear the current thread pointer.  */
;    _tx_thread_current_ptr =  TX_NULL;
;
    MOV.L   R1, [R2]
    BRA  __tx_thread_schedule
;}
.end

