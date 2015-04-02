#include "ip2lipaddress.h"

#include <stdio.h>
#include <string.h>

#ifndef WIN32
#  include <sys/socket.h>
#  include <netinet/in.h>
#else
#ifdef WIN32
#  include <Winsock2.h>
#endif
#endif

int IP2LocationIPv6Cmp(const ipv6le128_t *a, const ipv6le128_t *b)
{
  uint32_t x, y;
  if ((x = a->ui32[3]) == (y = b->ui32[3]) &&
      (x = a->ui32[2]) == (y = b->ui32[2]) &&
      (x = a->ui32[1]) == (y = b->ui32[1]) &&
      (x = a->ui32[0]) == (y = b->ui32[0]))
    return 0;
  else
    return (x > y) ? 1 : -1;
}

int IP2LocationIP2No(const char *ipstr, ipv6le128_t *ip)
{
  uint32_t buff[4];
  if (inet_pton(AF_INET, ipstr, (void *)buff) == 1) {

    ip->ipv4.addr = ntohl(buff[0]);

    return 4;

  } else if (inet_pton(AF_INET6, ipstr, (void *)buff) == 1) {

    ip->ui32[0] = ntohl(buff[3]);
    ip->ui32[1] = ntohl(buff[2]);
    ip->ui32[2] = ntohl(buff[1]);
    ip->ui32[3] = ntohl(buff[0]);

    /* IPv4Map */
    if ( ip->ui32[1] == 0x0000FFFFU &&
         ! (ip->ui32[2] | ip->ui32[3]) )
      return 4;

    return 6;

  }

  return 0;
}

int IP2LocationIPBin2No(void *ipaddr, uint32_t ipsize, ipv6le128_t *ip)
{
  switch(ipsize) {
    case 4:
      {
        uint8_t *ipptr = ipaddr;
        ip->ipv4.addr = ((uint32_t)ipptr[0] << 24) |
                        ((uint32_t)ipptr[1] << 16) |
                        ((uint32_t)ipptr[2] << 8) |
                        ((uint32_t)ipptr[3]);
      }
      return 4;
    case 16:
      {
        uint32_t buff[4];
        memcpy(buff, ipaddr, ipsize);
        ip->ui32[0] = ntohl(buff[3]);
        ip->ui32[1] = ntohl(buff[2]);
        ip->ui32[2] = ntohl(buff[1]);
        ip->ui32[3] = ntohl(buff[0]);
      }
      /* IPv4Map */
      if ( ip->ui32[1] == 0x0000FFFFU &&
           ! (ip->ui32[2] | ip->ui32[3]) )
        return 4;

      return 6;
  }

  return 0;
}

int IP2LocationIPv4Str(const ipv6le128_t *ip, char out[16])
{
  return sprintf(out, "%d.%d.%d.%d", ip->ui8[3],
                                     ip->ui8[2],
                                     ip->ui8[1],
                                     ip->ui8[0]);
}

int IP2LocationULong128ToDecimal(const uint32_t bint[4], char out[IP2L_ULONG128_DECIMAL_SIZE])
{
  char buf[IP2L_ULONG128_DECIMAL_SIZE];
  int n = 3, head = sizeof(buf) - 1;
  char *p = out;

  buf[head] = 0;

  do {
    uint32_t segment = bint[n--], mask = 0x80000000;

    for ( ; mask != 0 ; mask >>= 1 ) {
      int i, carry;

      carry = ( (segment & mask) != 0 );

      for ( i = sizeof(buf) - 1; i >= head; --i ) {
        carry = ( (buf[i] += buf[i] + carry) > 9 );

        if ( carry )
          buf[i] -= 10;
      }

      if ( carry )
        buf[--head] = 1;
    }
  } while ( n >= 0 );

  n = sizeof(buf) - head;

  while ( head < (int)sizeof(buf) ) {
    *p++ = buf[head++] + '0';
  }
  *p = 0;

  return n;
}
