#include "ip2lipaddress.h"

/* on a LSB (LE) machine this does nothing */
void IP2LocationIPv6LETo128(ipv6le128_t *a) {
  (*a).ui64[1] = le64toh((*a).ui64[1]);
  (*a).ui64[0] = le64toh((*a).ui64[0]);
}

int IP2LocationIPv6Cmp(ipv6le128_t *a, ipv6le128_t *b) {
  uint64_t x, y;
  if ((x = (*a).ui64[1]) == (y = (*b).ui64[1])) {
    if ((x = (*a).ui64[0]) == (y = (*b).ui64[0])) {
      return 0;
    } else if (x > y) {
      return 1;
    } else {
      return -1;
    }
  } else if (x > y) {
    return 1;
  } else {
    return -1;
  }
}

int IP2LocationIPv6To128(char *ipaddr, ipv6le128_t *ipv6) {
  uint64_t buff[2];
  if (inet_pton(AF_INET6, ipaddr, (void *)&buff) == 1) {
    (*ipv6).ui64[0] = be64toh(buff[1]);
    (*ipv6).ui64[1] = be64toh(buff[0]);
    return 1;
  }
  return 0;
}

int IP2LocationIP2No(char* ipaddr, uint32_t *ip) {
  uint32_t tmp;
  if (inet_pton(AF_INET, ipaddr, (void *)&tmp) == 1) {
    *ip = be32toh(tmp);
    return 1;
  }
  return 0;
}
