/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "board_init.h"

#include "r_cmt_rx_if.h"
#include "r_smc_entry.h"

#include "platform.h"

#include "rx65n_cloud_kit_sensors.h"

void _tx_timer_interrupt();

// CMT Timer callback used as the system tick.
void timer_callback(void* pdata)
{
    _tx_timer_interrupt();
}

void board_init()
{
    uint32_t chan;

    // Setup SCI5 for printf output
    R_Config_SCI5_Start();

    // Create periodic timer for the system tick
    R_CMT_CreatePeriodic(100u, timer_callback, &chan);

    // Initialize the Option Board sensors
    init_sensors();
}
