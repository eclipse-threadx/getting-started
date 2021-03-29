/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "board_init.h"

#include "board.h"
#include "fsl_tempmon.h"
#include "pin_mux.h"

static void tempmon_init()
{
    tempmon_config_t config;
    TEMPMON_GetDefaultConfig(&config);
    TEMPMON_Init(TEMPMON, &config);
}

void board_init()
{
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    tempmon_init();    
}
