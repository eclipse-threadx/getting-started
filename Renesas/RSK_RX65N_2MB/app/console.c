/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "r_cg_macrodriver.h"
#include "Config_SCI8.h"

/*#include "fsl_debug_console.h"

int _read(int file, char *ptr, int len);
int _write(int file, char *ptr, int len);

int _read(int file, char *ptr, int len)
{
    int DataIdx;
    for (DataIdx = 0; DataIdx < len; DataIdx++)
    {
        *ptr++ = GETCHAR();
    }
    return len;
}

int _write(int file, char *ptr, int len)
{
    int DataIdx;
    for (DataIdx = 0; DataIdx < len; DataIdx++)
    {
        PUTCHAR(*ptr++);
    }
    return len;
}*/

void my_sw_charput_function(char c)
{
    R_Config_SCI8_Serial_Send((uint8_t*)&c, 1u);

    return;
}
