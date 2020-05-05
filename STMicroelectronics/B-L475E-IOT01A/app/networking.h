#ifndef _NETWORKING_H
#define _NETWORKING_H

#include "nx_api.h"
#include "nxd_dns.h"

extern NX_IP ip_0;
extern NX_PACKET_POOL main_pool;
extern NX_DNS dns_client;

int threadx_net_init(void);

#endif // _NETWORKING_H