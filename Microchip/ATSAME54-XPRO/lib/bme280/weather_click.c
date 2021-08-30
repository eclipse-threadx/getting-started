/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "weather_click.h"

#include "atmel_start.h"

#include "hal_i2c_m_sync.h"

static struct bme280_dev bme280;

static int8_t bme_i2c_read(uint8_t reg_addr, uint8_t* reg_data, uint32_t len, void* intf_ptr)
{
    uint8_t buffer[2];

    buffer[0] = reg_addr;
    buffer[1] = 0;
    i2c_m_sync_set_slaveaddr(&I2C_AT24MAC, BME280_I2C_ADDR_PRIM, I2C_M_SEVEN);
    io_write(&I2C_AT24MAC.io, buffer, 1);
    delay_ms(100);
    io_read(&I2C_AT24MAC.io, reg_data, len);

    return 0;
}

static int8_t bme_i2c_write(uint8_t reg_addr, const uint8_t* reg_data, uint32_t len, void* intf_ptr)
{
    uint8_t buffer[2];

    buffer[0] = reg_addr;
    buffer[1] = *reg_data;
    i2c_m_sync_set_slaveaddr(&I2C_AT24MAC, BME280_I2C_ADDR_PRIM, I2C_M_SEVEN);
    io_write(&I2C_AT24MAC.io, buffer, 2);

    return 0;
}

static void bme_delay_us(uint32_t period, void* intf_ptr)
{
    delay_ms(period / 1000);
}

uint8_t init_weather_click(void)
{
    uint8_t dev_addr = BME280_I2C_ADDR_PRIM;

    i2c_m_sync_enable(&I2C_AT24MAC);

    bme280.intf_ptr = &dev_addr;
    bme280.intf     = BME280_I2C_INTF;
    bme280.read     = bme_i2c_read;
    bme280.write    = bme_i2c_write;
    bme280.delay_us = bme_delay_us;

    return bme280_init(&bme280);
}

int8_t read_bme280(struct bme280_data* data)
{
    int8_t rslt;
    uint8_t settings_sel;
    uint32_t req_delay;
//    struct bme280_data comp_data;

    // Recommended mode of operation: Indoor navigation
    bme280.settings.osr_h  = BME280_OVERSAMPLING_1X;
    bme280.settings.osr_p  = BME280_OVERSAMPLING_16X;
    bme280.settings.osr_t  = BME280_OVERSAMPLING_2X;
    bme280.settings.filter = BME280_FILTER_COEFF_16;

    settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;

    rslt = bme280_set_sensor_settings(settings_sel, &bme280);

    // Calculate the minimum delay required between consecutive measurement based upon the sensor enabled
    // and the oversampling configuration.
    req_delay = bme280_cal_meas_delay(&bme280.settings);

    rslt = bme280_set_sensor_mode(BME280_FORCED_MODE, &bme280);
    // Wait for the measurement to complete and print data @25Hz
    bme280.delay_us(req_delay * 1000, bme280.intf_ptr);
    rslt = bme280_get_sensor_data(BME280_ALL, data, &bme280);

    return rslt;
}
