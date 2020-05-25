/*
 * Copyright 2019, Cypress Semiconductor Corporation or a subsidiary of 
 * Cypress Semiconductor Corporation. All Rights Reserved.
 * 
 * This software, associated documentation and materials ("Software"),
 * is owned by Cypress Semiconductor Corporation
 * or one of its subsidiaries ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products. Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 */
#include <stdint.h>
#include "wwd_constants.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{

    unsigned int    extended_mode : 1;
    /* delay which can be introduced from the end of address set stage, till memory controller */
    /* probes the data bus */
    /* it is given in nanoseconds, the driver will convert it to number of HCLK clock cycles */
    /* the value should lie in the range 0*HCLK up to 15*HCLK */
    unsigned int    data_phase_duration;
    unsigned int    address_set_duration; /* in nanoseconds */
    /* enable burst protocol for read operations */
    /* applicable only for synchronous memories */
    unsigned int    rd_burst : 1;
    /* enable burst protocol for write operations */
    /* applicable only for synchronous memories */
    unsigned int    wr_burst : 1;
    /* enables the NWAIT signal in asynchronous memories */
    unsigned int    async_wait_enabled : 1;
    /* enables the wait enabled signal in synchronous memories */
    unsigned int    sync_wait_enabled : 1;
    /* the polarity of the of the wait signal, 0 - active low, 1 - active high */
    unsigned int    wait_polarity : 1;
    /* specifies the access mode, when extended mode is selected */
    /* a=0, b=1, c=2, or d=3 */
    unsigned int    access_mode : 2;
    /* applicable only for synchronous devices. Number of clocks to issue from the */
    /* moment when address is set till moment when the data is set up  */
    /* in nanoseconds, the memory driver will convert to the value in the range 1*CLK up to 17*CLK */
    unsigned int    data_latency;
    /* output clock frequency, will be used for synchronous memories only */
    /* the memory driver will pick up the proper clock divider using this value */
    /* If the result value is less than that 1/(2*HCLK), it will select the maximum clock freq which is /(2*HCLK) */
    /* It will mean that the memory controller can't support this frequency */
    /* If the result value is more than 1/(17*HCLK), it will return with an error */
    uint32_t        clock_freq;
    /* minimum time between consecutive transactions, this value is in nanoseconds */
    unsigned int    bus_turnaround;
    /* address hold phase duration */
    unsigned int    address_hold_duration;
    /* address set phase duration, applicable only for asynchronous memories */



    /* enable extended mode, when mode and settings for writing transactions are different from modes and */
    /* settings for reading transactions */
    /* and are configured with settings prepended with wr */
    unsigned int    extended_mod : 1;
/* { */
    unsigned int    wr_address_set_duration; /* in nanoseconds */
    unsigned int    wr_address_hold_duration; /* in nanoseconds */
    unsigned int    wr_data_phase_duration; /* in nanoseconds */
    unsigned int    wr_datast; /* in nanoseconds */
    /* minimum time between consecutive writing transactions, this value is in nanoseconds */
    unsigned int    wr_bus_turnaround;
    /* same as access_mode, we are selecting a write mode */
    unsigned int    wr_access_mode: 2;

    /* Number of clocks to issue from the */
    /* moment when address is set till moment when the data is set up  */
    /* in nanoseconds, the memory driver will convert to the value in the range 1*CLK up to 17*CLK */
    unsigned int    wr_data_latency;
/* } */
}stm32f4xx_platform_nor_sram_t;


typedef struct
{
    unsigned int    extended_mode : 1;
}stm32f4xx_platform_nand_t;

#ifdef __cplusplus
} /* extern "C" */
#endif
