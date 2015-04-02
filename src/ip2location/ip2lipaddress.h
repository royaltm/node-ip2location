#ifndef HAVE_IP2LOC_IPADDRESS_H
#define HAVE_IP2LOC_IPADDRESS_H

#include <stdint.h>
#include "inet_pton.h"

typedef struct ipv6le128 {
  union {
    struct {
      uint32_t addr;
      uint32_t pad[3];
    } ipv4;
    uint32_t ui32[4];
    uint8_t ui8[16];
  };
} ipv6le128_t;


#define IP2L_ULONG128_DECIMAL_SIZE 40

int IP2LocationIPv6Cmp(const ipv6le128_t *a, const ipv6le128_t *b);
int IP2LocationIP2No(const char* ipaddr, ipv6le128_t *ipv6);
int IP2LocationIPBin2No(void *ipaddr, uint32_t ipsize, ipv6le128_t *ip);
int IP2LocationIPv4Str(const ipv6le128_t *ip, char out[16]);
int IP2LocationULong128ToDecimal(const uint32_t bint[4], char out[IP2L_ULONG128_DECIMAL_SIZE]);

#endif /* HAVE_IP2LOC_IPADDRESS_H */
