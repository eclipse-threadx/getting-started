#include <stdio.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "sntp_client.h"

int _gettimeofday(struct timeval* tp, void* tzvp);

int _gettimeofday(struct timeval* tp, void* tzvp) 
{
    tp->tv_sec = sntp_get_time();
    tp->tv_usec = 0;
    return 0;
}

