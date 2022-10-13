/* Generated configuration header file - do not edit */
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
 * File Name    : r_wifi_sx_ulpgn_config.h
 * Version      : 1.0
 * Description  : SX ULPGN WiFi driver Configuration.
 *********************************************************************************************************************/
/**********************************************************************************************************************
 * History : DD.MM.YYYY Version  Description
 *         : 01.01.2020 1.00     First Release
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Includes   <System Includes> , "Project Includes"
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Macro definitions
 *********************************************************************************************************************/
#ifndef R_WIFI_SX_ULPGN_CONFIG_H
#define R_WIFI_SX_ULPGN_CONFIG_H

#define WIFI_CFG_SCI_CHANNEL                  (0)

#define WIFI_CFG_SCI_INTERRUPT_LEVEL          (14)

#define WIFI_CFG_SCI_SECOND_CHANNEL           (1)

#define WIFI_CFG_SCI_PCLK_HZ                  (60000000)  // peripheral clock speed; e.g. 60000000 is 60MHz

#define WIFI_CFG_SCI_BAUDRATE                 (460800)

#define WIFI_CFG_SCI_USE_FLOW_CONTROL         (1)

#define WIFI_CFG_RESET_PORT                    D
#define WIFI_CFG_RESET_PIN                     0

#define WIFI_CFG_RTS_PORT                      2
#define WIFI_CFG_RTS_PIN                       2

#define WIFI_CFG_CREATABLE_SOCKETS            (4)

#define WIFI_CFG_SOCKETS_RECEIVE_BUFFER_SIZE  (4096)

#define WIFI_CFG_USE_CALLBACK_FUNCTION        (0)

#define WIFI_CFG_CALLBACK_FUNCTION_NAME       (NULL)

/**********************************************************************************************************************
 Global Typedef definitions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 External global variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Exported global functions
 *********************************************************************************************************************/

#endif /* R_WIFI_SX_ULPGN_CONFIG_H */
