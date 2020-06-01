#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"

int main() 
{
	BOARD_InitPins();
	BOARD_BootClockRUN();
	BOARD_InitDebugConsole();

	asm("bkpt 255");	//TODO: insert your application code here
	return 0;
}