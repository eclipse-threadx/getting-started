/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "r_cg_macrodriver.h"

#include "Config_SCI8.h"

#include "tx_api.h"

TX_MUTEX printf_mutex;
TX_SEMAPHORE printf_semaphore;

void printf_init(void)
{
    UINT res;

    res = tx_mutex_create(&printf_mutex, "printf mutex", TX_INHERIT);
    if (res != TX_SUCCESS)
    {
        for (;;)
        {
        }
    }

    res = tx_semaphore_create(&printf_semaphore, "printf semaphore", 0u);
    if (res != TX_SUCCESS)
    {
        for (;;)
        {
        }
    }
}

void printf_transmit_end(void)
{
    tx_semaphore_put(&printf_semaphore);
}

void my_sw_charput_function(char c)
{
    tx_mutex_get(&printf_mutex, TX_WAIT_FOREVER);

    R_Config_SCI8_Serial_Send(&c, 1u);

    tx_semaphore_get(&printf_semaphore, TX_WAIT_FOREVER);

    tx_mutex_put(&printf_mutex);

    return;
}

