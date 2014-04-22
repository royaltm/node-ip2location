#ifndef WIN32
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#else
#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#ifndef uint8_t
#define uint8_t unsigned char
#endif

#ifndef int32_t 
#define int32_t int
#endif

#ifndef int64_t
#define int64_t long long int
#endif

#ifndef uint64_t
#define uint64_t unsigned long long int
#endif

#ifndef uint32_t
#ifndef WIN32
#define uint32_t int
#else
#define uint32_t unsigned int
#endif
#endif
#endif

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
