#include "board_init.h"

#include <stdio.h>

#include "fsl_iomuxc.h"

#include "nx_driver_imxrt1062.h"

#include "board.h"
#include "pin_mux.h"
#include "peripherals.h"

void* __RAM_segment_used_end__ = 0;

void nx_ethernet_driver(NX_IP_DRIVER* driver_req_ptr)
{
    return nx_driver_imx;
}

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
