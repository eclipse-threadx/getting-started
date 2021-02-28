/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef RX_I2C_API_H_
#define RX_I2C_API_H_

#include <stdint.h>

int8_t rx_i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);
int8_t rx_i2c_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);
void rx_delay_ms(uint32_t period);

#endif
