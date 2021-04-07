/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

// https://github.com/BoschSensortec/BME68x-Sensor-API
// https://github.com/BoschSensortec/BMI160_driver

#include "rx65n_cloud_kit_sensors.h"

#include <string.h>

#include "platform.h"
#include "r_sci_iic_rx_if.h"

#include "rx_i2c_api.h"

static struct bme68x_dev bme680;
static struct bmi160_dev bmi160;
static struct isl29035_dev isl_dev;

static uint8_t bme680_dev_addr;

static int8_t bme_i2c_read(uint8_t reg_addr, uint8_t* reg_data, uint32_t len, void* intf_ptr)
{
    uint8_t dev_addr = *(uint8_t*)intf_ptr;    
    return rx_i2c_read(dev_addr, reg_addr, reg_data, len);
}

static int8_t bme_i2c_write(uint8_t reg_addr, uint8_t* reg_data, uint32_t len, void* intf_ptr)
{
    uint8_t dev_addr = *(uint8_t*)intf_ptr;    
    return rx_i2c_write(dev_addr, reg_addr, reg_data, len);
}

static void bme_delay_us(uint32_t period, void* intf_ptr)
{
    rx_delay_ms(period / 1000);
}

static int8_t bme68x_initialize(void)
{
    int8_t rslt;

    // Initialize
    bme680_dev_addr = BME68X_I2C_ADDR_LOW;
    bme680.read     = bme_i2c_read;
    bme680.write    = bme_i2c_write;
    bme680.delay_us = bme_delay_us;
    bme680.intf     = BME68X_I2C_INTF;
    bme680.intf_ptr = &bme680_dev_addr;
    rslt            = bme68x_init(&bme680);
    if (rslt != BME68X_OK)
    {
        return rslt;
    }

    struct bme68x_conf conf;
    conf.filter  = BME68X_FILTER_OFF;
    conf.odr     = BME68X_ODR_NONE;
    conf.os_hum  = BME68X_OS_16X;
    conf.os_pres = BME68X_OS_1X;
    conf.os_temp = BME68X_OS_2X;
    rslt         = bme68x_set_conf(&conf, &bme680);
    if (rslt != BME68X_OK)
    {
        return rslt;
    }

    // Heater config
    struct bme68x_heatr_conf heatr_conf;
    heatr_conf.enable     = BME68X_ENABLE;
    heatr_conf.heatr_temp = 300;
    heatr_conf.heatr_dur  = 100;
    rslt                  = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &bme680);
    if (rslt != BME68X_OK)
    {
        return rslt;
    }

    return rslt;
}

static int8_t bmi160_initialize(void)
{
    int8_t rslt;

    // Initialize
    bmi160.id        = BMI160_I2C_ADDR;
    bmi160.interface = BMI160_I2C_INTF;
    bmi160.read      = rx_i2c_read;
    bmi160.write     = rx_i2c_write;
    bmi160.delay_ms  = rx_delay_ms;

    rslt = bmi160_init(&bmi160);
    if (rslt != BMI160_OK)
    {
        return rslt;
    }

    // Select the Output data rate, range of accelerometer sensor
    bmi160.accel_cfg.odr   = BMI160_ACCEL_ODR_1600HZ;
    bmi160.accel_cfg.range = BMI160_ACCEL_RANGE_4G;
    bmi160.accel_cfg.bw    = BMI160_ACCEL_BW_NORMAL_AVG4;

    // Select the power mode of accelerometer sensor
    bmi160.accel_cfg.power = BMI160_ACCEL_NORMAL_MODE;

    // Select the Output data rate, range of Gyroscope sensor
    bmi160.gyro_cfg.odr   = BMI160_GYRO_ODR_3200HZ;
    bmi160.gyro_cfg.range = BMI160_GYRO_RANGE_2000_DPS;
    bmi160.gyro_cfg.bw    = BMI160_GYRO_BW_NORMAL_MODE;

    // Select the power mode of Gyroscope sensor
    bmi160.gyro_cfg.power = BMI160_GYRO_NORMAL_MODE;

    // Set the sensor configuration
    rslt = bmi160_set_sens_conf(&bmi160);

    return rslt;
}

static int8_t isl29035_initialize(void)
{
    int8_t status = 0;

    /* ISL29035 Sensor Initialization */
    isl_dev.id        = ISL29035_I2C_ADDR;
    isl_dev.interface = ISL29035_I2C_INTF;
    isl_dev.read      = rx_i2c_read;
    isl_dev.write     = rx_i2c_write;
    isl_dev.delay_ms  = rx_delay_ms;

    status = isl29035_init(&isl_dev);
    if (status != ISL29035_OK)
    {
        return status;
    }
    /* Configure ISL29035 ALS Sensor */
    status = isl29035_configure(&isl_dev);
    if (status != ISL29035_OK)
    {
        return status;
    }

    return status;
}

uint8_t init_sensors(void)
{
    sci_iic_return_t ret;
    sci_iic_info_t iic_info;

    // Open I2C driver instance
    iic_info.dev_sts = SCI_IIC_NO_INIT;
    iic_info.ch_no   = 2;

    ret = R_SCI_IIC_Open(&iic_info);
    if (ret != SCI_IIC_SUCCESS)
    {
        return ret;
    }

    ret = bme68x_initialize();
    if (ret != BME68X_OK)
    {
        return ret;
    }

    ret = bmi160_initialize();
    if (ret != BMI160_OK)
    {
        return ret;
    }

    ret = isl29035_initialize();
    if (ret != ISL29035_OK)
    {
        return ret;
    }

    return ret;
}

int8_t read_bme680(struct bme68x_data* data)
{
    int8_t rslt = 0;
    uint8_t n_fields;

    memset(data, 0, sizeof(*data));

    rslt = bme68x_set_op_mode(BME68X_FORCED_MODE, &bme680);
    if (BME68X_OK != rslt)
    {
        return rslt;
    }

    rslt = bme68x_get_data(BME68X_FORCED_MODE, data, &n_fields, &bme680);
    if (BME68X_OK != rslt)
    {
        return rslt;
    }

    return rslt;
}

int8_t read_bmi160_accel(struct bmi160_sensor_data* data)
{
    int8_t rslt;
    struct bmi160_sensor_data accel;

    memset(data, 0, sizeof(*data));

    // To read only Accel data
    rslt = bmi160_get_sensor_data(BMI160_ACCEL_SEL, data, NULL, &bmi160);
    return rslt;
}

int8_t read_bmi160_gyro(struct bmi160_sensor_data* data)
{
    int8_t rslt;
    struct bmi160_sensor_data accel;

    memset(data, 0, sizeof(*data));

    // To read only Accel data
    rslt = bmi160_get_sensor_data(BMI160_GYRO_SEL, NULL, data, &bmi160);
    return rslt;
}

int8_t read_isl29035(double* als)
{
    return isl29035_read_als_data(&isl_dev, als);
}
