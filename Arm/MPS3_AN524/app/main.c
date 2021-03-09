/* Copyright (c) 2021 Linaro Limited. */

#include <stdio.h>

#include "board_init.h"

int main(void)
{
    /* Initialise the board */
    board_init();

    printf("Azure RTOS running on MPS3 board\n");

    
    /* tx_kernel_enter(); */

    return 0;
}
