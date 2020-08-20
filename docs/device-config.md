# TODO

1. Move common serial dialogue into GSG/core instead of within the /app directory within each board.
1. Edit linker script such that there is a region of flash reserved for device configuration information to prevent being overwritten by the program.
1. Auto timeout when user doesn't input anything through serial. Some investigation has been done into making scanf timeout, but the solution might involve an onboard user button press instead of serial console input.
1. Enable the end to end for NXP and MxChip boards.