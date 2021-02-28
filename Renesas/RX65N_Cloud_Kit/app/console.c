/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "r_cg_macrodriver.h"

#include "Config_SCI5.h"

#include "tx_api.h"

TX_SEMAPHORE printf_semaphore;

void printf_init(void)
{
    UINT res;

    res = tx_semaphore_create(&printf_semaphore, "printf semaphore", 1);
    if (res != TX_SUCCESS)
    {
        while (true)
        {
        }
    }
}

void printf_transmit_end(void)
{
    tx_semaphore_put(&printf_semaphore);
}

int read(int file, char* ptr, int len)
{
    int DataIdx;

    for (DataIdx = 0; DataIdx < len; DataIdx++)
    {
        *ptr++ = charget();
    }

    return len;
}

int write(int file, char* ptr, int len)
{
    int DataIdx;

    tx_semaphore_get(&printf_semaphore, TX_WAIT_FOREVER);

    R_Config_SCI5_Serial_Send(ptr, len);

    return len;
}
