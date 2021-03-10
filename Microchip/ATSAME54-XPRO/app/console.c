/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */
   
#include <stdio_io.h>

#ifdef __GNUC__
int _read(int file, char *ptr, int len)
#elif __ICCARM__
size_t __read(int file, unsigned char *ptr, size_t len)
#else
#error unknown compiler
#endif
{
	int n = 0;

	if (file != 0) {
		return -1;
	}

	n = stdio_io_read((uint8_t *)ptr, len);
	if (n < 0) {
		return -1;
	}

	return n;
}

#ifdef __GNUC__
int _write(int file, char *ptr, int len)
#elif __ICCARM__
size_t __write(int file, unsigned char *ptr, size_t len)
#else
#error unknown compiler
#endif
{
	int n = 0;

	if ((file != 1) && (file != 2) && (file != 3)) {
		return -1;
	}

	n = stdio_io_write((const uint8_t *)ptr, len);
	if (n < 0) {
		return -1;
	}

	return n;
}
