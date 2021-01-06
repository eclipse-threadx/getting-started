/*
 ******************************************************************************
 * @file    read_data_simple.c
 * @author  Sensors Software Solution Team
 * @brief   This file show the simplest way to get data from sensor.
 *
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
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
 * - STEVAL_MKI109V3
 * - NUCLEO_F411RE + X_NUCLEO_IKS01A2
 *
 * and STM32CubeMX tool with STM32CubeF4 MCU Package
 *
 * Used interfaces:
 *
 * STEVAL_MKI109V3    - Host side:   USB (Virtual COM)
 *                    - Sensor side: SPI(Default) / I2C(supported)
 *
 * NUCLEO_STM32F411RE + X_NUCLEO_IKS01A2 - Host side: UART(COM) to USB bridge
 *                                       - I2C(Default) / SPI(N/A)
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
#include "lps22hb_reg.h"

#include "sensor.h"

extern I2C_HandleTypeDef I2cHandle;
extern UART_HandleTypeDef UartHandle;

#define hi2c1 I2cHandle

typedef union{
  int16_t i16bit;
  uint8_t u8bit[2];
} axis1bit16_t;

typedef union{
  int32_t i32bit;
  uint8_t u8bit[4];
} axis1bit32_t;

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static axis1bit32_t data_raw_pressure;
static axis1bit16_t data_raw_temperature;
static uint8_t whoamI, rst;

/* Extern variables ----------------------------------------------------------*/

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

/* Main Example --------------------------------------------------------------*/

/* Initialize mems driver interface */
static stmdev_ctx_t dev_ctx =
{
    platform_write,
    platform_read,
    &hi2c1,
};

Sensor_StatusTypeDef lps22hb_config(void)
{
  Sensor_StatusTypeDef ret = SENSOR_OK;
  whoamI =0;
  /* Check device ID */  
  lps22hb_device_id_get(&dev_ctx, &whoamI);
  if(whoamI != LPS22HB_ID)
  {
    ret = SENSOR_ERROR;
  }
  else
  {
  /* Restore default configuration */
  lps22hb_reset_set(&dev_ctx, PROPERTY_ENABLE);
  do {
    lps22hb_reset_get(&dev_ctx, &rst);
  } while (rst);
 
  /* Enable Block Data Update */
  //lps22hb_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);

  /* Can be enabled low pass filter on output */
  lps22hb_low_pass_filter_mode_set(&dev_ctx, LPS22HB_LPF_ODR_DIV_2);

  /* Can be set Data-ready signal on INT_DRDY pin */
  //lps22hb_drdy_on_int_set(&dev_ctx, PROPERTY_ENABLE);

  /* Set Output Data Rate */
  lps22hb_data_rate_set(&dev_ctx, LPS22HB_ODR_10_Hz);
  }
  return ret;
}

static uint32_t timeout = 5;
lps22hb_t lps22hb_data_read(void)
{
  lps22hb_t reading = {0};
    uint8_t reg;
    /* Read output only if new value is available */
    while((reg!=1) && (timeout>0))
    {
      lps22hb_press_data_ready_get(&dev_ctx, &reg);
      timeout--;
    }
    
    memset(data_raw_pressure.u8bit, 0x00, sizeof(int32_t));
    lps22hb_pressure_raw_get(&dev_ctx, data_raw_pressure.u8bit);
    reading.pressure_hPa = lps22hb_from_lsb_to_hpa(data_raw_pressure.i32bit);
      
    memset(data_raw_temperature.u8bit, 0x00, sizeof(int16_t));
    lps22hb_temperature_raw_get(&dev_ctx, data_raw_temperature.u8bit);
    reading.temperature_degC = lps22hb_from_lsb_to_degc(data_raw_temperature.i16bit);

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
    HAL_I2C_Mem_Write(handle, LPS22HB_I2C_ADD_L, reg,
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
    HAL_I2C_Mem_Read(handle, LPS22HB_I2C_ADD_L, reg,
                     I2C_MEMADD_SIZE_8BIT, bufp, len, 1000);
  }
  return 0;
}
