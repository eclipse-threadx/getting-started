/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "stm32f4xx_hal.h"

#include "board_init.h"

int __io_putchar(int ch);
int __io_getchar(void);
int _read(int file, char* ptr, int len);
int _write(int file, char* ptr, int len);

int __io_putchar(int ch)
{
    HAL_UART_Transmit(&UartHandle, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

int __io_getchar(void)
{
    uint8_t ch;
    HAL_UART_Receive(&UartHandle, &ch, 1, HAL_MAX_DELAY);

    /* Echo character back to console */
    HAL_UART_Transmit(&UartHandle, &ch, 1, HAL_MAX_DELAY);

    /* And cope with Windows */
    if (ch == '\r')
    {
        uint8_t ret = '\n';
        HAL_UART_Transmit(&UartHandle, &ret, 1, HAL_MAX_DELAY);
    }

    return ch;
}

int _read(int file, char* ptr, int len)
{
    int DataIdx;

    for (DataIdx = 0; DataIdx < len; DataIdx++)
    {
        *ptr++ = __io_getchar();
    }

    return len;
}

int _write(int file, char* ptr, int len)
{
    int DataIdx;

    for (DataIdx = 0; DataIdx < len; DataIdx++)
    {
        __io_putchar(*ptr++);
    }
    return len;
}
