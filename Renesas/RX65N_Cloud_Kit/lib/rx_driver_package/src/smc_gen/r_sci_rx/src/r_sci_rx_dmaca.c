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
 * Copyright (C) 2020 Renesas Electronics Corporation. All rights reserved.
 *********************************************************************************************************************/
/**********************************************************************************************************************
* File Name    : r_sci_rx_dmaca.c
* Description  : 
***********************************************************************************************************************
* History : DD.MM.YYYY Version Description
*           25.08.2020 1.00    Initial Release
***********************************************************************************************************************/

/**********************************************************************************************************************
 Includes   <System Includes> , "Project Includes"
 *********************************************************************************************************************/
#include "platform.h"
#include "r_sci_rx_private.h"
#include "r_sci_rx_if.h"
#if ((TX_DTC_DMACA_ENABLE & 0x02) || (RX_DTC_DMACA_ENABLE & 0x02))
#include "r_dmaca_rx_if.h"
#include "r_sci_rx_dmaca.h"
#include "r_sci_rx_platform.h"

/**********************************************************************************************************************
 Macro definitions
 *********************************************************************************************************************/
#define SCI_PRV_RX_FIFO_THRESHOLD   (hdl->rom->regs->FCR.BIT.RTRG)

/**********************************************************************************************************************
 Typedef definitions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Exported global variables
 *********************************************************************************************************************/


/**********************************************************************************************************************
 Private (static) variables and functions
 *********************************************************************************************************************/
static void sci0_dmac_rx_callback(void);
static void sci1_dmac_rx_callback(void);
static void sci2_dmac_rx_callback(void);
static void sci3_dmac_rx_callback(void);
static void sci4_dmac_rx_callback(void);
static void sci5_dmac_rx_callback(void);
static void sci6_dmac_rx_callback(void);
static void sci7_dmac_rx_callback(void);
static void sci8_dmac_rx_callback(void);
static void sci9_dmac_rx_callback(void);
static void sci10_dmac_rx_callback(void);
static void sci11_dmac_rx_callback(void);
static void sci12_dmac_rx_callback(void);
static void sci_dmac_rx_handler(sci_hdl_t const hdl);

static void sci0_dmac_tx_callback(void);
static void sci1_dmac_tx_callback(void);
static void sci2_dmac_tx_callback(void);
static void sci3_dmac_tx_callback(void);
static void sci4_dmac_tx_callback(void);
static void sci5_dmac_tx_callback(void);
static void sci6_dmac_tx_callback(void);
static void sci7_dmac_tx_callback(void);
static void sci8_dmac_tx_callback(void);
static void sci9_dmac_tx_callback(void);
static void sci10_dmac_tx_callback(void);
static void sci11_dmac_tx_callback(void);
static void sci12_dmac_tx_callback(void);
static void sci_dmac_tx_handler(sci_hdl_t const hdl);

static uint8_t tx_dummy_buf = SCI_CFG_DUMMY_TX_BYTE;
static uint8_t rx_dummy_buf;

#if SCI_CFG_FIFO_INCLUDED
/**********************************************************************************************************************
* Function Name: sci_txfifo_dmaca_create
* Description  : This function create DMAC to transmit data from p_src to SCI TX FIFO (by DMAC without CPU)
* Arguments    : hdl -
*                    handle for channel (ptr to chan control block)
*                p_src -
*                    The address of source data need to be sent
*                lenght -
*                    The number of data need to be sent
* Return Value : SCI_SUCCESS -
*                    Create DMAC successfully
*                SCI_ERR_DMACA -
*                    Create DMAC NOT successfully
 *********************************************************************************************************************/
sci_err_t sci_txfifo_dmaca_create(sci_hdl_t const hdl, uint8_t *p_src, uint16_t const length)
{
    sci_err_t     err_sci = SCI_SUCCESS;
    dmaca_return_t err_dmaca = DMACA_SUCCESS;
    dmaca_stat_t   stat_dmaca;
    dmaca_transfer_data_cfg_t tx_cfg_dmaca;
    sci_fifo_ctrl_t          *p_tctrl_dma;
    p_tctrl_dma = &hdl->queue[hdl->qindex_app_tx];

    tx_cfg_dmaca.act_source           = hdl->rom->dmaca_tx_act_src;
    tx_cfg_dmaca.transfer_mode        = DMACA_TRANSFER_MODE_BLOCK;
    tx_cfg_dmaca.data_size            = DMACA_DATA_SIZE_BYTE;
    tx_cfg_dmaca.dtie_request         = DMACA_TRANSFER_END_INTERRUPT_ENABLE;
    tx_cfg_dmaca.interrupt_sel        = DMACA_CLEAR_INTERRUPT_FLAG_BEGINNING_TRANSFER;
    tx_cfg_dmaca.des_addr_mode        = DMACA_DES_ADDR_FIXED;
    tx_cfg_dmaca.p_des_addr           = (void *)&hdl->rom->regs->FTDR.BYTE.L;

    tx_cfg_dmaca.request_source       = DMACA_TRANSFER_REQUEST_PERIPHERAL;
    tx_cfg_dmaca.esie_request         = DMACA_TRANSFER_ESCAPE_END_INTERRUPT_DISABLE;
    tx_cfg_dmaca.rptie_request        = DMACA_REPEAT_SIZE_END_INTERRUPT_DISABLE;
    tx_cfg_dmaca.sarie_request        = DMACA_SRC_ADDR_EXT_REP_AREA_OVER_INTERRUPT_DISABLE;
    tx_cfg_dmaca.darie_request        = DMACA_DES_ADDR_EXT_REP_AREA_OVER_INTERRUPT_DISABLE;
    tx_cfg_dmaca.src_addr_repeat_area = DMACA_SRC_ADDR_EXT_REP_AREA_NONE;
    tx_cfg_dmaca.des_addr_repeat_area = DMACA_DES_ADDR_EXT_REP_AREA_NONE;
    tx_cfg_dmaca.offset_value         = 0;

    if (NULL == p_src)
    {
        tx_cfg_dmaca.repeat_block_side = DMACA_REPEAT_BLOCK_SOURCE;
        tx_cfg_dmaca.src_addr_mode     = DMACA_SRC_ADDR_FIXED;
        tx_cfg_dmaca.p_src_addr        = (void *)&tx_dummy_buf;
    }
    else
    {
        tx_cfg_dmaca.repeat_block_side = DMACA_REPEAT_BLOCK_DESTINATION;
        tx_cfg_dmaca.src_addr_mode     = DMACA_SRC_ADDR_INCR;
        tx_cfg_dmaca.p_src_addr        = (void *)p_src;
    }

    tx_cfg_dmaca.transfer_count = (uint32_t)(length / hdl->rom->dtc_dmaca_tx_block_size);
    tx_cfg_dmaca.block_size     = hdl->rom->dtc_dmaca_tx_block_size;

    err_dmaca = R_DMACA_Open(hdl->rom->dmaca_tx_channel);
    if (DMACA_SUCCESS == err_dmaca)
    {
        err_dmaca = R_DMACA_Control(hdl->rom->dmaca_tx_channel, DMACA_CMD_DISABLE, &stat_dmaca);
    }

    if (DMACA_SUCCESS == err_dmaca)
    {
        if (length >= tx_cfg_dmaca.block_size)
        {
            if (NULL == p_src)
            {
                p_tctrl_dma->p_tx_fraction_buf = NULL;
            }
            else
            {
                p_tctrl_dma->p_tx_fraction_buf = (uint8_t*)((uint32_t)tx_cfg_dmaca.p_src_addr + (tx_cfg_dmaca.transfer_count * tx_cfg_dmaca.block_size));
            }

            p_tctrl_dma->tx_fraction = (length - (tx_cfg_dmaca.transfer_count * tx_cfg_dmaca.block_size));

            if (SCI_CH0 == hdl->rom->chan)
            {
                err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_tx_channel,(void *)sci0_dmac_tx_callback);
            }
            else if (SCI_CH1 == hdl->rom->chan)
            {
                err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_tx_channel,(void *)sci1_dmac_tx_callback);
            }
            else if (SCI_CH2 == hdl->rom->chan)
            {
                err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_tx_channel,(void *)sci2_dmac_tx_callback);
            }
            else if (SCI_CH3 == hdl->rom->chan)
            {
                err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_tx_channel,(void *)sci3_dmac_tx_callback);
            }
            else if (SCI_CH4 == hdl->rom->chan)
            {
                err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_tx_channel,(void *)sci4_dmac_tx_callback);
            }
            else if (SCI_CH5 == hdl->rom->chan)
            {
                err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_tx_channel,(void *)sci5_dmac_tx_callback);
            }
            else if (SCI_CH6 == hdl->rom->chan)
            {
                err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_tx_channel,(void *)sci6_dmac_tx_callback);
            }
            else if (SCI_CH7 == hdl->rom->chan)
            {
                err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_tx_channel,(void *)sci7_dmac_tx_callback);
            }
            else if (SCI_CH8 == hdl->rom->chan)
            {
                err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_tx_channel,(void *)sci8_dmac_tx_callback);
            }
            else if (SCI_CH9 == hdl->rom->chan)
            {
                err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_tx_channel,(void *)sci9_dmac_tx_callback);
            }
            else if (SCI_CH10 == hdl->rom->chan)
            {
                err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_tx_channel,(void *)sci10_dmac_tx_callback);
            }
            else if (SCI_CH11 == hdl->rom->chan)
            {
                err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_tx_channel,(void *)sci11_dmac_tx_callback);
            }
            else if (SCI_CH12 == hdl->rom->chan)
            {
                err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_tx_channel,(void *)sci12_dmac_tx_callback);
            }
            else
            {
                err_dmaca = DMACA_ERR_INTERNAL;
            }

            if (DMACA_SUCCESS == err_dmaca)
            {
                err_dmaca = R_DMACA_Create(hdl->rom->dmaca_tx_channel, &tx_cfg_dmaca);
            }

#if defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX71M) || defined(BSP_MCU_RX65N) || defined(BSP_MCU_RX66T) || defined(BSP_MCU_RX72T) || defined(BSP_MCU_RX72M) || defined(BSP_MCU_RX72N) || defined(BSP_MCU_RX66N)
            err_dmaca = R_DMACA_Int_Enable(hdl->rom->dmaca_tx_channel, *hdl->rom->ipr_txi);
#else
            err_dmaca = R_DMACA_Int_Enable(hdl->rom->dmaca_tx_channel, *hdl->rom->ipr);
#endif
            if (DMACA_SUCCESS == err_dmaca)
            {
                err_dmaca = R_DMACA_Control(hdl->rom->dmaca_tx_channel, DMACA_CMD_ENABLE, &stat_dmaca);
            }
        }
        else
        {
            p_tctrl_dma->p_tx_fraction_buf = p_src;
            p_tctrl_dma->tx_fraction = length;
        }
    }

    if (DMACA_SUCCESS != err_dmaca)
    {
        err_sci = SCI_ERR_DMACA;
    }

    return err_sci;
}
/**********************************************************************************************************************
 End of function sci_txfifo_dmaca_create
 *********************************************************************************************************************/

/**********************************************************************************************************************
* Function Name: sci_rxfifo_dmaca_create
* Description  : This function create DMAC to receive data from SCI RX FIFO and save to p_dst (by DMAC without CPU)
* Arguments    : hdl -
*                    handle for channel (ptr to chan control block)
*                p_dst -
*                    The address of destination need to be saved
*                lenght -
*                    The number of data need to be received
* Return Value : SCI_SUCCESS -
*                    Create DMAC successfully
*                SCI_ERR_DMACA -
*                    Create DMAC NOT successfully
 *********************************************************************************************************************/
sci_err_t sci_rxfifo_dmaca_create(sci_hdl_t const hdl, uint8_t *p_dst, uint16_t const length)
{
    sci_fifo_ctrl_t          *p_rctrl_dma;
    dmaca_transfer_data_cfg_t rx_cfg_dmaca;
    sci_err_t     err_sci = SCI_SUCCESS;
    dmaca_return_t err_dmaca = DMACA_SUCCESS;
    dmaca_stat_t   stat_dmaca;
    p_rctrl_dma = &hdl->queue[hdl->qindex_app_rx];

    rx_cfg_dmaca.act_source           = hdl->rom->dmaca_rx_act_src;
    rx_cfg_dmaca.transfer_mode        = DMACA_TRANSFER_MODE_BLOCK;
    rx_cfg_dmaca.data_size            = DMACA_DATA_SIZE_BYTE;
    rx_cfg_dmaca.dtie_request         = DMACA_TRANSFER_END_INTERRUPT_ENABLE;
    rx_cfg_dmaca.interrupt_sel        = DMACA_CLEAR_INTERRUPT_FLAG_BEGINNING_TRANSFER;
    rx_cfg_dmaca.src_addr_mode        = DMACA_SRC_ADDR_FIXED;
    rx_cfg_dmaca.p_src_addr           = (void *)&hdl->rom->regs->FRDR.BYTE.L;

    rx_cfg_dmaca.request_source       = DMACA_TRANSFER_REQUEST_PERIPHERAL;
    rx_cfg_dmaca.esie_request         = DMACA_TRANSFER_ESCAPE_END_INTERRUPT_DISABLE;
    rx_cfg_dmaca.rptie_request        = DMACA_REPEAT_SIZE_END_INTERRUPT_DISABLE;
    rx_cfg_dmaca.sarie_request        = DMACA_SRC_ADDR_EXT_REP_AREA_OVER_INTERRUPT_DISABLE;
    rx_cfg_dmaca.darie_request        = DMACA_DES_ADDR_EXT_REP_AREA_OVER_INTERRUPT_DISABLE;
    rx_cfg_dmaca.src_addr_repeat_area = DMACA_SRC_ADDR_EXT_REP_AREA_NONE;
    rx_cfg_dmaca.des_addr_repeat_area = DMACA_DES_ADDR_EXT_REP_AREA_NONE;
    rx_cfg_dmaca.offset_value         = 0;

    if (NULL == p_dst)
    {
        rx_cfg_dmaca.repeat_block_side = DMACA_REPEAT_BLOCK_DESTINATION;
        rx_cfg_dmaca.des_addr_mode     = DMACA_DES_ADDR_FIXED;
        rx_cfg_dmaca.p_des_addr        = (void *)&rx_dummy_buf;
    }
    else
    {
        rx_cfg_dmaca.repeat_block_side = DMACA_REPEAT_BLOCK_SOURCE;
        rx_cfg_dmaca.des_addr_mode     = DMACA_DES_ADDR_INCR;
        rx_cfg_dmaca.p_des_addr        = (void *)p_dst;
    }

    rx_cfg_dmaca.transfer_count     = (uint32_t)(length / hdl->rom->dtc_dmaca_rx_block_size);
    rx_cfg_dmaca.block_size     = hdl->rom->dtc_dmaca_rx_block_size;

    err_dmaca = R_DMACA_Open(hdl->rom->dmaca_rx_channel);
    if (DMACA_SUCCESS == err_dmaca)
    {
        err_dmaca = R_DMACA_Control(hdl->rom->dmaca_rx_channel, DMACA_CMD_DISABLE, &stat_dmaca);
    }

    if (DMACA_SUCCESS == err_dmaca)
    {
        if (length >= hdl->rx_dflt_thresh)
        {
            if (NULL == p_dst)
            {
                p_rctrl_dma->p_rx_fraction_buf = NULL;
            }
            else
            {
                p_rctrl_dma->p_rx_fraction_buf = (uint8_t*)((uint32_t)rx_cfg_dmaca.p_des_addr + (rx_cfg_dmaca.transfer_count * rx_cfg_dmaca.block_size));
            }

            p_rctrl_dma->rx_fraction = (length - (rx_cfg_dmaca.transfer_count * rx_cfg_dmaca.block_size));
            SCI_PRV_RX_FIFO_THRESHOLD = hdl->rx_dflt_thresh;

            if (SCI_CH0 == hdl->rom->chan)
            {
                err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_rx_channel,(void *)sci0_dmac_rx_callback);
            }
            else if (SCI_CH1 == hdl->rom->chan)
            {
                err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_rx_channel,(void *)sci1_dmac_rx_callback);
            }
            else if (SCI_CH2 == hdl->rom->chan)
            {
                err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_rx_channel,(void *)sci2_dmac_rx_callback);
            }
            else if (SCI_CH3 == hdl->rom->chan)
            {
                err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_rx_channel,(void *)sci3_dmac_rx_callback);
            }
            else if (SCI_CH4 == hdl->rom->chan)
            {
                err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_rx_channel,(void *)sci4_dmac_rx_callback);
            }
            else if (SCI_CH5 == hdl->rom->chan)
            {
                err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_rx_channel,(void *)sci5_dmac_rx_callback);
            }
            else if (SCI_CH6 == hdl->rom->chan)
            {
                err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_rx_channel,(void *)sci6_dmac_rx_callback);
            }
            else if (SCI_CH7 == hdl->rom->chan)
            {
                err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_rx_channel,(void *)sci7_dmac_rx_callback);
            }
            else if (SCI_CH8 == hdl->rom->chan)
            {
                err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_rx_channel,(void *)sci8_dmac_rx_callback);
            }
            else if (SCI_CH9 == hdl->rom->chan)
            {
                err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_rx_channel,(void *)sci9_dmac_rx_callback);
            }
            else if (SCI_CH10 == hdl->rom->chan)
            {
                err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_rx_channel,(void *)sci10_dmac_rx_callback);
            }
            else if (SCI_CH11 == hdl->rom->chan)
            {
                err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_rx_channel,(void *)sci11_dmac_rx_callback);
            }
            else if (SCI_CH12 == hdl->rom->chan)
            {
                err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_rx_channel,(void *)sci12_dmac_rx_callback);
            }
            else
            {
                err_dmaca = DMACA_ERR_INTERNAL;
            }
            
            if (DMACA_SUCCESS == err_dmaca)
            {
                err_dmaca = R_DMACA_Create(hdl->rom->dmaca_rx_channel, &rx_cfg_dmaca);
            }
            if (DMACA_SUCCESS == err_dmaca)
            {
                err_dmaca = R_DMACA_Int_Enable(hdl->rom->dmaca_rx_channel, *hdl->rom->ipr_rxi);
            }
            if (DMACA_SUCCESS == err_dmaca)
            {
                err_dmaca = R_DMACA_Control(hdl->rom->dmaca_rx_channel, DMACA_CMD_ENABLE, &stat_dmaca);
            }
        }
        else
        {
            p_rctrl_dma->p_rx_fraction_buf = p_dst;
            p_rctrl_dma->rx_fraction = length;
            SCI_PRV_RX_FIFO_THRESHOLD = length;
        }
    }

    if (DMACA_SUCCESS != err_dmaca)
    {
        err_sci = SCI_ERR_DMACA;
    }

    return err_sci;
}
/**********************************************************************************************************************
 End of function sci_rxfifo_dmaca_create
 *********************************************************************************************************************/

#endif


/**********************************************************************************************************************
* Function Name: sci_tx_dmaca_create
* Description  : This function create DMAC to transmit data from p_src to SCI TDR (by DMAC without CPU)
* Arguments    : hdl -
*                    handle for channel (ptr to chan control block)
*                p_src -
*                    The address of source data need to be sent
*                lenght -
*                    The number of data need to be sent
* Return Value : SCI_SUCCESS -
*                    Create DMAC successfully
*                SCI_ERR_DMACA -
*                    Create DMAC NOT successfully
 *********************************************************************************************************************/
sci_err_t sci_tx_dmaca_create(sci_hdl_t const hdl, uint8_t *p_src, uint16_t const length)
{
        sci_fifo_ctrl_t          *p_tctrl_dma;
        dmaca_transfer_data_cfg_t tx_cfg_dmaca;
        sci_err_t     err_sci = SCI_SUCCESS;
        dmaca_return_t err_dmaca = DMACA_SUCCESS;
        dmaca_stat_t   stat_dmaca;


        p_tctrl_dma = &hdl->queue[hdl->qindex_app_tx];

        tx_cfg_dmaca.act_source           = hdl->rom->dmaca_tx_act_src;
        tx_cfg_dmaca.transfer_mode        = DMACA_TRANSFER_MODE_NORMAL;
        tx_cfg_dmaca.data_size            = DMACA_DATA_SIZE_BYTE;
        tx_cfg_dmaca.dtie_request         = DMACA_TRANSFER_END_INTERRUPT_ENABLE;
        tx_cfg_dmaca.interrupt_sel        = DMACA_CLEAR_INTERRUPT_FLAG_BEGINNING_TRANSFER;
        tx_cfg_dmaca.des_addr_mode        = DMACA_DES_ADDR_FIXED;
        tx_cfg_dmaca.p_des_addr           = (void *)&hdl->rom->regs->TDR;

        tx_cfg_dmaca.request_source       = DMACA_TRANSFER_REQUEST_PERIPHERAL;
        tx_cfg_dmaca.esie_request         = DMACA_TRANSFER_ESCAPE_END_INTERRUPT_DISABLE;
        tx_cfg_dmaca.rptie_request        = DMACA_REPEAT_SIZE_END_INTERRUPT_DISABLE;
        tx_cfg_dmaca.sarie_request        = DMACA_SRC_ADDR_EXT_REP_AREA_OVER_INTERRUPT_DISABLE;
        tx_cfg_dmaca.darie_request        = DMACA_DES_ADDR_EXT_REP_AREA_OVER_INTERRUPT_DISABLE;
        tx_cfg_dmaca.src_addr_repeat_area = DMACA_SRC_ADDR_EXT_REP_AREA_NONE;
        tx_cfg_dmaca.des_addr_repeat_area = DMACA_DES_ADDR_EXT_REP_AREA_NONE;
        tx_cfg_dmaca.offset_value         = 0;

        if (NULL == p_src)
        {
            tx_cfg_dmaca.src_addr_mode     = DMACA_SRC_ADDR_FIXED;
            tx_cfg_dmaca.p_src_addr        = (void *)&tx_dummy_buf;
        }
        else
        {
            tx_cfg_dmaca.repeat_block_side = DMACA_REPEAT_BLOCK_DISABLE;
            tx_cfg_dmaca.src_addr_mode     = DMACA_SRC_ADDR_INCR;
            tx_cfg_dmaca.p_src_addr        = (void *)p_src;
        }

        tx_cfg_dmaca.transfer_count = (uint32_t)(length);

        err_dmaca = R_DMACA_Open(hdl->rom->dmaca_tx_channel);
        if (DMACA_SUCCESS == err_dmaca)
        {
            err_dmaca = R_DMACA_Control(hdl->rom->dmaca_tx_channel, DMACA_CMD_DISABLE, &stat_dmaca);
        }

        if (DMACA_SUCCESS == err_dmaca)
        {
            if (length > 0)
            {
                if (NULL == p_src)
                {
                    p_tctrl_dma->p_tx_fraction_buf = NULL;
                }
                else
                {
                    p_tctrl_dma->p_tx_fraction_buf = (uint8_t*)((uint32_t)tx_cfg_dmaca.p_src_addr);
                }

                p_tctrl_dma->tx_fraction = 0;
                if (SCI_CH0 == hdl->rom->chan)
                {
                    err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_tx_channel,(void *)sci0_dmac_tx_callback);
                }
                else if (SCI_CH1 == hdl->rom->chan)
                {
                    err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_tx_channel,(void *)sci1_dmac_tx_callback);
                }
                else if (SCI_CH2 == hdl->rom->chan)
                {
                    err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_tx_channel,(void *)sci2_dmac_tx_callback);
                }
                else if (SCI_CH3 == hdl->rom->chan)
                {
                    err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_tx_channel,(void *)sci3_dmac_tx_callback);
                }
                else if (SCI_CH4 == hdl->rom->chan)
                {
                    err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_tx_channel,(void *)sci4_dmac_tx_callback);
                }
                else if (SCI_CH5 == hdl->rom->chan)
                {
                    err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_tx_channel,(void *)sci5_dmac_tx_callback);
                }
                else if (SCI_CH6 == hdl->rom->chan)
                {
                    err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_tx_channel,(void *)sci6_dmac_tx_callback);
                }
                else if (SCI_CH7 == hdl->rom->chan)
                {
                    err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_tx_channel,(void *)sci7_dmac_tx_callback);
                }
                else if (SCI_CH8 == hdl->rom->chan)
                {
                    err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_tx_channel,(void *)sci8_dmac_tx_callback);
                }
                else if (SCI_CH9 == hdl->rom->chan)
                {
                    err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_tx_channel,(void *)sci9_dmac_tx_callback);
                }
                else if (SCI_CH10 == hdl->rom->chan)
                {
                    err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_tx_channel,(void *)sci10_dmac_tx_callback);
                }
                else if (SCI_CH11 == hdl->rom->chan)
                {
                    err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_tx_channel,(void *)sci11_dmac_tx_callback);
                }
                else if (SCI_CH12 == hdl->rom->chan)
                {
                    err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_tx_channel,(void *)sci12_dmac_tx_callback);
                }
                else
                {
                    err_dmaca = DMACA_ERR_INTERNAL;
                }

                if (DMACA_SUCCESS == err_dmaca)
                {
                    err_dmaca = R_DMACA_Create(hdl->rom->dmaca_tx_channel, &tx_cfg_dmaca);
                }

#if defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX71M) || defined(BSP_MCU_RX65N) || defined(BSP_MCU_RX66T) || defined(BSP_MCU_RX72T) || defined(BSP_MCU_RX72M) || defined(BSP_MCU_RX72N) || defined(BSP_MCU_RX66N)
                    err_dmaca = R_DMACA_Int_Enable(hdl->rom->dmaca_tx_channel, *hdl->rom->ipr_txi);
#else
                    err_dmaca = R_DMACA_Int_Enable(hdl->rom->dmaca_tx_channel, *hdl->rom->ipr);
#endif
                if (DMACA_SUCCESS == err_dmaca)
                {
                    err_dmaca = R_DMACA_Control(hdl->rom->dmaca_tx_channel, DMACA_CMD_ENABLE, &stat_dmaca);
                }
            }
            else
            {
                
            }
        }

        if (DMACA_SUCCESS != err_dmaca)
        {
            err_sci = SCI_ERR_DMACA;
        }

        return err_sci;
}
/**********************************************************************************************************************
 End of function sci_tx_dmaca_create
 *********************************************************************************************************************/

/**********************************************************************************************************************
* Function Name: sci_rx_dmaca_create
* Description  : This function create DMAC to receive data from SCI RDR and save to p_dst (by DMAC without CPU)
* Arguments    : hdl -
*                    handle for channel (ptr to chan control block)
*                p_dst -
*                    The address of destination need to be saved
*                lenght -
*                    The number of data need to be received
* Return Value : SCI_SUCCESS -
*                    Create DMAC successfully
*                SCI_ERR_DMACA -
*                    Create DMAC NOT successfully
 *********************************************************************************************************************/
sci_err_t sci_rx_dmaca_create(sci_hdl_t const hdl, uint8_t *p_dst, uint16_t const length)
{
        sci_fifo_ctrl_t          *p_rctrl_dma;
        sci_err_t     err_sci = SCI_SUCCESS;
        dmaca_return_t err_dmaca = DMACA_SUCCESS;
        dmaca_stat_t   stat_dmaca;
        dmaca_transfer_data_cfg_t rx_cfg_dmaca;

        p_rctrl_dma = &hdl->queue[hdl->qindex_app_rx];

        rx_cfg_dmaca.act_source           = hdl->rom->dmaca_rx_act_src;
        rx_cfg_dmaca.transfer_mode        = DMACA_TRANSFER_MODE_NORMAL;
        rx_cfg_dmaca.data_size            = DMACA_DATA_SIZE_BYTE;
        rx_cfg_dmaca.dtie_request         = DMACA_TRANSFER_END_INTERRUPT_ENABLE;
        rx_cfg_dmaca.interrupt_sel        = DMACA_CLEAR_INTERRUPT_FLAG_BEGINNING_TRANSFER;
        rx_cfg_dmaca.src_addr_mode        = DMACA_SRC_ADDR_FIXED;
        rx_cfg_dmaca.p_src_addr           = (void *)&hdl->rom->regs->RDR;

        rx_cfg_dmaca.request_source       = DMACA_TRANSFER_REQUEST_PERIPHERAL;
        rx_cfg_dmaca.esie_request         = DMACA_TRANSFER_ESCAPE_END_INTERRUPT_DISABLE;
        rx_cfg_dmaca.rptie_request        = DMACA_REPEAT_SIZE_END_INTERRUPT_DISABLE;
        rx_cfg_dmaca.sarie_request        = DMACA_SRC_ADDR_EXT_REP_AREA_OVER_INTERRUPT_DISABLE;
        rx_cfg_dmaca.darie_request        = DMACA_DES_ADDR_EXT_REP_AREA_OVER_INTERRUPT_DISABLE;
        rx_cfg_dmaca.src_addr_repeat_area = DMACA_SRC_ADDR_EXT_REP_AREA_NONE;
        rx_cfg_dmaca.des_addr_repeat_area = DMACA_DES_ADDR_EXT_REP_AREA_NONE;
        rx_cfg_dmaca.offset_value         = 0;

        if (NULL == p_dst)
        {
            rx_cfg_dmaca.repeat_block_side = DMACA_REPEAT_BLOCK_DESTINATION;
            rx_cfg_dmaca.des_addr_mode     = DMACA_DES_ADDR_FIXED;
            rx_cfg_dmaca.p_des_addr        = (void *)&rx_dummy_buf;
        }
        else
        {
            rx_cfg_dmaca.repeat_block_side = DMACA_REPEAT_BLOCK_DISABLE;
            rx_cfg_dmaca.des_addr_mode     = DMACA_DES_ADDR_INCR;
            rx_cfg_dmaca.p_des_addr        = (void *)p_dst;
        }

        rx_cfg_dmaca.transfer_count     = (uint32_t)(length);

        err_dmaca = R_DMACA_Open(hdl->rom->dmaca_rx_channel);
        if (DMACA_SUCCESS == err_dmaca)
        {
            err_dmaca = R_DMACA_Control(hdl->rom->dmaca_rx_channel, DMACA_CMD_DISABLE, &stat_dmaca);
        }

        if (DMACA_SUCCESS == err_dmaca)
        {
            if (length > 0)
            {
                if (NULL == p_dst)
                {
                    p_rctrl_dma->p_rx_fraction_buf = NULL;
                }
                else
                {
                    p_rctrl_dma->p_rx_fraction_buf = (uint8_t*)((uint32_t)rx_cfg_dmaca.p_des_addr);
                }

                p_rctrl_dma->rx_fraction = 0;

                if (SCI_CH0 == hdl->rom->chan)
                {
                    err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_rx_channel,(void *)sci0_dmac_rx_callback);
                }
                else if (SCI_CH1 == hdl->rom->chan)
                {
                    err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_rx_channel,(void *)sci1_dmac_rx_callback);
                }
                else if (SCI_CH2 == hdl->rom->chan)
                {
                    err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_rx_channel,(void *)sci2_dmac_rx_callback);
                }
                else if (SCI_CH3 == hdl->rom->chan)
                {
                    err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_rx_channel,(void *)sci3_dmac_rx_callback);
                }
                else if (SCI_CH4 == hdl->rom->chan)
                {
                    err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_rx_channel,(void *)sci4_dmac_rx_callback);
                }
                else if (SCI_CH5 == hdl->rom->chan)
                {
                    err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_rx_channel,(void *)sci5_dmac_rx_callback);
                }
                else if (SCI_CH6 == hdl->rom->chan)
                {
                    err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_rx_channel,(void *)sci6_dmac_rx_callback);
                }
                else if (SCI_CH7 == hdl->rom->chan)
                {
                    err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_rx_channel,(void *)sci7_dmac_rx_callback);
                }
                else if (SCI_CH8 == hdl->rom->chan)
                {
                    err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_rx_channel,(void *)sci8_dmac_rx_callback);
                }
                else if (SCI_CH9 == hdl->rom->chan)
                {
                    err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_rx_channel,(void *)sci9_dmac_rx_callback);
                }
                else if (SCI_CH10 == hdl->rom->chan)
                {
                    err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_rx_channel,(void *)sci10_dmac_rx_callback);
                }
                else if (SCI_CH11 == hdl->rom->chan)
                {
                    err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_rx_channel,(void *)sci11_dmac_rx_callback);
                }
                else if (SCI_CH12 == hdl->rom->chan)
                {
                    err_dmaca = R_DMACA_Int_Callback(hdl->rom->dmaca_rx_channel,(void *)sci12_dmac_rx_callback);
                }
                else
                {
                    err_dmaca = DMACA_ERR_INTERNAL;
                }

                if (DMACA_SUCCESS == err_dmaca)
                {
                    err_dmaca = R_DMACA_Create(hdl->rom->dmaca_rx_channel, &rx_cfg_dmaca);
                }
                if (DMACA_SUCCESS == err_dmaca)
                {
#if defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX71M) || defined(BSP_MCU_RX65N) || defined(BSP_MCU_RX66T) || defined(BSP_MCU_RX72T) || defined(BSP_MCU_RX72M) || defined(BSP_MCU_RX72N) || defined(BSP_MCU_RX66N)
                    err_dmaca = R_DMACA_Int_Enable(hdl->rom->dmaca_rx_channel, *hdl->rom->ipr_rxi);
#else
                    err_dmaca = R_DMACA_Int_Enable(hdl->rom->dmaca_rx_channel, *hdl->rom->ipr);
#endif
                }
                if (DMACA_SUCCESS == err_dmaca)
                {
                    err_dmaca = R_DMACA_Control(hdl->rom->dmaca_rx_channel, DMACA_CMD_ENABLE, &stat_dmaca);
                }
            }
        }

        if (DMACA_SUCCESS != err_dmaca)
        {
            err_sci = SCI_ERR_DMACA;
        }

        return err_sci;
}
/**********************************************************************************************************************
 End of function sci_tx_dmaca_create
 *********************************************************************************************************************/

/**********************************************************************************************************************
* Function Name: sci0_dmac_rx_callback
* Description  : DMAC interrupt routines for SCI0 channel
* Return Value : None
 *********************************************************************************************************************/
static void sci0_dmac_rx_callback(void)
{
#if SCI_CFG_CH0_INCLUDED
    sci_dmac_rx_handler(&ch0_ctrl);
#endif
}
/**********************************************************************************************************************
 End of function sci0_dmac_rx_callback
 *********************************************************************************************************************/

/**********************************************************************************************************************
* Function Name: sci1_dmac_rx_callback
* Description  : DMAC interrupt routines for SCI1 channel
* Return Value : None
 *********************************************************************************************************************/
static void sci1_dmac_rx_callback(void)
{
#if SCI_CFG_CH1_INCLUDED
    sci_dmac_rx_handler(&ch1_ctrl);
#endif
}
/**********************************************************************************************************************
 End of function sci1_dmac_rx_callback
 *********************************************************************************************************************/

/**********************************************************************************************************************
* Function Name: sci2_dmac_rx_callback
* Description  : DMAC interrupt routines for SCI2 channel
* Return Value : None
 *********************************************************************************************************************/
static void sci2_dmac_rx_callback(void)
{
#if SCI_CFG_CH2_INCLUDED
    sci_dmac_rx_handler(&ch2_ctrl);
#endif
}
/**********************************************************************************************************************
 End of function sci2_dmac_rx_callback
 *********************************************************************************************************************/

/**********************************************************************************************************************
* Function Name: sci3_dmac_rx_callback
* Description  : DMAC interrupt routines for SCI3 channel
* Return Value : None
 *********************************************************************************************************************/
static void sci3_dmac_rx_callback(void)
{
#if SCI_CFG_CH3_INCLUDED
    sci_dmac_rx_handler(&ch3_ctrl);
#endif
}
/**********************************************************************************************************************
 End of function sci3_dmac_rx_callback
 *********************************************************************************************************************/

/**********************************************************************************************************************
* Function Name: sci4_dmac_rx_callback
* Description  : DMAC interrupt routines for SCI4 channel
* Return Value : None
 *********************************************************************************************************************/
static void sci4_dmac_rx_callback(void)
{
#if SCI_CFG_CH4_INCLUDED
    sci_dmac_rx_handler(&ch4_ctrl);
#endif
}
/**********************************************************************************************************************
 End of function sci_dmac_rx_callback
 *********************************************************************************************************************/

/**********************************************************************************************************************
* Function Name: sci5_dmac_rx_callback
* Description  : DMAC interrupt routines for SCI5 channel
* Return Value : None
 *********************************************************************************************************************/
static void sci5_dmac_rx_callback(void)
{
#if SCI_CFG_CH5_INCLUDED
    sci_dmac_rx_handler(&ch5_ctrl);
#endif
}
/**********************************************************************************************************************
 End of function sci_dmac_rx_callback
 *********************************************************************************************************************/

/**********************************************************************************************************************
* Function Name: sci6_dmac_rx_callback
* Description  : DMAC interrupt routines for SCI6 channel
* Return Value : None
 *********************************************************************************************************************/
static void sci6_dmac_rx_callback(void)
{
#if SCI_CFG_CH6_INCLUDED
    sci_dmac_rx_handler(&ch6_ctrl);
#endif
}
/**********************************************************************************************************************
 End of function sci_dmac_rx_callback
 *********************************************************************************************************************/

/**********************************************************************************************************************
* Function Name: sci7_dmac_rx_callback
* Description  : DMAC interrupt routines for SCI7 channel
* Return Value : None
 *********************************************************************************************************************/
static void sci7_dmac_rx_callback(void)
{
#if SCI_CFG_CH7_INCLUDED
    sci_dmac_rx_handler(&ch7_ctrl);
#endif
}
/**********************************************************************************************************************
 End of function sci_dmac_rx_callback
 *********************************************************************************************************************/

/**********************************************************************************************************************
* Function Name: sci8_dmac_rx_callback
* Description  : DMAC interrupt routines for SCI8 channel
* Return Value : None
 *********************************************************************************************************************/
static void sci8_dmac_rx_callback(void)
{
#if SCI_CFG_CH8_INCLUDED
    sci_dmac_rx_handler(&ch8_ctrl);
#endif
}
/**********************************************************************************************************************
 End of function sci8_dmac_rx_callback
 *********************************************************************************************************************/

/**********************************************************************************************************************
* Function Name: sci9_dmac_rx_callback
* Description  : DMAC interrupt routines for SCI9 channel
* Return Value : None
 *********************************************************************************************************************/
static void sci9_dmac_rx_callback(void)
{
#if SCI_CFG_CH9_INCLUDED
    sci_dmac_rx_handler(&ch9_ctrl);
#endif
}
/**********************************************************************************************************************
 End of function sci9_dmac_rx_callback
 *********************************************************************************************************************/

/**********************************************************************************************************************
* Function Name: sci10_dmac_rx_callback
* Description  : DMAC interrupt routines for SCI10 channel
* Return Value : None
 *********************************************************************************************************************/
static void sci10_dmac_rx_callback(void)
{
#if SCI_CFG_CH10_INCLUDED
    sci_dmac_rx_handler(&ch10_ctrl);
#endif
}
/**********************************************************************************************************************
 End of function sci10_dmac_rx_callback
 *********************************************************************************************************************/

/**********************************************************************************************************************
* Function Name: sci11_dmac_rx_callback
* Description  : DMAC interrupt routines for SCI11 channel
* Return Value : None
 *********************************************************************************************************************/
static void sci11_dmac_rx_callback(void)
{
#if SCI_CFG_CH11_INCLUDED
    sci_dmac_rx_handler(&ch11_ctrl);
#endif
}
/**********************************************************************************************************************
 End of function sci11_dmac_rx_callback
 *********************************************************************************************************************/

/**********************************************************************************************************************
* Function Name: sci12_dmac_rx_callback
* Description  : DMAC interrupt routines for SCI12 channel
* Return Value : None
 *********************************************************************************************************************/
static void sci12_dmac_rx_callback(void)
{
#if SCI_CFG_CH12_INCLUDED
    sci_dmac_rx_handler(&ch12_ctrl);
#endif
}
/**********************************************************************************************************************
 End of function sci12_dmac_rx_callback
 *********************************************************************************************************************/

/**********************************************************************************************************************
* Function Name: sci_dmac_rx_handler
* Description  : This function is called to close and disable interrupt for DMAC channel, 
                 when SCI receive data completely by DMAC or remain fraction data need to be receive by CPU.
                 If remain fraction data need to be receive by CPU (in FIFO mode), 
                 this function set threshold of RX FIFO to number of fraction data.
* Arguments    : hdl -
*                    handle for channel (ptr to chan control block)
* Return Value : None
 *********************************************************************************************************************/
static void sci_dmac_rx_handler(sci_hdl_t const hdl)
{
    volatile sci_fifo_ctrl_t  *p_ctrl;

    sci_cb_args_t              args;


#if (RX_DTC_DMACA_ENABLE & 0x02)
    if (SCI_DMACA_ENABLE == hdl->rom->dtc_dmaca_rx_enable)
    {
        if(4 == hdl->rom->dmaca_rx_channel || 5 == hdl->rom->dmaca_rx_channel || 6 == hdl->rom->dmaca_rx_channel || 7 == hdl->rom->dmaca_rx_channel)
        {
            dmaca_stat_t   stat_dmaca;
            R_DMACA_Control(hdl->rom->dmaca_rx_channel, DMACA_CMD_DTIF_STATUS_CLR, &stat_dmaca);
        }

        R_DMACA_Int_Disable(hdl->rom->dmaca_rx_channel);
        R_DMACA_Close(hdl->rom->dmaca_rx_channel);
    }
#endif
    p_ctrl = &hdl->queue[hdl->qindex_int_rx];

    if (0 == p_ctrl->rx_fraction)
    {
#if (SCI_CFG_SSPI_INCLUDED || SCI_CFG_SYNC_INCLUDED)
        if ((SCI_MODE_SYNC == hdl->mode) || (SCI_MODE_SSPI == hdl->mode))
        {
            hdl->tx_idle = true;
        }
#endif
        p_ctrl->rx_cnt = p_ctrl->rx_fraction;

        /* If at threshold level, clear bit so can get another RXIF interrupt.
         * Do not re-arm if Receive() request not outstanding (Async)
         */
#if SCI_CFG_FIFO_INCLUDED
        if (true == hdl->fifo_ctrl)
        {
            if ((1 == hdl->rom->regs->SSRFIFO.BIT.RDF) && (0 != p_ctrl->rx_cnt))
            {
                hdl->rom->regs->SSRFIFO.BIT.RDF = 0;
            }
        }
#endif
#if (RX_DTC_DMACA_ENABLE & 0x02)
        if (SCI_DMACA_ENABLE == hdl->rom->dtc_dmaca_rx_enable)
        {

            /* Do callback if available */
            if ((NULL != hdl->callback) && (FIT_NO_FUNC != hdl->callback))
            {
                args.hdl = hdl;
#if (SCI_CFG_SSPI_INCLUDED || SCI_CFG_SYNC_INCLUDED)
                if (SCI_MODE_SYNC == hdl->mode || SCI_MODE_SSPI == hdl->mode)
                {
                    args.event = SCI_EVT_RX_SYNC_DONE;
                }
                else
#endif
                {
                    args.event = SCI_EVT_RX_DONE;
                }

                /* Casting to void type is valid */
                hdl->callback((void *)&args);
            }
        }
#endif
    }
    else
    {
#if SCI_CFG_FIFO_INCLUDED
        if (true == hdl->fifo_ctrl)
        {
            SCI_PRV_RX_FIFO_THRESHOLD = p_ctrl->rx_fraction;
        }
#endif
    }
}
/**********************************************************************************************************************
 End of function sci_dmac_rx_handler
 *********************************************************************************************************************/

/**********************************************************************************************************************
* Function Name: sci0_dmac_tx_callback
* Description  : DMAC interrupt routines for SCI0 channel
* Return Value : None
 *********************************************************************************************************************/
static void sci0_dmac_tx_callback(void)
{
#if SCI_CFG_CH0_INCLUDED
    sci_dmac_tx_handler(&ch0_ctrl);
#endif
}
/**********************************************************************************************************************
 End of function sci0_dmac_tx_callback
 *********************************************************************************************************************/

/**********************************************************************************************************************
* Function Name: sci1_dmac_tx_callback
* Description  : DMAC interrupt routines for SCI1 channel
* Return Value : None
 *********************************************************************************************************************/
static void sci1_dmac_tx_callback(void)
{
#if SCI_CFG_CH1_INCLUDED
    sci_dmac_tx_handler(&ch1_ctrl);
#endif
}
/**********************************************************************************************************************
 End of function sci1_dmac_tx_callback
 *********************************************************************************************************************/

/**********************************************************************************************************************
* Function Name: sci2_dmac_tx_callback
* Description  : DMAC interrupt routines for SCI2 channel
* Return Value : None
 *********************************************************************************************************************/
static void sci2_dmac_tx_callback(void)
{
#if SCI_CFG_CH2_INCLUDED
    sci_dmac_tx_handler(&ch2_ctrl);
#endif
}
/**********************************************************************************************************************
 End of function sci2_dmac_tx_callback
 *********************************************************************************************************************/

/**********************************************************************************************************************
* Function Name: sci3_dmac_tx_callback
* Description  : DMAC interrupt routines for SCI3 channel
* Return Value : None
 *********************************************************************************************************************/
static void sci3_dmac_tx_callback(void)
{
#if SCI_CFG_CH3_INCLUDED
    sci_dmac_tx_handler(&ch3_ctrl);
#endif
}
/**********************************************************************************************************************
 End of function sci3_dmac_tx_callback
 *********************************************************************************************************************/

/**********************************************************************************************************************
* Function Name: sci4_dmac_tx_callback
* Description  : DMAC interrupt routines for SCI4 channel
* Return Value : None
 *********************************************************************************************************************/
static void sci4_dmac_tx_callback(void)
{
#if SCI_CFG_CH4_INCLUDED
    sci_dmac_tx_handler(&ch4_ctrl);
#endif
}
/**********************************************************************************************************************
 End of function sci_dmac_tx_callback
 *********************************************************************************************************************/

/**********************************************************************************************************************
* Function Name: sci5_dmac_tx_callback
* Description  : DMAC interrupt routines for SCI5 channel
* Return Value : None
 *********************************************************************************************************************/
static void sci5_dmac_tx_callback(void)
{
#if SCI_CFG_CH5_INCLUDED
    sci_dmac_tx_handler(&ch5_ctrl);
#endif
}
/**********************************************************************************************************************
 End of function sci_dmac_tx_callback
 *********************************************************************************************************************/

/**********************************************************************************************************************
* Function Name: sci6_dmac_tx_callback
* Description  : DMAC interrupt routines for SCI6 channel
* Return Value : None
 *********************************************************************************************************************/
static void sci6_dmac_tx_callback(void)
{
#if SCI_CFG_CH6_INCLUDED
    sci_dmac_tx_handler(&ch6_ctrl);
#endif
}
/**********************************************************************************************************************
 End of function sci_dmac_tx_callback
 *********************************************************************************************************************/

/**********************************************************************************************************************
* Function Name: sci7_dmac_tx_callback
* Description  : DMAC interrupt routines for SCI7 channel
* Return Value : None
 *********************************************************************************************************************/
static void sci7_dmac_tx_callback(void)
{
#if SCI_CFG_CH7_INCLUDED
    sci_dmac_tx_handler(&ch7_ctrl);
#endif
}
/**********************************************************************************************************************
 End of function sci_dmac_tx_callback
 *********************************************************************************************************************/

/**********************************************************************************************************************
* Function Name: sci8_dmac_tx_callback
* Description  : DMAC interrupt routines for SCI8 channel
* Return Value : None
 *********************************************************************************************************************/
static void sci8_dmac_tx_callback(void)
{
#if SCI_CFG_CH8_INCLUDED
    sci_dmac_tx_handler(&ch8_ctrl);
#endif
}
/**********************************************************************************************************************
 End of function sci8_dmac_tx_callback
 *********************************************************************************************************************/

/**********************************************************************************************************************
* Function Name: sci9_dmac_tx_callback
* Description  : DMAC interrupt routines for SCI9 channel
* Return Value : None
 *********************************************************************************************************************/
static void sci9_dmac_tx_callback(void)
{
#if SCI_CFG_CH9_INCLUDED
    sci_dmac_tx_handler(&ch9_ctrl);
#endif
}
/**********************************************************************************************************************
 End of function sci9_dmac_tx_callback
 *********************************************************************************************************************/

/**********************************************************************************************************************
* Function Name: sci10_dmac_tx_callback
* Description  : DMAC interrupt routines for SCI10 channel
* Return Value : None
 *********************************************************************************************************************/
static void sci10_dmac_tx_callback(void)
{
#if SCI_CFG_CH10_INCLUDED
    sci_dmac_tx_handler(&ch10_ctrl);
#endif
}
/**********************************************************************************************************************
 End of function sci10_dmac_tx_callback
 *********************************************************************************************************************/

/**********************************************************************************************************************
* Function Name: sci11_dmac_tx_callback
* Description  : DMAC interrupt routines for SCI11 channel
* Return Value : None
 *********************************************************************************************************************/
static void sci11_dmac_tx_callback(void)
{
#if SCI_CFG_CH11_INCLUDED
    sci_dmac_tx_handler(&ch11_ctrl);
#endif
}
/**********************************************************************************************************************
 End of function sci11_dmac_tx_callback
 *********************************************************************************************************************/

/**********************************************************************************************************************
* Function Name: sci12_dmac_tx_callback
* Description  : DMAC interrupt routines for SCI12 channel
* Return Value : None
 *********************************************************************************************************************/
static void sci12_dmac_tx_callback(void)
{
#if SCI_CFG_CH12_INCLUDED
    sci_dmac_tx_handler(&ch12_ctrl);
#endif
}
/**********************************************************************************************************************
 End of function sci12_dmac_tx_callback
 *********************************************************************************************************************/

/**********************************************************************************************************************
* Function Name: sci_dmac_tx_handler
* Description  : This function is called to close and disable interrupt for DMAC channel,
                 when SCI transmit data completely by DMAC.
* Arguments    : hdl -
*                    handle for channel (ptr to chan control block)
* Return Value : None
 *********************************************************************************************************************/
static void sci_dmac_tx_handler(sci_hdl_t const hdl)
{
#if (RX_DTC_DMACA_ENABLE & 0x02)
    if (SCI_DMACA_ENABLE == hdl->rom->dtc_dmaca_rx_enable)
    {
        if(4 == hdl->rom->dmaca_tx_channel || 5 == hdl->rom->dmaca_tx_channel || 6 == hdl->rom->dmaca_tx_channel || 7 == hdl->rom->dmaca_tx_channel)
        {
            dmaca_stat_t   stat_dmaca;
            R_DMACA_Control(hdl->rom->dmaca_tx_channel, DMACA_CMD_DTIF_STATUS_CLR, &stat_dmaca);
        }
    }
#endif
}
/**********************************************************************************************************************
 End of function sci_dmac_tx_handler
*/
#endif /* ((TX_DTC_DMACA_ENABLE & 0x02) || (RX_DTC_DMACA_ENABLE & 0x02)) */
