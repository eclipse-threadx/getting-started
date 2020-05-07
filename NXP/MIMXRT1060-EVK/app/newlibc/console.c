#include "fsl_debug_console.h"

int _read(int file, char *ptr, int len);
int _write(int file, char *ptr, int len);

int _read(int file, char *ptr, int len)
{
	int DataIdx;
	for (DataIdx = 0; DataIdx < len; DataIdx++)
	{
		*ptr++ = GETCHAR();
	}
	return len;
}

int _write(int file, char *ptr, int len)
{
	int DataIdx;
	for (DataIdx = 0; DataIdx < len; DataIdx++)
	{
		PUTCHAR(*ptr++);
	}
	return len;
}
