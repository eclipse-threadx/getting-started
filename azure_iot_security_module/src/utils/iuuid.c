/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "asc_security_core/utils/irand.h"
#include "asc_security_core/utils/iuuid.h"

#define UUID4_LEN 37

static bool _initialized = false;
static uint64_t _xorshift128plus(uint64_t *s);

static uint64_t _seed[2];


int iuuid_generate(char* buf) {
    if (!_initialized) {
        _seed[0] = (uint64_t)irand_int() << 32 | irand_int();
        _seed[1] = (uint64_t)irand_int() << 32 | irand_int();

        _initialized = true;
    }

    static const char *template = "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx";
    static const char *chars = "0123456789abcdef";
    union {
        unsigned char b[16];
        uint64_t word[2];
    } s;

    const char *p;
    int i, n;

    // get random
    s.word[0] = _xorshift128plus(_seed);
    s.word[1] = _xorshift128plus(_seed);

    // build string
    p = template;
    i = 0;

    while (*p) {
        n = s.b[i >> 1];
        n = (i & 1) ? (n >> 4) : (n & 0xf);

        switch (*p) {
            case 'x':
                *buf = chars[n];
                i++;
                break;
            case 'y':
                *buf = chars[(n & 0x3) + 8];
                i++;
                break;
            default:
                *buf = *p;
        }

        buf++;
        p++;
    }
    *buf = '\0';

    return 0;
}

static uint64_t _xorshift128plus(uint64_t *s) {
    // http://xorshift.di.unimi.it/xorshift128plus.c
    uint64_t s1 = s[0];
    const uint64_t s0 = s[1];
    s[0] = s0;
    s1 ^= s1 << 23;
    s[1] = s1 ^ s0 ^ (s1 >> 18) ^ (s0 >> 5);
    return s[1] + s0;
}