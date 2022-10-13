/**********************************************************************************************************************
 * DISCLAIMER
 * This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
 * other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
 * applicable laws, including copyright laws.
 * THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
 * THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
 * EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
 * SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO
 * THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
 * this software. By using this software, you agree to the additional terms and conditions found by accessing the
 * following link:
 * http://www.renesas.com/disclaimer
 *
 * Copyright (C) 2021 Renesas Electronics Corporation. All rights reserved.
 *********************************************************************************************************************/
/**********************************************************************************************************************
 * File Name    : r_wifi_sx_ulpgn_private.h
 * Version      : 1.0
 * Description  : Private functions definition for SX-ULPGN.
 *********************************************************************************************************************/
/**********************************************************************************************************************
 * History : DD.MM.YYYY Version  Description
 *         : dd.mm.2021 1.00     First Release
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Includes   <System Includes> , "Project Includes"
 *********************************************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

#include "r_wifi_sx_ulpgn_if.h"
#include "r_wifi_sx_ulpgn_os_wrap.h"
#include "r_sci_rx_if.h"
#include "r_byteq_if.h"
#include "r_sci_rx_pinset.h"

/**********************************************************************************************************************
 Macro definitions
 *********************************************************************************************************************/
#ifndef R_WIFI_SX_ULPGN_PRIVATE_H
#define R_WIFI_SX_ULPGN_PRIVATE_H

#if !defined(WIFI_CFG_SCI_CHANNEL)
#error "Error! Need to define WIFI_CFG_SCI_CHANNEL in r_wifi_sx_ulpgn_config.h"
#endif

#if !defined(WIFI_CFG_SCI_SECOND_CHANNEL)
#error "Error! Need to define WIFI_CFG_SCI_SECOND_CHANNEL in r_wifi_sx_ulpgn_config.h"
#endif

#if SCI_CFG_TEI_INCLUDED != 1
#error "Error! Need to set SCI_CFG_TEI_INCLUDED is '1' in r_sci_rx_config.h"
#endif

/* Configuration */
#define UART_TBL_MAX                   (2)       // HSUART port table (default:2)
#define SOCK_TBL_MAX                   (WIFI_CFG_CREATABLE_SOCKETS)            // Socket table (default:4)
#define SOCK_BUF_MAX                   (WIFI_CFG_SOCKETS_RECEIVE_BUFFER_SIZE)  // Socket buffer
#define CERT_PROFILE_MAX               (5)       // Number of Certificate profiles
#define CERT_HOSTNAME_MAX              (256)     // Number of Certificate host name

#define ATCMD_RESP_TIMEOUT             (15000)   // Timeout threshold for AT command response (msec)
#define SX_ULPGN_AT_CMD_BUF_MAX        (512)
#define SX_ULPGN_AT_RESP_BUF_MAX       (2048)
#define SX_ULPGN_AT_TIMEOUT            (10000)
#define SX_ULPGN_BAUD_DEFAULT          (115200)
#define SX_ULPGN_ATBSIZE               (1420)

/* Change socket */
#define SOCKET_CHANGE_DELAY            (30)
#define ATUSTAT_ATTEMPTS               (2)

#if defined(__CCRX__) || defined(__ICCRX__) || defined(__RX__)
/* Reset port pin macros.  */
#define WIFI_RESET_DDR(x, y)           (WIFI_RESET_DDR_PREPROC(x, y))
#define WIFI_RESET_DDR_PREPROC(x, y)   ((PORT ## x .PDR.BIT.B ## y))
#define WIFI_RESET_DR(x, y)            (WIFI_RESET_DR_PREPROC(x, y))
#define WIFI_RESET_DR_PREPROC(x, y)    ((PORT ## x .PODR.BIT.B ## y))

/* RTS port pin macros.  */
#define WIFI_RTS_DDR(x, y)             (WIFI_RTS_DDR_PREPROC(x, y))
#define WIFI_RTS_DDR_PREPROC(x, y)     ((PORT ## x .PDR.BIT.B ## y))
#define WIFI_RTS_DR(x, y)              (WIFI_RTS_DR_PREPROC(x, y))
#define WIFI_RTS_DR_PREPROC(x, y)      ((PORT ## x .PODR.BIT.B ## y))
#endif

/* IP address(xxx.xxx.xxx.xxx) into ULONG */
#define IPADR_UB_TO_UL(adr1, adr2, adr3, adr4) \
    ((((adr1) & 0x000000FF) << 24) | (((adr2) & 0x000000FF) << 16) |\
    (((adr3) & 0x000000FF) << 8)  | (((adr4) & 0x000000FF)))

/* Debug mode */
#define DEBUGLOG                        (0)
#define DEBUG_ATCMD                     (0)

/* Debug print mode */
#define FREERTOS_IDT                    (0)
#if FREERTOS_IDT != 0
#define DBG_PRINTF                      vLoggingPrintf
#else
#define DBG_PRINTF                      printf
#endif

/**********************************************************************************************************************
 Global Typedef definitions
 *********************************************************************************************************************/
/* UART table information */
typedef struct
{
    sci_hdl_t   sci_hdl;
    sci_cfg_t   sci_config;
    volatile uint8_t tx_end_flag;
} st_uart_tbl_t;

/* Socket Timer */
typedef struct
{
    OS_TICK   threshold;        /* Timeout threshold */
    OS_TICK   tick_sta;         /* Tick of Timer start  */
} st_sock_timer_t;

/* Socket table information */
typedef struct
{
    uint8_t     status;
    uint8_t     ipver;
    uint8_t     protocol;
    byteq_hdl_t byteq_hdl;
    uint32_t    put_err_cnt;
    uint8_t     recv_buf[SOCK_BUF_MAX];
    st_sock_timer_t timer_tx;
    st_sock_timer_t timer_rx;
    struct
    {
        uint8_t    enable;
        uint8_t    cert_id;
    } ssl;
} st_sock_tbl_t;

/* Certificate profile */
typedef struct
{
    char        host_name[CERT_HOSTNAME_MAX];   /* host name      */
    uint32_t    host_address;   /* host address   */
    uint8_t     cert_id;        /* certificate id */
} st_cert_profile_t;

/* WIFI FIT module status */
typedef enum
{
    MODULE_DISCONNECTED = 0,    /* Disconnected WIFI module */
    MODULE_CONNECTED,           /* Connected WIFI module    */
    MODULE_ACCESSPOINT,         /* Connected access point   */
} e_wifi_module_status_t;

/* return code for common functions */
typedef enum
{
    E_OK      = 0,              /* return = OK      */
    E_FAIL    = -1              /* return = failure */
} e_func_result_t;

/* UART HW flow control */
typedef enum
{
    RTS_OFF   = 0,              /* RTS = ON         */
    RTS_ON                      /* RTS = OFF        */
} e_flow_ctrl_t;

/* Mode in single channel */
typedef enum
{
    MODE_COMMAND     = 0,       /* Command mode     */
    MODE_TRANSPARENT            /* Transparent mode */
} e_single_ch_mode_t;

/* Result code */
typedef enum
{
    AT_OK               = 0,    /* OK               */
    AT_CONNECT          = 1,    /* CONNECT          */
    AT_RING             = 2,    /* RING             */
    AT_NOCARRIER        = 3,    /* NO CARRIER       */
    AT_ERROR            = 4,    /* ERROR            */
    AT_NODIALTONE       = 6,    /* NO DIALTONE      */
    AT_BUSY             = 7,    /* BUSY             */
    AT_NOANSER          = 8,    /* NO ANSWER        */
    AT_INTERNAL_TIMEOUT = 253,  /* INTERNAL TIMEOUT */
    AT_INTERNAL_ERROR   = 254,  /* INTERNAL ERROR   */
    AT_MAX              = 255   /* STOPPER          */
} e_rslt_code_t;

typedef enum
{
    DATA_NOT_FOUND = 0,
    DATA_FOUND,
} e_atcmd_read_t;

/* Cast uint32_t -> uint8_t */
typedef union
{
    uint32_t    ul;
    struct
    {
        uint8_t hh;
        uint8_t hl;
        uint8_t lh;
        uint8_t ll;
    } b;
} u_cast_t;

/**********************************************************************************************************************
 External global variables
 *********************************************************************************************************************/
extern st_uart_tbl_t g_uart_tbl[UART_TBL_MAX];

/**********************************************************************************************************************
 Exported global functions
 *********************************************************************************************************************/

/* r_wifi_sx_ulpgn_atcmd.c */
/**********************************************************************************************************************
 * Function Name: at_send
 * Description  : Send AT commands on UART.
 * Arguments    : port
 *                cmd
 * Return Value : None
 *********************************************************************************************************************/
void at_send (uint8_t port, const char *cmd, ...);

/**********************************************************************************************************************
 * Function Name: at_recv
 * Description  : Receive response and return RESULT CODE.
 * Arguments    : port
 * Return Value : AT_OK
 *                AT_CONNECT
 *                AT_RING
 *                AT_NOCARRIER
 *                AT_ERROR
 *                AT_NODIALTONE
 *                AT_BUSY
 *                AT_NOANSER
 *                AT_TIMEOUT
 *                AT_INTERNAL_TIMEOUT
 *                AT_INTERNAL_ERROR
 *********************************************************************************************************************/
e_rslt_code_t at_recv (uint8_t port);

/**********************************************************************************************************************
 * Function Name: at_exec
 * Description  : Send AT commands and Receive response on UART.
 * Arguments    : port
 *                cmd
 * Return Value : Same as at_recv() function.
 *********************************************************************************************************************/
e_rslt_code_t at_exec (uint8_t port, const char *cmd, ...);

/**********************************************************************************************************************
 * Function Name: at_read
 * Description  : Read buffer with prefix. (example at_read("abcd = %s\r\n", p_char) )
 * Arguments    : response_fmt
 * Return Value : 0     : data not found
 *                other : data found
 *********************************************************************************************************************/
uint32_t at_read (const char *response_fmt, ...);

/**********************************************************************************************************************
 * Function Name: at_read_wo_prefix
 * Description  : Read buffer without prefix. (example at_read_wo_prefix("%s\r\n", p_char) )
 * Arguments    : response_fmt
 * Return Value : same as vsscanf() function.
 *********************************************************************************************************************/
int32_t at_read_wo_prefix (const char *response_fmt, ...);

/**********************************************************************************************************************
 * Function Name: at_move_to_next_line
 * Description  : Move pointer to next line of response buffer.
 * Arguments    : None
 * Return Value : None
 *********************************************************************************************************************/
void at_move_to_next_line (void);

/**********************************************************************************************************************
 * Function Name: at_move_to_first_line
 * Description  : Set pointer to first line of response buffer.
 * Arguments    : None
 * Return Value : None
 *********************************************************************************************************************/
void at_move_to_first_line (void);

/**********************************************************************************************************************
 * Function Name: at_get_current_line
 * Description  : Get pointer on current line of response buffer.
 * Arguments    : None
 * Return Value : Pointer of current line
 *********************************************************************************************************************/
uint8_t * at_get_current_line (void);

/**********************************************************************************************************************
 * Function Name: post_err_event
 * Description  : Post error event to user callback function.
 * Arguments    : err_event - error event
 *                sock_idx  - socket index
 * Return Value : None
 *********************************************************************************************************************/
void post_err_event (wifi_err_event_enum_t err_event, uint32_t sock_idx);

#endif /* R_WIFI_SX_ULPGN_PRIVATE_H */
