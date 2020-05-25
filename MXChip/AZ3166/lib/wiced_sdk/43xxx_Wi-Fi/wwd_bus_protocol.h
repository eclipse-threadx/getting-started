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
#ifndef INCLUDED_SDIO_WWD_BUS_PROTOCOL_H
#define INCLUDED_SDIO_WWD_BUS_PROTOCOL_H

#include "wwd_buffer.h"
#include "internal/wwd_thread_internal.h"
#include "platform/wwd_sdio_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/
#define BIT_MASK( x )         (( 1 << x ) - 1 )

#undef WWD_BUS_HAS_HEADER

#define WWD_BUS_HEADER_SIZE                     ( 0 )

#define WWD_BUS_USE_STATUS_REPORT_SCHEME        ( 1 == 0 )

#define WWD_BUS_MAX_BACKPLANE_TRANSFER_SIZE     ( WICED_PAYLOAD_MTU )
#define WWD_BUS_BACKPLANE_READ_PADD_SIZE        ( 0 )

/******************************************************
 *             Structures
 ******************************************************/
#pragma pack(1)
typedef struct
{
    unsigned char stuff_bits;
    unsigned int  ocr :24;
} sdio_cmd5_argument_t;

typedef struct
{
    unsigned int  _unique2         : 9; /* 0-8   */
    unsigned int  register_address :17; /* 9-25  */
    unsigned int  _unique          : 2; /* 26-27 */
    unsigned int  function_number  : 3; /* 28-30 */
    unsigned int  rw_flag          : 1; /* 31    */
} sdio_cmd5x_argument_t;

typedef struct
{
    uint8_t       write_data;           /* 0 - 7 */
    unsigned int  _stuff2          : 1; /* 8     */
    unsigned int  register_address :17; /* 9-25  */
    unsigned int  _stuff           : 1; /* 26    */
    unsigned int  raw_flag         : 1; /* 27    */
    unsigned int  function_number  : 3; /* 28-30 */
    unsigned int  rw_flag          : 1; /* 31    */
} wwd_bus_sdio_cmd52_argument_t;

typedef struct
{
    unsigned int  count            : 9; /* 0-8   */
    unsigned int  register_address :17; /* 9-25  */
    unsigned int  op_code          : 1; /* 26    */
    unsigned int  block_mode       : 1; /* 27    */
    unsigned int  function_number  : 3; /* 28-30 */
    unsigned int  rw_flag          : 1; /* 31    */
} wwd_bus_sdio_cmd53_argument_t;

typedef union
{
    uint32_t              value;
    sdio_cmd5_argument_t  cmd5;
    sdio_cmd5x_argument_t cmd5x;
    wwd_bus_sdio_cmd52_argument_t cmd52;
    wwd_bus_sdio_cmd53_argument_t cmd53;
} sdio_cmd_argument_t;

typedef struct
{
    unsigned int  ocr              :24; /* 0-23  */
    unsigned int  stuff_bits       : 3; /* 24-26 */
    unsigned int  memory_present   : 1; /* 27    */
    unsigned int  function_count   : 3; /* 28-30 */
    unsigned int  c                : 1; /* 31    */
} sdio_response4_t;

typedef struct
{
    uint8_t       data;                /* 0-7   */
    uint8_t       response_flags;       /* 8-15  */
    uint16_t      stuff;               /* 16-31 */
} sdio_response5_t;

typedef struct
{
    uint16_t      card_status;          /* 0-15  */
    uint16_t      rca;                 /* 16-31 */
} sdio_response6_t;

typedef union
{
    uint32_t                    value;
    sdio_response4_t            r4;
    sdio_response5_t            r5;
    sdio_response6_t            r6;
} sdio_response_t;

#pragma pack()

/******************************************************
 *             Macros
 ******************************************************/
#ifdef WWD_ENABLE_STATS
#define WWD_BUS_STATS_INCREMENT_VARIABLE( var )                            \
    do { wwd_bus_stats.var++; } while ( 0 )

#define WWD_BUS_STATS_CONDITIONAL_INCREMENT_VARIABLE( condition, var )     \
    do { if ( condition ) { wwd_bus_stats.var++; }} while ( 0 )

typedef struct
{
    uint32_t cmd52;             /* Number of cmd52 reads/writes issued */
    uint32_t cmd53_read;        /* Number of cmd53 reads */
    uint32_t cmd53_write;       /* Number of cmd53 writes */
    uint32_t cmd52_fail;        /* Number of cmd52 read/write fails */
    uint32_t cmd53_read_fail;   /* Number of cmd53 read fails */
    uint32_t cmd53_write_fail;  /* Number of cmd53 write fails */
    uint32_t oob_intrs;         /* Number of OOB interrupts generated by wlan chip */
    uint32_t sdio_intrs;        /* Number of SDIO interrupts generated by wlan chip */
    uint32_t error_intrs;       /* Number of SDIO error interrupts generated by wlan chip */
    uint32_t read_aborts;       /* Number of times read aborts are called */
} wwd_bus_stats_t;

extern wwd_bus_stats_t wwd_bus_stats;

#else /* WWD_ENABLE_STATS */
#define WWD_BUS_STATS_INCREMENT_VARIABLE( var )
#define WWD_BUS_STATS_CONDITIONAL_INCREMENT_VARIABLE( condition, var )
#endif /* WWD_ENABLE_STATS */

/******************************************************
 *             Structures
 ******************************************************/

/******************************************************
 *             Function declarations
 ******************************************************/

/******************************************************
 *             Global variables
 ******************************************************/

#ifdef __cplusplus
} /*extern "C" */
#endif

#endif /* ifndef INCLUDED_SDIO_WWD_BUS_PROTOCOL_H */
