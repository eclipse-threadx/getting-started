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
 * File Name    : r_wifi_sx_ulpgn_api.c
 * Version      : 1.0
 * Description  : API functions definition for SX ULPGN.
 *********************************************************************************************************************/
/**********************************************************************************************************************
 * History : DD.MM.YYYY Version  Description
 *         : DD.MM.YYYY 1.00     First Release
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Includes   <System Includes> , "Project Includes"
 *********************************************************************************************************************/
#include "r_wifi_sx_ulpgn_private.h"

/**********************************************************************************************************************
 Macro definitions
 *********************************************************************************************************************/
#define PORT_HSUART1         (0)        // Port number for HSUART1
#define PORT_HSUART2         (1)        // Port number for HSUART2
#define MUTEX_TX             (0x01)     // WIFI API(TX)
#define MUTEX_RX             (0x02)     // WIFI API(RX)

/**********************************************************************************************************************
 Local Typedef definitions
 *********************************************************************************************************************/
/* SCI configuration */
typedef struct
{
    sci_ch_t ch;           /* SCI channel */
    void (*func)(void);    /* Function name of SCI Port configuration */
    uint16_t tx_size;      /* TX buffer size */
    uint16_t rx_size;      /* RX buffer size */
} st_sci_conf_t;

/**********************************************************************************************************************
 Exported global variables
 *********************************************************************************************************************/
uint8_t  g_cmd_port;
uint8_t  g_data_port;
st_uart_tbl_t g_uart_tbl[UART_TBL_MAX];
st_sock_tbl_t g_sock_tbl[SOCK_TBL_MAX];
volatile uint8_t g_now_sock_idx  = 0;
volatile uint8_t g_prev_sock_idx = 0;
volatile uint8_t g_sock_tbl_index_old = 0;

/**********************************************************************************************************************
 Private (static) variables and functions
 *********************************************************************************************************************/
/* Initialize */
static void initialize_memory (void);

/* System state control */
static void wifi_system_state_set (e_wifi_module_status_t state);
static e_wifi_module_status_t wifi_system_state_get (void);

/* Mode in single channel (command or transparent) */
static void single_ch_mode_set (uint8_t mode);
static uint8_t single_ch_mode_get (void);
static int32_t enter_command_mode (void);
static int32_t enter_transparent_mode (void);

/* Sub functions */
static wifi_err_t disconnect_ap_sub (void);
static wifi_err_t close_socket_sub (uint8_t sock_idx);
static uint32_t get_ipaddr (wifi_ip_configuration_t * p_cfg);
static uint32_t get_ipaddr_by_name (uint8_t * name);
static int32_t get_uart_statistics (uint32_t * p_recv, uint32_t * p_sent);
static int32_t get_server_certificate (wifi_certificate_infomation_t * p_cert);
static int32_t change_socket_index (uint8_t sock_idx);
static uint32_t get_statictics_prev_socket (void);

/* Mutex control */
static wifi_err_t mutex_create (void);
static int32_t mutex_take (uint8_t mutex_flag);
static void mutex_give (uint8_t mutex_flag);

/* BYTEQ control for socket */
static wifi_err_t socket_byteq_open (void);
static void socket_byteq_close (void);

/* WIFI module control */
static void sx_ulpgn_hw_reset (void);
static void sx_ulpgn_close (void);

/* FIT module Wrapper */
static sci_err_t wrap_sci_send (uint8_t port, uint8_t *p_src, uint16_t const length);
static sci_err_t wrap_sci_ctrl (uint8_t port, sci_cmd_t const cmd, void * p_args);
static sci_err_t wrap_sci_recv (uint8_t port, uint8_t * p_dst, uint16_t const length);
static byteq_err_t wrap_byteq_put (uint8_t sock_idx, uint8_t const byte);

/* Port configurations */
static void flow_ctrl_init (void);
static void flow_ctrl_set (uint32_t flow);
static st_sci_conf_t * get_port_config (uint8_t port);
static int32_t cmd_port_open (uint8_t port, void (* const p_cb)(void *p_args));
static int32_t data_port_open (uint8_t port, void (* const p_cb)(void *p_args));
static void uart_port_close (uint8_t  port);

/* SCI callback functions for HSUART */
static void cb_sci_hsuart1_initial (void * pArgs);
static void cb_sci_hsuart1_for_data (void * pArgs);
static void cb_sci_hsuart2_for_cmd (void * pArgs);
static void cb_sci_err (sci_cb_evt_t event);

/* SCI configurations */
static const st_sci_conf_t s_sci_cfg[] =
{
#if SCI_CFG_CH0_INCLUDED == 1
    {SCI_CH0 , R_SCI_PinSet_SCI0  ,SCI_CFG_CH0_TX_BUFSIZ  ,SCI_CFG_CH0_RX_BUFSIZ  },
#endif
#if SCI_CFG_CH1_INCLUDED == 1
    {SCI_CH1 , R_SCI_PinSet_SCI1  ,SCI_CFG_CH1_TX_BUFSIZ  ,SCI_CFG_CH1_RX_BUFSIZ  },
#endif
#if SCI_CFG_CH2_INCLUDED == 1
    {SCI_CH2 , R_SCI_PinSet_SCI2  ,SCI_CFG_CH2_TX_BUFSIZ  ,SCI_CFG_CH2_RX_BUFSIZ  },
#endif
#if SCI_CFG_CH3_INCLUDED == 1
    {SCI_CH3 , R_SCI_PinSet_SCI3  ,SCI_CFG_CH3_TX_BUFSIZ  ,SCI_CFG_CH3_RX_BUFSIZ  },
#endif
#if SCI_CFG_CH4_INCLUDED == 1
    {SCI_CH4 , R_SCI_PinSet_SCI4  ,SCI_CFG_CH4_TX_BUFSIZ  ,SCI_CFG_CH4_RX_BUFSIZ  },
#endif
#if SCI_CFG_CH5_INCLUDED == 1
    {SCI_CH5 , R_SCI_PinSet_SCI5  ,SCI_CFG_CH5_TX_BUFSIZ  ,SCI_CFG_CH5_RX_BUFSIZ  },
#endif
#if SCI_CFG_CH6_INCLUDED == 1
    {SCI_CH6 , R_SCI_PinSet_SCI6  ,SCI_CFG_CH6_TX_BUFSIZ  ,SCI_CFG_CH6_RX_BUFSIZ  },
#endif
#if SCI_CFG_CH7_INCLUDED == 1
    {SCI_CH7 , R_SCI_PinSet_SCI7  ,SCI_CFG_CH7_TX_BUFSIZ  ,SCI_CFG_CH7_RX_BUFSIZ  },
#endif
#if SCI_CFG_CH8_INCLUDED == 1
    {SCI_CH8 , R_SCI_PinSet_SCI8  ,SCI_CFG_CH8_TX_BUFSIZ  ,SCI_CFG_CH8_RX_BUFSIZ  },
#endif
#if SCI_CFG_CH9_INCLUDED == 1
    {SCI_CH9 , R_SCI_PinSet_SCI9  ,SCI_CFG_CH9_TX_BUFSIZ  ,SCI_CFG_CH9_RX_BUFSIZ  },
#endif
#if SCI_CFG_CH10_INCLUDED == 1
    {SCI_CH10, R_SCI_PinSet_SCI10 ,SCI_CFG_CH10_TX_BUFSIZ ,SCI_CFG_CH10_RX_BUFSIZ },
#endif
#if SCI_CFG_CH11_INCLUDED == 1
    {SCI_CH11, R_SCI_PinSet_SCI11 ,SCI_CFG_CH11_TX_BUFSIZ ,SCI_CFG_CH11_RX_BUFSIZ },
#endif
#if SCI_CFG_CH12_INCLUDED == 1
    {SCI_CH12, R_SCI_PinSet_SCI12 ,SCI_CFG_CH12_TX_BUFSIZ ,SCI_CFG_CH12_RX_BUFSIZ },
#endif
    {SCI_NUM_CH, NULL ,0 ,0 }
};

/* Transfer sx-uplgn port number (HSUART1 , HSUART2) -> SCI channel */
static const uint8_t s_port_to_sci[] =
{
    WIFI_CFG_SCI_CHANNEL,          /* HSUART1 */
    WIFI_CFG_SCI_SECOND_CHANNEL    /* HSUART2 */
};

/* Max UART Ports */
static uint8_t  s_uart_port_max;

/* Max sockets */
static uint8_t  s_sockets_max;

/* WIFI system state */
static e_wifi_module_status_t s_wifi_system_state = MODULE_DISCONNECTED;

/* Executing AT command (ATO) */
static uint8_t s_atcmd_exec_ato = 0;

/* Mode in single channel  */
static uint8_t s_single_ch_mode = MODE_COMMAND;

/* certificate profiles */
static st_cert_profile_t s_cert_profile[CERT_PROFILE_MAX];
static wifi_certificate_infomation_t s_cert_info;

/* IP address */
static u_cast_t s_ip;
static u_cast_t s_msk;
static u_cast_t s_dns;

/* OS parameters */
static OS_MUTEX s_binary_sem_tx;
static OS_MUTEX s_binary_sem_rx;
static const OS_TICK s_sem_block_time = OS_WRAP_MS_TO_TICKS(10000UL);

static uint32_t s_sock_data_cnt = 0;
static uint32_t s_sock_data_cnt_old = 0;
static uint32_t s_uart1_rx_cnt  = 0;

/**********************************************************************************************************************
 * Function Name: initialize_memory
 * Description  : Initialize memory and tables.
 * Arguments    : none
 * Return Value : none
 *********************************************************************************************************************/
static void initialize_memory(void)
{
    /* Tables */
    memset(g_uart_tbl, 0, sizeof(g_uart_tbl));
    memset(g_sock_tbl, 0, sizeof(g_sock_tbl));

    /* Index, Counter */
    g_now_sock_idx  = 0;
    g_prev_sock_idx = 0;
    g_sock_tbl_index_old = 0;
    s_sock_data_cnt = 0;
    s_sock_data_cnt_old = 0;
    s_uart1_rx_cnt  = 0;
    s_atcmd_exec_ato = 0;

    /* Max UART Ports */
    s_uart_port_max = (WIFI_CFG_SCI_CHANNEL != WIFI_CFG_SCI_SECOND_CHANNEL) ? 2 : 1;

    /* Max sockets */
    s_sockets_max = (WIFI_CFG_SCI_CHANNEL != WIFI_CFG_SCI_SECOND_CHANNEL) ? WIFI_CFG_CREATABLE_SOCKETS : 1;
}
/**********************************************************************************************************************
 * End of function initialize_memory
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_Open
 * Description  : Open WIFI Module.
 * Arguments    : none
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_TAKE_MUTEX
 *                WIFI_ERR_SERIAL_OPEN
 *                WIFI_ERR_BYTEQ_OPEN
 *                WIFI_ERR_ALREADY_OPEN
 *                WIFI_ERR_MODULE_COM
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_Open(void)
{
    sci_baud_t change_baud;
    wifi_err_t api_ret = WIFI_SUCCESS;

    /* Disconnected WiFi module? */
    if (MODULE_DISCONNECTED != wifi_system_state_get())
    {
        return WIFI_ERR_ALREADY_OPEN;
    }

    /* Memory initialize */
    initialize_memory();

    /* Mutex initialize */
    if (WIFI_SUCCESS != mutex_create())
    {
        api_ret = WIFI_ERR_TAKE_MUTEX;
        goto END_INITIALIZE;
    }

    /* Reset WIFI module */
    sx_ulpgn_hw_reset();

    /* Port table initialize : HSUART1 = command port, HSUART2 = None */
    g_cmd_port = PORT_HSUART1;

    /* Open command port */
    if (E_OK != cmd_port_open(g_cmd_port, cb_sci_hsuart1_initial))
    {
        api_ret = WIFI_ERR_SERIAL_OPEN;
        goto END_INITIALIZE;
    }

    /* Apply factory default configuration. */
    at_exec(g_cmd_port, "AT&F\r");

    /* reboots the system */
    at_exec(g_cmd_port, "ATZ\r");

    /* Show firmware revision */
    at_exec(g_cmd_port, "ATWREV\r");

#if DEBUG_ATCMD == 0
    /* Disable echo back */
    at_exec(g_cmd_port, "ATE0\r");
#endif

    /* Disconnect from currently connected Access Point */
    at_exec(g_cmd_port, "ATWD\r");

    /* UART transmission flow control busy retry timeout = 0-255 (x10msec) */
    at_exec(g_cmd_port, "ATS108=1\r");

    /* Escape(+++) guard time = 10 (x20msec) */
    at_exec(g_cmd_port, "ATS12=10\r");

    /* Configure buffer threshold */
    at_exec(g_cmd_port, "ATBSIZE=%d\r", SX_ULPGN_ATBSIZE);

    /* Receive timeout = 1-60000 msec  */
    at_exec(g_cmd_port, "ATTO=1\r");

    /* socket auto close = disable  */
    at_exec(g_cmd_port, "ATS110=0\r");

    /* Auto transparent mode = disable  */
    at_exec(g_cmd_port, "ATS105=0\r");

    /* Command Port = HSUART2, Data Port = HSUART1 */
    if (2 == s_uart_port_max)
    {
        /* Command Port = HSUART2 */
        if (AT_OK != at_exec(g_cmd_port, "ATUART=2\r"))
        {
            api_ret = WIFI_ERR_MODULE_COM;
            goto END_INITIALIZE;
        }
        os_wrap_sleep(2000, UNIT_MSEC);

        /* Close current command port */
        uart_port_close(g_cmd_port);

        /* Port table initialize : HSUART1 = data port, HSUART2 = command port */
        g_cmd_port = PORT_HSUART2;
        g_data_port = PORT_HSUART1;

        /* Open command port */
        if (E_OK != cmd_port_open(g_cmd_port, cb_sci_hsuart2_for_cmd))
        {
            api_ret = WIFI_ERR_SERIAL_OPEN;
            goto END_INITIALIZE;
        }

        /* Configure UART parameters for specific UART port. */
#if (WIFI_CFG_SCI_USE_FLOW_CONTROL == 1)
        if (AT_OK != at_exec(g_cmd_port, "ATBX1=%d,,,,h\r", WIFI_CFG_SCI_BAUDRATE))
#else
        if (AT_OK != at_exec(g_cmd_port, "ATBX1=%d,,,,\r", WIFI_CFG_SCI_BAUDRATE))
#endif
        {
            api_ret = WIFI_ERR_MODULE_COM;
            goto END_INITIALIZE;
        }

        /* Open data port */
        if (E_OK != data_port_open(g_data_port, cb_sci_hsuart1_for_data))
        {
            api_ret = WIFI_ERR_SERIAL_OPEN;
            goto END_INITIALIZE;
        }

        /* Configure UART port : ATUART=<port>,[data port] */
        if (AT_OK != at_exec(g_cmd_port, "ATUART=2,1\r"))
        {
            api_ret = WIFI_ERR_MODULE_COM;
            goto END_INITIALIZE;
        }
    }
    else
    {
        /* Single Connection */
        g_cmd_port = PORT_HSUART1;
        g_data_port = PORT_HSUART1;

        /* Configure UART parameters for specific UART port. */
#if (WIFI_CFG_SCI_USE_FLOW_CONTROL == 1)
        if (AT_OK != at_exec(g_cmd_port, "ATBX1=%d,,,,h\r", WIFI_CFG_SCI_BAUDRATE))
#else
        if (AT_OK != at_exec(g_cmd_port, "ATBX1=%d,,,,\r", WIFI_CFG_SCI_BAUDRATE))
#endif
        {
            api_ret = WIFI_ERR_MODULE_COM;
            goto END_INITIALIZE;
        }

        /* Set baud rate for data port */
        change_baud.pclk = WIFI_CFG_SCI_PCLK_HZ;
        change_baud.rate = WIFI_CFG_SCI_BAUDRATE;
        R_SCI_Control(g_uart_tbl[g_data_port].sci_hdl, SCI_CMD_CHANGE_BAUD, &change_baud);
    }
    os_wrap_sleep(2000, UNIT_MSEC);

    /* Initialize BYTEQ on socket table */
    if (WIFI_SUCCESS != socket_byteq_open())
    {
        api_ret = WIFI_ERR_BYTEQ_OPEN;
        goto END_INITIALIZE;
    }

    /* Set flow control for data port */
    flow_ctrl_init();

END_INITIALIZE:
    if (WIFI_SUCCESS == api_ret)
    {
        /* Set WIFI State to "Connected WiFi module" */
        wifi_system_state_set(MODULE_CONNECTED);
    }
    else
    {
        sx_ulpgn_close();
    }
    return api_ret;
}
/**********************************************************************************************************************
 * End of function R_WIFI_SX_ULPGN_Open
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_Close
 * Description  : Close WIFI Module.
 * Arguments    : none
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_TAKE_MUTEX
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_Close(void)
{
    /* Disconnected WiFi module? */
    if (MODULE_DISCONNECTED == wifi_system_state_get())
    {
        return WIFI_SUCCESS;
    }

    /* Take mutex */
    if (E_OK != mutex_take(MUTEX_TX | MUTEX_RX))
    {
        return WIFI_ERR_TAKE_MUTEX;
    }

    disconnect_ap_sub();

    /* Give mutex */
    mutex_give(MUTEX_TX | MUTEX_RX);

    /* Close module */
    sx_ulpgn_close();

    return WIFI_SUCCESS;
}
/**********************************************************************************************************************
 * End of function R_WIFI_SX_ULPGN_Close
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_SetDnsServerAddress
 * Description  : Set DNS Server Address.
 * Arguments    : dns_address1 - First DNS address
 *                dns_address2 - Second DNS address
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_NOT_OPEN
 *                WIFI_ERR_TAKE_MUTEX
 *                WIFI_ERR_MODULE_COM
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_SetDnsServerAddress(uint32_t dns_address1, uint32_t dns_address2)
{
    wifi_err_t api_ret = WIFI_SUCCESS;

    /* Connected WiFi module? */
    if (MODULE_DISCONNECTED == wifi_system_state_get())
    {
        return WIFI_ERR_NOT_OPEN;
    }

    /* Take mutex */
    if (E_OK != mutex_take(MUTEX_TX | MUTEX_RX))
    {
        return WIFI_ERR_TAKE_MUTEX;
    }

    /* enter to command mode in single channel */
    if (E_OK != enter_command_mode())
    {
        api_ret = WIFI_ERR_MODULE_COM;
        goto RELEASE_MUTEX;
    }

    if (0 != dns_address1)
    {
        /* set dns address1 */
        s_dns.ul = dns_address1;
        if (AT_OK != at_exec(g_cmd_port, "ATNDNSSVR1=%d.%d.%d.%d\r", s_dns.b.ll, s_dns.b.lh, s_dns.b.hl, s_dns.b.hh))
        {
            api_ret = WIFI_ERR_MODULE_COM;
            goto RELEASE_MUTEX;
        }
    }

    if (0 != dns_address2)
    {
        /* set dns address2 */
        s_dns.ul = dns_address2;
        if (AT_OK != at_exec(g_cmd_port, "ATNDNSSVR2=%d.%d.%d.%d\r", s_dns.b.ll, s_dns.b.lh, s_dns.b.hl, s_dns.b.hh))
        {
            api_ret = WIFI_ERR_MODULE_COM;
        }
    }

RELEASE_MUTEX:
    mutex_give(MUTEX_TX | MUTEX_RX);
    return api_ret;
}
/**********************************************************************************************************************
 * End of function R_WIFI_SX_ULPGN_SetDnsServerAddress
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_Scan
 * Description  : Scan Access points.
 * Arguments    : ap_results
 *                max_networks
 *                exist_ap_list
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_NOT_OPEN
 *                WIFI_ERR_PARAMETER
 *                WIFI_ERR_MODULE_COM
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_Scan(wifi_scan_result_t * ap_results, uint32_t max_networks, uint32_t * exist_ap_list)
{
    wifi_err_t     api_ret = WIFI_SUCCESS;
    e_rslt_code_t  at_rslt;
    int32_t        i;
    static uint8_t retry_max = 3;
    static uint8_t ssid_tmp[33];
    uint32_t       vals[6];

    /* Disconnected WiFi module? */
    if (MODULE_DISCONNECTED == wifi_system_state_get())
    {
        return WIFI_ERR_NOT_OPEN;
    }

    /* Check parameters */
    if ((NULL == ap_results) || (NULL == exist_ap_list))
    {
        return WIFI_ERR_PARAMETER;
    }

    /* Take mutex */
    if (E_OK != mutex_take(MUTEX_TX | MUTEX_RX))
    {
        return WIFI_ERR_TAKE_MUTEX;
    }

    /* Show AP scan result. */
    *exist_ap_list = 0;
    for (i = 0; i < retry_max; i++ )
    {
        at_rslt = at_exec(g_cmd_port, "ATWS\r");
        if (AT_OK == at_rslt)
        {
            break;
        }
        os_wrap_sleep(1000, UNIT_MSEC);
    }

    /* Not found access point? */
    if (AT_OK != at_rslt)
    {
        api_ret = WIFI_ERR_MODULE_COM;
        goto RELEASE_MUTEX;
    }

    for (i = 0;; i++ )
    {
        /* ssid */
        if (DATA_NOT_FOUND == at_read("ssid = %[^\r\n]%*c", ssid_tmp))
        {
            break;
        }
        (* exist_ap_list)++;
        if (i >= max_networks)
        {
            continue;
        }
        memcpy(ap_results[i].ssid, ssid_tmp, sizeof(ssid_tmp));

        /* bssid */
        at_read("bssid = %2x:%2x:%2x:%2x:%2x:%2x\r\n",
            &vals[0], &vals[1], &vals[2], &vals[3], &vals[4], &vals[5]);
        ap_results[i].bssid[0] = (uint8_t)(vals[0] & 0xff);
        ap_results[i].bssid[1] = (uint8_t)(vals[1] & 0xff);
        ap_results[i].bssid[2] = (uint8_t)(vals[2] & 0xff);
        ap_results[i].bssid[3] = (uint8_t)(vals[3] & 0xff);
        ap_results[i].bssid[4] = (uint8_t)(vals[4] & 0xff);
        ap_results[i].bssid[5] = (uint8_t)(vals[5] & 0xff);

        /* channel */
        at_read("channel = %d\r\n", &vals[0]);
        ap_results[i].channel = (int8_t)(vals[0] & 0xff);

        /* indicator */
        at_read("indicator = %d\r\n", &vals[0]);
        ap_results[i].rssi = (int8_t)(vals[0] & 0xff);

        /* security */
        if (0 == strncmp((const char *)at_get_current_line(), "security = NONE!", 16))
        {
            /* Open */
            ap_results[i].security = WIFI_SECURITY_OPEN;
        }
        else
        {
            at_move_to_next_line();
            if (0 == strncmp((const char *)at_get_current_line(), "RSN/WPA2=", 9))
            {
                /* WPA2 */
                ap_results[i].security = WIFI_SECURITY_WPA2;
            }
            else if (0 == strncmp((const char *)at_get_current_line(), "WPA=", 4))
            {
                /* WPA */
                ap_results[i].security = WIFI_SECURITY_WPA;
            }
            else
            {
                /* WEP */
                ap_results[i].security = WIFI_SECURITY_WEP;
            }
        }
    }

RELEASE_MUTEX:
    mutex_give(MUTEX_TX | MUTEX_RX);
    return api_ret;
}
/**********************************************************************************************************************
 * End of function R_WIFI_SX_ULPGN_Scan
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_Connect
 * Description  : Connect to Access Point.
 * Arguments    : ssid
 *                pass
 *                security
 *                dhcp_enable
 *                ip_config
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_NOT_OPEN
 *                WIFI_ERR_PARAMETER
 *                WIFI_ERR_TAKE_MUTEX
 *                WIFI_ERR_MODULE_COM
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_Connect(const uint8_t * ssid, const uint8_t * pass,
        uint32_t security, uint8_t dhcp_enable, wifi_ip_configuration_t * ip_config)
{
    int32_t ret;
    uint8_t wpa_ver;
    uint8_t retry_count;
    static uint8_t retry_max = 5;
    wifi_err_t api_ret = WIFI_SUCCESS;

    /* Disconnected WiFi module? */
    if (MODULE_DISCONNECTED == wifi_system_state_get())
    {
        return WIFI_ERR_NOT_OPEN;
    }

    /* Already connected access point? */
    if (0 == R_WIFI_SX_ULPGN_IsConnected())
    {
        return WIFI_ERR_PARAMETER;
    }

    /* Check parameters */
    if ((NULL == ssid) || (NULL == pass) || (NULL == ip_config))
    {
        return WIFI_ERR_PARAMETER;
    }
    if ((WIFI_SECURITY_WPA != security) && (WIFI_SECURITY_WPA2 != security))
    {
        return WIFI_ERR_PARAMETER;
    }

    /* Take mutex */
    if (E_OK != mutex_take(MUTEX_TX | MUTEX_RX))
    {
        return WIFI_ERR_TAKE_MUTEX;
    }

    /* DHCP setting */
    if (AT_OK != at_exec(g_cmd_port, "ATNDHCP=%d\r", dhcp_enable))
    {
        api_ret = WIFI_ERR_MODULE_COM;
        goto RELEASE_MUTEX;
    }

    /* Disabled DHCP?  */
    if (0 == dhcp_enable)
    {
        /* Set static IP address */
        s_ip.ul  = ip_config->ipaddress;
        s_msk.ul = ip_config->subnetmask;
        s_dns.ul = ip_config->gateway;
        if (AT_OK != at_exec(g_cmd_port, "ATNSET=%d.%d.%d.%d,%d.%d.%d.%d,%d.%d.%d.%d\r",
                             s_ip.b.ll,  s_ip.b.lh,  s_ip.b.hl,  s_ip.b.hh,
                             s_msk.b.ll, s_msk.b.lh, s_msk.b.hl, s_msk.b.hh,
                             s_dns.b.ll, s_dns.b.lh, s_dns.b.hl, s_dns.b.hh))
        {
            api_ret = WIFI_ERR_MODULE_COM;
            goto RELEASE_MUTEX;
        }
    }
    os_wrap_sleep(2000, UNIT_MSEC);

    /* Set WPA version */
    wpa_ver = (WIFI_SECURITY_WPA == security) ? 1 : 2;

    /* Connect WIFI access point */
    for (retry_count = 0; retry_count < retry_max; retry_count++ )
    {
        /* Connect to WPA-configured Access Point */
        ret = at_exec(g_cmd_port, "ATWAWPA=%s,%d,1,1,%s\r", ssid, wpa_ver, pass);
        if (AT_OK == ret)
        {
            break;
        }

        /* Disconnect from currently connected Access Point */
        at_exec(g_cmd_port, "ATWD\r");
    }
    if (AT_OK != ret)
    {
        api_ret = WIFI_ERR_MODULE_COM;
        goto RELEASE_MUTEX;
    }

    /* Wait for IP address is assigned */
    if (0 != dhcp_enable)
    {
        for (retry_count = 0; retry_count < retry_max; retry_count++ )
        {
            os_wrap_sleep(2500, UNIT_MSEC);
            get_ipaddr(ip_config);
            if (0 != ip_config->ipaddress)
            {
                break;
            }
        }
        if (0 == ip_config->ipaddress)
        {
            /* Disconnect from currently connected Access Point */
            at_exec(g_cmd_port, "ATWD\r");
            api_ret = WIFI_ERR_MODULE_COM;
            goto RELEASE_MUTEX;
        }
    }

    /* Set WIFI State to "Connected access point" */
    wifi_system_state_set(MODULE_ACCESSPOINT);

RELEASE_MUTEX:
    mutex_give(MUTEX_TX | MUTEX_RX);
    return api_ret;
}
/**********************************************************************************************************************
 * End of function R_WIFI_SX_ULPGN_Connect
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_Disconnect
 * Description  : Disconnect from Access Point.
 * Arguments    : none
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_NOT_OPEN
 *                WIFI_ERR_TAKE_MUTEX
 *                WIFI_ERR_MODULE_COM
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_Disconnect(void)
{
    wifi_err_t api_ret = WIFI_SUCCESS;

    /* Disconnected WiFi module? */
    if (MODULE_DISCONNECTED == wifi_system_state_get())
    {
        return WIFI_ERR_NOT_OPEN;
    }

    /* Take mutex */
    if (E_OK != mutex_take(MUTEX_TX | MUTEX_RX))
    {
        return WIFI_ERR_TAKE_MUTEX;
    }

    if (WIFI_SUCCESS != disconnect_ap_sub())
    {
        api_ret = WIFI_ERR_MODULE_COM;
        goto RELEASE_MUTEX;
    }

    /* Set WIFI State to "Connected WiFi module" */
    wifi_system_state_set(MODULE_CONNECTED);

RELEASE_MUTEX:
    /* Give mutex */
    mutex_give(MUTEX_TX | MUTEX_RX);

    return api_ret;
}
/**********************************************************************************************************************
 * End of function R_WIFI_SX_ULPGN_Disconnect
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_IsConnected
 * Description  : Check connected access point.
 * Arguments    : none
 * Return Value : 0  - connected
 *                -1 - disconnected
 *********************************************************************************************************************/
int32_t R_WIFI_SX_ULPGN_IsConnected(void)
{
    /* Connected access point? */
    if (MODULE_ACCESSPOINT == wifi_system_state_get())
    {
        return 0;
    }
    else
    {
        return (-1);
    }
}
/**********************************************************************************************************************
 * End of function R_WIFI_SX_ULPGN_IsConnected
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_GetMacAddress
 * Description  : Get MAC Address.
 * Arguments    : mac_address.
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_NOT_OPEN
 *                WIFI_ERR_TAKE_MUTEX
 *                WIFI_ERR_PARAMETER
 *                WIFI_ERR_MODULE_COM
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_GetMacAddress(uint8_t * mac_address)
{
    wifi_err_t api_ret = WIFI_SUCCESS;
    uint32_t mac[6];
    uint32_t i;

    /* Disconnected WiFi module? */
    if (MODULE_DISCONNECTED == wifi_system_state_get())
    {
        return WIFI_ERR_NOT_OPEN;
    }

    /* Check parameter */
    if (NULL == mac_address)
    {
        return WIFI_ERR_PARAMETER;
    }

    /* Take mutex */
    if (E_OK != mutex_take(MUTEX_TX | MUTEX_RX))
    {
        return WIFI_ERR_TAKE_MUTEX;
    }

    /* Show current Wi-Fi status. */
    memset(mac, 0, sizeof(mac));
    if (AT_OK == at_exec(g_cmd_port, "ATW\r"))
    {
        /* Get MAC address */
        at_read("Mac Addr     =   %2x:%2x:%2x:%2x:%2x:%2x\r\n", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
        for (i = 0; i < 6; i++)
        {
            mac_address[i] = (uint8_t)(mac[i] & 0xff);
        }
    }
    else
    {
        api_ret = WIFI_ERR_MODULE_COM;
    }

    mutex_give(MUTEX_TX | MUTEX_RX);
    return api_ret;
}
/**********************************************************************************************************************
 * End of function R_WIFI_SX_ULPGN_GetMacAddress
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_GetIpAddress
 * Description  : Get IP Address.
 * Arguments    : ip_config.
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_NOT_OPEN
 *                WIFI_ERR_TAKE_MUTEX
 *                WIFI_ERR_PARAMETER
 *                WIFI_ERR_MODULE_COM
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_GetIpAddress(wifi_ip_configuration_t * ip_config)
{
    wifi_err_t api_ret = WIFI_SUCCESS;

    /* Disconnected WiFi module? */
    if (MODULE_DISCONNECTED == wifi_system_state_get())
    {
        return WIFI_ERR_NOT_OPEN;
    }

    /* Check parameter */
    if (NULL == ip_config)
    {
        return WIFI_ERR_PARAMETER;
    }

    /* Take mutex */
    if (E_OK != mutex_take(MUTEX_TX | MUTEX_RX))
    {
        return WIFI_ERR_TAKE_MUTEX;
    }

    /* Get IP address */
    if (AT_OK != get_ipaddr(ip_config))
    {
        api_ret = WIFI_ERR_MODULE_COM;
    }

    /* Give mutex */
    mutex_give(MUTEX_TX | MUTEX_RX);

    return api_ret;
}
/**********************************************************************************************************************
 * End of function R_WIFI_SX_ULPGN_GetIpAddress
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_CreateSocket
 * Description  : Create TCP/UDP socket
 * Arguments    : type
 *                ip_version
 * Return Value : Positive number - created socket index
 *                WIFI_ERR_PARAMETER
 *                WIFI_ERR_NOT_CONNECT
 *                WIFI_ERR_SOCKET_CREATE
 *********************************************************************************************************************/
int32_t R_WIFI_SX_ULPGN_CreateSocket(uint32_t type, uint32_t ip_version)
{
    uint16_t i;
    int32_t  ret = WIFI_ERR_SOCKET_CREATE;

    /* Connected access point? */
    if (0 != R_WIFI_SX_ULPGN_IsConnected())
    {
        return WIFI_ERR_NOT_CONNECT;
    }

    /* Check parameters */
    if (((WIFI_SOCKET_IP_PROTOCOL_TCP != type) && (WIFI_SOCKET_IP_PROTOCOL_UDP != type)) ||
        (WIFI_SOCKET_IP_VERSION_4 != ip_version))
    {
        return WIFI_ERR_PARAMETER;
    }

    /* Take mutex */
    if (0 != mutex_take(MUTEX_TX | MUTEX_RX))
    {
        return WIFI_ERR_TAKE_MUTEX;
    }

    for (i = 0; i < s_sockets_max; i++ )
    {
        if (ULPGN_SOCKET_STATUS_CLOSED == g_sock_tbl[i].status)
        {
            g_sock_tbl[i].status = ULPGN_SOCKET_STATUS_SOCKET;  /* socket status   */
            g_sock_tbl[i].ipver = (uint8_t)ip_version;          /* ip_version      */
            g_sock_tbl[i].protocol = (uint8_t)type;             /* type            */
            g_sock_tbl[i].ssl.enable  = 0;                      /* SSL enable flag */
            g_sock_tbl[i].ssl.cert_id = 0;                      /* Certificate ID  */
            R_BYTEQ_Flush(g_sock_tbl[i].byteq_hdl);
            ret = (int32_t) i;
            break;
        }
    }

    /* Give mutex. */
    mutex_give(MUTEX_TX | MUTEX_RX);

    return ret;
}
/**********************************************************************************************************************
 * End of function R_WIFI_SX_ULPGN_CreateSocket
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_ConnectSocket
 * Description  : Open client mode TCP/UDP socket.
 * Arguments    : socket_number
 *                ip_address
 *                port
 *                destination
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_PARAMETER
 *                WIFI_ERR_SOCKET_NUM
 *                WIFI_ERR_TAKE_MUTEX
 *                WIFI_ERR_MODULE_COM
 *                WIFI_ERR_NOT_CONNECT
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_ConnectSocket(uint8_t socket_number, uint32_t ip_address, uint16_t port, char * destination)
{
    wifi_err_t api_ret = WIFI_SUCCESS;
    uint8_t i;
    uint8_t socket_type;
    uint8_t exist_cert_profile = 0;
    static char certificate_file[32];
    static char calist_file[32];
    static const char * atcmd_tbl[2] = { "ATNCTCP", "ATNCUDP" }; /* AT_cmd strings : TCP=ATNCTCP, UDP=ATNCUDP */

    /* Connect access point? */
    if (0 != R_WIFI_SX_ULPGN_IsConnected())
    {
        return WIFI_ERR_NOT_CONNECT;
    }

    /* Check parameters */
    if ((socket_number >= s_sockets_max) || (0 == ip_address) || (0 == port))
    {
        return WIFI_ERR_PARAMETER;
    }

    /* socket created? */
    if (ULPGN_SOCKET_STATUS_SOCKET != g_sock_tbl[socket_number].status)
    {
        return WIFI_ERR_SOCKET_NUM;
    }

    /* Take mutex */
    if (E_OK != mutex_take(MUTEX_TX | MUTEX_RX))
    {
        return WIFI_ERR_TAKE_MUTEX;
    }

    /* change to next socket index */
    if (E_OK != change_socket_index(socket_number))
    {
        api_ret = WIFI_ERR_CHANGE_SOCKET;
        goto RELEASE_MUTEX;
    }
    os_wrap_sleep(200, UNIT_MSEC);

    /* Get socket type(TCP or UDP) */
    socket_type = g_sock_tbl[socket_number].protocol;

    if (WIFI_SOCKET_IP_PROTOCOL_TCP == socket_type)
    {
        /* SSL */
        if (1 == g_sock_tbl[socket_number].ssl.enable)
        {
            /* Find and Set certificate ID */
            for (i = 0; i < CERT_PROFILE_MAX; i++ )
            {
                /* Find certificate id by IP address or Host name */
                if (ip_address == s_cert_profile[i].host_address)
                {
                    g_sock_tbl[socket_number].ssl.cert_id = i;
                    exist_cert_profile = 1;
                    break;
                }
                if (0 != s_cert_profile[i].host_name[0])
                {
                    if (ip_address == get_ipaddr_by_name((uint8_t *)s_cert_profile[i].host_name))
                    {
                        g_sock_tbl[socket_number].ssl.cert_id = i;
                        exist_cert_profile = 1;
                        break;
                    }
                }
            }
            if (0 == exist_cert_profile)
            {
                api_ret = WIFI_ERR_MODULE_COM;
                goto RELEASE_MUTEX;
            }

            /* Initialize TLS context used by HTTPS client or SSL socket.  */
            if (AT_OK != at_exec(g_cmd_port, "ATNSSL=2,1\r"))
            {
                api_ret = WIFI_ERR_MODULE_COM;
                goto RELEASE_MUTEX;
            }

            /* Set Certificate and CA list name */
            sprintf((char *)certificate_file, "cert%d.crt", g_sock_tbl[socket_number].ssl.cert_id);
            sprintf((char *)calist_file, "calist%d.crt", g_sock_tbl[socket_number].ssl.cert_id);

            /* Specify Certificate or CA list for HTTPS client or SSL socket. */
            if (E_OK != get_server_certificate(&s_cert_info))
            {
                api_ret = WIFI_ERR_MODULE_COM;
                goto RELEASE_MUTEX;
            }

            if (0 == s_cert_info.num_of_files)
            {
                api_ret = WIFI_ERR_MODULE_COM;
                goto RELEASE_MUTEX;
            }

            /* Certificate */
            for (i = 0; i < s_cert_info.num_of_files; i++ )
            {
                if (0 == strcmp((char *)(&s_cert_info.cert[i].file_name[0]), certificate_file))
                {
                    /* ATNSSLLD=<role,name,type> */
                    if (AT_OK != at_exec(g_cmd_port, "ATNSSLLD=2,%s,%d\r", s_cert_info.cert[i].file_name, 1))
                    {
                        api_ret = WIFI_ERR_MODULE_COM;
                        goto RELEASE_MUTEX;
                    }
                    break;
                }
            }

            /* CA list */
            for (i = 0; i < s_cert_info.num_of_files; i++ )
            {
                if (0 == strcmp((char *)(&s_cert_info.cert[i].file_name[0]), calist_file))
                {
                    /* ATNSSLLD=<role,name,type> */
                    if (AT_OK != at_exec(g_cmd_port, "ATNSSLLD=2,%s,%d\r", s_cert_info.cert[i].file_name, 2))
                    {
                        api_ret = WIFI_ERR_MODULE_COM;
                        goto RELEASE_MUTEX;
                    }
                    break;
                }
            }

            /* enable SSL alert */
            if (AT_OK != at_exec(g_cmd_port, "ATNSSLCFG=2,4,,,1\r"))
            {
                api_ret = WIFI_ERR_MODULE_COM;
                goto RELEASE_MUTEX;
            }

            /* enable matching test */
            if (NULL != destination)
            {
                if (AT_OK != at_exec(g_cmd_port, "ATNSSLCFG=2,,%s,,\r", destination))
                {
                    api_ret = WIFI_ERR_MODULE_COM;
                    goto RELEASE_MUTEX;
                }
            }

            /* Socket type = SSL */
            socket_type = WIFI_SOCKET_IP_PROTOCOL_SSL;
        }
    }

    /* Create network socket. : ATNSOCK=<type>, [family] */
    if (AT_OK != at_exec(g_cmd_port, "ATNSOCK=%d,%d\r", socket_type, 4))
    {
        api_ret = WIFI_ERR_SOCKET_CREATE;
        goto RELEASE_MUTEX;
    }

    /* Get socket type(TCP or UDP) */
    socket_type = g_sock_tbl[socket_number].protocol;

    /* Open client mode TCP or UDP socket. */
    s_ip.ul = ip_address;
    if (AT_OK != at_exec(g_cmd_port, "%s=%d.%d.%d.%d,%d\r",
                         atcmd_tbl[socket_type], s_ip.b.ll, s_ip.b.lh, s_ip.b.hl, s_ip.b.hh, port))
    {
        at_exec(g_cmd_port, "ATNCLOSE\r");
        R_BSP_SoftwareDelay(500, BSP_DELAY_MILLISECS);
        api_ret = WIFI_ERR_MODULE_COM;
        goto RELEASE_MUTEX;
    }

    /* Configure destination address / port for UDP transmission. */
    if (WIFI_SOCKET_IP_PROTOCOL_UDP == socket_type)
    {
        if (AT_OK != at_exec(g_cmd_port, "ATNSENDTO=%d.%d.%d.%d,%d\r",
                             s_ip.b.ll, s_ip.b.lh, s_ip.b.hl, s_ip.b.hh, port))
        {
            at_exec(g_cmd_port, "ATNCLOSE\r");
            R_BSP_SoftwareDelay(500, BSP_DELAY_MILLISECS);
            api_ret = WIFI_ERR_MODULE_COM;
            goto RELEASE_MUTEX;
        }
    }

    g_sock_tbl[socket_number].status = ULPGN_SOCKET_STATUS_CONNECTED;

RELEASE_MUTEX:
    mutex_give(MUTEX_TX | MUTEX_RX);
    return api_ret;
}
/**********************************************************************************************************************
 * End of function R_WIFI_SX_ULPGN_ConnectSocket
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_SendSocket
 * Description  : Send data on connecting socket.
 * Arguments    : socket_number
 *                data
 *                length
 *                timeout_ms
 * Return Value : Positive number - number of sent data
 *                WIFI_ERR_PARAMETER
 *                WIFI_ERR_SOCKET_NUM
 *                WIFI_ERR_NOT_CONNECT
 *                WIFI_ERR_TAKE_MUTEX
 *                WIFI_ERR_MODULE_COM
 *********************************************************************************************************************/
int32_t R_WIFI_SX_ULPGN_SendSocket(uint8_t socket_number, uint8_t * data, uint32_t length, uint32_t timeout_ms)
{
    int32_t     send_idx;
    int32_t     send_length;
    int32_t     api_ret = 0;
    OS_TICK     tick_tmp;
    uint32_t    txsize_max = get_port_config(g_data_port)->tx_size;

    /* Connect access point? */
    if (0 != R_WIFI_SX_ULPGN_IsConnected())
    {
        return WIFI_ERR_NOT_CONNECT;
    }

    /* Check parameters */
    if ((socket_number >= s_sockets_max) || (NULL == data))
    {
        return WIFI_ERR_PARAMETER;
    }

    /* Not connect? */
    if (ULPGN_SOCKET_STATUS_CONNECTED != g_sock_tbl[socket_number].status)
    {
        return WIFI_ERR_SOCKET_NUM;
    }

    /* Take mutex */
    if (E_OK != mutex_take(MUTEX_TX))
    {
        return WIFI_ERR_TAKE_MUTEX;
    }

    if (socket_number != g_now_sock_idx)
    {
        /* Take mutex */
        if (E_OK != mutex_take(MUTEX_RX))
        {
            return WIFI_ERR_TAKE_MUTEX;
        }

        /* change to next socket index */
        if (E_OK != change_socket_index(socket_number))
        {
            api_ret = WIFI_ERR_CHANGE_SOCKET;
            goto RELEASE_MUTEX;
        }
        mutex_give(MUTEX_RX);
    }

    /* enter to transparent mode in single channel */
    if (E_OK != enter_transparent_mode())
    {
        api_ret = WIFI_ERR_MODULE_COM;
        goto RELEASE_MUTEX;
    }

    /*
     *  Send
     */
    send_idx = 0;
    g_sock_tbl[socket_number].timer_tx.threshold = OS_WRAP_MS_TO_TICKS(timeout_ms);
    if (0 < timeout_ms)
    {
        g_sock_tbl[socket_number].timer_tx.tick_sta = os_wrap_tickcount_get();
    }
    while (send_idx < length)
    {
        if ((length - send_idx) > txsize_max)
        {
            send_length = txsize_max;
        }
        else
        {
            send_length = length - send_idx;
        }

        if (SCI_SUCCESS != wrap_sci_send(g_data_port, (uint8_t *)data + send_idx, send_length))
        {
            break;
        }

        while (1)
        {
            if (0 != g_uart_tbl[g_data_port].tx_end_flag)
            {
                break;
            }
            os_wrap_sleep(1, UNIT_TICK);
        }
        send_idx += send_length;

        /* timeout? */
        if (0 < timeout_ms)
        {
            tick_tmp = os_wrap_tickcount_get() - g_sock_tbl[socket_number].timer_tx.tick_sta;
            if (g_sock_tbl[socket_number].timer_tx.threshold <= tick_tmp)
            {
                break;
            }
        }
        os_wrap_sleep(1, UNIT_TICK);
    }
    api_ret = send_idx;

RELEASE_MUTEX:
    mutex_give(MUTEX_TX);
    return api_ret;
}
/**********************************************************************************************************************
 * End of function R_WIFI_SX_ULPGN_SendSocket
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_ReceiveSocket
 * Description  : Receive data on connecting socket.
 * Arguments    : socket_number
 *                data
 *                length
 *                timeout_ms
 * Return Value : Positive number - number of received data
 *                WIFI_ERR_PARAMETER
 *                WIFI_ERR_SOCKET_NUM
 *                WIFI_ERR_NOT_CONNECT
 *                WIFI_ERR_TAKE_MUTEX
 *                WIFI_ERR_CHANGE_SOCKET
 *                WIFI_ERR_MODULE_COM
 *********************************************************************************************************************/
int32_t R_WIFI_SX_ULPGN_ReceiveSocket(uint8_t socket_number, uint8_t * data, uint32_t length, uint32_t timeout_ms)
{
    int32_t         api_ret = WIFI_SUCCESS;
    uint32_t        recvcnt;
    byteq_err_t     byteq_ret;
    OS_TICK         tick_tmp;

    /* Connect access point? */
    if (0 != R_WIFI_SX_ULPGN_IsConnected())
    {
        return WIFI_ERR_NOT_CONNECT;
    }

    /* Check parameters */
    if ((socket_number >= s_sockets_max) || (NULL == data) || (0 == length))
    {
        return WIFI_ERR_PARAMETER;
    }

    /*  socket connected? */
    if (ULPGN_SOCKET_STATUS_CONNECTED != g_sock_tbl[socket_number].status)
    {
        return WIFI_ERR_SOCKET_NUM;
    }

    /* Take mutex */
    if (E_OK != mutex_take(MUTEX_RX))
    {
        return WIFI_ERR_TAKE_MUTEX;
    }

    if (socket_number != g_now_sock_idx)
    {
        /* Take mutex */
        if (E_OK != mutex_take(MUTEX_TX))
        {
            return WIFI_ERR_TAKE_MUTEX;
        }

        /* change to next socket index */
        if (E_OK != change_socket_index(socket_number))
        {
            api_ret = WIFI_ERR_CHANGE_SOCKET;
            goto RELEASE_MUTEX;
        }
        mutex_give(MUTEX_TX);
    }

    /* enter to transparent mode in single channel */
    if (E_OK != enter_transparent_mode())
    {
        api_ret = WIFI_ERR_MODULE_COM;
        goto RELEASE_MUTEX;
    }

    /*
     * Receive
     */
    recvcnt = 0;
    g_sock_tbl[socket_number].timer_rx.threshold = OS_WRAP_MS_TO_TICKS(timeout_ms);
    if (0 < timeout_ms)
    {
        g_sock_tbl[socket_number].timer_rx.tick_sta = os_wrap_tickcount_get();
    }
    while (1)
    {
#if defined(__CCRX__) || defined(__ICCRX__) || defined (__RX__)
        R_BSP_InterruptsDisable();
        byteq_ret = R_BYTEQ_Get(g_sock_tbl[socket_number].byteq_hdl, (data + recvcnt));
        R_BSP_InterruptsEnable();
#endif
        if (BYTEQ_SUCCESS == byteq_ret)
        {
            recvcnt++;
            if (recvcnt >= length)
            {
                break;
            }
            continue;
        }

        /* timeout? */
        if (0 < timeout_ms)
        {
            tick_tmp = os_wrap_tickcount_get() - g_sock_tbl[socket_number].timer_rx.tick_sta;
            if (g_sock_tbl[socket_number].timer_rx.threshold <= tick_tmp)
            {
                break;
            }
        }
        os_wrap_sleep(1, UNIT_TICK);
    }
    api_ret = recvcnt;

RELEASE_MUTEX:
    mutex_give(MUTEX_RX);
    return api_ret;
}
/**********************************************************************************************************************
 * End of function R_WIFI_SX_ULPGN_ReceiveSocket
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_ShutdownSocket
 * Description  : Shutdown connecting socket.
 * Arguments    : socket_number
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_NOT_CONNECT
 *                WIFI_ERR_SOCKET_NUM
 *                WIFI_ERR_CHANGE_SOCKET
 *                WIFI_ERR_MODULE_COM
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_ShutdownSocket(uint8_t socket_number)
{
    wifi_err_t api_ret = WIFI_SUCCESS;

    /* Connected access point? */
    if (0 != R_WIFI_SX_ULPGN_IsConnected())
    {
        return WIFI_ERR_NOT_CONNECT;
    }

    /* Check parameters */
    if (socket_number >= s_sockets_max)
    {
        return WIFI_ERR_SOCKET_NUM;
    }

    /* Take mutex */
    if (E_OK != mutex_take(MUTEX_TX | MUTEX_RX))
    {
        return WIFI_ERR_TAKE_MUTEX;
    }

    api_ret = close_socket_sub(socket_number);

    mutex_give(MUTEX_TX | MUTEX_RX);
    return api_ret;
}
/**********************************************************************************************************************
 * End of function R_WIFI_SX_ULPGN_ShutdownSocket
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_CloseSocket
 * Description  : Disconnect connecting socket.
 * Arguments    : socket_number
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_NOT_CONNECT
 *                WIFI_ERR_SOCKET_NUM
 *                WIFI_ERR_TAKE_MUTEX
 *                WIFI_ERR_CHANGE_SOCKET
 *                WIFI_ERR_MODULE_COM
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_CloseSocket(uint8_t socket_number)
{
    wifi_err_t api_ret = WIFI_SUCCESS;

    /* Connected access point? */
    if (0 != R_WIFI_SX_ULPGN_IsConnected())
    {
        return WIFI_ERR_NOT_CONNECT;
    }

    /* Check parameters */
    if (socket_number >= s_sockets_max)
    {
        return WIFI_ERR_SOCKET_NUM;
    }

    /* Take mutex */
    if (E_OK != mutex_take(MUTEX_TX | MUTEX_RX))
    {
        return WIFI_ERR_TAKE_MUTEX;
    }

    if (ULPGN_SOCKET_STATUS_SOCKET != g_sock_tbl[socket_number].status)
    {
        api_ret = close_socket_sub(socket_number);
    }

    R_BYTEQ_Flush(g_sock_tbl[socket_number].byteq_hdl);
    g_sock_tbl[socket_number].put_err_cnt = 0;
    g_sock_tbl[socket_number].ssl.enable  = 0;
    g_sock_tbl[socket_number].ssl.cert_id = 0;
    g_sock_tbl[socket_number].status = ULPGN_SOCKET_STATUS_CLOSED;

    /* Give mutex */
    mutex_give(MUTEX_TX | MUTEX_RX);

    return api_ret;
}
/**********************************************************************************************************************
 * End of function R_WIFI_SX_ULPGN_CloseSocket
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_DnsQuery
 * Description  : Execute DNS query.
 * Arguments    : domain_name
 *                ip_address
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_NOT_CONNECT
 *                WIFI_ERR_PARAMETER
 *                WIFI_ERR_MODULE_COM
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_DnsQuery(uint8_t * domain_name, uint32_t * ip_address)
{
    wifi_err_t api_ret = WIFI_SUCCESS;

    /* Connected access point? */
    if (0 != R_WIFI_SX_ULPGN_IsConnected())
    {
        return WIFI_ERR_NOT_CONNECT;
    }

    /* Check parameters */
    if ((NULL == domain_name) || (NULL == ip_address))
    {
        return WIFI_ERR_PARAMETER;
    }

    /* Take mutex */
    if (E_OK != mutex_take(MUTEX_TX | MUTEX_RX))
    {
        return WIFI_ERR_TAKE_MUTEX;
    }

    /* Get IP address by domain name */
    *ip_address = get_ipaddr_by_name(domain_name);
    if (0 == (*ip_address))
    {
        api_ret = WIFI_ERR_MODULE_COM;
    }

    /* Give mutex */
    mutex_give(MUTEX_TX | MUTEX_RX);

    return api_ret;
}
/**********************************************************************************************************************
 * End of function R_WIFI_SX_ULPGN_DnsQuery
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_Ping
 * Description  : Execute Ping command.
 * Arguments    : ip_address
 *                count
 *                interval_ms
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_PARAMETER
 *                WIFI_ERR_NOT_CONNECT
 *                WIFI_ERR_TAKE_MUTEX
 *                WIFI_ERR_MODULE_COM
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_Ping(uint32_t ip_address, uint16_t count, uint32_t interval_ms)
{
    uint32_t i;
    wifi_err_t api_ret = WIFI_SUCCESS;
    uint16_t success_count = 0;

    /* Connected access point? */
    if (0 != R_WIFI_SX_ULPGN_IsConnected())
    {
        return WIFI_ERR_NOT_CONNECT;
    }

    /* Check parameters */
    if ((0 == ip_address) || (0 == count))
    {
        return WIFI_ERR_PARAMETER;
    }

    /* Take mutex */
    if (E_OK != mutex_take(MUTEX_TX | MUTEX_RX))
    {
        return WIFI_ERR_TAKE_MUTEX;
    }

    /* Send ping to specified IP address. */
    for (i = 0; i < count; i++ )
    {
        /* Insert interval (except in first time) */
        if (0 != i)
        {
            os_wrap_sleep(interval_ms, UNIT_MSEC);
        }

        s_ip.ul = ip_address;
        if (AT_OK == at_exec(g_cmd_port, "ATNPING=%d.%d.%d.%d\r", s_ip.b.ll, s_ip.b.lh, s_ip.b.hl, s_ip.b.hh))
        {
            success_count++;
        }
    }

    if (0 == success_count)
    {
        api_ret = WIFI_ERR_MODULE_COM;
    }

    /* Give mutex */
    mutex_give(MUTEX_TX | MUTEX_RX);

    return api_ret;
}
/**********************************************************************************************************************
 * End of function R_WIFI_SX_ULPGN_Ping
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_GetVersion
 * Description  : Get FIT module version.
 * Arguments    : none
 * Return Value : FIT module version
 *********************************************************************************************************************/
uint32_t R_WIFI_SX_ULPGN_GetVersion(void)
{
    /* These version macros are defined in r_wifi_sx_ulpgn_if.h. */
    return ((((uint32_t) WIFI_SX_ULPGN_CFG_VERSION_MAJOR) << 16) | (uint32_t) WIFI_SX_ULPGN_CFG_VERSION_MINOR);
}
/**********************************************************************************************************************
 * End of function R_WIFI_SX_ULPGN_GetVersion
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_GetTcpSocketStatus
 * Description  : Get TCP socket status.
 * Arguments    : socket_number
 * Return Value : -1    - not exist
 *                other - socket status
 *********************************************************************************************************************/
int32_t R_WIFI_SX_ULPGN_GetTcpSocketStatus(uint8_t socket_number)
{
    int32_t  ret = (-1);
    static char sock_status[24];
    static char sock_type[8];
    static uint32_t  sock_ver;
    uint16_t i;

    static const uint8_t * p_sock_sts_tbl[ULPGN_SOCKET_STATUS_MAX] =
    {
        "CLOSED", "SOCKET", "BOUND", "LISTEN", "CONNECTED", "BROKEN"
    };

    /* Check parameters */
    if (socket_number >= s_sockets_max)
    {
        return (-1);
    }

    /* Take mutex */
    if (E_OK != mutex_take(MUTEX_TX | MUTEX_RX))
    {
        return WIFI_ERR_TAKE_MUTEX;
    }

    /* enter to command mode in single channel */
    if (E_OK != enter_command_mode())
    {
        ret = (-1);
        goto RELEASE_MUTEX;
    }

    /* change to next socket index */
    if (E_OK != change_socket_index(socket_number))
    {
        ret = (-1);
        goto RELEASE_MUTEX;
    }

    /* Query current socket status. */
    if (AT_OK != at_exec(g_cmd_port, "ATNSTAT\r"))
    {
        ret = (-1);
        goto RELEASE_MUTEX;
    }

    /* Get socket status from response */
    at_move_to_next_line(); // Echo back
    at_read_wo_prefix("%[^,],%[^,],%d\r\n", sock_status, sock_type, &sock_ver);
    at_move_to_next_line(); // CONNECTED,TCP,4

    /* match? */
    for (i = 0; i < ULPGN_SOCKET_STATUS_MAX; i++ )
    {
        /* matched socket status? */
        if (0 == strcmp((char *)sock_status, (char *)p_sock_sts_tbl[i]))
        {
            ret = i;
            break;
        }
    }

RELEASE_MUTEX:
    mutex_give(MUTEX_TX | MUTEX_RX);

    return ret;
}
/**********************************************************************************************************************
 * End of function R_WIFI_SX_ULPGN_GetTcpSocketStatus
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_RequestTlsSocket
 * Description  : Request TLS socket communication.
 * Arguments    : socket_number
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_SOCKET_NUM
 *                WIFI_ERR_NOT_CONNECT
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_RequestTlsSocket(uint8_t socket_number)
{
    /* Connected access point? */
    if (0 != R_WIFI_SX_ULPGN_IsConnected())
    {
        return WIFI_ERR_NOT_CONNECT;
    }

    /* Check parameters */
    if ((socket_number >= s_sockets_max) || (ULPGN_SOCKET_STATUS_SOCKET != g_sock_tbl[socket_number].status))
    {
        return WIFI_ERR_SOCKET_NUM;
    }
    g_sock_tbl[socket_number].ssl.enable = 1;
    return WIFI_SUCCESS;
}
/**********************************************************************************************************************
 * End of function R_WIFI_SX_ULPGN_RequestTlsSocket
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_WriteServerCertificate
 * Description  : Write server certificate to WiFi module.
 * Arguments    : data_id
 *                data_type
 *                certificate
 *                certificate_length
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_PARAMETER
 *                WIFI_ERR_NOT_OPEN
 *                WIFI_ERR_TAKE_MUTEX
 *                WIFI_ERR_MODULE_COM
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_WriteServerCertificate(uint32_t data_id, uint32_t data_type,
        const uint8_t * certificate, uint32_t certificate_length)
{
    wifi_err_t api_ret = WIFI_SUCCESS;
    static uint32_t send_idx;
    static uint32_t send_length;
    static uint32_t txsize_max;

    /* Disconnected WiFi module? */
    if (MODULE_DISCONNECTED == wifi_system_state_get())
    {
        return WIFI_ERR_NOT_OPEN;
    }

    /* Check parameters */
    if ((NULL == certificate) || (0 == certificate_length) || (CERT_PROFILE_MAX <= data_id))
    {
        return WIFI_ERR_PARAMETER;
    }

    /* Take mutex. */
    if (E_OK != mutex_take(MUTEX_TX | MUTEX_RX))
    {
        return WIFI_ERR_TAKE_MUTEX;
    }

    /* Send AT command : ATNSSLCERT */
    if (0 == data_type)
    {
        /* CA list */
        at_send(g_cmd_port, "ATNSSLCERT=calist%d.crt,%d\r", data_id, certificate_length);
    }
    else
    {
        /* Certificate */
        at_send(g_cmd_port, "ATNSSLCERT=cert%d.crt,%d\r", data_id, certificate_length);
    }

    os_wrap_sleep(10, UNIT_MSEC);

    /* Send file CA list or Certificate */
    send_idx = 0;
    txsize_max = get_port_config(g_cmd_port)->tx_size;
    while (send_idx < certificate_length)
    {
        if ((certificate_length - send_idx) > txsize_max)
        {
            send_length = txsize_max;
        }
        else
        {
            send_length = certificate_length - send_idx;
        }

        /* data send */
        if (SCI_SUCCESS != wrap_sci_send(g_cmd_port, (uint8_t *) certificate + send_idx, send_length))
        {
            api_ret = WIFI_ERR_MODULE_COM;
            goto RELEASE_MUTEX;
        }

        /* Wait for transmit end */
        while (1)
        {
            if (0 != g_uart_tbl[g_cmd_port].tx_end_flag)
            {
                break;
            }
            os_wrap_sleep(1, UNIT_TICK);
        }
        send_idx += send_length;
    }

    /* Receive response */
    if (AT_OK != at_recv(g_cmd_port))
    {
        api_ret = WIFI_ERR_MODULE_COM;
    }

RELEASE_MUTEX:
    mutex_give(MUTEX_TX | MUTEX_RX);

    return api_ret;
}
/**********************************************************************************************************************
 * End of function R_WIFI_SX_ULPGN_WriteServerCertificate
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_EraseServerCertificate
 * Description  : Erase server certificate on WiFi module.
 * Arguments    : certificate_name
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_PARAMETER
 *                WIFI_ERR_TAKE_MUTEX
 *                WIFI_ERR_NOT_OPEN
 *                WIFI_ERR_MODULE_COM
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_EraseServerCertificate(uint8_t * certificate_name)
{
    wifi_err_t api_ret = WIFI_ERR_PARAMETER;
    uint8_t i;

    /* Disconnected WiFi module? */
    if (MODULE_DISCONNECTED == wifi_system_state_get())
    {
        return WIFI_ERR_NOT_OPEN;
    }

    /* Check parameters */
    if (NULL == certificate_name)
    {
        return WIFI_ERR_PARAMETER;
    }

    /* Take mutex. */
    if (E_OK != mutex_take(MUTEX_TX | MUTEX_RX))
    {
        return WIFI_ERR_TAKE_MUTEX;
    }

    /* Erase certificate file */
    if (E_OK != get_server_certificate(&s_cert_info))
    {
        api_ret = WIFI_ERR_MODULE_COM;
        goto RELEASE_MUTEX;
    }

    for (i = 0; i < s_cert_info.num_of_files; i++ )
    {
        /* Is certificate name matched? */
        if (0 == strcmp((char *)(&s_cert_info.cert[i].file_name[0]), (char *)certificate_name))
        {
            if (AT_OK == at_exec(g_cmd_port, "ATNSSLCERT=%s,0\r", (char *)certificate_name))
            {
                api_ret = WIFI_SUCCESS;
            }
            else
            {
                api_ret = WIFI_ERR_MODULE_COM;
            }
            break;
        }
    }

RELEASE_MUTEX:
    mutex_give(MUTEX_TX | MUTEX_RX);

    return api_ret;
}
/**********************************************************************************************************************
 * End of function R_WIFI_SX_ULPGN_EraseServerCertificate
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_GetServerCertificate
 * Description  : Get stored server certificate on WiFi module.
 * Arguments    : wifi_certificate_information
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_PARAMETER
 *                WIFI_ERR_TAKE_MUTEX
 *                WIFI_ERR_MODULE_COM
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_GetServerCertificate(wifi_certificate_infomation_t * wifi_certificate_information)
{
    wifi_err_t api_ret = WIFI_SUCCESS;

    /* Disconnected WiFi module? */
    if (MODULE_DISCONNECTED == wifi_system_state_get())
    {
        return WIFI_ERR_NOT_OPEN;
    }

    /* Check parameters */
    if (NULL == wifi_certificate_information)
    {
        return WIFI_ERR_PARAMETER;
    }

    /* Take mutex. */
    if (E_OK != mutex_take(MUTEX_TX | MUTEX_RX))
    {
        return WIFI_ERR_TAKE_MUTEX;
    }

    if (E_OK != get_server_certificate(wifi_certificate_information))
    {
        api_ret = WIFI_ERR_MODULE_COM;
    }

    /* Give mutex. */
    mutex_give(MUTEX_TX | MUTEX_RX);

    return api_ret;
}
/**********************************************************************************************************************
 * End of function R_WIFI_SX_ULPGN_GetServerCertificate
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_EraseAllServerCertificate
 * Description  : Erase all stored server certificate on WiFi module.
 * Arguments    : none
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_NOT_OPEN
 *                WIFI_ERR_TAKE_MUTEX
 *                WIFI_ERR_MODULE_COM
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_EraseAllServerCertificate(void)
{
    uint8_t    i;
    wifi_err_t api_ret = WIFI_SUCCESS;

    /* Disconnected WiFi module? */
    if (MODULE_DISCONNECTED == wifi_system_state_get())
    {
        return WIFI_ERR_NOT_OPEN;
    }

    /* Take mutex */
    if (E_OK != mutex_take(MUTEX_TX | MUTEX_RX))
    {
        return WIFI_ERR_TAKE_MUTEX;
    }

    /* Exist Certificate File? */
    if (E_OK != get_server_certificate(&s_cert_info))
    {
        api_ret = WIFI_ERR_MODULE_COM;
        goto RELEASE_MUTEX;
    }

    if (0 == s_cert_info.num_of_files)
    {
        api_ret = WIFI_SUCCESS;
        goto RELEASE_MUTEX;
    }

    /* Erase certificate */
    for (i = 0; i < s_cert_info.num_of_files; i++ )
    {
        if (AT_OK != at_exec(g_cmd_port, "ATNSSLCERT=%s,0\r", (char *)(&s_cert_info.cert[i].file_name[0])))
        {
            api_ret = WIFI_ERR_MODULE_COM;
            break;
        }
    }

RELEASE_MUTEX:
    mutex_give(MUTEX_TX | MUTEX_RX);

    return api_ret;
}
/**********************************************************************************************************************
 * End of function R_WIFI_SX_ULPGN_EraseAllServerCertificate
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WIFI_SX_ULPGN_SetCertificateProfile
 * Description  : Associate server information to certificate.
 * Arguments    : certificate_id
 *                ip_address
 *                server_name
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_PARAMETER
 *********************************************************************************************************************/
wifi_err_t R_WIFI_SX_ULPGN_SetCertificateProfile(uint8_t certificate_id, uint32_t ip_address, char * server_name)
{
    /* Check parameter */
    if (CERT_PROFILE_MAX <= certificate_id)
    {
        return WIFI_ERR_PARAMETER;
    }

    if (NULL == server_name)
    {
        memset(s_cert_profile[certificate_id].host_name, 0, sizeof(s_cert_profile[certificate_id].host_name));
    }
    else
    {
        if ('\0' == *server_name)
        {
            memset(s_cert_profile[certificate_id].host_name, 0, sizeof(s_cert_profile[certificate_id].host_name));
        }
        else
        {
            strcpy(s_cert_profile[certificate_id].host_name, server_name);
        }
    }
    s_cert_profile[certificate_id].host_address = ip_address;
    s_cert_profile[certificate_id].cert_id = certificate_id;
    return WIFI_SUCCESS;
}
/**********************************************************************************************************************
 * End of function R_WIFI_SX_ULPGN_SetCertificateProfile
 *********************************************************************************************************************/

/*
 * System state control
 */
/**********************************************************************************************************************
 * Function Name: wifi_system_state_set
 * Description  : Set WIFI system state.
 * Arguments    : state
 * Return Value : none
 *********************************************************************************************************************/
static void wifi_system_state_set(e_wifi_module_status_t state)
{
    s_wifi_system_state = state;
}
/**********************************************************************************************************************
 * End of function wifi_system_state_set
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: wifi_system_state_get
 * Description  : Get WiFi system state.
 * Arguments    : none
 * Return Value : wifi_system_status_t WIFI system state
 *********************************************************************************************************************/
static e_wifi_module_status_t wifi_system_state_get(void)
{
    return s_wifi_system_state;
}
/**********************************************************************************************************************
 * End of function wifi_system_state_get
 *********************************************************************************************************************/

/*
 * Sub functions for API
 */
/**********************************************************************************************************************
 * Function Name: disconnect_ap_sub
 * Description  : Disconnect access point (sub function).
 * Arguments    : none
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_MODULE_COM
 *********************************************************************************************************************/
static wifi_err_t disconnect_ap_sub(void)
{
    /* Not connected access point? */
    if (0 != R_WIFI_SX_ULPGN_IsConnected())
    {
        return WIFI_SUCCESS;
    }

    /* enter to command mode in single channel */
    if (E_OK != enter_command_mode())
    {
        return WIFI_ERR_MODULE_COM;
    }

    /* Disconnect from currently connected Access Point */
    if (AT_OK != at_exec(g_cmd_port, "ATWD\r"))
    {
        return WIFI_ERR_MODULE_COM;
    }

    /* resets the counter */
    at_exec(g_cmd_port, "ATUSTAT=\r");

    /* Index, Counter */
    g_now_sock_idx  = 0;
    g_prev_sock_idx = 0;
    g_sock_tbl_index_old = 0;
    s_sock_data_cnt = 0;
    s_sock_data_cnt_old = 0;
    s_uart1_rx_cnt = 0;

    return WIFI_SUCCESS;
}
/**********************************************************************************************************************
 * End of function disconnect_ap_sub
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: close_socket_sub
 * Description  : Closing socket (sub function)
 * Arguments    : sock_idx
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_CHANGE_SOCKET
 *                WIFI_ERR_MODULE_COM
 *********************************************************************************************************************/
static wifi_err_t close_socket_sub(uint8_t sock_idx)
{
    /* change to next socket index */
    if (E_OK != change_socket_index(sock_idx))
    {
        return WIFI_ERR_CHANGE_SOCKET;
    }

    /* enter to command mode in single channel */
    if (E_OK != enter_command_mode())
    {
        return WIFI_ERR_MODULE_COM;
    }

    /* Close network socket. */
    if (AT_OK != at_exec(g_cmd_port, "ATNCLOSE\r"))
    {
        return WIFI_ERR_MODULE_COM;
    }

    g_sock_tbl[sock_idx].status = ULPGN_SOCKET_STATUS_SOCKET;
    R_BSP_SoftwareDelay(500, BSP_DELAY_MILLISECS);
    return WIFI_SUCCESS;
}
/**********************************************************************************************************************
 * End of function close_socket_sub
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: get_ipaddr
 * Description  : Get IP address by AT command(ATNSET).
 * Arguments    : p_cfg
 * Return Value : IP address (0:OK, other:FAIL)
 *********************************************************************************************************************/
static uint32_t get_ipaddr(wifi_ip_configuration_t * p_cfg)
{
    uint32_t ret;
    static uint32_t ip[4];
    static uint32_t msk[4];
    static uint32_t gw[4];

    /* Initialize */
    memset(p_cfg, 0, sizeof(wifi_ip_configuration_t));

    /* query current IPv4 settings. */
    ret = at_exec(g_cmd_port, "ATNSET=?\r");
    if (AT_OK == ret)
    {
        at_read("IP:%d.%d.%d.%d, Mask:%d.%d.%d.%d, Gateway:%d.%d.%d.%d\r\n",
                &ip[0], &ip[1], &ip[2], &ip[3],
                &msk[0], &msk[1], &msk[2], &msk[3],
                &gw[0], &gw[1], &gw[2], &gw[3]);

        p_cfg->ipaddress  = IPADR_UB_TO_UL(ip[0], ip[1], ip[2], ip[3]);
        p_cfg->subnetmask = IPADR_UB_TO_UL(msk[0], msk[1], msk[2], msk[3]);
        p_cfg->gateway    = IPADR_UB_TO_UL(gw[0], gw[1], gw[2], gw[3]);
    }
    return ret;
}
/**********************************************************************************************************************
 * End of function get_ipaddr
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: get_ipaddr_by_name
 * Description  : Get host IP address by host name.
 * Arguments    : domain_name
 * Return Value : IP address (0:fail , other:assigned IP address)
 *********************************************************************************************************************/
static uint32_t get_ipaddr_by_name(uint8_t * name)
{
    uint32_t ret = 0;
    uint32_t rslt;
    uint32_t ip[4];

    /* Perform DNS query. */
    if (AT_OK != at_exec(g_cmd_port, "ATNDNSQUERY=%s\r", name))
    {
        return ret;
    }

    at_move_to_next_line();
    at_read_wo_prefix("%d\r\n", &rslt);

    /* success? */
    if (1 == rslt)
    {
        at_move_to_next_line();
        at_read_wo_prefix("%d.%d.%d.%d\r\n", &ip[0], &ip[1], &ip[2], &ip[3]);
        ret = IPADR_UB_TO_UL(ip[0], ip[1], ip[2], ip[3]);
    }
    return ret;
}
/**********************************************************************************************************************
 * End of function get_ipaddr_by_name
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: change_socket_index
 * Description  : change socket index.
 * Arguments    : sock_idx
 * Return Value : E_OK    success
 *                E_FAIL  failed
 *********************************************************************************************************************/
static int32_t change_socket_index(uint8_t sock_idx)
{
    int32_t ret = E_OK;

    /* Check parameter */
    if (sock_idx == g_now_sock_idx)
    {
        return E_OK;
    }

    os_wrap_sleep(SOCKET_CHANGE_DELAY, UNIT_MSEC);

    /* Flow control = ON */
    flow_ctrl_set(RTS_ON);

    R_BSP_InterruptsDisable();
    g_prev_sock_idx = g_now_sock_idx;
    R_BSP_InterruptsEnable();

    /* Are there data on previous socket? */
    s_sock_data_cnt = get_statictics_prev_socket();

    while (1)
    {
        /* Send AT command : ATNSOCKINDEX */
        ret = at_exec(g_cmd_port, "ATNSOCKINDEX=%d\r", sock_idx);
        if (AT_BUSY != ret)
        {
            break;
        }

        /* Flow control = OFF */
        flow_ctrl_set(RTS_OFF);

        /* Are there data on previous socket? */
        s_sock_data_cnt = get_statictics_prev_socket();

        while (1)
        {
            if (s_uart1_rx_cnt == s_sock_data_cnt)
            {
                /* Flow control = ON */
                flow_ctrl_set(RTS_ON);
                break;
            }
            os_wrap_sleep(1, UNIT_TICK);
        }
    }

    /* Result = "OK" ? */
    if (AT_OK == ret)
    {
        /* coming data from previous socket? */
        if (s_sock_data_cnt_old < s_sock_data_cnt)
        {
            s_sock_data_cnt_old = s_sock_data_cnt;
            g_sock_tbl_index_old = g_prev_sock_idx;
        }

        R_BSP_InterruptsDisable();
        g_now_sock_idx = sock_idx;
        R_BSP_InterruptsEnable();
        ret = E_OK;
    }
    else
    {
        ret = E_FAIL;
    }
    flow_ctrl_set(RTS_OFF);

    return ret;
}
/**********************************************************************************************************************
 * End of function change_socket_index
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: get_statictics_prev_socket
 * Description  : Get statistics on previous socket
 * Arguments    : none
 * Return Value : number of sent data on previous socket
 *********************************************************************************************************************/
static uint32_t get_statictics_prev_socket(void)
{
    uint32_t recv_tmp;
    uint32_t send_prev;
    uint32_t send_cur;
    uint32_t cnt;

    cnt = 0;
    get_uart_statistics(&recv_tmp, &send_prev);

    while (1)
    {
        get_uart_statistics(&recv_tmp, &send_cur);
        if (send_prev == send_cur)
        {
            cnt++;
        }
        else
        {
            send_prev = send_cur;
            cnt = 0;
        }
        if (ATUSTAT_ATTEMPTS <= cnt)
        {
            break;
        }
    }
    return send_cur;
}
/**********************************************************************************************************************
 * End of function get_statictics_prev_socket
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: get_uart_statistics
 * Description  : get current UART statistics by AT command(ATUSTAT).
 * Arguments    : p_recv - number of bytes received
 *                p_sent - number of bytes sent on UART
 * Return Value : E_OK    success
 *                E_FAIL  failed
 *********************************************************************************************************************/
static int32_t get_uart_statistics(uint32_t * p_recv, uint32_t * p_sent)
{
    int32_t ret = E_FAIL;

    /* enter to command mode in single channel */
    if (E_OK != enter_command_mode())
    {
        return E_FAIL;
    }

    /* Query current UART statistics */
    if (AT_OK == at_exec(g_cmd_port, "ATUSTAT\r"))
    {
        at_read("recv=%d sent=%d", p_recv, p_sent);
        ret = E_OK;
    }
    else
    {
        ret = E_FAIL;
    }

    return ret;
}
/**********************************************************************************************************************
 * End of function get_uart_statistics
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: get_server_certificate
 * Description  : Get stored server certificate on WiFi module.
 * Arguments    : p_cert
 * Return Value : E_OK    success
 *                E_FAIL  failed
 *********************************************************************************************************************/
static int32_t get_server_certificate(wifi_certificate_infomation_t * p_cert)
{
    uint8_t i;
    int32_t ret = E_FAIL;
    uint32_t num_of_files;

    /* Initialize */
    memset(p_cert, 0, sizeof(wifi_certificate_infomation_t));

    /*  Query CA list used by HTTPS client. */
    if (AT_OK == at_exec(g_cmd_port, "ATNSSLCERT=?\r"))
    {
        at_move_to_next_line();
        at_read_wo_prefix("%d\r\n", &num_of_files);
        p_cert->num_of_files = (uint8_t)(num_of_files & 0xff);
        for (i = 0; i < p_cert->num_of_files; i++ )
        {
            at_move_to_next_line();
            at_read_wo_prefix("%s\r\n", &p_cert->cert[i].file_name[0]);
        }
        ret = E_OK;
    }
    else
    {
        ret = E_FAIL;
    }

    return ret;
}
/**********************************************************************************************************************
 * End of function get_server_certificate
 *********************************************************************************************************************/

/*
 * Sub functions for single channel mode
 */
/**********************************************************************************************************************
 * Function Name: single_ch_mode_set
 * Description  : Set mode in single channel.
 * Arguments    : mode
 *                 - MODE_COMMAND
 *                 - MODE_TRANSPARENT
 * Return Value : none
 *********************************************************************************************************************/
static void single_ch_mode_set(uint8_t mode)
{
    s_single_ch_mode = mode;
}
/**********************************************************************************************************************
 * End of function single_ch_mode_set
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: single_ch_mode_get
 * Description  : Get mode in single channel.
 * Arguments    : none
 * Return Value : MODE_COMMAND
 *                MODE_TRANSPARENT
 *********************************************************************************************************************/
static uint8_t single_ch_mode_get(void)
{
    return s_single_ch_mode;
}
/**********************************************************************************************************************
 * End of function single_ch_mode_get
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: enter_command_mode
 * Description  : enter to command mode in single channel.
 * Arguments    : none
 * Return Value : E_OK    success
 *                E_FAIL  failed
 *********************************************************************************************************************/
static int32_t enter_command_mode(void)
{
    if ((1 == s_uart_port_max) && (MODE_TRANSPARENT == single_ch_mode_get()))
    {
        R_BSP_SoftwareDelay(210, BSP_DELAY_MILLISECS);
        at_send(g_cmd_port, "+++");
        R_BSP_SoftwareDelay(185, BSP_DELAY_MILLISECS);
        single_ch_mode_set(MODE_COMMAND);
        R_BSP_SoftwareDelay(25, BSP_DELAY_MILLISECS);
        if (AT_OK != at_recv(g_cmd_port))
        {
            single_ch_mode_set(MODE_TRANSPARENT);
            return E_FAIL;
        }
    }
    return E_OK;
}
/**********************************************************************************************************************
 * End of function enter_command_mode
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: enter_transparent_mode
 * Description  : enter to transparent mode in single channel.
 * Arguments    : none.
 * Return Value : E_OK    success
 *                E_FAIL  failed
 *********************************************************************************************************************/
static int32_t enter_transparent_mode(void)
{
    int32_t ret = E_OK;

    if ((1 == s_uart_port_max) && (MODE_COMMAND == single_ch_mode_get()))
    {
        s_atcmd_exec_ato = 1;
        at_send(g_cmd_port, "ATO\r");
        if (AT_OK != at_recv(g_cmd_port))
        {
            ret = E_FAIL;
        }
        s_atcmd_exec_ato = 0;
        flow_ctrl_set(RTS_OFF);
    }
    return ret;
}
/**********************************************************************************************************************
 * End of function enter_transparent_mode
 *********************************************************************************************************************/

/*
 * Mutex control
 */
/**********************************************************************************************************************
 * Function Name: mutex_create
 * Description  : Create WiFi Module mutex.
 * Arguments    : none
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_TAKE_MUTEX
 *********************************************************************************************************************/
static wifi_err_t mutex_create(void)
{
    wifi_err_t api_ret = WIFI_SUCCESS;

    os_wrap_mutex_delete(&s_binary_sem_tx);
    if (OS_WRAP_SUCCESS != os_wrap_mutex_create(&s_binary_sem_tx, "SX-ULPGN tx mutex"))
    {
        return WIFI_ERR_TAKE_MUTEX;
    }
    os_wrap_mutex_delete(&s_binary_sem_rx);
    if (OS_WRAP_SUCCESS != os_wrap_mutex_create(&s_binary_sem_rx, "SX-ULPGN rx mutex"))
    {
        return WIFI_ERR_TAKE_MUTEX;
    }
    return api_ret;
}
/**********************************************************************************************************************
 * End of function mutex_create
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: mutex_take
 * Description  : Take mutex for WiFi module.
 * Arguments    : mutex_flag
 * Return Value : E_OK    success
 *                E_FAIL  failed
 *********************************************************************************************************************/
static int32_t mutex_take(uint8_t mutex_flag)
{
    if (0 != (mutex_flag & MUTEX_TX))
    {
        if (OS_WRAP_SUCCESS != os_wrap_mutex_take(&s_binary_sem_tx, s_sem_block_time, CALL_NOT_ISR))
        {
            return E_FAIL;
        }
    }

    if (0 != (mutex_flag & MUTEX_RX))
    {
        if (OS_WRAP_SUCCESS != os_wrap_mutex_take(&s_binary_sem_rx, s_sem_block_time, CALL_NOT_ISR))
        {
            return E_FAIL;
        }
    }

    return E_OK;
}
/**********************************************************************************************************************
 * End of function mutex_take
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: mutex_give
 * Description  : Give mutex for WiFi module.
 * Arguments    : mutex_flg
 * Return Value : none
 *********************************************************************************************************************/
static void mutex_give(uint8_t mutex_flg)
{
    if (0 != (mutex_flg & MUTEX_RX))
    {
        os_wrap_mutex_give(&s_binary_sem_rx, CALL_NOT_ISR);
        os_wrap_sleep(1, UNIT_TICK);
    }
    if (0 != (mutex_flg & MUTEX_TX))
    {
        os_wrap_mutex_give(&s_binary_sem_tx, CALL_NOT_ISR);
        os_wrap_sleep(1, UNIT_TICK);
    }
}
/**********************************************************************************************************************
 * End of function mutex_give
 *********************************************************************************************************************/

/*
 * BYTEQ control for socket
 */
/**********************************************************************************************************************
 * Function Name: socket_byteq_open
 * Description  : open BYTEQ in socket table.
 * Arguments    : max_sock - maximum of socket tables
 * Return Value : WIFI_SUCCESS
 *                WIFI_ERR_BYTEQ_OPEN
 *********************************************************************************************************************/
static wifi_err_t socket_byteq_open(void)
{
    uint8_t i;
    wifi_err_t api_ret = WIFI_SUCCESS;

    for (i = 0; i < s_sockets_max; i++ )
    {
        if (BYTEQ_SUCCESS !=
                R_BYTEQ_Open(g_sock_tbl[i].recv_buf, SOCK_BUF_MAX, &g_sock_tbl[i].byteq_hdl))
        {
            api_ret = WIFI_ERR_BYTEQ_OPEN;
            break;
        }
    }
    return api_ret;
}
/**********************************************************************************************************************
 * End of function socket_byteq_open
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: socket_byteq_close
 * Description  : close BYTEQ on socket table.
 * Arguments    : none
 * Return Value : none
 *********************************************************************************************************************/
static void socket_byteq_close(void)
{
    uint8_t i;

    for (i = 0; i < s_sockets_max; i++ )
    {
        if (0 != g_sock_tbl[i].byteq_hdl)
        {
            R_BYTEQ_Close(g_sock_tbl[i].byteq_hdl);
        }
    }
}
/**********************************************************************************************************************
 * End of function socket_byteq_close
 *********************************************************************************************************************/

/*
 * WIFI module control
 */
/**********************************************************************************************************************
 * Function Name: sx_ulpgn_hw_reset
 * Description  : Reset SX-ULPGN.
 * Arguments    : none
 * Return Value : none
 *********************************************************************************************************************/
static void sx_ulpgn_hw_reset(void)
{
    /* Phase 3 WIFI Module hardware reset   */
    WIFI_RESET_DDR(WIFI_CFG_RESET_PORT, WIFI_CFG_RESET_PIN) = 1;  /* output */
    WIFI_RESET_DR(WIFI_CFG_RESET_PORT, WIFI_CFG_RESET_PIN)  = 0;  /* low    */
    R_BSP_SoftwareDelay(30, BSP_DELAY_MILLISECS);
    WIFI_RESET_DR(WIFI_CFG_RESET_PORT, WIFI_CFG_RESET_PIN)  = 1;  /* high   */
    R_BSP_SoftwareDelay(250, BSP_DELAY_MILLISECS);
}
/**********************************************************************************************************************
 * End of function sx_ulpgn_hw_reset
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: sx_ulpgn_close
 * Description  : Close WIFI module.
 * Arguments    : none
 * Return Value : none
 *********************************************************************************************************************/
static void sx_ulpgn_close(void)
{
    uart_port_close(PORT_HSUART2);
    uart_port_close(PORT_HSUART1);
    os_wrap_mutex_delete(&s_binary_sem_rx);
    os_wrap_mutex_delete(&s_binary_sem_tx);
    socket_byteq_close();
    wifi_system_state_set(MODULE_DISCONNECTED);
}
/**********************************************************************************************************************
 * End of function sx_ulpgn_close
 *********************************************************************************************************************/

/*
 * Port configuration
 */
/**********************************************************************************************************************
 * Function Name: flow_ctrl_init
 * Description  : Initialize HW flow control.
 * Arguments    : none
 * Return Value : none
 *********************************************************************************************************************/
static void flow_ctrl_init(void)
{
    wrap_sci_ctrl(g_data_port, SCI_CMD_EN_CTS_IN, NULL);
    WIFI_RTS_DDR(WIFI_CFG_RTS_PORT, WIFI_CFG_RTS_PIN) = 1;  /* Output */
    WIFI_RTS_DR(WIFI_CFG_RTS_PORT, WIFI_CFG_RTS_PIN)  = 0;  /* low    */
}
/**********************************************************************************************************************
 * End of function flow_ctrl_init
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: flow_ctrl_set
 * Description  : Set HW flow control.
 * Arguments    : flow (RTS_OFF, RTS_ON)
 * Return Value : none
 *********************************************************************************************************************/
static void flow_ctrl_set(uint32_t flow)
{
    WIFI_RTS_DR(WIFI_CFG_RTS_PORT, WIFI_CFG_RTS_PIN) = flow;
}
/**********************************************************************************************************************
 * End of function flow_ctrl_set
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: get_port_config
 * Description  : get port(HSUART1 or HSUART2) configuration table pointer.
 * Arguments    : port - (0:HSUART1, 1:HSUART2)
 * Return Value : SUCCESS : SCI configuration table(st_sci_conf_t) pointer by port.
 *                FAIL    : NULL
 *********************************************************************************************************************/
static st_sci_conf_t * get_port_config(uint8_t port)
{
    uint16_t i;
    uint8_t  ch;
    st_sci_conf_t * p_tbl = NULL;

    if (port < s_uart_port_max)
    {
        /* port number -> SCI channel number */
        ch = s_port_to_sci[port];

        /* Set table pointer */
        for (i = 0;; i++ )
        {
            if (SCI_NUM_CH == s_sci_cfg[i].ch)
            {
                break;
            }
            if (ch == s_sci_cfg[i].ch)
            {
                p_tbl = (st_sci_conf_t *)&s_sci_cfg[i];
                break;
            }
        }
    }
    return p_tbl;
}
/**********************************************************************************************************************
 * End of function get_port_config
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: cmd_port_open
 * Description  : Initialize SCI for Command port.
 * Arguments    : port  - Command port number
 *                p_cb  - Callback function of SCI interrupts
 * Return Value : E_OK    success
 *                E_FAIL  failed
 *********************************************************************************************************************/
static int32_t cmd_port_open(uint8_t port, void (* const p_cb)(void *p_args))
{
    st_uart_tbl_t * p_uart = &g_uart_tbl[port];
    st_sci_conf_t * p_cfg = get_port_config(port);

    if (NULL == p_cfg)
    {
        return E_FAIL;
    }

    /* Port settings */
    p_cfg->func();

    memset(&p_uart->sci_hdl , 0, sizeof(sci_hdl_t));
    p_uart->sci_config.async.baud_rate    = SX_ULPGN_BAUD_DEFAULT;
    p_uart->sci_config.async.clk_src      = SCI_CLK_INT;
    p_uart->sci_config.async.data_size    = SCI_DATA_8BIT;
    p_uart->sci_config.async.parity_en    = SCI_PARITY_OFF;
    p_uart->sci_config.async.parity_type  = SCI_EVEN_PARITY;
    p_uart->sci_config.async.stop_bits    = SCI_STOPBITS_1;
    p_uart->sci_config.async.int_priority = WIFI_CFG_SCI_INTERRUPT_LEVEL;
    if (SCI_SUCCESS != R_SCI_Open(p_cfg->ch, SCI_MODE_ASYNC, &p_uart->sci_config, p_cb, &p_uart->sci_hdl))
    {
        return E_FAIL;
    }
    return E_OK;
}
/**********************************************************************************************************************
 * End of function cmd_port_open
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: data_port_open
 * Description  : Initialize SCI for Data port.
 * Arguments    : port  - Data port number
 *                p_cb  - Callback function of SCI interrupts
 * Return Value : E_OK    success
 *                E_FAIL  failed
 *********************************************************************************************************************/
static int32_t data_port_open(uint8_t port, void (* const p_cb)(void *p_args))
{
    st_uart_tbl_t * p_uart = &g_uart_tbl[port];
    st_sci_conf_t * p_cfg = get_port_config(port);
    sci_err_t sci_err;

    if (NULL == p_cfg)
    {
        return E_FAIL;
    }

    /* Port settings */
    p_cfg->func();

    memset(&p_uart->sci_hdl , 0, sizeof(sci_hdl_t));
    p_uart->sci_config.async.baud_rate    = WIFI_CFG_SCI_BAUDRATE;
    p_uart->sci_config.async.clk_src      = SCI_CLK_INT;
    p_uart->sci_config.async.data_size    = SCI_DATA_8BIT;
    p_uart->sci_config.async.parity_en    = SCI_PARITY_OFF;
    p_uart->sci_config.async.parity_type  = SCI_EVEN_PARITY;
    p_uart->sci_config.async.stop_bits    = SCI_STOPBITS_1;
    p_uart->sci_config.async.int_priority = WIFI_CFG_SCI_INTERRUPT_LEVEL;
    sci_err = R_SCI_Open(p_cfg->ch, SCI_MODE_ASYNC, &p_uart->sci_config, p_cb, &p_uart->sci_hdl);
    if (SCI_SUCCESS != sci_err)
    {
        return E_FAIL;
    }

    return E_OK;
}
/**********************************************************************************************************************
 * End of function data_port_open
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: uart_port_close
 * Description  : Close serial port.
 * Arguments    : port  - HSUART port number
 * Return Value : none
 *********************************************************************************************************************/
static void uart_port_close(uint8_t port)
{
    st_uart_tbl_t * p_uart = &g_uart_tbl[port];

    if (0 != p_uart->sci_hdl)
    {
        R_SCI_Control(p_uart->sci_hdl, SCI_CMD_RX_Q_FLUSH, NULL);
        R_SCI_Control(p_uart->sci_hdl, SCI_CMD_TX_Q_FLUSH, NULL);
        R_SCI_Close(p_uart->sci_hdl);
    }
}
/**********************************************************************************************************************
 * End of function uart_port_close
 *********************************************************************************************************************/

/*
 * Callback functions
 */
/**********************************************************************************************************************
 * Function Name: cb_sci_hsuart1_initial
 * Description  : SCI callback function of HSUART1 initial.
 * Arguments    : pArgs
 * Return Value : none
 *********************************************************************************************************************/
static void cb_sci_hsuart1_initial(void * pArgs)
{
    sci_cb_args_t * p_args = (sci_cb_args_t *) pArgs;
    uint8_t data;
    static uint8_t buf[64];
    static uint8_t cnt = 0;

    if (SCI_EVT_RX_CHAR == p_args->event)
    {
        /* From RXI interrupt; received character data is in p_args->byte */
        if (MODE_TRANSPARENT == single_ch_mode_get())
        {
            wrap_sci_recv(g_data_port, &data, 1);
            if (BYTEQ_SUCCESS != wrap_byteq_put(g_now_sock_idx, data))
            {
                g_sock_tbl[g_now_sock_idx].put_err_cnt++;
                post_err_event(WIFI_EVENT_SOCKET_RXQ_OVF_ERR, g_now_sock_idx);
            }
        }
        else
        {
            /* Executing ATO command? */
            if (1 == s_atcmd_exec_ato)
            {
                if (63 <= cnt)
                {
                    cnt = 0;
                }
                buf[cnt] = (uint8_t)p_args->byte;
                buf[cnt + 1] = 0;
                if ('\n' == buf[cnt])
                {
                    /* result is OK? */
                    if (0 == strcmp((const char *)buf, "OK\r\n"))
                    {
                        single_ch_mode_set(MODE_TRANSPARENT);
                        flow_ctrl_set(RTS_ON);
                    }
                    cnt = 0;
                }
                else
                {
                    cnt++;
                }
            }
        }
    }
    else if (SCI_EVT_TEI == p_args->event)
    {
        g_uart_tbl[g_cmd_port].tx_end_flag = 1;
    }
    else
    {
        cb_sci_err(p_args->event);
    }
}
/**********************************************************************************************************************
 * End of function cb_sci_hsuart1_initial
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: cb_sci_hsuart1_for_data
 * Description  : SCI callback function of serial data port.
 * Arguments    : pArgs
 * Return Value : none
 *********************************************************************************************************************/
static void cb_sci_hsuart1_for_data(void * pArgs)
{
    sci_cb_args_t * p_args = (sci_cb_args_t *) pArgs;
    uint8_t         data;
    static int32_t  cnt;
    static uint8_t  sock_idx;

    if (SCI_EVT_RX_CHAR == p_args->event)
    {
        s_uart1_rx_cnt++;
        sock_idx = g_now_sock_idx;

        /* changed socket? */
        if (g_prev_sock_idx != sock_idx)
        {
            /* received on previous socket? */
            cnt = (int32_t)(s_sock_data_cnt - s_uart1_rx_cnt);
            if (0 <= cnt)
            {
                /* index = previous socket */
                sock_idx = g_sock_tbl_index_old;
            }
        }

        /* Move received data to g_sock_tbl[current_index] */
        wrap_sci_recv(g_data_port, &data, 1);
        if (BYTEQ_SUCCESS != wrap_byteq_put(sock_idx, data))
        {
            g_sock_tbl[sock_idx].put_err_cnt++;
            post_err_event(WIFI_EVENT_SOCKET_RXQ_OVF_ERR, sock_idx);
        }
    }
    else if (SCI_EVT_TEI == p_args->event)
    {
        g_uart_tbl[g_data_port].tx_end_flag = 1;
    }
    else
    {
        cb_sci_err(p_args->event);
    }
}
/**********************************************************************************************************************
 * End of function cb_sci_hsuart1_for_data
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: cb_sci_hsuart2_for_cmd
 * Description  : SCI callback function of serial secondary port.
 * Arguments    : pArgs
 * Return Value : none
 *********************************************************************************************************************/
static void cb_sci_hsuart2_for_cmd(void * pArgs)
{
    sci_cb_args_t * p_args = (sci_cb_args_t *) pArgs;

    if (SCI_EVT_RX_CHAR == p_args->event)
    {
        R_BSP_NOP();
    }
    else if (SCI_EVT_TEI == p_args->event)
    {
        g_uart_tbl[g_cmd_port].tx_end_flag = 1;
    }
    else
    {
        cb_sci_err(p_args->event);
    }
}
/**********************************************************************************************************************
 * End of function cb_sci_hsuart2_for_cmd
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: cb_sci_err
 * Description  : SCI callback function of error event.
 * Arguments    : event
 * Return Value : none
 *********************************************************************************************************************/
static void cb_sci_err(sci_cb_evt_t event)
{
    if (SCI_EVT_RXBUF_OVFL == event)
    {
        /* From RXI interrupt; rx queue is full */
        post_err_event(WIFI_EVENT_SERIAL_RXQ_OVF_ERR, 0);
    }
    else if (SCI_EVT_OVFL_ERR == event)
    {
        /* From receiver overflow error interrupt */
        post_err_event(WIFI_EVENT_SERIAL_OVF_ERR, 0);
    }
    else if (SCI_EVT_FRAMING_ERR == event)
    {
        /* From receiver framing error interrupt */
        post_err_event(WIFI_EVENT_SERIAL_FLM_ERR, 0);
    }
    else
    {
        /* Do nothing */
    }
}
/**********************************************************************************************************************
 * End of function cb_sci_err
 *********************************************************************************************************************/

/*
 * Wrapper
 */
/**********************************************************************************************************************
 * Function Name: wrap_sci_send
 * Description  : Wrapped R_SCI_Send().
 * Arguments    : port  - HSUART port number
 *                p_src
 *                length
 * Return Value : Same as R_SCI_Send() function
 *********************************************************************************************************************/
static sci_err_t wrap_sci_send(uint8_t port, uint8_t *p_src, uint16_t const length)
{
    g_uart_tbl[port].tx_end_flag = 0;
    return R_SCI_Send(g_uart_tbl[port].sci_hdl, p_src, length);
}
/**********************************************************************************************************************
 * End of function wrap_sci_send
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: wrap_sci_ctrl
 * Description  : Wrapped R_SCI_Control().
 * Arguments    : port  - HSUART port number
 *                cmd
 *                p_args
 * Return Value : Same as R_SCI_Control() function
 *********************************************************************************************************************/
static sci_err_t wrap_sci_ctrl(uint8_t port, sci_cmd_t const cmd, void * p_args)
{
    return R_SCI_Control(g_uart_tbl[port].sci_hdl, cmd, p_args);
}
/**********************************************************************************************************************
 * End of function wrap_sci_ctrl
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: wrap_sci_recv
 * Description  : Wrapped R_SCI_Receive().
 * Arguments    : port  - HSUART port number
 *                p_dst
 *                length
 * Return Value : Same as R_SCI_Receive() function
 *********************************************************************************************************************/
static sci_err_t wrap_sci_recv(uint8_t port, uint8_t * p_dst, uint16_t const length)
{
    return R_SCI_Receive(g_uart_tbl[port].sci_hdl, p_dst, length);
}
/**********************************************************************************************************************
 * End of function wrap_sci_recv
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: wrap_byteq_put
 * Description  : Wrapped R_BYTEQ_Put().
 * Arguments    : sock_idx (0-3)
 *                byte
 * Return Value : Same as R_BYTEQ_Put() function
 *********************************************************************************************************************/
static byteq_err_t wrap_byteq_put(uint8_t sock_idx, uint8_t const byte)
{
    return R_BYTEQ_Put(g_sock_tbl[sock_idx].byteq_hdl, byte);
}
/**********************************************************************************************************************
 * End of function wrap_byteq_put
 *********************************************************************************************************************/

static uint32_t get_dnsaddr(uint32_t *dns_address, uint32_t *dns_address_count)
{
    uint32_t ret;
    uint32_t rslt;
    uint32_t dnsaddr[4];

    *dns_address = 0;
    memset(dns_address, 0, *dns_address_count * sizeof(uint32_t));

    /* Perform DNS query. */
    ret = at_exec(g_cmd_port, "ATNDNSSVR=\?\r");
    if (AT_OK == ret)
    {
    	at_move_to_next_line();
    	at_read_wo_prefix("%d\r\n", &rslt);

        // we can only return as many as we have asked for
        if (rslt > *dns_address_count)
        {
            rslt = *dns_address_count;
        }

        for (uint32_t i = 0; i < rslt; ++i)
        {
    		at_move_to_next_line();
    		at_read_wo_prefix("%d.%d.%d.%d\r\n", &dnsaddr[0], &dnsaddr[1], &dnsaddr[2], &dnsaddr[3]);
    		dns_address[i]  = IPADR_UB_TO_UL(dnsaddr[0], dnsaddr[1], dnsaddr[2], dnsaddr[3]);
        }

        // return the number of entries processed
        *dns_address_count = rslt;
    }

    return ret;
}


wifi_err_t R_WIFI_SX_ULPGN_GetDnsServerAddress(uint32_t *dns_address, uint32_t *dns_address_count)
{
    wifi_err_t api_ret = WIFI_SUCCESS;

    /* Disconnected WiFi module? */
    if (MODULE_DISCONNECTED == wifi_system_state_get())
    {
        return WIFI_ERR_NOT_OPEN;
    }

    /* Check parameter */
    if (NULL == dns_address)
    {
        return WIFI_ERR_PARAMETER;
    }

    /* Take mutex */
    if (E_OK != mutex_take(MUTEX_TX | MUTEX_RX))
    {
        return WIFI_ERR_TAKE_MUTEX;
    }

    /* Get IP address */
    if (AT_OK != get_dnsaddr(dns_address, dns_address_count))
    {
        api_ret = WIFI_ERR_MODULE_COM;
    }

    /* Give mutex */
    mutex_give(MUTEX_TX | MUTEX_RX);

    return api_ret;
}
