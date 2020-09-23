/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "board_init.h"

#include "board.h"
#include "pin_mux.h"

void board_init()
{
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
}
