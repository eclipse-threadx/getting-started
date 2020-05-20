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

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "asc_security_core/utils/network_utils.h"

#define AF_INET 2
#define AF_INET6 3

static const char *inet_ntop(int af, const void *a0, char *s, uint32_t l);
static int isdigit(int c);
static int hexval(unsigned c);
static int inet_pton(int af, const char *s, void *a0);

const char* network_utils_inet_ntop(NETWORK_PROTOCOL network_protocol, const void* source, char* destination, uint32_t size) {
    int af = network_protocol == NETWORK_PROTOCOL_IPV4 ? AF_INET : AF_INET6;
    return inet_ntop(af, source, destination, size);
}

int network_utils_inet_pton(NETWORK_PROTOCOL network_protocol, const char* source, void* destination) {
    int af = network_protocol == NETWORK_PROTOCOL_IPV4 ? AF_INET : AF_INET6;
    return inet_pton(af, source, destination);
}

// https://github.com/bpowers/musl/blob/master/src/network/inet_ntop.c
static const char *inet_ntop(int af, const void *a0, char *s, uint32_t l) {
    const unsigned char *a = a0;
    size_t i, j, max, best;
    char buf[100];

    switch (af) {
    case AF_INET:
        if (snprintf(s, l, "%d.%d.%d.%d", a[0],a[1],a[2],a[3]) < (int)l)
            return s;
        break;
    case AF_INET6:
        if (memcmp(a, "\0\0\0\0\0\0\0\0\0\0\377\377", 12))
            snprintf(buf, sizeof buf,
                "%x:%x:%x:%x:%x:%x:%x:%x",
                256*a[0]+a[1],256*a[2]+a[3],
                256*a[4]+a[5],256*a[6]+a[7],
                256*a[8]+a[9],256*a[10]+a[11],
                256*a[12]+a[13],256*a[14]+a[15]);
        else
            snprintf(buf, sizeof buf,
                "%x:%x:%x:%x:%x:%x:%d.%d.%d.%d",
                256*a[0]+a[1],256*a[2]+a[3],
                256*a[4]+a[5],256*a[6]+a[7],
                256*a[8]+a[9],256*a[10]+a[11],
                a[12],a[13],a[14],a[15]);
        /* Replace longest /(^0|:)[:0]{2,}/ with "::" */
        for (i=best=0, max=2; buf[i]; i++) {
            if (i && buf[i] != ':') continue;
            j = strspn(buf+i, ":0");
            if (j>max) best=i, max=j;
        }
        if (max>2) {
            buf[best] = buf[best+1] = ':';
            memmove(buf+best+2, buf+best+max, i-best-max+1);
        }
        if (strlen(buf) < l) {
            strcpy(s, buf);
            return s;
        }
        break;
    default:
        return 0;
    }
    return 0;
}

// https://github.com/bpowers/musl/blob/master/src/ctype/isdigit.c
static int isdigit(int c) {
	return (unsigned)c-'0' < 10;
}

// https://github.com/bpowers/musl/blob/master/src/network/inet_pton.c
static int hexval(unsigned c) {
    if (c-'0'<10) return (int)(c-'0');
    c |= 32;
    if (c-'a'<6) return (int)(c-'a'+10);
    return -1;
}

// https://github.com/bpowers/musl/blob/master/src/network/inet_pton.c
static int inet_pton(int af, const char *s, void *a0) {
    uint16_t ip[8];
    unsigned char *a = a0;
    int i, j, v, d, brk=-1, need_v4=0;

    if (af==AF_INET) {
        for (i=0; i<4; i++) {
            for (v=j=0; j<3 && isdigit(s[j]); j++)
                v = 10*v + s[j]-'0';
            if (j==0 || (j>1 && s[0]=='0') || v>255) return 0;
            a[i] = (unsigned char)v;
            if (s[j]==0 && i==3) return 1;
            if (s[j]!='.') return 0;
            s += j+1;
        }
        return 0;
    } else if (af!=AF_INET6) {
        return -1;
    }

    if (*s==':' && *++s!=':') return 0;

    for (i=0; ; i++) {
        if (s[0]==':' && brk<0) {
            brk=i;
            ip[i&7]=0;
            if (!*++s) break;
            if (i==7) return 0;
            continue;
        }
        for (v=j=0; j<4 && (d=hexval((unsigned int)s[j]))>=0; j++)
            v=16*v+d;
        if (j==0) return 0;
        ip[i&7] = (unsigned char)v;
        if (!s[j] && (brk>=0 || i==7)) break;
        if (i==7) return 0;
        if (s[j]!=':') {
            if (s[j]!='.' || (i<6 && brk<0)) return 0;
            need_v4=1;
            i++;
            break;
        }
        s += j+1;
    }
    if (brk>=0) {
        memmove(ip+brk+7-i, ip+brk, (size_t)(2*(i+1-brk)));
        for (j=0; j<7-i; j++) ip[brk+j] = 0;
    }
    for (j=0; j<8; j++) {
        *a++ = (unsigned char)(ip[j]>>8);
        *a++ = (unsigned char)(ip[j]);
    }
    if (need_v4 && inet_pton(AF_INET, (void *)s, a-4) <= 0) return 0;
    return 1;
}
