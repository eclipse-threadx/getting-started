/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "r_cg_macrodriver.h"

#include "Config_SCI5.h"

volatile uint8_t tx_done;

void printf_transmit_end(void)
{
    tx_done = 1;
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
    tx_done = 0;

    R_Config_SCI5_Serial_Send(ptr, len);

    while (0 == tx_done)
    {
    	// wait for transmit complete
    }

    return len;
}
