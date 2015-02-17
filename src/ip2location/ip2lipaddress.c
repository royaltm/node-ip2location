#include "ip2lipaddress.h"

#ifndef WIN32
#  include <sys/socket.h>
#  include <netinet/in.h>
#else
#ifdef WIN32
#  include <Winsock2.h>
#endif
#endif

int IP2LocationIPv6Cmp(ipv6le128_t *a, ipv6le128_t *b)
{
  uint32_t x, y;
  if ((x = (*a).ui32[3]) == (y = (*b).ui32[3]) &&
      (x = (*a).ui32[2]) == (y = (*b).ui32[2]) &&
      (x = (*a).ui32[1]) == (y = (*b).ui32[1]) &&
      (x = (*a).ui32[0]) == (y = (*b).ui32[0]))
    return 0;
  else
    return (x > y) ? 1 : -1;
}

int IP2LocationIP2No(char *ipstr, ipv6le128_t *ip)
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
