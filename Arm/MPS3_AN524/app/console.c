/* Copyright (c) 2021 Linaro Limited. */

#include <assert.h>
#include <stdio.h>

#include "board_init.h"

/* Override `_write` function to redirect printf to uart console */
int _write(int fd, char *str, int len)
{
    (void)fd;
    int32_t ret;

    /* Send string and return the number of characters written */
    ret = Driver_USART0.Send(str, len);
    assert(ret == ARM_DRIVER_OK);

    /* Add a busy wait after sending. */
    while (Driver_USART0.GetStatus().tx_busy);

    return Driver_USART0.GetTxCount();
}
