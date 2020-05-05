#ifndef _NETWORK_H
#define _NETWORK_H

#include <stdbool.h>

#include "nx_api.h"
#include "nxd_dns.h"

extern NX_PACKET_POOL pool_0;
extern NX_IP ip_0;
extern NX_DNS dns_client;

bool network_init();

#endif // _NETWORK_H
