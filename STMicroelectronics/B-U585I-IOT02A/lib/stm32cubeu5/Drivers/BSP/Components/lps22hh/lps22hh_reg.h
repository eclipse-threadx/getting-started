/*
 ******************************************************************************
 * @file    lps22hh_reg.h
 * @author  Sensors Software Solution Team
 * @brief   This file contains all the functions prototypes for the
 *          lps22hh_reg.c driver.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef LPS22HH_REGS_H
#define LPS22HH_REGS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <math.h>

/** @addtogroup LPS22HH
  * @{
  *
  */

/** @defgroup  Endianness definitions
  * @{
  *
  */

#ifndef DRV_BYTE_ORDER
#ifndef __BYTE_ORDER__

#define DRV_LITTLE_ENDIAN 1234
#define DRV_BIG_ENDIAN    4321

/** if _BYTE_ORDER is not defined, choose the endianness of your architecture
  * by uncommenting the define which fits your platform endianness
  */
//#define DRV_BYTE_ORDER    DRV_BIG_ENDIAN
#define DRV_BYTE_ORDER    DRV_LITTLE_ENDIAN

#else /* defined __BYTE_ORDER__ */

#define DRV_LITTLE_ENDIAN  __ORDER_LITTLE_ENDIAN__
#define DRV_BIG_ENDIAN     __ORDER_BIG_ENDIAN__
#define DRV_BYTE_ORDER     __BYTE_ORDER__

#endif /* __BYTE_ORDER__*/
#endif /* DRV_BYTE_ORDER */

/**
  * @}
  *
  */

/** @defgroup STMicroelectronics sensors common types
  * @{
  *
  */

#ifndef MEMS_SHARED_TYPES
#define MEMS_SHARED_TYPES

typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
  uint8_t bit0       : 1;
  uint8_t bit1       : 1;
  uint8_t bit2       : 1;
  uint8_t bit3       : 1;
  uint8_t bit4       : 1;
  uint8_t bit5       : 1;
  uint8_t bit6       : 1;
  uint8_t bit7       : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
  uint8_t bit7       : 1;
  uint8_t bit6       : 1;
  uint8_t bit5       : 1;
  uint8_t bit4       : 1;
  uint8_t bit3       : 1;
  uint8_t bit2       : 1;
  uint8_t bit1       : 1;
  uint8_t bit0       : 1;
#endif /* DRV_BYTE_ORDER */
} bitwise_t;

#define PROPERTY_DISABLE                (0U)
#define PROPERTY_ENABLE                 (1U)

/** @addtogroup  Interfaces_Functions
  * @brief       This section provide a set of functions used to read and
  *              write a generic register of the device.
  *              MANDATORY: return 0 -> no Error.
  * @{
  *
  */

typedef int32_t (*stmdev_write_ptr)(void *, uint8_t, uint8_t *,
                                    uint16_t);
typedef int32_t (*stmdev_read_ptr) (void *, uint8_t, uint8_t *,
                                    uint16_t);

typedef struct {
  /** Component mandatory fields **/
  stmdev_write_ptr  write_reg;
  stmdev_read_ptr   read_reg;
  /** Customizable optional pointer **/
  void *handle;
} stmdev_ctx_t;

/**
  * @}
  *
  */

#endif /* MEMS_SHARED_TYPES */

#ifndef MEMS_UCF_SHARED_TYPES
#define MEMS_UCF_SHARED_TYPES

/** @defgroup    Generic address-data structure definition
  * @brief       This structure is useful to load a predefined configuration
  *              of a sensor.
  *              You can create a sensor configuration by your own or using
  *              Unico / Unicleo tools available on STMicroelectronics
  *              web site.
  *
  * @{
  *
  */

typedef struct {
  uint8_t address;
  uint8_t data;
} ucf_line_t;

/**
  * @}
  *
  */

#endif /* MEMS_UCF_SHARED_TYPES */

/**
  * @}
  *
  */

/** @defgroup LPS22HH_Infos
  * @{
  *
  */

/** I2C Device Address 8 bit format  if SA0=0 -> B9 if SA0=1 -> BB **/
#define LPS22HH_I2C_ADD_H                       0xBBU
#define LPS22HH_I2C_ADD_L                       0xB9U

/** Device Identification (Who am I) **/
#define LPS22HH_ID                              0xB3U

/**
  * @}
  *
  */

#define LPS22HH_INTERRUPT_CFG                   0x0BU
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
  uint8_t pe                              : 2;  /* ple + phe */
  uint8_t lir                             : 1;
  uint8_t diff_en                         : 1;
  uint8_t reset_az                        : 1;
  uint8_t autozero                        : 1;
  uint8_t reset_arp                       : 1;
  uint8_t autorefp                        : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
  uint8_t autorefp                        : 1;
  uint8_t reset_arp                       : 1;
  uint8_t autozero                        : 1;
  uint8_t reset_az                        : 1;
  uint8_t diff_en                         : 1;
  uint8_t lir                             : 1;
  uint8_t pe                              : 2;  /* ple + phe */
#endif /* DRV_BYTE_ORDER */
} lps22hh_interrupt_cfg_t;

#define LPS22HH_THS_P_L                         0x0CU
typedef struct {
  uint8_t ths                             : 8;
} lps22hh_ths_p_l_t;

#define LPS22HH_THS_P_H                         0x0DU
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
  uint8_t ths                             : 7;
  uint8_t not_used_01                     : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
  uint8_t not_used_01                     : 1;
  uint8_t ths                             : 7;
#endif /* DRV_BYTE_ORDER */
} lps22hh_ths_p_h_t;

#define LPS22HH_IF_CTRL                         0x0EU
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
  uint8_t i2c_disable                     : 1;
  uint8_t i3c_disable                     : 1;
  uint8_t pd_dis_int1                     : 1;
  uint8_t sdo_pu_en                       : 1;
  uint8_t sda_pu_en                       : 1;
  uint8_t not_used_01                     : 2;
  uint8_t int_en_i3c                      : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
  uint8_t int_en_i3c                      : 1;
  uint8_t not_used_01                     : 2;
  uint8_t sda_pu_en                       : 1;
  uint8_t sdo_pu_en                       : 1;
  uint8_t pd_dis_int1                     : 1;
  uint8_t i3c_disable                     : 1;
  uint8_t i2c_disable                     : 1;
#endif /* DRV_BYTE_ORDER */
} lps22hh_if_ctrl_t;

#define LPS22HH_WHO_AM_I                        0x0FU
#define LPS22HH_CTRL_REG1                       0x10U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
  uint8_t sim                             : 1;
  uint8_t bdu                             : 1;
  uint8_t lpfp_cfg                        : 2;  /* en_lpfp + lpfp_cfg */
  uint8_t odr                             : 3;
  uint8_t not_used_01                     : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
  uint8_t not_used_01                     : 1;
  uint8_t odr                             : 3;
  uint8_t lpfp_cfg                        : 2;  /* en_lpfp + lpfp_cfg */
  uint8_t bdu                             : 1;
  uint8_t sim                             : 1;
#endif /* DRV_BYTE_ORDER */
} lps22hh_ctrl_reg1_t;

#define LPS22HH_CTRL_REG2                       0x11U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
  uint8_t one_shot                        : 1;
  uint8_t low_noise_en                    : 1;
  uint8_t swreset                         : 1;
  uint8_t not_used_01                     : 1;
  uint8_t if_add_inc                      : 1;
  uint8_t pp_od                           : 1;
  uint8_t int_h_l                         : 1;
  uint8_t boot                            : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
  uint8_t boot                            : 1;
  uint8_t int_h_l                         : 1;
  uint8_t pp_od                           : 1;
  uint8_t if_add_inc                      : 1;
  uint8_t not_used_01                     : 1;
  uint8_t swreset                         : 1;
  uint8_t low_noise_en                    : 1;
  uint8_t one_shot                        : 1;
#endif /* DRV_BYTE_ORDER */
} lps22hh_ctrl_reg2_t;

#define LPS22HH_CTRL_REG3                       0x12U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
  uint8_t int_s                           : 2;
  uint8_t drdy                            : 1;
  uint8_t int_f_ovr                       : 1;
  uint8_t int_f_wtm                       : 1;
  uint8_t int_f_full                      : 1;
  uint8_t not_used_01                     : 2;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
  uint8_t not_used_01                     : 2;
  uint8_t int_f_full                      : 1;
  uint8_t int_f_wtm                       : 1;
  uint8_t int_f_ovr                       : 1;
  uint8_t drdy                            : 1;
  uint8_t int_s                           : 2;
#endif /* DRV_BYTE_ORDER */
} lps22hh_ctrl_reg3_t;

#define LPS22HH_FIFO_CTRL                       0x13U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
uint8_t f_mode                          :
  3;  /* f_mode + trig_modes */
  uint8_t stop_on_wtm                     : 1;
  uint8_t not_used_01                     : 4;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
  uint8_t not_used_01                     : 4;
  uint8_t stop_on_wtm                     : 1;
uint8_t f_mode                          :
  3;  /* f_mode + trig_modes */
#endif /* DRV_BYTE_ORDER */
} lps22hh_fifo_ctrl_t;

#define LPS22HH_FIFO_WTM                        0x14U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
  uint8_t wtm                             : 7;
  uint8_t not_used_01                     : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
  uint8_t not_used_01                     : 1;
  uint8_t wtm                             : 7;
#endif /* DRV_BYTE_ORDER */
} lps22hh_fifo_wtm_t;

#define LPS22HH_REF_P_L                         0x15U
#define LPS22HH_REF_P_H                         0x16U
#define LPS22HH_RPDS_L                          0x18U
#define LPS22HH_RPDS_H                          0x19U
#define LPS22HH_INT_SOURCE                      0x24U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
  uint8_t ph                              : 1;
  uint8_t pl                              : 1;
  uint8_t ia                              : 1;
  uint8_t not_used_01                     : 4;
  uint8_t boot_on                         : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
  uint8_t boot_on                         : 1;
  uint8_t not_used_01                     : 4;
  uint8_t ia                              : 1;
  uint8_t pl                              : 1;
  uint8_t ph                              : 1;
#endif /* DRV_BYTE_ORDER */
} lps22hh_int_source_t;

#define LPS22HH_FIFO_STATUS1                    0x25U
#define LPS22HH_FIFO_STATUS2                    0x26U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
  uint8_t not_used_01                     : 5;
  uint8_t fifo_full_ia                    : 1;
  uint8_t fifo_ovr_ia                     : 1;
  uint8_t fifo_wtm_ia                     : 1;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
  uint8_t fifo_wtm_ia                     : 1;
  uint8_t fifo_ovr_ia                     : 1;
  uint8_t fifo_full_ia                    : 1;
  uint8_t not_used_01                     : 5;
#endif /* DRV_BYTE_ORDER */
} lps22hh_fifo_status2_t;

#define LPS22HH_STATUS                          0x27U
typedef struct {
#if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
  uint8_t p_da                            : 1;
  uint8_t t_da                            : 1;
  uint8_t not_used_01                     : 2;
  uint8_t p_or                            : 1;
  uint8_t t_or                            : 1;
  uint8_t not_used_02                     : 2;
#elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
  uint8_t not_used_02                     : 2;
  uint8_t t_or                            : 1;
  uint8_t p_or                            : 1;
  uint8_t not_used_01                     : 2;
  uint8_t t_da                            : 1;
  uint8_t p_da                            : 1;
#endif /* DRV_BYTE_ORDER */
} lps22hh_status_t;

#define LPS22HH_PRESS_OUT_XL                    0x28U
#define LPS22HH_PRESS_OUT_L                     0x29U
#define LPS22HH_PRESS_OUT_H                     0x2AU
#define LPS22HH_TEMP_OUT_L                      0x2BU
#define LPS22HH_TEMP_OUT_H                      0x2CU
#define LPS22HH_FIFO_DATA_OUT_PRESS_XL          0x78U
#define LPS22HH_FIFO_DATA_OUT_PRESS_L           0x79U
#define LPS22HH_FIFO_DATA_OUT_PRESS_H           0x7AU
#define LPS22HH_FIFO_DATA_OUT_TEMP_L            0x7BU
#define LPS22HH_FIFO_DATA_OUT_TEMP_H            0x7CU

/**
  * @defgroup LPS22HH_Register_Union
  * @brief    This union group all the registers that has a bitfield
  *           description.
  *           This union is useful but not need by the driver.
  *
  *           REMOVING this union you are compliant with:
  *           MISRA-C 2012 [Rule 19.2] -> " Union are not allowed "
  *
  * @{
  *
  */
typedef union {
  lps22hh_interrupt_cfg_t        interrupt_cfg;
  lps22hh_if_ctrl_t              if_ctrl;
  lps22hh_ctrl_reg1_t            ctrl_reg1;
  lps22hh_ctrl_reg2_t            ctrl_reg2;
  lps22hh_ctrl_reg3_t            ctrl_reg3;
  lps22hh_fifo_ctrl_t            fifo_ctrl;
  lps22hh_fifo_wtm_t             fifo_wtm;
  lps22hh_int_source_t           int_source;
  lps22hh_fifo_status2_t         fifo_status2;
  lps22hh_status_t               status;
  bitwise_t                      bitwise;
  uint8_t                        byte;
} lps22hh_reg_t;

/**
  * @}
  *
  */

int32_t lps22hh_read_reg(stmdev_ctx_t *ctx, uint8_t reg,
                         uint8_t *data,
                         uint16_t len);
int32_t lps22hh_write_reg(stmdev_ctx_t *ctx, uint8_t reg,
                          uint8_t *data,
                          uint16_t len);

float_t lps22hh_from_lsb_to_hpa(uint32_t lsb);
float_t lps22hh_from_lsb_to_celsius(int16_t lsb);

int32_t lps22hh_autozero_rst_set(stmdev_ctx_t *ctx, uint8_t val);
int32_t lps22hh_autozero_rst_get(stmdev_ctx_t *ctx, uint8_t *val);

int32_t lps22hh_autozero_set(stmdev_ctx_t *ctx, uint8_t val);
int32_t lps22hh_autozero_get(stmdev_ctx_t *ctx, uint8_t *val);

int32_t lps22hh_pressure_snap_rst_set(stmdev_ctx_t *ctx, uint8_t val);
int32_t lps22hh_pressure_snap_rst_get(stmdev_ctx_t *ctx,
                                      uint8_t *val);

int32_t lps22hh_pressure_snap_set(stmdev_ctx_t *ctx, uint8_t val);
int32_t lps22hh_pressure_snap_get(stmdev_ctx_t *ctx, uint8_t *val);

int32_t lps22hh_block_data_update_set(stmdev_ctx_t *ctx, uint8_t val);
int32_t lps22hh_block_data_update_get(stmdev_ctx_t *ctx,
                                      uint8_t *val);

typedef enum {
  LPS22HH_POWER_DOWN          = 0x00,
  LPS22HH_ONE_SHOOT           = 0x08,
  LPS22HH_1_Hz                = 0x01,
  LPS22HH_10_Hz               = 0x02,
  LPS22HH_25_Hz               = 0x03,
  LPS22HH_50_Hz               = 0x04,
  LPS22HH_75_Hz               = 0x05,
  LPS22HH_1_Hz_LOW_NOISE      = 0x11,
  LPS22HH_10_Hz_LOW_NOISE     = 0x12,
  LPS22HH_25_Hz_LOW_NOISE     = 0x13,
  LPS22HH_50_Hz_LOW_NOISE     = 0x14,
  LPS22HH_75_Hz_LOW_NOISE     = 0x15,
  LPS22HH_100_Hz              = 0x06,
  LPS22HH_200_Hz              = 0x07,
} lps22hh_odr_t;
int32_t lps22hh_data_rate_set(stmdev_ctx_t *ctx, lps22hh_odr_t val);
int32_t lps22hh_data_rate_get(stmdev_ctx_t *ctx, lps22hh_odr_t *val);

int32_t lps22hh_pressure_ref_set(stmdev_ctx_t *ctx, int16_t val);
int32_t lps22hh_pressure_ref_get(stmdev_ctx_t *ctx, int16_t *val);

int32_t lps22hh_pressure_offset_set(stmdev_ctx_t *ctx, int16_t val);
int32_t lps22hh_pressure_offset_get(stmdev_ctx_t *ctx, int16_t *val);

typedef struct {
  lps22hh_int_source_t    int_source;
  lps22hh_fifo_status2_t  fifo_status2;
  lps22hh_status_t        status;
} lps22hh_all_sources_t;
int32_t lps22hh_all_sources_get(stmdev_ctx_t *ctx,
                                lps22hh_all_sources_t *val);

int32_t lps22hh_status_reg_get(stmdev_ctx_t *ctx,
                               lps22hh_status_t *val);

int32_t lps22hh_press_flag_data_ready_get(stmdev_ctx_t *ctx,
                                          uint8_t *val);

int32_t lps22hh_temp_flag_data_ready_get(stmdev_ctx_t *ctx,
                                         uint8_t *val);

int32_t lps22hh_pressure_raw_get(stmdev_ctx_t *ctx, uint32_t *buff);

int32_t lps22hh_temperature_raw_get(stmdev_ctx_t *ctx, int16_t *buff);

int32_t lps22hh_fifo_pressure_raw_get(stmdev_ctx_t *ctx,
                                      uint32_t *buff);

int32_t lps22hh_fifo_temperature_raw_get(stmdev_ctx_t *ctx,
                                         int16_t *buff);

int32_t lps22hh_device_id_get(stmdev_ctx_t *ctx, uint8_t *buff);

int32_t lps22hh_reset_set(stmdev_ctx_t *ctx, uint8_t val);
int32_t lps22hh_reset_get(stmdev_ctx_t *ctx, uint8_t *val);

int32_t lps22hh_auto_increment_set(stmdev_ctx_t *ctx, uint8_t val);
int32_t lps22hh_auto_increment_get(stmdev_ctx_t *ctx, uint8_t *val);

int32_t lps22hh_boot_set(stmdev_ctx_t *ctx, uint8_t val);
int32_t lps22hh_boot_get(stmdev_ctx_t *ctx, uint8_t *val);

typedef enum {
  LPS22HH_LPF_ODR_DIV_2    = 0,
  LPS22HH_LPF_ODR_DIV_9    = 2,
  LPS22HH_LPF_ODR_DIV_20   = 3,
} lps22hh_lpfp_cfg_t;
int32_t lps22hh_lp_bandwidth_set(stmdev_ctx_t *ctx,
                                 lps22hh_lpfp_cfg_t val);
int32_t lps22hh_lp_bandwidth_get(stmdev_ctx_t *ctx,
                                 lps22hh_lpfp_cfg_t *val);

typedef enum {
  LPS22HH_I2C_ENABLE    = 0,
  LPS22HH_I2C_DISABLE   = 1,
} lps22hh_i2c_disable_t;
int32_t lps22hh_i2c_interface_set(stmdev_ctx_t *ctx,
                                  lps22hh_i2c_disable_t val);
int32_t lps22hh_i2c_interface_get(stmdev_ctx_t *ctx,
                                  lps22hh_i2c_disable_t *val);

typedef enum {
  LPS22HH_I3C_ENABLE                 = 0x00,
  LPS22HH_I3C_ENABLE_INT_PIN_ENABLE  = 0x10,
  LPS22HH_I3C_DISABLE                = 0x11,
} lps22hh_i3c_disable_t;
int32_t lps22hh_i3c_interface_set(stmdev_ctx_t *ctx,
                                  lps22hh_i3c_disable_t val);
int32_t lps22hh_i3c_interface_get(stmdev_ctx_t *ctx,
                                  lps22hh_i3c_disable_t *val);

typedef enum {
  LPS22HH_PULL_UP_DISCONNECT    = 0,
  LPS22HH_PULL_UP_CONNECT       = 1,
} lps22hh_pu_en_t;
int32_t lps22hh_sdo_sa0_mode_set(stmdev_ctx_t *ctx,
                                 lps22hh_pu_en_t val);
int32_t lps22hh_sdo_sa0_mode_get(stmdev_ctx_t *ctx,
                                 lps22hh_pu_en_t *val);
int32_t lps22hh_sda_mode_set(stmdev_ctx_t *ctx, lps22hh_pu_en_t val);
int32_t lps22hh_sda_mode_get(stmdev_ctx_t *ctx, lps22hh_pu_en_t *val);

typedef enum {
  LPS22HH_SPI_4_WIRE  = 0,
  LPS22HH_SPI_3_WIRE  = 1,
} lps22hh_sim_t;
int32_t lps22hh_spi_mode_set(stmdev_ctx_t *ctx, lps22hh_sim_t val);
int32_t lps22hh_spi_mode_get(stmdev_ctx_t *ctx, lps22hh_sim_t *val);

typedef enum {
  LPS22HH_INT_PULSED   = 0,
  LPS22HH_INT_LATCHED  = 1,
} lps22hh_lir_t;
int32_t lps22hh_int_notification_set(stmdev_ctx_t *ctx,
                                     lps22hh_lir_t val);
int32_t lps22hh_int_notification_get(stmdev_ctx_t *ctx,
                                     lps22hh_lir_t *val);

typedef enum {
  LPS22HH_PUSH_PULL   = 0,
  LPS22HH_OPEN_DRAIN  = 1,
} lps22hh_pp_od_t;
int32_t lps22hh_pin_mode_set(stmdev_ctx_t *ctx, lps22hh_pp_od_t val);
int32_t lps22hh_pin_mode_get(stmdev_ctx_t *ctx, lps22hh_pp_od_t *val);

typedef enum {
  LPS22HH_ACTIVE_HIGH = 0,
  LPS22HH_ACTIVE_LOW  = 1,
} lps22hh_int_h_l_t;
int32_t lps22hh_pin_polarity_set(stmdev_ctx_t *ctx,
                                 lps22hh_int_h_l_t val);
int32_t lps22hh_pin_polarity_get(stmdev_ctx_t *ctx,
                                 lps22hh_int_h_l_t *val);

int32_t lps22hh_pin_int_route_set(stmdev_ctx_t *ctx,
                                  lps22hh_ctrl_reg3_t *val);
int32_t lps22hh_pin_int_route_get(stmdev_ctx_t *ctx,
                                  lps22hh_ctrl_reg3_t *val);

typedef enum {
  LPS22HH_NO_THRESHOLD  = 0,
  LPS22HH_POSITIVE      = 1,
  LPS22HH_NEGATIVE      = 2,
  LPS22HH_BOTH          = 3,
} lps22hh_pe_t;
int32_t lps22hh_int_on_threshold_set(stmdev_ctx_t *ctx,
                                     lps22hh_pe_t val);
int32_t lps22hh_int_on_threshold_get(stmdev_ctx_t *ctx,
                                     lps22hh_pe_t *val);

int32_t lps22hh_int_treshold_set(stmdev_ctx_t *ctx, uint16_t buff);
int32_t lps22hh_int_treshold_get(stmdev_ctx_t *ctx, uint16_t *buff);

typedef enum {
  LPS22HH_BYPASS_MODE            = 0,
  LPS22HH_FIFO_MODE              = 1,
  LPS22HH_STREAM_MODE            = 2,
  LPS22HH_DYNAMIC_STREAM_MODE    = 3,
  LPS22HH_BYPASS_TO_FIFO_MODE    = 5,
  LPS22HH_BYPASS_TO_STREAM_MODE  = 6,
  LPS22HH_STREAM_TO_FIFO_MODE    = 7,
} lps22hh_f_mode_t;
int32_t lps22hh_fifo_mode_set(stmdev_ctx_t *ctx,
                              lps22hh_f_mode_t val);
int32_t lps22hh_fifo_mode_get(stmdev_ctx_t *ctx,
                              lps22hh_f_mode_t *val);

int32_t lps22hh_fifo_stop_on_wtm_set(stmdev_ctx_t *ctx, uint8_t val);
int32_t lps22hh_fifo_stop_on_wtm_get(stmdev_ctx_t *ctx, uint8_t *val);

int32_t lps22hh_fifo_watermark_set(stmdev_ctx_t *ctx, uint8_t val);
int32_t lps22hh_fifo_watermark_get(stmdev_ctx_t *ctx, uint8_t *val);

int32_t lps22hh_fifo_data_level_get(stmdev_ctx_t *ctx, uint8_t *buff);

int32_t lps22hh_fifo_src_get(stmdev_ctx_t *ctx,
                             lps22hh_fifo_status2_t *val);

int32_t lps22hh_fifo_full_flag_get(stmdev_ctx_t *ctx, uint8_t *val);

int32_t lps22hh_fifo_ovr_flag_get(stmdev_ctx_t *ctx, uint8_t *val);

int32_t lps22hh_fifo_wtm_flag_get(stmdev_ctx_t *ctx, uint8_t *val);

int32_t lps22hh_fifo_ovr_on_int_set(stmdev_ctx_t *ctx, uint8_t val);
int32_t lps22hh_fifo_ovr_on_int_get(stmdev_ctx_t *ctx, uint8_t *val);

int32_t lps22hh_fifo_threshold_on_int_set(stmdev_ctx_t *ctx,
                                          uint8_t val);
int32_t lps22hh_fifo_threshold_on_int_get(stmdev_ctx_t *ctx,
                                          uint8_t *val);

int32_t lps22hh_fifo_full_on_int_set(stmdev_ctx_t *ctx, uint8_t val);
int32_t lps22hh_fifo_full_on_int_get(stmdev_ctx_t *ctx, uint8_t *val);

/**
  * @}
  *
  */

#ifdef __cplusplus
}
#endif

#endif /*LPS22HH_REGS_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
