/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file
 * to avoid losing it when reconfiguring.
 */
#ifndef DRIVER_INIT_INCLUDED
#define DRIVER_INIT_INCLUDED

#include "atmel_start_pins.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <hal_atomic.h>
#include <hal_delay.h>
#include <hal_gpio.h>
#include <hal_init.h>
#include <hal_io.h>
#include <hal_sleep.h>

#include <hal_usart_sync.h>

#include <hal_i2c_m_sync.h>

#include <hal_mac_async.h>

extern struct usart_sync_descriptor TARGET_IO;

extern struct i2c_m_sync_desc I2C_AT24MAC;

extern struct mac_async_descriptor MACIF;

void TARGET_IO_PORT_init(void);
void TARGET_IO_CLOCK_init(void);
void TARGET_IO_init(void);

void I2C_AT24MAC_CLOCK_init(void);
void I2C_AT24MAC_init(void);
void I2C_AT24MAC_PORT_init(void);

void MACIF_CLOCK_init(void);
void MACIF_init(void);
void MACIF_PORT_init(void);
void MACIF_example(void);

/**
 * \brief Perform system initialization, initialize pins and clocks for
 * peripherals
 */
void system_init(void);

#ifdef __cplusplus
}
#endif
#endif // DRIVER_INIT_INCLUDED
