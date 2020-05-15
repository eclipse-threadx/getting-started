/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */
   
#ifndef _SNTP_CLIENT_H
#define _SNTP_CLIENT_H

#include <stdbool.h>

unsigned long sntp_get_time();
bool sntp_wait_for_sync();
bool sntp_start();

#endif // _SNTP_CLIENT_H