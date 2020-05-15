/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */
   
#include <sys/stat.h>
#include <errno.h>

#include "sntp_client.h"

extern int errno;

register char * stack_ptr asm("sp");

int _gettimeofday(struct timeval* tp, void* tzvp);
int _close(int file);
int _fstat(int file, struct stat *st);
int _isatty(int file);
int _lseek(int file, int ptr, int dir);

int _gettimeofday(struct timeval* tp, void* tzvp) 
{
    tp->tv_sec = sntp_get_time();
    tp->tv_usec = 0;
    return 0;
}

int _close(int file)
{
	return -1;
}

int _fstat(int file, struct stat *st)
{
	st->st_mode = S_IFCHR;
	return 0;
}

int _isatty(int file)
{
	return 1;
}

int _lseek(int file, int ptr, int dir)
{
	return 0;
}
