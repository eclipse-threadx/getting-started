/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef _SCREEN_H
#define _SCREEN_H

#include <_ansi.h>

_BEGIN_STD_C

/* Enumration for line on the screen */
typedef enum
{
    L0 = 0,
    L1 = 18,
    L2 = 36,
    L3 = 54
} LINE_NUM;

void screen_print(char* str, LINE_NUM line);

_END_STD_C

#endif // _SCREEN_H