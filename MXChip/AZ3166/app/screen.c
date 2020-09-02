/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "screen.h"
#include "ssd1306.h"

void screen_print(char* str, LINE_NUM line)
{
    ssd1306_Fill(Black);
    ssd1306_SetCursor(2, line);
    ssd1306_WriteString(str, Font_11x18, White);
    ssd1306_UpdateScreen();
}