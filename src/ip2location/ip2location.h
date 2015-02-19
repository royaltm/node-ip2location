#ifndef HAVE_IP2LOCATION_H
#define HAVE_IP2LOCATION_H

#include "ip2lipaddress.h"
#include "ip2ldatabase.h"

#define IP2L_NOT_FOUND   0
#define IP2L_DATA_STRING 1
#define IP2L_DATA_FLOAT  2

#define IP2L_DB_POSITION_COUNT 19

typedef enum {
  IP2L_COUNTRY_SHORT_INDEX      =  0,
  IP2L_COUNTRY_LONG_INDEX       =  1,
  IP2L_REGION_INDEX             =  2,
  IP2L_CITY_INDEX               =  3,
  IP2L_ISP_INDEX                =  4,
  IP2L_LATITUDE_INDEX           =  5,
  IP2L_LONGITUDE_INDEX          =  6,
  IP2L_DOMAIN_INDEX             =  7,
  IP2L_ZIPCODE_INDEX            =  8,
  IP2L_TIMEZONE_INDEX           =  9,
  IP2L_NETSPEED_INDEX           = 10,
  IP2L_IDDCODE_INDEX            = 11,
  IP2L_AREACODE_INDEX           = 12,
  IP2L_WEATHERSTATIONCODE_INDEX = 13,
  IP2L_WEATHERSTATIONNAME_INDEX = 14,
  IP2L_MCC_INDEX                = 15,
  IP2L_MNC_INDEX                = 16,
  IP2L_MOBILEBRAND_INDEX        = 17,
  IP2L_ELEVATION_INDEX          = 18,
  IP2L_USAGETYPE_INDEX          = 19,
  IP2L_INDEX_MAX                = IP2L_USAGETYPE_INDEX
} IP2LOCATION_DATA_INDEX;

#define IP2L_MASK(index)             (1U << (index))

#define IP2L_COUNTRY_SHORT_MASK      IP2L_MASK(IP2L_COUNTRY_SHORT_INDEX)
#define IP2L_COUNTRY_LONG_MASK       IP2L_MASK(IP2L_COUNTRY_LONG_INDEX)
#define IP2L_REGION_MASK             IP2L_MASK(IP2L_REGION_INDEX)
#define IP2L_CITY_MASK               IP2L_MASK(IP2L_CITY_INDEX)
#define IP2L_ISP_MASK                IP2L_MASK(IP2L_ISP_INDEX)
#define IP2L_LATITUDE_MASK           IP2L_MASK(IP2L_LATITUDE_INDEX)
#define IP2L_LONGITUDE_MASK          IP2L_MASK(IP2L_LONGITUDE_INDEX)
#define IP2L_DOMAIN_MASK             IP2L_MASK(IP2L_DOMAIN_INDEX)
#define IP2L_ZIPCODE_MASK            IP2L_MASK(IP2L_ZIPCODE_INDEX)
#define IP2L_TIMEZONE_MASK           IP2L_MASK(IP2L_TIMEZONE_INDEX)
#define IP2L_NETSPEED_MASK           IP2L_MASK(IP2L_NETSPEED_INDEX)
#define IP2L_IDDCODE_MASK            IP2L_MASK(IP2L_IDDCODE_INDEX)
#define IP2L_AREACODE_MASK           IP2L_MASK(IP2L_AREACODE_INDEX)
#define IP2L_WEATHERSTATIONCODE_MASK IP2L_MASK(IP2L_WEATHERSTATIONCODE_INDEX)
#define IP2L_WEATHERSTATIONNAME_MASK IP2L_MASK(IP2L_WEATHERSTATIONNAME_INDEX)
#define IP2L_MCC_MASK                IP2L_MASK(IP2L_MCC_INDEX)
#define IP2L_MNC_MASK                IP2L_MASK(IP2L_MNC_INDEX)
#define IP2L_MOBILEBRAND_MASK        IP2L_MASK(IP2L_MOBILEBRAND_INDEX)
#define IP2L_ELEVATION_MASK          IP2L_MASK(IP2L_ELEVATION_INDEX)
#define IP2L_USAGETYPE_MASK          IP2L_MASK(IP2L_USAGETYPE_INDEX)

typedef enum {
  IP2LOCATION_FILE_IO,
  IP2LOCATION_FILE_MMAP,
  IP2LOCATION_SHARED_MEMORY,
  IP2LOCATION_CACHE_MEMORY,
} IP2LOCATION_ACCESS_TYPE;

typedef struct {
  char *filename;
  FILE *filehandle;
  size_t filesize;
  uint8_t *cache;
  IP2LMemoryMapList *mml_node;
  uint32_t databasecount;
  uint32_t databaseaddr;
  uint32_t v6databasecount;
  uint32_t v6databaseaddr;
  uint32_t mode_mask;
  IP2LOCATION_ACCESS_TYPE access_type;
  uint8_t databasetype;
  uint8_t databasecolumn;
  uint8_t databaseday;
  uint8_t databasemonth;
  uint8_t databaseyear;
  uint8_t offsets[IP2L_DB_POSITION_COUNT];
} IP2Location;

/* public methods */

IP2Location *IP2LocationOpen(char *db, IP2LOCATION_ACCESS_TYPE mtype, char *shared_name);
void IP2LocationClose(IP2Location *loc);
int IP2LocationDeleteShared(IP2Location *loc);
uint32_t IP2LocationFindRow(IP2Location *loc, char *ip);
uint32_t IP2LocationFindRowIPV6(IP2Location *loc, ipv6le128_t *ip6no);
uint32_t IP2LocationFindRowIPV4(IP2Location *loc, uint32_t ipno);
int IP2LocationRowData(IP2Location *loc,
                       IP2LOCATION_DATA_INDEX index,
                       uint32_t rowoffset,
                       const void *data[]);
int IP2LocationRowString(IP2Location *loc,
                                IP2LOCATION_DATA_INDEX index,
                                uint32_t rowoffset,
                                char * const buff);
int IP2LocationDBhasIPV6(IP2Location *log);

#endif /* HAVE_IP2LOCATION_H */