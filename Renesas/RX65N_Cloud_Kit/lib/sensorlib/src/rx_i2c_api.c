/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "rx_i2c_api.h"

#include "platform.h"
#include "r_bsp_common.h"
#include "r_sci_iic_rx_if.h"

static void sensor_callback(void)
{
}

int8_t rx_i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t* reg_data, uint16_t len)
{
    uint8_t err;
    sci_iic_info_t iic_info;

    iic_info.p_slv_adr    = &dev_addr;
    iic_info.p_data1st    = &reg_addr;
    iic_info.p_data2nd    = reg_data;
    iic_info.dev_sts      = SCI_IIC_NO_INIT;
    iic_info.ch_no        = 2;
    iic_info.cnt1st       = 1;
    iic_info.cnt2nd       = len;
    iic_info.callbackfunc = &sensor_callback;

    err = R_SCI_IIC_MasterReceive(&iic_info);
    if (SCI_IIC_SUCCESS == err)
    {
        while (SCI_IIC_FINISH != iic_info.dev_sts)
        {
        }
    }

    return err;
}

int8_t rx_i2c_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t* reg_data, uint16_t len)
{
    sci_iic_return_t ret;
    sci_iic_info_t iic_info;

    iic_info.p_slv_adr    = &dev_addr;
    iic_info.p_data1st    = &reg_addr;
    iic_info.p_data2nd    = reg_data;
    iic_info.dev_sts      = SCI_IIC_NO_INIT;
    iic_info.ch_no        = 2;
    iic_info.cnt1st       = 1;
    iic_info.cnt2nd       = len;
    iic_info.callbackfunc = &sensor_callback;

    ret = R_SCI_IIC_MasterSend(&iic_info);
    if (SCI_IIC_SUCCESS == ret)
    {
        while (SCI_IIC_FINISH != iic_info.dev_sts)
        {
        }
    }

    return ret;
}

void rx_delay_ms(uint32_t period)
{
    R_BSP_SoftwareDelay(period, BSP_DELAY_MILLISECS);
}
