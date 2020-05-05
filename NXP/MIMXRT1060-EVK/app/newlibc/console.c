#include "fsl_debug_console.h"

__attribute__((weak)) int _read(int file, char *ptr, int len)
{
	int DataIdx;
	for (DataIdx = 0; DataIdx < len; DataIdx++)
	{
		*ptr++ = GETCHAR();
	}
	return len;
}

__attribute__((weak)) int _write(int file, char *ptr, int len)
{
	int DataIdx;
	for (DataIdx = 0; DataIdx < len; DataIdx++)
	{
		PUTCHAR(*ptr++);
	}
	return len;
}
