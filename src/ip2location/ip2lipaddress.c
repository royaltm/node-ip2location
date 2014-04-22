#include "ip2lipaddress.h"

int IP2LocationIPv6Cmp(ipv6le128_t *a, ipv6le128_t *b) {
  uint32_t x, y;
  if ((x = (*a).ui32[3]) == (y = (*b).ui32[3]) &&
      (x = (*a).ui32[2]) == (y = (*b).ui32[2]) &&
      (x = (*a).ui32[1]) == (y = (*b).ui32[1]) &&
      (x = (*a).ui32[0]) == (y = (*b).ui32[0]))
    return 0;
  else
    return (x > y) ? 1 : -1;
}

int IP2LocationIPv6To128(char *ipaddr, ipv6le128_t *ipv6) {
  uint32_t buff[4];
  if (inet_pton(AF_INET6, ipaddr, (void *)&buff) == 1) {
    (*ipv6).ui32[0] = ntohl(buff[3]);
    (*ipv6).ui32[1] = ntohl(buff[2]);
    (*ipv6).ui32[2] = ntohl(buff[1]);
    (*ipv6).ui32[3] = ntohl(buff[0]);
    return 1;
  }
  return 0;
}

int IP2LocationIP2No(char* ipaddr, uint32_t *ip) {
  uint32_t tmp;
  if (inet_pton(AF_INET, ipaddr, (void *)&tmp) == 1) {
    *ip = ntohl(tmp);
    return 1;
  }
  return 0;
}
