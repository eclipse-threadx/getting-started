/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */
  
#include <string.h>
#include <stdio.h>
#include "ssd1306.h"
#include "screen.h"

void screen_print(char* str, LINE_NUM line)
{
  ssd1306_Fill(Black);
  ssd1306_SetCursor(2, line);
  ssd1306_WriteString(str, Font_11x18, White);
  ssd1306_UpdateScreen();
}