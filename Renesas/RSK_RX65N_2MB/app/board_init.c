/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "board_init.h"

#include "r_cmt_rx_if.h"
#include "r_ether_rx_if.h"
#include "r_ether_rx_pinset.h"
#include "r_smc_entry.h"

#include "platform.h"

#include "Config_SCI8.h"

void _tx_timer_interrupt();

// CMT Timer callback used as the system tick.
void timer_callback(void* pdata)
{
    _tx_timer_interrupt();
}

void board_init()
{
    uint32_t chan;
    ether_param_t eth_param = {0};

    // Setup SCI8 for printf output
    R_Config_SCI8_Start();

    // Create periodic timer for the system tick
    R_CMT_CreatePeriodic(100u, timer_callback, &chan);

    // Setup Ethernet hardware
    R_ETHER_Initial();

    R_ETHER_PinSet_ETHERC0_MII();

    eth_param.channel = 0u;
    R_ETHER_Control(CONTROL_POWER_ON, eth_param);
}
