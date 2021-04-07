/*
 * isl29035_sensor.h
 *
 *  Created on: Jan 3, 2018
 *      Author: Rajkumar.Thiagarajan
 */

#ifndef ISL29035_SENSOR_H_
#define ISL29035_SENSOR_H_

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include "platform.h"

#define ISL29035_I2C_ADDR       (0x44)
#define ISL29035_I2C_INTF       (0)

#define CONFIG_ISL29035_MODE_ALS                    1
#define CONFIG_ISL29035_LUX_RANGE_1K                1
#define CONFIG_ISL29035_INTEGRATION_TIME_6500         1

/* Register address */
#define ISL29035_COMMAND_I_REG      0x00
#define ISL29035_OPMODE_SHIFT       5
#define ISL29035_OPMODE_MASK        (7 << ISL29035_OPMODE_SHIFT)
#define ISL29035_INT_BIT_SHIFT      2
#define ISL29035_INT_BIT_MASK       (1 << ISL29035_INT_BIT_SHIFT)
#define ISL29035_INT_PRST_SHIFT     0
#define ISL29035_INT_PRST_MASK      (3 << ISL29035_INT_BIT_SHIFT)

#define ISL29035_OPMODE_OFF     0
#define ISL29035_OPMODE_ALS_ONCE    1
#define ISL29035_OPMODE_IR_ONCE     2
#define ISL29035_OPMODE_ALS_CONT    5
#define ISL29035_OPMODE_IR_CONT     6

#define ISL29035_COMMAND_II_REG     0x01
#define ISL29035_LUX_RANGE_SHIFT    0
#define ISL29035_LUX_RANGE_MASK     (3 << ISL29035_LUX_RANGE_SHIFT)
#define ISL29035_ADC_RES_SHIFT      2
#define ISL29035_ADC_RES_MASK       (3 << ISL29035_ADC_RES_SHIFT)

#define ISL29035_DATA_LSB_REG       0x02
#define ISL29035_DATA_MSB_REG       0x03
#define ISL29035_INT_LT_LSB_REG     0x04
#define ISL29035_INT_LT_MSB_REG     0x05
#define ISL29035_INT_HT_LSB_REG     0x06
#define ISL29035_INT_HT_MSB_REG     0x07

#define ISL29035_REG_DEVICE_ID      0x0F
#define ISL29035_DEVICE_ID_SHIFT    0x03
#define ISL29035_DEVICE_ID_MASK     (0x7 << ISL29035_DEVICE_ID_SHIFT)
#define ISL29035_DEVICE_ID          0x5
#define ISL29035_BOUT_SHIFT         0x07
#define ISL29035_BOUT_MASK          (0x01 << ISL29035_BOUT_SHIFT)

#define ISL29035_CHIP_ID          (0x28)


#if CONFIG_ISL29035_MODE_ALS
    #define ISL29035_ACTIVE_OPMODE      ISL29035_OPMODE_ALS_CONT
    #define ISL29035_ACTIVE_CHAN        SENSOR_CHAN_LIGHT
#elif CONFIG_ISL29035_MODE_IR
    #define ISL29035_ACTIVE_OPMODE      ISL29035_OPMODE_IR_CONT
    #define ISL29035_ACTIVE_CHAN        SENSOR_CHAN_IR
#endif

#define ISL29035_ACTIVE_OPMODE_BITS     \
    (ISL29035_ACTIVE_OPMODE << ISL29035_OPMODE_SHIFT)

#if CONFIG_ISL29035_LUX_RANGE_1K
    #define ISL29035_LUX_RANGE_IDX      0
    #define ISL29035_LUX_RANGE      1000
#elif CONFIG_ISL29035_LUX_RANGE_4K
    #define ISL29035_LUX_RANGE_IDX      1
    #define ISL29035_LUX_RANGE      4000
#elif CONFIG_ISL29035_LUX_RANGE_16K
    #define ISL29035_LUX_RANGE_IDX      2
    #define ISL29035_LUX_RANGE      16000
#elif CONFIG_ISL29035_LUX_RANGE_64K
    #define ISL29035_LUX_RANGE_IDX      3
    #define ISL29035_LUX_RANGE      64000
#endif

#define ISL29035_LUX_RANGE_BITS         \
    (ISL29035_LUX_RANGE_IDX << ISL29035_LUX_RANGE_SHIFT)

#if CONFIG_ISL29035_INTEGRATION_TIME_26
    #define ISL29035_ADC_RES_IDX        3
#elif CONFIG_ISL29035_INTEGRATION_TIME_410
    #define ISL29035_ADC_RES_IDX        2
#elif CONFIG_ISL29035_INTEGRATION_TIME_6500
    #define ISL29035_ADC_RES_IDX        1
#elif CONFIG_ISL29035_INTEGRATION_TIME_105K
    #define ISL29035_ADC_RES_IDX        0
#endif

#define ISL29035_ADC_RES_BITS           \
    (ISL29035_ADC_RES_IDX << ISL29035_ADC_RES_SHIFT)


/** Error code definitions */
#define ISL29035_OK                         (0)
#define ISL29035_E_NULL_PTR                 (-1)
#define ISL29035_E_COM_FAIL                 (-2)
#define ISL29035_E_DEV_NOT_FOUND            (-3)


#define ISL29035_LUX_RANGE_1000        (0)
#define ISL29035_LUX_RANGE_4000        (1)
#define ISL29035_LUX_RANGE_16000       (2)
#define ISL29035_LUX_RANGE_64000       (3)

#define ISL29035_ADC_RES_65536         (16)
#define ISL29035_ADC_RES_4096          (12)
#define ISL29035_ADC_RES_256           (8)
#define ISL29035_ADC_RES_16            (4)

/*****************************************************************************/
/* type definitions */
typedef int8_t (*isl29035_com_fptr_t)(uint8_t dev_addr, uint8_t reg_addr,
        uint8_t *data, uint16_t len);

typedef void (*isl29035_delay_fptr_t)(uint32_t period);


struct isl29035_dev {
    /*! Chip Id */
    uint8_t chip_id;
    /*! Device Id */
    uint8_t id;
    /*! 0 - I2C , 1 - SPI Interface */
    uint8_t interface;

    uint8_t adc_res;

    uint8_t adc_range;

    uint8_t adc_mode;

    /*! Read function pointer */
     isl29035_com_fptr_t read;
     /*! Write function pointer */
     isl29035_com_fptr_t write;
     /*!  Delay function pointer */
     isl29035_delay_fptr_t delay_ms;
};


int8_t isl29035_init(struct isl29035_dev *dev);
int8_t isl29035_configure(struct isl29035_dev *dev);
int8_t isl29035_read_als_data(struct isl29035_dev *dev, double *als_val);

#endif /* ISL29035_SENSOR_H_ */
