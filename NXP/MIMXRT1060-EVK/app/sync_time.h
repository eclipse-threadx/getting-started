#ifndef _TIME_C
#define _TIME_C

#include <stdbool.h>

bool sntp_init();
void sntp_deinit();

bool sntp_sync();

#endif // _TIME_C