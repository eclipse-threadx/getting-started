#include "board_init.h"

#include "board.h"
#include "pin_mux.h"
#include "peripherals.h"

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
