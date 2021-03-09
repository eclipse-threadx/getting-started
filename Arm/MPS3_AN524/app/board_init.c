/* Copyright (c) 2021 Linaro Limited. */

#include <stdio.h>
#include <assert.h>

#include "board_init.h"

/* Initialise uart console */
static void console_init()
{
    int32_t ret;

    /* Initialize the USART driver */
    ret = Driver_USART0.Initialize(NULL);
    assert(ret == ARM_DRIVER_OK);

    /* Power up the USART peripheral */
    ret = Driver_USART0.PowerControl(ARM_POWER_FULL);
    assert(ret == ARM_DRIVER_OK);

    /* Configure the USART to 115200 (DEFAULT_UART_BAUDRATE) Bits/sec */
    ret = Driver_USART0.Control(ARM_USART_MODE_ASYNCHRONOUS,
                               DEFAULT_UART_BAUDRATE);
    assert(ret == ARM_DRIVER_OK);

    /* Enable Transmitter line */
    Driver_USART0.Control(ARM_USART_CONTROL_TX, 1);
}

/* Initialise the board */
void board_init()
{
    /* Initialise uart console */
    console_init();
}
