/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */
   
#include <stdio.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

extern int errno;
extern int _end;

caddr_t _sbrk(int incr);
int _close(int file);
int _fstat(int file, struct stat *st);
int _isatty(int file);
int _lseek(int file, int ptr, int dir);
void _exit(int status);
void _kill(int pid, int sig);
int _getpid(void);

caddr_t _sbrk(int incr)
{
    static unsigned char* heap = NULL;
    unsigned char*        prev_heap;

    if (heap == NULL) 
    {
        heap = (unsigned char*)&_end;
    }
    prev_heap = heap;

    heap += incr;

    return (caddr_t)prev_heap;
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