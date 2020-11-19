/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef _SCREEN_H
#define _SCREEN_H

/* Enumration for line on the screen */
typedef enum
{
    L0 = 0,
    L1 = 18,
    L2 = 36,
    L3 = 54
} LINE_NUM;

void screen_print(char* str, LINE_NUM line);
void screen_printn(const char* str, unsigned int str_length, LINE_NUM line);

#endif // _SCREEN_H