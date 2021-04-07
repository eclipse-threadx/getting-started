/*
 * isl29035_sensor.c
 *
 *  Created on: Jan 3, 2018
 *      Author: Rajkumar.Thiagarajan
 */
#include "isl29035_sensor.h"


/*!
 * @brief This internal API is used to validate the device structure pointer for
 * null conditions.
 */
static int8_t null_ptr_check(const struct isl29035_dev *dev)
{
    int8_t rslt;

    if ((dev == NULL) || (dev->read == NULL) || (dev->write == NULL) || (dev->delay_ms == NULL)) {
        rslt = ISL29035_E_NULL_PTR;
    } else {
        /* Device structure is fine */
        rslt = ISL29035_OK;
    }

    return rslt;
}

static int8_t isl29035_get_regs(uint8_t reg_addr, uint8_t *data, uint16_t len, const struct isl29035_dev *dev)
{
    int8_t rslt = ISL29035_OK;

    /* Null-pointer check */
    if ((dev == NULL) || (dev->read == NULL)) {
        rslt = ISL29035_E_NULL_PTR;
    } else {
        rslt = dev->read(dev->id,reg_addr, data, len);
        if (rslt != ISL29035_OK)
            rslt = ISL29035_E_COM_FAIL;
    }

    return rslt;
}

/*!
 * @brief This API writes the given data to the register address
 * of sensor.
 */
static int8_t isl29035_set_regs(uint8_t reg_addr, uint8_t *data, uint16_t len, const struct isl29035_dev *dev)
{
    int8_t rslt = ISL29035_OK;

    /* Null-pointer check */
    if ((dev == NULL) || (dev->write == NULL)) {
        rslt = ISL29035_E_NULL_PTR;
    } else {
        rslt = dev->write(dev->id,reg_addr, data, len);
        if (rslt != ISL29035_OK)
            rslt = ISL29035_E_COM_FAIL;
    }

    return rslt;
}

static int8_t isl29035_set_range(struct isl29035_dev *dev)
{
    int8_t rslt;
    uint8_t val;

    rslt = isl29035_get_regs(ISL29035_COMMAND_II_REG, &val, 1, dev);
    if(rslt != ISL29035_OK)
        rslt = ISL29035_E_COM_FAIL;

    val = (uint8_t)((val & ISL29035_LUX_RANGE_MASK) | dev->adc_range);

    rslt = isl29035_set_regs(ISL29035_COMMAND_II_REG, &val, 1, dev);
    if(rslt != ISL29035_OK)
        rslt = ISL29035_E_COM_FAIL;

    return rslt;
}

static int8_t isl29035_set_mode(struct isl29035_dev *dev)
{
    int8_t rslt;
    uint8_t val;

    rslt = isl29035_get_regs(ISL29035_COMMAND_I_REG, &val, 1, dev);
    if(rslt != ISL29035_OK)
        rslt = ISL29035_E_COM_FAIL;

    val = (uint8_t)((val & ISL29035_OPMODE_MASK) | dev->adc_mode);

    rslt = isl29035_set_regs(ISL29035_COMMAND_I_REG, &val, 1, dev);
    if(rslt != ISL29035_OK)
        rslt = ISL29035_E_COM_FAIL;

    return rslt;
}

static int8_t isl29035_set_res(struct isl29035_dev *dev)
{
    int8_t rslt;
    uint8_t val;

    rslt = isl29035_get_regs(ISL29035_COMMAND_II_REG, &val, 1, dev);
    if(rslt != ISL29035_OK)
        rslt = ISL29035_E_COM_FAIL;

    val = (uint8_t)((val & ISL29035_ADC_RES_MASK) | dev->adc_res);

    rslt = isl29035_set_regs(ISL29035_COMMAND_II_REG, &val, 1, dev);
    if(rslt != ISL29035_OK)
        rslt = ISL29035_E_COM_FAIL;

    return rslt;
}

int8_t isl29035_init(struct isl29035_dev *dev)
{
    int8_t rslt;
    uint8_t chip_id;
    uint8_t val;

    /* Null-pointer check */
    rslt = null_ptr_check(dev);

    if (rslt == ISL29035_OK) {
        /* Read the Chip id */
        rslt = isl29035_get_regs(ISL29035_REG_DEVICE_ID, &chip_id, 1, dev);

        if (rslt == ISL29035_OK)
        {
            /* Clear the blownout bit */
            val = (uint8_t)~ISL29035_BOUT_MASK;
            rslt = isl29035_set_regs(ISL29035_REG_DEVICE_ID, &val, 1, dev);
        }
        else
            return rslt;
    }

    return rslt;
}

int8_t isl29035_configure(struct isl29035_dev *dev)
{
    int8_t rslt;
    uint8_t val;

    /* set command registers to set default attributes */
    val = 0;
    rslt = isl29035_set_regs(ISL29035_COMMAND_I_REG, &val, 1, dev);
    if(rslt != ISL29035_OK)
        return rslt;

    rslt = isl29035_set_regs(ISL29035_COMMAND_II_REG, &val, 1, dev);
    if(rslt != ISL29035_OK)
        return rslt;

    /* Set operation mode */
    dev->adc_mode = ISL29035_ACTIVE_OPMODE_BITS;
    rslt = isl29035_set_mode(dev);
    if(rslt != ISL29035_OK)
        return rslt;

    /* Set the lux range */
    dev->adc_range = ISL29035_LUX_RANGE_BITS;
    rslt = isl29035_set_range(dev);
    if(rslt != ISL29035_OK)
        return rslt;

    /* Set ADC resolution */
    dev->adc_res = ISL29035_ADC_RES_BITS;
    rslt = isl29035_set_res(dev);
    if(rslt != ISL29035_OK)
        return rslt;

    return rslt;
}

static float compute_scaling(struct isl29035_dev *dev)
{
    int8_t rslt = ISL29035_OK;
    uint8_t val, reg_val;
    float scale;
    uint8_t adc_res = 0, lux;
    uint16_t lux_range = 0;

    /*
     * Equation is lux = (Range/2 ^ n) * raw_data
     */

    rslt = isl29035_get_regs(ISL29035_COMMAND_II_REG, &reg_val, 1, dev);
    if(rslt != ISL29035_OK)
        return rslt;

    //get the ADC range value
    val = (uint8_t)((reg_val & ISL29035_ADC_RES_MASK) >> 2);
    switch(val)
    {
        case 0: adc_res = 16; break;
        case 1: adc_res = 12; break;
        case 2: adc_res = 8;  break;
        case 3: adc_res = 4;  break;
        default: break;
    }

    //get the ADC resolution value
    lux = (uint8_t)(reg_val & ISL29035_LUX_RANGE_MASK);
    switch(lux)
    {
        case ISL29035_LUX_RANGE_1000: lux_range = 1000; break;
        case ISL29035_LUX_RANGE_4000: lux_range = 4000; break;
        case ISL29035_LUX_RANGE_16000: lux_range = 16000; break;
        case ISL29035_LUX_RANGE_64000: lux_range = 64000; break;
        default: break;
    }

//    scale = (float)(lux_range / (pow(2,adc_res)));
    scale = (lux_range / (float)(1 << adc_res));
    return scale;
}

int8_t isl29035_read_als_data(struct isl29035_dev *dev, double *als_val)
{
    int8_t rslt = ISL29035_OK;
    uint8_t als_data[2];
    uint16_t als_raw;
    float scale = 0.0;

    rslt = isl29035_get_regs(ISL29035_DATA_LSB_REG, &als_data[0], 2, dev);
    if(rslt != ISL29035_OK)
        return rslt;

    als_raw = (uint16_t)((als_data[1] << 8) | als_data[0]);

    //compute scaling factor
    scale = compute_scaling(dev);

    *als_val = (als_raw * scale);

    return rslt;
}
