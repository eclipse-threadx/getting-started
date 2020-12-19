/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No 
* other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all 
* applicable laws, including copyright laws. 
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, 
* FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM 
* EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES 
* SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS 
* SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of 
* this software. By using this software, you agree to the additional terms and conditions found by accessing the 
* following link:
* http://www.renesas.com/disclaimer 
*
* Copyright (C) 2013-2020 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : r_cmt_rx_if.h
* Description  : This module creates a timer tick using a CMT channel.
***********************************************************************************************************************/
/**********************************************************************************************************************
* History : DD.MM.YYYY Version Description               
*         : 06.11.2013 2.10    First GSCE Release.
*         : 22.04.2014 2.30    Updates for RX110, RX64M support.
*         : 10.11.2014 2.40    Added support for RX113.
*         : 12.04.2014 2.41    Updated demo project.
*         : 12.04.2014 2.50    Updated demo project.
*         : 01.04.2015 2.60    Added support for RX231.
*         : 30.09.2015 2.70    Added support for RX23T.
*         : 01.10.2015 2.80    Added support for RX130.
*         : 01.12.2015 2.90    Added support for RX230, RX24T.
*         : 15.06.2016 2.91    Added the demo of the RX64M group.
*         : 01.10.2016 3.00    Added support for RX65N.
*         : 19.12.2016 3.10    Added support for RX24U, RX24T(512KB).
*         : 21.07.2017 3.20    Added support for RX65N-2M, RX130-512KB.
*         : 31.10.2017 3.21    Added the demo for RX65N, RX65N-2M.
*         : 28.09.2018 3.30    Added support for RX66T.
*         : 16.11.2018 3.31    Added XML document number.
*         : 01.02.2019 3.40    Added support for RX72T, RX65N-64pin.
*                              Added new feature: get/set interrupt priority of CMT channel.
*         : 20.05.2019 4.00    Added support for GNUC and ICCRX.
*         : 28.06.2019 4.10    Added support for RX23W.
*         : 15.08.2019 4.20    Added support for RX72M.
*         : 25.11.2019 4.30    Added support RX13T, RX66N, RX72N.
*         : 29.11.2019 4.31    Fixed issues in power_on() and power_off().
*         : 31.03.2020 4.40    Added support for RX23E-A.
*         : 29.05.2020 4.50    Added support BLE for RX23W; CMT2, CMT3 are protected for RX23W.
*         : 30.06.2020 4.60    Changed revision to reflect demo upgrade.
*         : 31.08.2020 4.70    Added condition for _RI_TRACE_TIMER macro
*                              Fixed warning when using RI600V4 with device has 2 CMT channels
***********************************************************************************************************************/
#ifndef CMT_HEADER_FILE
#define CMT_HEADER_FILE

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
#include "platform.h"
/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/

#if R_BSP_VERSION_MAJOR < 5
    #error "This module must use BSP module of Rev.5.00 or higher. Please use the BSP module of Rev.5.00 or higher."
#endif

/* Version Number of API. */
#define CMT_RX_VERSION_MAJOR            (4)
#define CMT_RX_VERSION_MINOR            (70)

/* This define is used with the R_CMT_Control() function if not channel needs to input. */
#define CMT_RX_NO_CHANNEL               (0xFFFFFFFF)

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/
/* Commands that can be used with R_CMT_Control() function. */
typedef enum
{
    CMT_RX_CMD_IS_CHANNEL_COUNTING = 0,    //Used for determining if a particular CMT channel is currently being used
    CMT_RX_CMD_PAUSE,                      //Pause a running timer without releasing it
    CMT_RX_CMD_RESTART,                    //Restart a paused timer from zero
    CMT_RX_CMD_RESUME,                     //Restart a paused timer without zeroing it first
    CMT_RX_CMD_GET_NUM_CHANNELS,           //Used for getting number of CMT channels on this MCU
    CMT_RX_CMD_SET_PRIORITY,               //Set the interrupt priority of the CMT channel
    CMT_RX_CMD_GET_PRIORITY                //Get the interrupt priority of the CMT channel
} cmt_commands_t;

/* CMT interrupt priority values that can be used with R_CMT_Control() function. */
typedef enum
{
    CMT_PRIORITY_0 = 0, /* 0: Interrupt is disabled */
    CMT_PRIORITY_1,     /* 1: Lowest interrupt priority */
    CMT_PRIORITY_2,
    CMT_PRIORITY_3,
    CMT_PRIORITY_4,
    CMT_PRIORITY_5,
    CMT_PRIORITY_6,
    CMT_PRIORITY_7,
    CMT_PRIORITY_8,
    CMT_PRIORITY_9,
    CMT_PRIORITY_10,
    CMT_PRIORITY_11,
    CMT_PRIORITY_12,
    CMT_PRIORITY_13,
    CMT_PRIORITY_14,
    CMT_PRIORITY_MAX     /* 15: Highest interrupt priority */
} cmt_priority_t;

/***********************************************************************************************************************
Exported global functions (to be accessed by other files)
***********************************************************************************************************************/
bool R_CMT_CreatePeriodic(uint32_t frequency_hz, void (* callback)(void * pdata), uint32_t * channel);
bool R_CMT_CreatePeriodicAssignChannelPriority(uint32_t frequency_hz, void (* callback)(void * pdata), uint32_t channel, cmt_priority_t priority);
bool R_CMT_CreateOneShot(uint32_t period_us, void (* callback)(void * pdata), uint32_t * channel);
bool R_CMT_CreateOneShotAssignChannelPriority(uint32_t period_us, void (* callback)(void * pdata), uint32_t channel, cmt_priority_t priority);
bool R_CMT_Control(uint32_t channel, cmt_commands_t command, void * pdata);
bool R_CMT_Stop(uint32_t channel);
uint32_t R_CMT_GetVersion(void);

#endif /* CMT_HEADER_FILE */

