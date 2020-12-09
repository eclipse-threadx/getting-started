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
* File Name    : r_cmt_rx.c
* Description  : This module creates timer ticks using CMT channels or one-shot events based on period in uS. 
***********************************************************************************************************************/
/**********************************************************************************************************************
* History : DD.MM.YYYY Version Description
*         : 06.11.2013 2.10    First GSCE Release.
*         : 11.03.2014 2.20    Added support for RX110.
*                              Fixes bug in R_CMT_Control CMT_RX_CMD_GET_NUM_CHANNELS command.
*         : 22.04.2014 2.30    Added support for RX64M.
*         : 10.11.2014 2.40    Added support for RX113.
*         : 20.02.2015 2.50    Added support for RX71M.
*         : 01.04.2015 2.60    Added support for RX231.
*         : 30.09.2015 2.70    Added support for RX23T.
*         : 01.10.2015 2.80    Added support for RX130.
*         : 01.12.2015 2.90    Added support for RX230, RX24T.
*         : 15.03.2016 3.00    Added support for RX65N.
*         : 19.12.2016 3.10    Added support for RX24U, RX24T(512KB).
*                              Fix to stop timer before callback function is executed.
*         : 21.07.2017 3.20    Added support for RX65N-2M, RX130-512KB.
*         : 28.09.2018 3.30    Added support for RX66T.
*                              Added support for RTOS.
*                              Fixed GSCE coding rule.
*         : 01.02.2019 3.40    Added support for RX72T, RX65N-64pin
*                              Added new feature: get/set interrupt priority of CMT channel.
*         : 20.05.2019 4.00    Added support for GNUC and ICCRX.
*         : 28.06.2019 4.10    Added support for RX23W.
*         : 15.08.2019 4.20    Added support for RX72M.
*                              Fixed warnings in IAR.
*         : 25.11.2019 4.30    Added support RX13T, RX66N, RX72N.
*                              Modified comment of API function to Doxygen style.
*                              Added support for atomic control.
*                              Removed support for Generation 1 devices.
*                              Fixed to comply with GSCE Coding Standards Rev.6.00.
*                              Changed for FIT+RTOS
*         : 29.11.2019 4.31    Fixed issues in power_on() and power_off().
*         : 31.03.2020 4.40    Added support for RX23E-A.
*         : 29.05.2020 4.50    Added support BLE for RX23W; CMT2, CMT3 are protected for RX23W.
*         : 31.08.2020 4.70    Fixed warning when using RI600V4 with device has 2 CMT channels
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
#include "platform.h"
#include "r_cmt_rx_config.h" /* Configuration header file */
#include "r_cmt_rx_if.h"     /* Interface header file */

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
/* Define the number of CMT channels based on MCU type. */
#if defined(BSP_MCU_RX64_ALL) || defined(BSP_MCU_RX113) || defined(BSP_MCU_RX71_ALL)    || \
    defined(BSP_MCU_RX231)    || defined(BSP_MCU_RX230) || defined(BSP_MCU_RX23W) || defined(BSP_MCU_RX23T)    ||\
    defined(BSP_MCU_RX24_ALL) || defined(BSP_MCU_RX65_ALL) || defined(BSP_MCU_RX66_ALL) || defined(BSP_MCU_RX72_ALL)
    #define CMT_RX_NUM_CHANNELS        (4)
#elif defined(BSP_MCU_RX111)  || defined(BSP_MCU_RX110)    || defined(BSP_MCU_RX130)  || defined(BSP_MCU_RX13T) || defined(BSP_MCU_RX23E_A)
    #define CMT_RX_NUM_CHANNELS        (2)
#else
    #error "Error! Number of channels for this MCU is not defined in r_cmt_rx.c"
#endif

#if defined(BSP_MCU_RX23W)
#undef CMT_RX_NUM_CHANNELS
#define CMT_RX_NUM_CHANNELS (2)
#endif /* BSP_MCU_RX23W */

/* Max number of ticks for counter without overflowing/underflowing. */
#define CMT_RX_MAX_TIMER_TICKS         (0x10000)

/* Starting with RX63x MCUs, there are 2 peripheral clocks: PCLKA and PCLKB. PCLKA is only used by the Ethernet block.
   This means that PCLKB would match functionality of PCLK in RX62x devices as far as the CMT is concerned. */
#if defined(BSP_MCU_RX11_ALL) || defined(BSP_MCU_RX64_ALL) || defined(BSP_MCU_RX71_ALL) || defined(BSP_MCU_RX113)    || \
    defined(BSP_MCU_RX23_ALL) || defined(BSP_MCU_RX13_ALL) || defined(BSP_MCU_RX24_ALL) || \
    defined(BSP_MCU_RX65_ALL) || defined(BSP_MCU_RX66_ALL) || defined(BSP_MCU_RX72_ALL)
    #define CMT_PCLK_HZ                 (BSP_PCLKB_HZ)
#else
    #define CMT_PCLK_HZ                 (BSP_PCLK_HZ)
#endif

/* Which MCUs have register protection. */
#if defined(BSP_MCU_RX11_ALL) || defined(BSP_MCU_RX64_ALL) || defined(BSP_MCU_RX71_ALL) || \
    defined(BSP_MCU_RX23_ALL) || defined(BSP_MCU_RX13_ALL) || defined(BSP_MCU_RX24_ALL) || defined(BSP_MCU_RX65_ALL) || \
    defined(BSP_MCU_RX66_ALL) || defined(BSP_MCU_RX72_ALL)
    #define CMT_REG_PROTECT             (1)
#else
    #define CMT_REG_PROTECT             (0)
#endif


/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/
/* Enum for current mode of CMT channel. */
typedef enum
{
    CMT_RX_MODE_DISABLED = 0,
    CMT_RX_MODE_PERIODIC,
    CMT_RX_MODE_ONE_SHOT
} cmt_modes_t;

/***********************************************************************************************************************
Private global variables and functions
***********************************************************************************************************************/
/* Used to prevent having duplicate code for each channel. This only works if the channels are identical (just at 
   different locations in memory). This is easy to tell by looking in iodefine.h and seeing if the same structure
   was used for all channels. */
static volatile struct st_cmt0 R_BSP_EVENACCESS_SFR * const g_cmt_channels[CMT_RX_NUM_CHANNELS] =
{
/* Currently all RX MCUs have either 2 or 4 CMT channels. */
#if   CMT_RX_NUM_CHANNELS == 4
    /* MCUs have 4 CMT channels. */
    &CMT0, &CMT1, &CMT2, &CMT3
#elif CMT_RX_NUM_CHANNELS == 2
    &CMT0, &CMT1
#else
    #error "Error! r_cmt_rx is not setup for this many CMT channels. Please refer to g_cmt_channels[] in r_cmt_rx.c"
#endif
};

/* Holds current mode of each CMT channel. */
#if BSP_CFG_RTOS_USED == 0      /* Non-OS */
static cmt_modes_t g_cmt_modes[CMT_RX_NUM_CHANNELS];
#elif BSP_CFG_RTOS_USED == 1    /* FreeRTOS */
#if   CMT_RX_NUM_CHANNELS == 4
    #if BSP_CFG_RTOS_SYSTEM_TIMER == 0
        static cmt_modes_t g_cmt_modes[CMT_RX_NUM_CHANNELS] = {CMT_RX_MODE_PERIODIC, CMT_RX_MODE_DISABLED, CMT_RX_MODE_DISABLED, CMT_RX_MODE_DISABLED };
    #elif BSP_CFG_RTOS_SYSTEM_TIMER == 1
        static cmt_modes_t g_cmt_modes[CMT_RX_NUM_CHANNELS] = {CMT_RX_MODE_DISABLED, CMT_RX_MODE_PERIODIC, CMT_RX_MODE_DISABLED, CMT_RX_MODE_DISABLED };
    #elif BSP_CFG_RTOS_SYSTEM_TIMER == 2
        static cmt_modes_t g_cmt_modes[CMT_RX_NUM_CHANNELS] = {CMT_RX_MODE_DISABLED, CMT_RX_MODE_DISABLED, CMT_RX_MODE_PERIODIC, CMT_RX_MODE_DISABLED };
    #elif BSP_CFG_RTOS_SYSTEM_TIMER == 3
        static cmt_modes_t g_cmt_modes[CMT_RX_NUM_CHANNELS] = {CMT_RX_MODE_DISABLED, CMT_RX_MODE_DISABLED, CMT_RX_MODE_DISABLED, CMT_RX_MODE_PERIODIC };
    #else
        #error "Error! Invalid channel"
    #endif
#elif CMT_RX_NUM_CHANNELS == 2
    #if BSP_CFG_RTOS_SYSTEM_TIMER == 0
        static cmt_modes_t g_cmt_modes[CMT_RX_NUM_CHANNELS] = {CMT_RX_MODE_PERIODIC, CMT_RX_MODE_DISABLED };
    #elif BSP_CFG_RTOS_SYSTEM_TIMER == 1
        static cmt_modes_t g_cmt_modes[CMT_RX_NUM_CHANNELS] = {CMT_RX_MODE_DISABLED, CMT_RX_MODE_PERIODIC };
    #else
        #error "Error! Invalid channel"
    #endif
#else
    #error "Error! r_cmt_rx is not setup for this many CMT channels. Please refer to g_cmt_channels[] in r_cmt_rx.c"
#endif
//#elif BSP_CFG_RTOS_USED == 2    /* SEGGER embOS */
//#elif BSP_CFG_RTOS_USED == 3    /* Micrium MicroC/OS */
#elif BSP_CFG_RTOS_USED == 4    /* Renesas RI600V4 & RI600PX */
/* CMT 1 or 2 channels are reserved for System & Trace timer. */
static cmt_modes_t g_cmt_modes[CMT_RX_NUM_CHANNELS] = {
    /*---------- CMT0 initial value ----------*/
#if BSP_CFG_RTOS_SYSTEM_TIMER==0 || _RI_TRACE_TIMER==0
     CMT_RX_MODE_PERIODIC /* CMT0 is reserved for RI600V4/RI600PX. */
#else
     CMT_RX_MODE_DISABLED /* CMT0 is unused. */
#endif
    /*---------- CMT1 initial value ----------*/
#if BSP_CFG_RTOS_SYSTEM_TIMER==1 || _RI_TRACE_TIMER==1
    ,CMT_RX_MODE_PERIODIC  /* CMT1 is reserved for RI600V4/RI600PX. */
#else
    ,CMT_RX_MODE_DISABLED  /* CMT1 is unused. */
#endif
    /*-------------------- 2 pairs of CMT --------------------*/
#if CMT_RX_NUM_CHANNELS > 2
    /*---------- CMT2 initial value ----------*/
#if BSP_CFG_RTOS_SYSTEM_TIMER == 2 || _RI_TRACE_TIMER==2
    ,CMT_RX_MODE_PERIODIC /* CMT2 is reserved for RI600V4/RI600PX. */
#else
    ,CMT_RX_MODE_DISABLED /* CMT2 is unused. */
#endif
    /*---------- CMT3 initial value ----------*/
#if BSP_CFG_RTOS_SYSTEM_TIMER == 3 || _RI_TRACE_TIMER==3
    ,CMT_RX_MODE_PERIODIC  /* CMT3 is reserved for RI600V4/RI600PX. */
#else
    ,CMT_RX_MODE_DISABLED  /* CMT3 is unused. */
#endif
    /*--------------------------------------------------------*/
#endif/* CMT_RX_NUM_CHANNELS */
};
#else /* Non-OS & others */
static cmt_modes_t g_cmt_modes[CMT_RX_NUM_CHANNELS];
#endif/* BSP_CFG_RTOS_USED */

/* This array holds the available clock dividers. For example, if PCLK/8 is available, then '8' would be an entry.
   Note that the index of the divider in the array should correspond to the bit setting used for the CKS[1:0] bitfield.
   For example, 128 is at index 2 in the array and to choose PCLK/128 in the CMCR register you would set the CKS[1:0] 
   bitfield to 2. */
static const uint32_t g_cmt_clock_dividers[] = { 8, 32, 128, 512 };

/* Array that holds user's callback functions. */
#if BSP_CFG_RTOS_USED == 0      /* Non-OS */
static void  (* g_cmt_callbacks[CMT_RX_NUM_CHANNELS])(void * pdata);
#elif BSP_CFG_RTOS_USED == 1    /* FreeRTOS */
static void  (* g_cmt_callbacks[CMT_RX_NUM_CHANNELS])(void * pdata);
#elif BSP_CFG_RTOS_USED == 2    /* SEGGER embOS */
#elif BSP_CFG_RTOS_USED == 3    /* Micrium MicroC/OS */
#elif BSP_CFG_RTOS_USED == 4    /* Renesas RI600V4 & RI600PX */
    #if CMT_RX_NUM_CHANNELS == 2
        #if ((BSP_CFG_RTOS_SYSTEM_TIMER != 0 && _RI_TRACE_TIMER != 0) ||\
            (BSP_CFG_RTOS_SYSTEM_TIMER != 1 && _RI_TRACE_TIMER != 1))
static void  (* g_cmt_callbacks[CMT_RX_NUM_CHANNELS])(void * pdata);
        #endif
    #else
            static void  (* g_cmt_callbacks[CMT_RX_NUM_CHANNELS])(void * pdata);
    #endif
#else
            static void  (* g_cmt_callbacks[CMT_RX_NUM_CHANNELS])(void * pdata);
#endif

/* Internal functions. */
static bool cmt_lock_state(void);
static void cmt_unlock_state(void);
static bool cmt_find_channel(uint32_t * channel);
static void power_on(uint32_t channel);
static void power_off(uint32_t channel);
static void cmt_counter_start(uint32_t channel);
static void cmt_counter_start_priority(uint32_t channel, cmt_priority_t priority);
static void cmt_counter_stop(uint32_t channel);
static bool cmt_setup_channel(uint32_t channel, uint32_t frequency_hz);
#if BSP_CFG_RTOS_USED == 0      /* Non-OS */
static void cmt_isr_common(uint32_t channel);
#elif BSP_CFG_RTOS_USED == 1    /* FreeRTOS */
static void cmt_isr_common(uint32_t channel);
#elif BSP_CFG_RTOS_USED == 2    /* SEGGER embOS */
#elif BSP_CFG_RTOS_USED == 3    /* Micrium MicroC/OS */
#elif BSP_CFG_RTOS_USED == 4    /* Renesas RI600V4 & RI600PX */
    #if CMT_RX_NUM_CHANNELS == 2
        #if ((BSP_CFG_RTOS_SYSTEM_TIMER != 0 && _RI_TRACE_TIMER != 0) ||\
            (BSP_CFG_RTOS_SYSTEM_TIMER != 1 && _RI_TRACE_TIMER != 1))
            static void cmt_isr_common(uint32_t channel);
        #endif
    #else
        static void cmt_isr_common(uint32_t channel);
    #endif
#else
static void cmt_isr_common(uint32_t channel);
#endif
static bool cmt_create(uint32_t frequency_hz, void (* callback)(void * pdata), cmt_modes_t mode, uint32_t * channel);
static bool cmt_create_priority(uint32_t frequency_hz, void (* callback)(void * pdata), cmt_modes_t mode, uint32_t channel, cmt_priority_t priority);

/***********************************************************************************************************************
* Function Name: R_CMT_CreatePeriodic
********************************************************************************************************************//**
* @brief This function finds an unused CMT channel, configures it for the requested frequency,
* associates a user callback function with the timer’s interrupt, and powers up and starts the timer
* @param[in] frequency_hz Desired frequency in Hz. The range and resolution of the timer is determined by settings of the peripheral clock.
* The best pre-scaler for the CMT channel is chosen by the driver.\n
* @param[in] callback Pointer to the user’s callback function.  It should receive a single void * argument.\n
* @param[in] *channel The CMT FIT module finds the first CMT channel that is not in use and assigns it to the caller.
* This allows multiple drivers to use the CMT driver without having to pre-assign all timer channels.
* This argument provides a way for the driver to indicate back to the caller which channel has been assigned.\n
* @retval true  Successful; CMT initialized
* @retval false     No free CMT channels available, or invalid settings
* @details The R_CMT_CreatePeriodic function finds an unused CMT channel, assigns it to the caller,
* and registers a user callback function to be called upon compare match events.\n
* The CMT is configured to generate compare matches at the frequency specified in the call.\n
* @note 1.  Maximum periodic frequency\n
* In hardware, the CMT timer maximum clocking speed is limited to PCLK/8.
* However, when using the periodic timer function to generate a clock, be aware that interrupt and callback processing takes some time.
* As requested frequency rises, interrupt and callback processing will take an increasing percentage of the processor’s time.
* At some point, too much time is consumed to leave any time for other useful work.
* So this will limit the maximum frequency that can be generated.
*  The maximum practical frequency will depend on your system design, but in general, frequencies up to a few kilohertz are reasonable.\n
*/
bool R_CMT_CreatePeriodic (uint32_t frequency_hz, void (* callback)(void * pdata), uint32_t * channel)
{
    return cmt_create(frequency_hz, callback, CMT_RX_MODE_PERIODIC, channel);
} 
/* End of function R_CMT_CreatePeriodic */

/***********************************************************************************************************************
* Function Name: R_CMT_CreatePeriodicAssignChannelPriority
* ******************************************************************************************************************//**
* @brief This function configures desired CMT channel for the requested frequency and desired interrupt priority level,
* associates a user callback function with the timer’s interrupt, powers up and starts the timer
* @param[in] frequency_hz Desired frequency in Hz. The range and resolution of the timer is determined by settings of the peripheral clock.
* The best pre-scaler for the CMT channel is chosen by the driver.\n
* @param[in] callback Pointer to the user’s callback function. It should receive a single void * argument.\n
* @param[in] channel Desired CMT channel that is used to configure.\n
* @param[in] priority Desired priority level of timer’s interrupt:\n
* CMT_PRIORITY_0: Interrupt is disabled\n
* CMT_PRIORITY_1: Lowest interrupt priority\n
* CMT_PRIORITY_2\n
* CMT_PRIORITY_3\n
* CMT_PRIORITY_4\n
* CMT_PRIORITY_5\n
* CMT_PRIORITY_6\n
* CMT_PRIORITY_7\n
* CMT_PRIORITY_8\n
* CMT_PRIORITY_9\n
* CMT_PRIORITY_10\n
* CMT_PRIORITY_11\n
* CMT_PRIORITY_12\n
* CMT_PRIORITY_13\n
* CMT_PRIORITY_14\n
* CMT_PRIORITY_15: Highest interrupt priority\n
* @retval true  Successful; CMT initialized\n
* @retval false     No free CMT channels available, or invalid settings\n
* @details The R_CMT_CreatePeriodicAssignChannelPriority assigns desired CMT channel to the caller with desired
* interrupt priority level and registers a user callback function to be called upon compare match events.
* The CMT is configured to generate compare matches at the frequency specified in the call.\n
* @note 1.  Maximum periodic frequency\n
* In hardware, the CMT timer maximum clocking speed is limited to PCLK/8.
* However, when using the periodic timer function to generate a clock, be aware that interrupt and callback processing takes some time.
* As requested frequency rises, interrupt and callback processing will take an increasing percentage of the processor’s time.
* At some point, too much time is consumed to leave any time for other useful work.
* So this will limit the maximum frequency that can be generated.
*  The maximum practical frequency will depend on your system design, but in general, frequencies up to a few kilohertz are reasonable.\n
*/
bool R_CMT_CreatePeriodicAssignChannelPriority (uint32_t frequency_hz, void (* callback)(void * pdata), uint32_t channel, cmt_priority_t priority)
{
    return cmt_create_priority(frequency_hz, callback, CMT_RX_MODE_PERIODIC, channel, priority);
}
/* End of function R_CMT_CreatePeriodicAssignChannelPriority */

/***********************************************************************************************************************
* Function Name: R_CMT_CreateOneShot
* ******************************************************************************************************************//**
 @brief This function finds an unused CMT channel, configures it for the requested period,
* associates a user callback function with the timer’s interrupt, and powers up and starts the timer
* @param[in] period_us Desired period in microseconds.
* The range and resolution of the timer is determined by settings of the peripheral clock.
* The best pre-scaler for the CMT channel is chosen by the driver.\n
* @param[in] callback Pointer to the user’s callback function.  It should data a single void * argument.\n
* @param[in] channel The CMT FIT module finds the first CMT channel that is not in use and assigns it to the caller.
* This allows multiple drivers to use the CMT driver without having to pre-assign all timer channels.
* This argument provides a way for the driver to indicate back to the caller which channel has been assigned.\n
* @retval true  Successful; CMT initialized\n
* @retval false     No free CMT channels available, or invalid settings\n
* @details The R_CMT_CreateOneShot function finds an unused CMT channel, assigns it to the caller,
*  and registers a user callback function to be called upon the compare match event.\n
* The CMT is configured to generate a compare match after the period specified in the call.
* The timer is shut down after a single compare match event.\n
*/
bool R_CMT_CreateOneShot (uint32_t period_us, void (* callback)(void * pdata), uint32_t * channel)
{    
    return cmt_create((1000000/period_us), callback, CMT_RX_MODE_ONE_SHOT, channel);
}
/* End of function R_CMT_CreateOneShot */

/***********************************************************************************************************************
* Function Name: R_CMT_CreateOneShotAssignChannelPriority
********************************************************************************************************************//**
* @brief This function configures the desired CMT channel for the requested period with desired interrupt priority level,
* associates a user callback function with the timer’s interrupt, powers up and starts the timer
* @param[in] period_us Desired period in microseconds.
* The range and resolution of the timer is determined by settings of the peripheral clock.
* The best pre-scaler for the CMT channel is chosen by the driver.\n
* @param[in] callback Pointer to the user’s callback function. It should data a single void * argument.\n
* @param[in] channel Desired CMT channel that is used to configure.\n
* @param[in] priority Desired priority level of timer’s interrupt:\n
* CMT_PRIORITY_0: Interrupt is disabled\n
* CMT_PRIORITY_1: Lowest interrupt priority\n
* CMT_PRIORITY_2\n
* CMT_PRIORITY_3\n
* CMT_PRIORITY_4\n
* CMT_PRIORITY_5\n
* CMT_PRIORITY_6\n
* CMT_PRIORITY_7\n
* CMT_PRIORITY_8\n
* CMT_PRIORITY_9\n
* CMT_PRIORITY_10\n
* CMT_PRIORITY_11\n
* CMT_PRIORITY_12\n
* CMT_PRIORITY_13\n
* CMT_PRIORITY_14\n
* CMT_PRIORITY_15: Highest interrupt priority\n
* @retval true  Successful; CMT initialized\n
* @retval false     No free CMT channels available, or invalid settings\n
* @details The R_CMT_CreateOneShotAssignChannelPriority assigns the desired
* CMT channel to the caller with desired interrupt priority level,
* and registers a user callback function to be called upon the compare match event.
* The CMT is configured to generate a compare match after the period specified in the call.
* The timer is shut down after a single compare match event.\n
* @note 1.  Invalid settings\n
* The function will return false if one of the following invalid settings occurs:
* invalid channel, invalid priority, channel was in used, or frequency could not be used.\n
*/
bool R_CMT_CreateOneShotAssignChannelPriority (uint32_t period_us, void (* callback)(void * pdata), uint32_t channel, cmt_priority_t priority)
{
    return cmt_create_priority((1000000/period_us), callback, CMT_RX_MODE_ONE_SHOT, channel, priority);
}
/* End of function R_CMT_CreateOneShotAssignChannelPriority */

/***********************************************************************************************************************
* Function Name: R_CMT_Stop
********************************************************************************************************************//**
* @brief Stops a CMT channel and powers down the CMT unit if possible
* @param[in] channel The CMT timer channel to stop\n
* @retval true  Successful; CMT closed\n
* @retval false     Invalid settings \n
* @details This function frees the CMT channel by clearing its assignment and disabling the associated interrupt.
* The CMT channel cannot be used again until it has been reopened with either 
* the R_CMT_CreatePeriodic or the R_CMT_CreateOneShot function.
* If the CMT channel is already used as RTOS system timer, a call to this function with this CMT channel as channel,
* will result in FALSE being returned
*/
bool R_CMT_Stop (uint32_t channel)
{
    /* Make sure valid channel number was input. */
#if BSP_CFG_RTOS_USED == 0      /* Non-OS */
    if (channel >= CMT_RX_NUM_CHANNELS)
#elif BSP_CFG_RTOS_USED == 1        /* FreeRTOS */
    if ((channel >= CMT_RX_NUM_CHANNELS) || (BSP_CFG_RTOS_SYSTEM_TIMER == channel))
//#elif BSP_CFG_RTOS_USED == 2  /* SEGGER embOS */
//#elif BSP_CFG_RTOS_USED == 3  /* Micrium MicroC/OS */
#elif BSP_CFG_RTOS_USED   == 4  /* Renesas RI600V4 & RI600PX */
    if ((channel >= CMT_RX_NUM_CHANNELS) || (BSP_CFG_RTOS_SYSTEM_TIMER == channel) || (_RI_TRACE_TIMER == channel))
#else /* Non-OS & others */
    if (channel >= CMT_RX_NUM_CHANNELS)
#endif/* BSP_CFG_RTOS_USED */
    {
        /* Invalid channel number was used.*/
        /*In case using RX23W, Channel 2 and 3 are reserved by BLE FIT modules v.1.00*/
        return false;
    }

    /* Stop counter. */
    cmt_counter_stop(channel);

    /* Turn off power to channel. */
    power_off(channel);

    /* Set mode to disabled. */
    g_cmt_modes[channel] = CMT_RX_MODE_DISABLED;

    /* Set channel as available. */
    R_BSP_HardwareUnlock((mcu_lock_t)(BSP_LOCK_CMT0 + channel));

    return true;
} /* End of function R_CMT_Stop */

/***********************************************************************************************************************
* Function Name: R_CMT_Control
********************************************************************************************************************//**
* @brief This function provides various ways to control and monitor a CMT channel
* @param[in] channel CMT channel number to control.\n
* @param[in] command Command to execute:\n
* CMT_RX_CMD_IS_CHANNEL_COUNTING\n
* CMT_RX_CMD_PAUSE\n
* CMT_RX_CMD_RESUME\n
* CMT_RX_CMD_RESTART\n
* CMT_RX_CMD_GET_NUM_CHANNELS\n
* CMT_RX_CMD_SET_PRIORITY\n
* CMT_RX_CMD_GET_PRIORITY\n
* @param[in] *pdata
* @retval true  The command completed properly. Check pdata
* @retval false     The command did not complete properly
* @details This function provides a number of commands:\n
* CMT_RX_CMD_IS_CHANNEL_COUNTING tells if a CMT channel is currently running.  Check *pdata.\n
* CMT_RX_CMD_PAUSE pauses a timer without closing it (without powering it off).\n
* CMT_RX_CMD_RESUME restarts a paused timer without resetting the counter to zero\n
* CMT_RX_CMD_RESTART restarts a paused timer after resetting the counter to zero\n
* CMT_RX_CMD_GET_NUM_CHANNELS returns the total number of channels available\n
* CMT_RX_CMD_SET_PRIORITY sets the interrupt priority of the CMT channel.\n
* CMT_RX_CMD_GET_PRIORITY gets the interrupt priority of the CMT channel.\n
* If the CMT channel is already used as RTOS system timer, a call to this function with this CMT channel as channel,
* and any of CMT_RX_CMD_IS_CHANNEL_COUNTING, CMT_RX_CMD_PAUSE,
* CMT_RX_CMD_RESUME, CMT_RX_CMD_RESTART as command, will result in FALSE being returned.
*/
bool R_CMT_Control (uint32_t channel, cmt_commands_t command, void * pdata)
{
    bool ret = true;
    cmt_priority_t priority;

#if BSP_CFG_RTOS_USED == 0      /* Non-OS */
#elif BSP_CFG_RTOS_USED == 1    /* FreeRTOS */
    if (CMT_RX_CMD_GET_NUM_CHANNELS == command)
    {
        /* Return the number of CMT channels on this MCU. */
        *(uint32_t *)pdata = CMT_RX_NUM_CHANNELS;
        ret = true;
        return ret;
    }
    else if ((channel == BSP_CFG_RTOS_SYSTEM_TIMER) || (channel >= CMT_RX_NUM_CHANNELS))
    {
        /* Specified CMT channel is being used by RTOS or Invalid channel */
        /* Modifying is NOT allowed for the other commands. */
        ret = false;
        return ret;
    }
    else
    {
        /* Do nothing. */
    }
//#elif BSP_CFG_RTOS_USED == 2    /* SEGGER embOS */
//#elif BSP_CFG_RTOS_USED == 3    /* Micrium MicroC/OS */
#elif BSP_CFG_RTOS_USED == 4    /* Renesas RI600V4 & RI600PX */

    if (CMT_RX_CMD_GET_NUM_CHANNELS == command)
    {
        /* Return the number of CMT channels on this MCU. */
        *(uint32_t *)pdata = CMT_RX_NUM_CHANNELS;
        ret = true;
        return ret;
    }
    else if ((channel == BSP_CFG_RTOS_SYSTEM_TIMER) || (channel == _RI_TRACE_TIMER) || (channel >= CMT_RX_NUM_CHANNELS))
    {
        /* Specified CMT channel is being used by RTOS or Invalid channel */
        /* Modifying is NOT allowed for the other commands. */
        ret = false;
        return ret;
    }
    else
    {
        /* Do nothing. */
    }
#else /* Non-OS & others */
#endif/* BSP_CFG_RTOS_USED */

    /* Process command that was input. */
    switch (command)
    {
        case CMT_RX_CMD_IS_CHANNEL_COUNTING:
        {
            /* Check input channel. */
            if (channel < CMT_RX_NUM_CHANNELS)
            {
                /* Valid channel input. Is this channel currently counting? */
                if (CMT_RX_MODE_DISABLED == g_cmt_modes[channel])
                {
                    /* pdata = false*/
                    *(bool *)pdata = false;
                }
                else
                {
                    /* pdata = true*/
                    *(bool *)pdata = true;
                }
            }
            else
            {
                ret = false;
            }
            break;
        }
#if BSP_CFG_RTOS_USED == 0   /*Unreachable code because of checksabove*/
        case CMT_RX_CMD_GET_NUM_CHANNELS:
        {
            /* number of CMT channels */
            *(uint32_t *)pdata = CMT_RX_NUM_CHANNELS;
            break;
        }
#endif
        case CMT_RX_CMD_PAUSE:
        {
            if (channel < CMT_RX_NUM_CHANNELS)
            {
                /* Valid channel input. Is this channel currently counting? */
                if (CMT_RX_MODE_DISABLED != g_cmt_modes[channel])
                {

                    switch (channel)
                    {
                        case 0:
                        {
                            /*CMT0.CMCNT count is stopped*/
                            CMT.CMSTR0.BIT.STR0 = 0;
                            break;
                        }
                        case 1:
                        {
                            /*CMT1.CMCNT count is stopped*/
                            CMT.CMSTR0.BIT.STR1 = 0;
                            break;
                        }
                        #if   CMT_RX_NUM_CHANNELS == 4
                        case 2:
                        {
                            /*CMT2.CMCNT count is stopped*/
                            CMT.CMSTR1.BIT.STR2 = 0;
                            break;
                        }
                        case 3:
                        {
                            /*CMT3.CMCNT count is stopped*/
                            CMT.CMSTR1.BIT.STR3 = 0;
                            break;
                        }
                        #endif

                        default:
                        {
                            break;
                        }
                    }
                }
            }
            else
            {
                ret = false;
            }
            break;
        }

        /* Both restart and resume do the same thing, except restart zeros the counter first */ 
        case CMT_RX_CMD_RESTART:
        case CMT_RX_CMD_RESUME:
        {
            if (channel < CMT_RX_NUM_CHANNELS)
            {
                /* Stop the timer once to prevent an interrupt from occurring */
                switch (channel)
                {
                    case 0:
                    {
                        /*CMT0.CMCNT count is stopped*/
                        CMT.CMSTR0.BIT.STR0 = 0;
                        break;
                    }
                    case 1:
                    {
                        /*CMT1.CMCNT count is stopped*/
                        CMT.CMSTR0.BIT.STR1 = 0;
                        break;
                    }
#if   CMT_RX_NUM_CHANNELS == 4
                    case 2:
                    {
                        /*CMT2.CMCNT count is stopped*/
                        CMT.CMSTR1.BIT.STR2 = 0;
                        break;
                    }
                    case 3:
                    {
                        /*CMT3.CMCNT count is stopped*/
                        CMT.CMSTR1.BIT.STR3 = 0;
                        break;
                    }
#endif
                    default:
                    {
                        break;
                    }
                }

                /* Valid channel input. Is this channel currently counting? */
                if (CMT_RX_MODE_DISABLED != g_cmt_modes[channel])
                {

                    if (CMT_RX_CMD_RESTART == command)
                    {
                      /* For restarting, clear counter. */
                      (*g_cmt_channels[channel]).CMCNT = 0;
                    }

                    switch (channel)
                    {
                        case 0:
                        {
                            /*CMT0.CMCNT count is started*/
                            CMT.CMSTR0.BIT.STR0 = 1;
                            break;
                        }
                        case 1:
                        {
                            /*CMT1.CMCNT count is started*/
                            CMT.CMSTR0.BIT.STR1 = 1;
                            break;
                        }
#if   CMT_RX_NUM_CHANNELS == 4
                        case 2:
                        {
                            /*CMT2.CMCNT count is started*/
                            CMT.CMSTR1.BIT.STR2 = 1;
                            break;
                        }
                        case 3:
                        {
                            /*CMT3.CMCNT count is started*/
                            CMT.CMSTR1.BIT.STR3 = 1;
                            break;
                        }
#endif
                        default:
                        {
                            break;
                        }
                    }
                }
            }
            else
            {
                ret = false;
            }
            break;
        }
        case CMT_RX_CMD_SET_PRIORITY:
        {
            /* Make sure that pdata is NOT NULL */
            if (NULL == pdata)
            {
                ret = false;
            }
            else
            {
                /* Get input priority value from pdata */
                priority = *(cmt_priority_t *)pdata;
            }

            /* Check if interrupt priority and channel are correct or not */
            if (true == ret)
            {
                if ((CMT_PRIORITY_0 > (int8_t) priority)
                 || (CMT_PRIORITY_MAX < priority)
                 || (CMT_RX_NUM_CHANNELS <= channel))
                {
                    ret = false;
                }
            }

            if (true == ret)
            {
                /* Acquire lock on input channel to check if it was created or not */
                if (true == R_BSP_HardwareLock((mcu_lock_t)(BSP_LOCK_CMT0 + channel)))
                {
                    /* Channel is NOT created yet, release lock on channel */
                    R_BSP_HardwareUnlock((mcu_lock_t)(BSP_LOCK_CMT0 + channel));

                    /* Set return value to false */
                    ret = false;
                }
                else
                {
                    switch (channel)
                    {
                        case 0:
                        {
                            /* Set interrupt priority using ICU register. */
                            IPR(CMT0, CMI0) = priority;
                            break;
                        }

                        case 1:
                        {
                            /* Set interrupt priority using ICU register. */
                            IPR(CMT1, CMI1) = priority;
                            break;
                        }

#if CMT_RX_NUM_CHANNELS == 4
                        case 2:
                        {
                            /* Set interrupt priority using ICU register. */
                            IPR(CMT2, CMI2) = priority;
                            break;
                        }

                        case 3:
                        {
                            /* Set interrupt priority using ICU register. */
                            IPR(CMT3, CMI3) = priority;
                            break;
                        }

#endif
                        default:
                        {
                            /* Should never get here. */
                            R_BSP_NOP();
                            break;
                        }
                    }
                }
            }
        }
            break;
        
        case CMT_RX_CMD_GET_PRIORITY:
        {
            /* Make sure that CMT channel is correct, pdata is NOT NULL */
            if ((CMT_RX_NUM_CHANNELS <= channel) || (NULL == pdata))
            {
                ret = false;
            }
            else
            {
                switch(channel)
                {
                    case 0:
                    {
                        /* Get priority of CMT channel 0 from IPR register */
                        priority = (cmt_priority_t)IPR(CMT0, CMI0);
                        break;
                    }

                    case 1:
                    {
                        /* Get priority of CMT channel 1 from IPR register */
                        priority = (cmt_priority_t)IPR(CMT1, CMI1);
                        break;
                    }

#if CMT_RX_NUM_CHANNELS == 4
                    case 2:
                    {
                        /* Get priority of CMT channel 2 from IPR register */
                        priority = (cmt_priority_t)IPR(CMT2, CMI2);
                        break;
                    }

                    case 3:
                    {
                        /* Get priority of CMT channel 3 from IPR register */
                        priority = (cmt_priority_t)IPR(CMT3, CMI3);
                        break;
                    }
                    
                    default:
                    {
                        /* Should never get here. */
                        ret = false;
                        break;
                    }
#endif
                }

                /* Update pdata value with priority for output */
                *(cmt_priority_t *)pdata = priority;
            }
            break;
        }

        default:
        {
            ret = false;
            break;
        }
    }

    return ret;
}
/***********************************************************************************************************************
* End of function R_CMT_Control
***********************************************************************************************************************/

/***********************************************************************************************************************
* Function Name: cmt_create
* Description  : Sets up a CMT channel based on user input options.
* Arguments    : frequency_hz -
*                    Frequency in Hz of how often to call the callback function.
*                callback -
*                    Which function to call when timer expires.
*                mode -
*                    Current mode of CMT channel
*                channel -
*                    Pointer of where to store which channel was used.
* Return Value : true - 
*                    Channel initialized successfully.
*                false -
*                    Invalid channel or period could not be used.
***********************************************************************************************************************/
static bool cmt_create (uint32_t frequency_hz, void (* callback)(void * pdata), cmt_modes_t mode, uint32_t * channel)
{
    /* Return value. */
    bool     ret = false;    

    /* Grab state to make sure we do not interfere with another operation. */
    if (cmt_lock_state() != true)
    {
        /* Another operation is already in progress. */
        return false;
    }

    /* Was a channel found? */
    if (false == cmt_find_channel(channel))
    {
        /* Channel not found or not available. */
        ret = false;
    }
    else
    {
        /* Look to see if we can support this frequency. */
        ret = cmt_setup_channel(*channel, frequency_hz);
    
        /* Set up ICU registers, save callback function, and start counter if valid divider was found. */
        if (true == ret)
        {        
            /* Set mode of operation. */
            g_cmt_modes[*channel] = mode;
    
            /* Save callback function to be used. */
#if BSP_CFG_RTOS_USED == 0      /* Non-OS */
            g_cmt_callbacks[*channel] = callback;
#elif BSP_CFG_RTOS_USED == 1        /* FreeRTOS */
            g_cmt_callbacks[*channel] = callback;
#elif BSP_CFG_RTOS_USED == 2    /* SEGGER embOS */
#elif BSP_CFG_RTOS_USED == 3    /* Micrium MicroC/OS */
#elif BSP_CFG_RTOS_USED == 4    /* Renesas RI600V4 & RI600PX */
    #if CMT_RX_NUM_CHANNELS == 2
        #if ((BSP_CFG_RTOS_SYSTEM_TIMER != 0 && _RI_TRACE_TIMER != 0) ||\
            (BSP_CFG_RTOS_SYSTEM_TIMER != 1 && _RI_TRACE_TIMER != 1))
            g_cmt_callbacks[*channel] = callback;
        #endif
    #else
            g_cmt_callbacks[*channel] = callback;
    #endif
#else
            g_cmt_callbacks[*channel] = callback;
#endif

    
            /* Start channel counting. */
            cmt_counter_start(*channel);
        }
        else
        {
            /* Could not support input frequency. Release lock on channel. */
            R_BSP_HardwareUnlock((mcu_lock_t)(BSP_LOCK_CMT0 + (*channel)));
        }
    }

    /* Release state so other operations can be performed. */
    cmt_unlock_state();

    return ret;
}
/* End of function cmt_create */

/***********************************************************************************************************************
* Function Name: cmt_create_priority
* Description  : Sets up a CMT channel based on user input options with interrupt priority.
* Arguments    : frequency_hz -
*                    Frequency in Hz of how often to call the callback function.
*                callback -
*                    Which function to call when timer expires.
*                mode -
*                    Current mode of CMT channel
*                channel -
*                    Which channel is used to create.
*                priority -
*                    Which interrupt priority is used to assign.
* Return Value : true -
*                    Channel initialized successfully.
*                false -
*                    Invalid channel, invalid priority, channel was in used, or frequency could not be used.
***********************************************************************************************************************/
static bool cmt_create_priority (uint32_t frequency_hz, void (* callback)(void * pdata), cmt_modes_t mode, uint32_t channel, cmt_priority_t priority)
{
    /* Return value. */
    bool     ret = false;

    /* Check if interrupt priority and channel are correct or not. */
    if ((CMT_PRIORITY_0 > (int8_t) priority) || (CMT_PRIORITY_MAX < priority) || (CMT_RX_NUM_CHANNELS <= channel))
    {
        return false;
    }

    /* Grab state to make sure we do not interfere with another operation. */
    if (cmt_lock_state() != true)
    {
        /* Another operation is already in progress. */
        return false;
    }

    /* Acquire lock on input channel. If return value is true then this channel is NOT in use. */
    if (true == R_BSP_HardwareLock((mcu_lock_t)(BSP_LOCK_CMT0 + channel)))
    {
        /* Look to see if we can support this frequency. */
        ret = cmt_setup_channel(channel, frequency_hz);

        /* Set up ICU registers, save callback function, and start counter if valid divider was found. */
        if (true == ret)
        {
            /* Set mode of operation. */
            g_cmt_modes[channel] = mode;

            /* Save callback function to be used. */
#if BSP_CFG_RTOS_USED == 0      /* Non-OS */
            g_cmt_callbacks[channel] = callback;
#elif BSP_CFG_RTOS_USED == 1        /* FreeRTOS */
            g_cmt_callbacks[channel] = callback;
#elif BSP_CFG_RTOS_USED == 2    /* SEGGER embOS */
#elif BSP_CFG_RTOS_USED == 3    /* Micrium MicroC/OS */
#elif BSP_CFG_RTOS_USED == 4    /* Renesas RI600V4 & RI600PX */
    #if CMT_RX_NUM_CHANNELS == 2
        #if ((BSP_CFG_RTOS_SYSTEM_TIMER != 0 && _RI_TRACE_TIMER != 0) ||\
            (BSP_CFG_RTOS_SYSTEM_TIMER != 1 && _RI_TRACE_TIMER != 1))
            g_cmt_callbacks[channel] = callback;
        #endif
    #else
            g_cmt_callbacks[channel] = callback;
    #endif
#else
            g_cmt_callbacks[channel] = callback;
#endif

            /* Start channel counting. */
            cmt_counter_start_priority(channel, priority);
        }
        else
        {
            /* Could not support input frequency. Release lock on channel. */
            R_BSP_HardwareUnlock((mcu_lock_t)(BSP_LOCK_CMT0 + (channel)));
        }
    }

    /* Release state so other operations can be performed. */
    cmt_unlock_state();

    return ret;
}
/* End of function cmt_create_priority */

/***********************************************************************************************************************
* Function Name: cmt_lock_state
* Description  : Attempt to lock the state so that we right to perform an operation.
* Arguments    : none
* Return Value : true - 
*                    State was obtained
*                false - 
*                    State was not obtained because code is busy with another on-going operation.
***********************************************************************************************************************/
static bool cmt_lock_state (void)
{
    bool ret;

    /* Attempt to acquire lock. */
    ret = R_BSP_HardwareLock(BSP_LOCK_CMT);

    return ret;
}
/* End of function cmt_lock_state */

/***********************************************************************************************************************
* Function Name: cmt_unlock_state
* Description  : Release state so that other operations can be performed.
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
static void cmt_unlock_state (void)
{
    /* Release lock. */
    R_BSP_HardwareUnlock(BSP_LOCK_CMT);
}
/* End of function cmt_unlock_state */

/***********************************************************************************************************************
* Function Name: cmt_find_channel
* Description  : Attempt to find open CMT channel to use.
* Arguments    : channel - 
*                    Channel number if channel was found.
* Return Value : true -
*                    Channel was found and locked.
*                false -
*                    No channels available.
***********************************************************************************************************************/
static bool cmt_find_channel (uint32_t * channel)
{
    bool     channel_found = false;
    uint32_t i;

    /* Look for an available channel. */
    /* WAIT_LOOP */
    for (i = 0; i < CMT_RX_NUM_CHANNELS; i++)
    {
#if BSP_CFG_RTOS_USED == 0      /* Non-OS */
#elif BSP_CFG_RTOS_USED == 1    /* FreeRTOS */
        if (i == BSP_CFG_RTOS_SYSTEM_TIMER)
        {
            /* Found CMT channel is being used for RTOS. */
            continue;
        }
//#elif BSP_CFG_RTOS_USED == 2    /* SEGGER embOS */
//#elif BSP_CFG_RTOS_USED == 3    /* Micrium MicroC/OS */
#elif BSP_CFG_RTOS_USED == 4    /* Renesas RI600V4 & RI600PX */
        if (i == BSP_CFG_RTOS_SYSTEM_TIMER || i == _RI_TRACE_TIMER)
        {
            /* Found CMT channel is being used for RTOS. */
            continue;
        }
#else /* Non-OS & others */
#endif/* BSP_CFG_RTOS_USED */
        /* Look for an available channel. */
        if (true == R_BSP_HardwareLock((mcu_lock_t)(BSP_LOCK_CMT0 + i)))
        {
            /* Channel found. */
            *channel = i;
            channel_found = true;
            
            break;            
        }
    }        

    return channel_found;
}
/* End of function cmt_find_channel */

/***********************************************************************************************************************
* Function Name: power_on
* Description  : Enables CMT channel.
* Arguments    : channel - 
*                    Channel number to enable.
* Return Value : none
***********************************************************************************************************************/
static void power_on (uint32_t channel)
{
#if ((R_BSP_VERSION_MAJOR == 5) && (R_BSP_VERSION_MINOR >= 30)) || (R_BSP_VERSION_MAJOR >= 6)
    bsp_int_ctrl_t int_ctrl;
#endif

#if CMT_REG_PROTECT == 1
    /* Enable writing to MSTP registers. */
    R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_LPC_CGC_SWR);
#endif

#if ((R_BSP_VERSION_MAJOR == 5) && (R_BSP_VERSION_MINOR >= 30)) || (R_BSP_VERSION_MAJOR >= 6)
    R_BSP_InterruptControl(BSP_INT_SRC_EMPTY, BSP_INT_CMD_FIT_INTERRUPT_DISABLE, &int_ctrl);
#endif

    /* Enable selected CMT Channel. Go ahead and make sure counter is stopped too. */
    switch (channel)
    {
        case 0:
            {
            /* Bring module out of stop state. */
            MSTP(CMT0) = 0;

            /* Stop counter. */
            CMT.CMSTR0.BIT.STR0 = 0;
            break;
            }
        case 1:
            {
            /* Bring module out of stop state. */
            MSTP(CMT1) = 0;

            /* Stop counter. */
            CMT.CMSTR0.BIT.STR1 = 0;        
            break;
            }
#if   CMT_RX_NUM_CHANNELS == 4
        case 2:
            {
            /* Bring module out of stop state. */
            MSTP(CMT2) = 0;

            /* Stop counter. */
            CMT.CMSTR1.BIT.STR2 = 0;        
            break;
            }
        case 3:
            {
            /* Bring module out of stop state. */
            MSTP(CMT3) = 0;

            /* Stop counter. */
            CMT.CMSTR1.BIT.STR3 = 0;       
            break;
            }
#endif
        default:
            {
            /* Should never get here. Valid channel number is checked above. */
            break;
            }
    }

#if ((R_BSP_VERSION_MAJOR == 5) && (R_BSP_VERSION_MINOR >= 30)) || (R_BSP_VERSION_MAJOR >= 6)
    R_BSP_InterruptControl(BSP_INT_SRC_EMPTY, BSP_INT_CMD_FIT_INTERRUPT_ENABLE, &int_ctrl);
#endif

#if CMT_REG_PROTECT == 1

    /* Disable writing to MSTP registers. */
    R_BSP_RegisterProtectEnable(BSP_REG_PROTECT_LPC_CGC_SWR);
#endif
}
/***********************************************************************************************************************
* End of function power_on
***********************************************************************************************************************/

/***********************************************************************************************************************
* Function Name: power_off
* Description  : Disables CMT channel.
* Arguments    : channel - 
*                    Channel number to disable.
* Return Value : none
***********************************************************************************************************************/
static void power_off (uint32_t channel)
{
#if ((R_BSP_VERSION_MAJOR == 5) && (R_BSP_VERSION_MINOR >= 30)) || (R_BSP_VERSION_MAJOR >= 6)
    bsp_int_ctrl_t int_ctrl;
#endif

#if CMT_REG_PROTECT == 1

    /* Enable writing to MSTP registers. */
    R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_LPC_CGC_SWR);
#endif

#if ((R_BSP_VERSION_MAJOR == 5) && (R_BSP_VERSION_MINOR >= 30)) || (R_BSP_VERSION_MAJOR >= 6)
    R_BSP_InterruptControl(BSP_INT_SRC_EMPTY, BSP_INT_CMD_FIT_INTERRUPT_DISABLE, &int_ctrl);
#endif

    /* Enable selected CMT Channel. Go ahead and make sure counter is stopped too. Since each MSTP bit controls
       2 channels, make sure to only disable CMT unit when both channels are off. */
    switch (channel)
    {
        case 0:
            {
            /* Stop counter. */
            CMT.CMSTR0.BIT.STR0 = 0;

            /* Only turn off CMT unit if both channels are off. */
            if (CMT_RX_MODE_DISABLED == g_cmt_modes[1])
            {
                /* Put module in stop state. */
                MSTP(CMT0) = 1;

            }
            break;
            }
        case 1:
            {
            /* Stop counter. */
            CMT.CMSTR0.BIT.STR1 = 0;        

            /* Only turn off CMT unit if both channels are off. */
            if (CMT_RX_MODE_DISABLED == g_cmt_modes[0])
            {
                /* Put module in stop state. */
                MSTP(CMT1) = 1;

            }
            break;
            }
#if   CMT_RX_NUM_CHANNELS == 4
        case 2:
            {
            /* Stop counter. */
            CMT.CMSTR1.BIT.STR2 = 0;        

            /* Only turn off CMT unit if both channels are off. */
            if (CMT_RX_MODE_DISABLED == g_cmt_modes[3])
            {
                /* Put module in stop state. */
                MSTP(CMT2) = 1;

            }
            break;
            }
        case 3:
            {
            /* Stop counter. */
            CMT.CMSTR1.BIT.STR3 = 0;       

            /* Only turn off CMT unit if both channels are off. */
            if (CMT_RX_MODE_DISABLED == g_cmt_modes[2])
            {
                /* Put module in stop state. */
                MSTP(CMT3) = 1;

            }
            break;
            }
#endif
        default:
            {
            /* Should never get here. Valid channel number is checked above. */
            break;
            }
    }

#if ((R_BSP_VERSION_MAJOR == 5) && (R_BSP_VERSION_MINOR >= 30)) || (R_BSP_VERSION_MAJOR >= 6)
    R_BSP_InterruptControl(BSP_INT_SRC_EMPTY, BSP_INT_CMD_FIT_INTERRUPT_ENABLE, &int_ctrl);
#endif

#if CMT_REG_PROTECT == 1
    /* Disable writing to MSTP registers. */
    R_BSP_RegisterProtectEnable(BSP_REG_PROTECT_LPC_CGC_SWR);
#endif
}
/***********************************************************************************************************************
* End of function power_off
***********************************************************************************************************************/

/***********************************************************************************************************************
* Function Name: cmt_counter_start
* Description  : Starts a CMT channel counting.
* Arguments    : channel -
*                    Channel number to use.
* Return Value : none
***********************************************************************************************************************/
static void cmt_counter_start (uint32_t channel)
{
    /* Enable compare match interrupt. */
    (*g_cmt_channels[channel]).CMCR.BIT.CMIE = 1;

    /* Clear counter. */
    (*g_cmt_channels[channel]).CMCNT = 0;

    /* Start counter channel. */
    switch (channel)
    {
        case 0:
            {
            /* Setup ICU registers. */
            IR(CMT0, CMI0)  = 0;                /* Clear any previously pending interrupts */
            IPR(CMT0, CMI0) = CMT_RX_CFG_IPR;   /* Set interrupt priority */
            R_BSP_InterruptRequestEnable(VECT(CMT0, CMI0));                /* Enable compare match interrupt */

            /* Start counter. */
            CMT.CMSTR0.BIT.STR0 = 1;
            break;
            }
        case 1:
            {
            /* Setup ICU registers. */
            IR(CMT1, CMI1)  = 0;                /* Clear any previously pending interrupts */
            IPR(CMT1, CMI1) = CMT_RX_CFG_IPR;   /* Set interrupt priority */
            R_BSP_InterruptRequestEnable(VECT(CMT1, CMI1));                /* Enable compare match interrupt */

            /* Start counter. */
            CMT.CMSTR0.BIT.STR1 = 1;
            break;
            }
#if   CMT_RX_NUM_CHANNELS == 4
        case 2:
            {
            /* Setup ICU registers. */
            IR(CMT2, CMI2)  = 0;                /* Clear any previously pending interrupts */
            IPR(CMT2, CMI2) = CMT_RX_CFG_IPR;   /* Set interrupt priority */
            R_BSP_InterruptRequestEnable(VECT(CMT2, CMI2));                /* Enable compare match interrupt */

            /* Start counter. */
            CMT.CMSTR1.BIT.STR2 = 1;
            break;
            }
        case 3:
            {
            /* Setup ICU registers. */
            IR(CMT3, CMI3)  = 0;                /* Clear any previously pending interrupts */
            IPR(CMT3, CMI3) = CMT_RX_CFG_IPR;   /* Set interrupt priority */
            R_BSP_InterruptRequestEnable(VECT(CMT3, CMI3));                /* Enable compare match interrupt */

            /* Start counter. */
            CMT.CMSTR1.BIT.STR3 = 1;
            break;
            }
#endif
        default:
        {
            /* Should never get here. Valid channel number is checked above. */
            break;
        }
    }
}
/***********************************************************************************************************************
* End of function cmt_counter_start
***********************************************************************************************************************/

/***********************************************************************************************************************
* Function Name: cmt_counter_start_priority
* Description  : Starts a CMT channel counting with interrupt priority.
* Arguments    : channel -
*                    Channel number to use.
*                priority -
*                    Which interrupt priority is used to assign.
* Return Value : none
***********************************************************************************************************************/
static void cmt_counter_start_priority (uint32_t channel, cmt_priority_t priority)
{
    /* Enable compare match interrupt. */
    (*g_cmt_channels[channel]).CMCR.BIT.CMIE = 1;

    /* Clear counter. */
    (*g_cmt_channels[channel]).CMCNT = 0;

    /* Start counter channel. */
    switch (channel)
    {
        case 0:
            {
            /* Setup ICU registers. */
            IR(CMT0, CMI0)  = 0;                /* Clear any previously pending interrupts */
            IPR(CMT0, CMI0) = priority;         /* Set interrupt priority */
            R_BSP_InterruptRequestEnable(VECT(CMT0, CMI0));                /* Enable compare match interrupt */

            /* Start counter. */
            CMT.CMSTR0.BIT.STR0 = 1;
            break;
            }
        case 1:
            {
            /* Setup ICU registers. */
            IR(CMT1, CMI1)  = 0;                /* Clear any previously pending interrupts */
            IPR(CMT1, CMI1) = priority;         /* Set interrupt priority */
            R_BSP_InterruptRequestEnable(VECT(CMT1, CMI1));                /* Enable compare match interrupt */

            /* Start counter. */
            CMT.CMSTR0.BIT.STR1 = 1;
            break;
            }
#if   CMT_RX_NUM_CHANNELS == 4
        case 2:
            {
            /* Setup ICU registers. */
            IR(CMT2, CMI2)  = 0;                /* Clear any previously pending interrupts */
            IPR(CMT2, CMI2) = priority;         /* Set interrupt priority */
            R_BSP_InterruptRequestEnable(VECT(CMT2, CMI2));                /* Enable compare match interrupt */

            /* Start counter. */
            CMT.CMSTR1.BIT.STR2 = 1;
            break;
            }
        case 3:
            {
            /* Setup ICU registers. */
            IR(CMT3, CMI3)  = 0;                /* Clear any previously pending interrupts */
            IPR(CMT3, CMI3) = priority;         /* Set interrupt priority */
            R_BSP_InterruptRequestEnable(VECT(CMT3, CMI3));                /* Enable compare match interrupt */

            /* Start counter. */
            CMT.CMSTR1.BIT.STR3 = 1;
            break;
            }
#endif
        default:
        {
            /* Should never get here. Valid channel number is checked above. */
            break;
        }
    }
}
/***********************************************************************************************************************
* End of function cmt_counter_start_priority
***********************************************************************************************************************/

/***********************************************************************************************************************
* Function Name: cmt_counter_stop
* Description  : Stops a CMT channel counting.
* Arguments    : channel -
*                    Channel number to use.
* Return Value : none
***********************************************************************************************************************/
static void cmt_counter_stop (uint32_t channel)
{
    /* Stop counter channel. */
    switch (channel)
    {
        case 0:
            {
            /* Stop counter. */
            CMT.CMSTR0.BIT.STR0 = 0;

            /* Set ICU registers. */
            R_BSP_InterruptRequestDisable(VECT(CMT0, CMI0));            /* Disable compare match interrupt */
            IPR(CMT0, CMI0) = 0;            /* Set interrupt priority */
            IR(CMT0, CMI0)  = 0;            /* Clear any previously pending interrupts */
            break;
            }
        case 1:
            {
            /* Stop counter. */
            CMT.CMSTR0.BIT.STR1 = 0;

            /* Set ICU registers. */
            R_BSP_InterruptRequestDisable(VECT(CMT1, CMI1));            /* Disable compare match interrupt */
            IPR(CMT1, CMI1) = 0;            /* Set interrupt priority */
            IR(CMT1, CMI1)  = 0;            /* Clear any previously pending interrupts */
            break;
            }
#if   CMT_RX_NUM_CHANNELS == 4
        case 2:
            {
            /* Stop counter. */
            CMT.CMSTR1.BIT.STR2 = 0;

            /* Set ICU registers. */
            R_BSP_InterruptRequestDisable(VECT(CMT2, CMI2));            /* Disable compare match interrupt */
            IPR(CMT2, CMI2) = 0;            /* Set interrupt priority */
            IR(CMT2, CMI2)  = 0;            /* Clear any previously pending interrupts */
            break;
            }
        case 3:
            {
            /* Stop counter. */
            CMT.CMSTR1.BIT.STR3 = 0;

            /* Set ICU registers. */
            R_BSP_InterruptRequestDisable(VECT(CMT3, CMI3));            /* Disable compare match interrupt */
            IPR(CMT3, CMI3) = 0;            /* Set interrupt priority */
            IR(CMT3, CMI3)  = 0;            /* Clear any previously pending interrupts */
            break;
            }
#endif
        default:
            {
            /* Should never get here. Valid channel number is checked above. */
            break;
            }
    }
}
/***********************************************************************************************************************
* End of function cmt_counter_stop
***********************************************************************************************************************/


/***********************************************************************************************************************
* Function Name: cmt_setup_channel
* Description  : Sets dividers and counter so compare match occurs at input frequency.
* Arguments    : channel - 
*                    Channel number to use.
*                frequency_hz -
*                    Frequency at which to set CMT channel
* Return Value : true -
*                    Channel setup completed.
*                false -
*                    Channel setup failed.
***********************************************************************************************************************/
static bool cmt_setup_channel (uint32_t channel, uint32_t frequency_hz)
{
    uint32_t i;
    bool     ret = false;

    /* Requested frequency must not be higher than PCLK. */
    if (((uint32_t)CMT_PCLK_HZ / 8) >= frequency_hz)
    {
        /* Choose clock. This is done by looking through the available dividers to see if we can match the frequency
           input by the user. */
        /* WAIT_LOOP */   
        for (i = 0; i < ((sizeof(g_cmt_clock_dividers))/(sizeof(g_cmt_clock_dividers[0]))); i++)
        {
            /* Determine minimum frequency this divider can hit. For example, if a PCLK/8 is used and PCLK is 48MHz, then
               the minimum frequency we can support is around 91.5Hz. This obtained by doing the following calculation:
               (PCLK / divider) / max_counter_value
               Example:
               (48,000,000 / 8) / 65,536 = 91.5 */
            if (frequency_hz > (((uint32_t)CMT_PCLK_HZ / g_cmt_clock_dividers[i]) / (uint32_t)CMT_RX_MAX_TIMER_TICKS))
            {
                /* Enable peripheral channel. */
                power_on(channel);

                /* We can use this divider. Figure out counter ticks needed for this frequency. */
                (*g_cmt_channels[channel]).CMCOR = (uint16_t)((((uint32_t)CMT_PCLK_HZ /g_cmt_clock_dividers[i])/frequency_hz) -1 );


                /* Set clock divider to be used. */
                (*g_cmt_channels[channel]).CMCR.BIT.CKS = i;

                /* Save that we did find a valid divider. */
                ret = true;

                /* No need to check other dividers. */
                break;
            }
        }
    }

    return ret;
}
/***********************************************************************************************************************
* End of function cmt_setup_channel
***********************************************************************************************************************/

/***********************************************************************************************************************
* Function Name: R_CMT_GetVersion
********************************************************************************************************************//**
* @brief This function returns the driver version number at runtime
* @return Version number with major and minor version digits packed into a single 32-bit value.
* @details The function returns the version of this module.
* The version number is encoded such that the top 2 bytes are the major version number
* and the bottom 2 bytes are the minor version number.
*/
uint32_t R_CMT_GetVersion (void)
{
    /* These version macros are defined in r_cmt_rx_if.h. */
    return ((((uint32_t)CMT_RX_VERSION_MAJOR) << 16) | (uint32_t)CMT_RX_VERSION_MINOR);
}
/* End of function R_CMT_GetVersion */

/***********************************************************************************************************************
* Function Name: cmt_isr_common
* Description  : Common part of ISR handling. Put here and made inline so changes will always apply to all channels.
* Arguments    : channel -
*                    Which channel this is for.
* Return Value : none
***********************************************************************************************************************/
#if BSP_CFG_RTOS_USED == 0      /* Non-OS */
R_BSP_PRAGMA_STATIC_INLINE(cmt_isr_common)
void cmt_isr_common (uint32_t channel)
{
    /* If this is one-shot mode then stop timer. */
    if (CMT_RX_MODE_ONE_SHOT == g_cmt_modes[channel])
    {
        R_CMT_Stop(channel);
    }

    /* Check for valid callback pointer before calling it. */
    if ((NULL != g_cmt_callbacks[channel]) && ((uint32_t)FIT_NO_FUNC != (uint32_t)g_cmt_callbacks[channel]))
    {
        /* Valid callback found, jump to it. */
        g_cmt_callbacks[channel]((void *)&channel);
    }
}
/* End of function cmt_isr_common */
#elif BSP_CFG_RTOS_USED == 1    /* FreeRTOS */
R_BSP_PRAGMA_STATIC_INLINE(cmt_isr_common)
void cmt_isr_common (uint32_t channel)
{
    /* If this is one-shot mode then stop timer. */
    if (CMT_RX_MODE_ONE_SHOT == g_cmt_modes[channel])
    {
        R_CMT_Stop(channel);
    }

    /* Check for valid callback pointer before calling it. */
    if ((NULL != g_cmt_callbacks[channel]) && ((uint32_t)FIT_NO_FUNC != (uint32_t)g_cmt_callbacks[channel]))
    {
        /* Valid callback found, jump to it. */
        g_cmt_callbacks[channel]((void *)&channel);
    }
}
/* End of function cmt_isr_common */
#elif BSP_CFG_RTOS_USED == 2    /* SEGGER embOS */
#elif BSP_CFG_RTOS_USED == 3    /* Micrium MicroC/OS */
#elif BSP_CFG_RTOS_USED == 4    /* Renesas RI600V4 & RI600PX */
    #if CMT_RX_NUM_CHANNELS == 2
        #if ((BSP_CFG_RTOS_SYSTEM_TIMER != 0 && _RI_TRACE_TIMER != 0) ||\
            (BSP_CFG_RTOS_SYSTEM_TIMER != 1 && _RI_TRACE_TIMER != 1))
            R_BSP_PRAGMA_STATIC_INLINE(cmt_isr_common)
            void cmt_isr_common (uint32_t channel)
            {
                /* If this is one-shot mode then stop timer. */
                if (CMT_RX_MODE_ONE_SHOT == g_cmt_modes[channel])
                {
                    R_CMT_Stop(channel);
                }

                /* Check for valid callback pointer before calling it. */
                if ((NULL != g_cmt_callbacks[channel]) && ((uint32_t)FIT_NO_FUNC != (uint32_t)g_cmt_callbacks[channel]))
                {
                    /* Valid callback found, jump to it. */
                    g_cmt_callbacks[channel]((void *)&channel);
                }
            }
            /* End of function cmt_isr_common */
        #endif
    #else
        R_BSP_PRAGMA_STATIC_INLINE(cmt_isr_common)
        void cmt_isr_common (uint32_t channel)
        {
            /* If this is one-shot mode then stop timer. */
            if (CMT_RX_MODE_ONE_SHOT == g_cmt_modes[channel])
            {
                R_CMT_Stop(channel);
            }

            /* Check for valid callback pointer before calling it. */
            if ((NULL != g_cmt_callbacks[channel]) && ((uint32_t)FIT_NO_FUNC != (uint32_t)g_cmt_callbacks[channel]))
            {
                /* Valid callback found, jump to it. */
                g_cmt_callbacks[channel]((void *)&channel);
            }
        }
        /* End of function cmt_isr_common */
    #endif /* End of  CMT_RX_NUM_CHANNELS */
#else /* Non-OS & others */
    R_BSP_PRAGMA_STATIC_INLINE(cmt_isr_common)
    void cmt_isr_common (uint32_t channel)
    {
        /* If this is one-shot mode then stop timer. */
        if (CMT_RX_MODE_ONE_SHOT == g_cmt_modes[channel])
        {
            R_CMT_Stop(channel);
        }

        /* Check for valid callback pointer before calling it. */
        if ((NULL != g_cmt_callbacks[channel]) && ((uint32_t)FIT_NO_FUNC != (uint32_t)g_cmt_callbacks[channel]))
        {
            /* Valid callback found, jump to it. */
            g_cmt_callbacks[channel]((void *)&channel);
        }
    }
#endif /* BSP_CFG_RTOS_USED */
/* End of function cmt_isr_common */

/***********************************************************************************************************************
* Function Name: cmt0_isr
* Description  : Interrupt when compare match for this channel occurs.
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
#if BSP_CFG_RTOS_USED == 0      /* Non-OS */
R_BSP_PRAGMA_STATIC_INTERRUPT (cmt0_isr,VECT(CMT0, CMI0))
R_BSP_ATTRIB_STATIC_INTERRUPT void cmt0_isr (void)
{
    cmt_isr_common(0);
}
#elif BSP_CFG_RTOS_USED == 1    /* FreeRTOS */
    #if (BSP_CFG_RTOS_SYSTEM_TIMER != 0)
        R_BSP_PRAGMA_STATIC_INTERRUPT (cmt0_isr,VECT(CMT0, CMI0))
        R_BSP_ATTRIB_STATIC_INTERRUPT void cmt0_isr (void)
        {
            cmt_isr_common(0);
        }
    #endif /* (BSP_CFG_RTOS_SYSTEM_TIMER != 0) */
#elif BSP_CFG_RTOS_USED == 2    /* SEGGER embOS */
#elif BSP_CFG_RTOS_USED == 3    /* Micrium MicroC/OS */
#elif BSP_CFG_RTOS_USED == 4    /* Renesas RI600V4 & RI600PX */
    #if (BSP_CFG_RTOS_SYSTEM_TIMER != 0 && _RI_TRACE_TIMER != 0)
        void cmt0_isr (void)
        {
            cmt_isr_common(0);
        }
    #endif /* (BSP_CFG_RTOS_SYSTEM_TIMER != 0 && _RI_TRACE_TIMER != 0) */
#else /* Non-OS & others */
R_BSP_PRAGMA_STATIC_INTERRUPT (cmt0_isr,VECT(CMT0, CMI0))
R_BSP_ATTRIB_STATIC_INTERRUPT void cmt0_isr (void)
{
    cmt_isr_common(0);
}
#endif/* BSP_CFG_RTOS_USED */
/* End of function cmt0_isr */

/***********************************************************************************************************************
* Function Name: cmt1_isr
* Description  : Interrupt when compare match for this channel occurs.
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
#if BSP_CFG_RTOS_USED == 0      /* Non-OS */
R_BSP_PRAGMA_STATIC_INTERRUPT (cmt1_isr,VECT(CMT1, CMI1))
R_BSP_ATTRIB_STATIC_INTERRUPT void cmt1_isr (void)
{
    cmt_isr_common(1);
}
#elif BSP_CFG_RTOS_USED == 1    /* FreeRTOS */
    #if (BSP_CFG_RTOS_SYSTEM_TIMER != 1)
        R_BSP_PRAGMA_STATIC_INTERRUPT (cmt1_isr,VECT(CMT1, CMI1))
        R_BSP_ATTRIB_STATIC_INTERRUPT void cmt1_isr (void)
        {
            cmt_isr_common(1);
        }
    #endif /* (BSP_CFG_RTOS_SYSTEM_TIMER != 1) */
#elif BSP_CFG_RTOS_USED == 2    /* SEGGER embOS */
#elif BSP_CFG_RTOS_USED == 3    /* Micrium MicroC/OS */
#elif BSP_CFG_RTOS_USED == 4    /* Renesas RI600V4 & RI600PX */
    #if (BSP_CFG_RTOS_SYSTEM_TIMER != 1 && _RI_TRACE_TIMER != 1)
        void cmt1_isr (void)
        {
            cmt_isr_common(1);
        }
    #endif /* (BSP_CFG_RTOS_SYSTEM_TIMER != 1 && _RI_TRACE_TIMER != 1) */
#else /* Non-OS & others */
R_BSP_PRAGMA_STATIC_INTERRUPT (cmt1_isr,VECT(CMT1, CMI1))
R_BSP_ATTRIB_STATIC_INTERRUPT void cmt1_isr (void)
{
    cmt_isr_common(1);
}
#endif/* BSP_CFG_RTOS_USED */
/* End of function cmt1_isr */

#if   CMT_RX_NUM_CHANNELS == 4

/***********************************************************************************************************************
* Function Name: cmt2_isr
* Description  : Interrupt when compare match for this channel occurs.
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
#if BSP_CFG_RTOS_USED == 0      /* Non-OS */
R_BSP_PRAGMA_STATIC_INTERRUPT(cmt2_isr,VECT(CMT2, CMI2))
R_BSP_ATTRIB_STATIC_INTERRUPT void cmt2_isr (void)
{
    cmt_isr_common(2);
}
#elif BSP_CFG_RTOS_USED == 1    /* FreeRTOS */
    #if (BSP_CFG_RTOS_SYSTEM_TIMER != 2)
        R_BSP_PRAGMA_STATIC_INTERRUPT(cmt2_isr,VECT(CMT2, CMI2))
        R_BSP_ATTRIB_STATIC_INTERRUPT void cmt2_isr (void)
        {
            cmt_isr_common(2);
        }
    #endif /* (BSP_CFG_RTOS_SYSTEM_TIMER != 2) */
#elif BSP_CFG_RTOS_USED == 2    /* SEGGER embOS */
#elif BSP_CFG_RTOS_USED == 3    /* Micrium MicroC/OS */
#elif BSP_CFG_RTOS_USED == 4    /* Renesas RI600V4 & RI600PX */
    #if (BSP_CFG_RTOS_SYSTEM_TIMER != 2 && _RI_TRACE_TIMER != 2)
        void cmt2_isr (void)
        {
            cmt_isr_common(2);
        }
    #endif /* (BSP_CFG_RTOS_SYSTEM_TIMER != 2 && _RI_TRACE_TIMER != 2) */
#else /* Non-OS & others */
R_BSP_PRAGMA_STATIC_INTERRUPT(cmt2_isr,VECT(CMT2, CMI2))
R_BSP_ATTRIB_STATIC_INTERRUPT void cmt2_isr (void)
{
    cmt_isr_common(2);
}
#endif/* BSP_CFG_RTOS_USED */
/* End of function cmt2_isr */

/***********************************************************************************************************************
* Function Name: cmt3_isr
* Description  : Interrupt when compare match for this channel occurs.
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
#if BSP_CFG_RTOS_USED == 0      /* Non-OS */
R_BSP_PRAGMA_STATIC_INTERRUPT( cmt3_isr,VECT(CMT3, CMI3))
R_BSP_ATTRIB_STATIC_INTERRUPT void cmt3_isr (void)
{
    cmt_isr_common(3);
}
#elif BSP_CFG_RTOS_USED == 1    /* FreeRTOS */
    #if (BSP_CFG_RTOS_SYSTEM_TIMER != 3)
        R_BSP_PRAGMA_STATIC_INTERRUPT (cmt3_isr,VECT(CMT3, CMI3))
        R_BSP_ATTRIB_STATIC_INTERRUPT void cmt3_isr (void)
        {
            cmt_isr_common(3);
        }
    #endif /* (BSP_CFG_RTOS_SYSTEM_TIMER != 3) */
#elif BSP_CFG_RTOS_USED == 2    /* SEGGER embOS */
#elif BSP_CFG_RTOS_USED == 3    /* Micrium MicroC/OS */
#elif BSP_CFG_RTOS_USED == 4    /* Renesas RI600V4 & RI600PX */
    #if (BSP_CFG_RTOS_SYSTEM_TIMER != 3 && _RI_TRACE_TIMER != 3)
        void cmt3_isr (void)
        {
            cmt_isr_common(3);
        }
    #endif /* (BSP_CFG_RTOS_SYSTEM_TIMER != 3 && _RI_TRACE_TIMER != 3) */
#else /* Non-OS & others */
R_BSP_PRAGMA_STATIC_INTERRUPT( cmt3_isr,VECT(CMT3, CMI3))
R_BSP_ATTRIB_STATIC_INTERRUPT void cmt3_isr (void)
{
    cmt_isr_common(3);
}
#endif/* BSP_CFG_RTOS_USED */
/* End of function cmt3_isr */
#endif /* End of #if CMT_RX_NUM_CHANNELS == 4 */
