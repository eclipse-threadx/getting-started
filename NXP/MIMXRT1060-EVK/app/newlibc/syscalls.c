/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */
   
#include <stdio.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "sntp_client.h"

extern int errno;

register char *stack_ptr asm("sp");

caddr_t _sbrk(int incr);
int _close(int file);
int _fstat(int file, struct stat *st);
int _isatty(int file);
int _lseek(int file, int ptr, int dir);
void _exit(int status);
void _kill(int pid, int sig);
int _getpid(void);
int _gettimeofday(struct timeval *tp, void *tzvp);

caddr_t _sbrk(int incr)
{
    //printf("requesting heap %d\r\n", incr);
    extern char end asm("end");
    extern char heap_limit asm("__heap_limit");

    static char *heap_end = 0;

    // initialise the first time
    if (heap_end == 0)
    {
        heap_end = &end;
    }

    int delta = heap_end - &heap_limit + incr;
    if (delta >= 0)
    {
        errno = ENOMEM;
        return (caddr_t) - 1;
    }

    char *prev_heap_end = heap_end;

    heap_end += incr;

    return (caddr_t)prev_heap_end;
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

void _exit(int status)
{
    printf("Exiting with status %d.\n", status);
    while (1);
}

void _kill(int pid, int sig)
{
    (void)pid;
    (void)sig;
    return;
}

int _getpid(void)
{
    return -1;
}

int _gettimeofday(struct timeval *tp, void *tzvp)
{
    tp->tv_sec = sntp_get_time();
    tp->tv_usec = 0;
    return 0;
}
