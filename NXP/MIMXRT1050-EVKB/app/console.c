/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */
   
#include "fsl_debug_console.h"

#ifdef __GNUC__
int _read(int file, char *ptr, int len)
#elif __ICCARM__
size_t __read(int file, unsigned char *ptr, size_t len)
#else
#error unknown compiler
#endif
{
	int DataIdx;
	for (DataIdx = 0; DataIdx < len; DataIdx++)
	{
		*ptr++ = GETCHAR();
	}
	return len;
}

#ifdef __GNUC__
int _write(int file, char *ptr, int len)
#elif __ICCARM__
size_t __write(int file, unsigned char *ptr, size_t len)
#else
#error unknown compiler
#endif
{
	int DataIdx;
	for (DataIdx = 0; DataIdx < len; DataIdx++)
	{
		PUTCHAR(*ptr++);
	}
	return len;
}
