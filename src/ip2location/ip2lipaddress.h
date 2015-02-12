#ifndef HAVE_IP2LOC_IPADDRESS_H
#define HAVE_IP2LOC_IPADDRESS_H

#include <stdint.h>
#include "inet_pton.h"

typedef struct ipv6le128 {
  union {
    uint32_t ui32[4];
    uint8_t ui8[16];
  };
} ipv6le128_t;

int IP2LocationIPv6Cmp(ipv6le128_t *a, ipv6le128_t *b);
int IP2LocationIPv6To128(char *ipaddr, ipv6le128_t *ipv6);
int IP2LocationIP2No(char* ipaddr, uint32_t *ip);

#endif /* HAVE_IP2LOC_IPADDRESS_H */