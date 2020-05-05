#include "board_init.h"

#include <stdio.h>

#include "fsl_iomuxc.h"

#include "board.h"
#include "pin_mux.h"
#include "peripherals.h"

void* __RAM_segment_used_end__ = 0;

// Faked system time. GMT: April, 10, 2020 12:00:00 AM. Epoch timestamp: 1577836800000 + 100 days.
//#define SYSTEM_TIME 1577836800000
//return (time_t)(SYSTEM_TIME + (tx_time_get()/TX_TIMER_TICKS_PER_SECOND));

void board_init()
{
	// init board hardware
    BOARD_InitBootPins();
	BOARD_InitENET();
	BOARD_InitBootClocks();
	BOARD_InitBootPeripherals();

	// init fsl debug console
    BOARD_InitDebugConsole();
}
