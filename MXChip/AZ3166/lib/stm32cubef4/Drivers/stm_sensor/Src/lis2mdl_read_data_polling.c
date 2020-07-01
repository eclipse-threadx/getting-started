/*
 ******************************************************************************
 * @file    read_data_simple.c
 * @author  Sensors Software Solution Team
 * @brief   This file show the simplest way to get data from sensor.
 *
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

/*
 * This example was developed using the following STMicroelectronics
 * evaluation boards:
 *
 * - STEVAL_MKI109V3 + STEVAL-MKI181V1
 * - NUCLEO_F411RE + X_NUCLEO_IKS01A3
 *
 * and STM32CubeMX tool with STM32CubeF4 MCU Package
 *
 * Used interfaces:
 *
 * STEVAL_MKI109V3    - Host side:   USB (Virtual COM)
 *                    - Sensor side: SPI(Default) / I2C(supported)
 *
 * NUCLEO_STM32F411RE - Host side: UART(COM) to USB bridge
 *                    - I2C(Default) / SPI(supported)
 *
 * If you need to run this example on a different hardware platform a
 * modification of the functions: `platform_write`, `platform_read`,
 * `tx_com` and 'platform_init' is required.
 *
 */

/* STMicroelectronics evaluation boards definition
 *
 * Please uncomment ONLY the evaluation boards in use.
 * If a different hardware is used please comment all
 * following target board and redefine yours.
 */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "lis2mdl_reg.h"
#include "sensor.h"


extern I2C_HandleTypeDef I2cHandle;
#define hi2c1 I2cHandle

typedef union{
  int16_t i16bit[3];
  uint8_t u8bit[6];
} axis3bit16_t;

typedef union{
  int16_t i16bit;
  uint8_t u8bit[2];
} axis1bit16_t;

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static axis3bit16_t data_raw_magnetic;
static axis1bit16_t data_raw_temperature;
static uint8_t whoamI, rst;

/* Extern variables ----------------------------------------------------------*/
#define    BOOT_TIME        20 //ms

/* Private functions ---------------------------------------------------------*/
/*
 *   WARNING:
 *   Functions declare in this section are defined at the end of this file
 *   and are strictly related to the hardware platform used.
 *
 */
static int32_t platform_write(void *handle, uint8_t reg, uint8_t *bufp,
                              uint16_t len);
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len);
// static void platform_delay(uint32_t ms);

static stmdev_ctx_t dev_ctx =
{
    platform_write,
    platform_read,
    &hi2c1,
};

static uint32_t timeout = 5;
/* Main Example --------------------------------------------------------------*/
Sensor_StatusTypeDef lis2mdl_config(void)
{
  Sensor_StatusTypeDef ret = SENSOR_OK;

  /* Wait sensor boot time */
  // platform_delay(BOOT_TIME);
  whoamI =0;

  /* Check device ID */
  lis2mdl_device_id_get(&dev_ctx, &whoamI);
  if(whoamI != LIS2MDL_ID)
  {
    ret = SENSOR_ERROR;
  }
  else
  {

    timeout = 5;
    /* Restore default configuration */
    lis2mdl_reset_set(&dev_ctx, PROPERTY_ENABLE);
  do {
    lis2mdl_reset_get(&dev_ctx, &rst);
    //printf(" Reset Error Magnetometer Sensor\r\n");
  } while (rst);

  /* Enable Block Data Update */
  lis2mdl_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);

  /* Set Output Data Rate */
  lis2mdl_data_rate_set(&dev_ctx, LIS2MDL_ODR_10Hz);

  /* Set / Reset sensor mode */
  lis2mdl_set_rst_mode_set(&dev_ctx, LIS2MDL_SENS_OFF_CANC_EVERY_ODR);

  /* Enable temperature compensation */
  lis2mdl_offset_temp_comp_set(&dev_ctx, PROPERTY_ENABLE);

  /* Set device in continuous mode */
  lis2mdl_operating_mode_set(&dev_ctx, LIS2MDL_CONTINUOUS_MODE);
  }
  return ret;
}
lis2mdl_data_t lis2mdl_data_read(void)
 {
   lis2mdl_data_t reading = {0};
    uint8_t reg;

    /* Read output only if new value is available */
    while((reg!=1) && (timeout>0))
    {
        lis2mdl_mag_data_ready_get(&dev_ctx, &reg);
        timeout --;
    }
      /* Read magnetic field data */
      memset(data_raw_magnetic.u8bit, 0x00, 3 * sizeof(int16_t));
      lis2mdl_magnetic_raw_get(&dev_ctx, data_raw_magnetic.u8bit);
      reading.magnetic_mG[0] = lis2mdl_from_lsb_to_mgauss(data_raw_magnetic.i16bit[0]);
      reading.magnetic_mG[1] = lis2mdl_from_lsb_to_mgauss(data_raw_magnetic.i16bit[1]);
      reading.magnetic_mG[2] = lis2mdl_from_lsb_to_mgauss(data_raw_magnetic.i16bit[2]);

      /* Read temperature data */
      memset(data_raw_temperature.u8bit, 0x00, sizeof(int16_t));
      lis2mdl_temperature_raw_get(&dev_ctx, data_raw_temperature.u8bit);
      reading.temperature_degC = lis2mdl_from_lsb_to_celsius(data_raw_temperature.i16bit);
    
    return reading;
  
}

/*
 * @brief  Write generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to write
 * @param  bufp      pointer to data to write in register reg
 * @param  len       number of consecutive register to write
 *
 */
static int32_t platform_write(void *handle, uint8_t reg, uint8_t *bufp,
                              uint16_t len)
{
  if (handle == &hi2c1)
  {
    /* Write multiple command */
    reg |= 0x80;
    HAL_I2C_Mem_Write(handle, LIS2MDL_I2C_ADD, reg,
                      I2C_MEMADD_SIZE_8BIT, bufp, len, 1000);
  }
  return 0;
}

/*
 * @brief  Read generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to read
 * @param  bufp      pointer to buffer that store the data read
 * @param  len       number of consecutive register to read
 *
 */
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len)
{
  if (handle == &hi2c1)
  {
    /* Read multiple command */
    reg |= 0x80;
    HAL_I2C_Mem_Read(handle, LIS2MDL_I2C_ADD, reg,
                     I2C_MEMADD_SIZE_8BIT, bufp, len, 1000);
  }
  return 0;
}

/*
 * @brief  platform specific delay (platform dependent)
 *
 * @param  ms        delay in ms
 *
 */
/* 
static void platform_delay(uint32_t ms)
{
  HAL_Delay(ms);
} */
