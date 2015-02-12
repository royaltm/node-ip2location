#ifndef HAVE_IP2LOCATION_H
#define HAVE_IP2LOCATION_H

#include "ip2lipaddress.h"
#include "ip2ldatabase.h"
#include "ip2locationdict.h"

#define IP2L_API_VERSION     4.0.0
#define IP2L_MAX_IPV4_RANGE  0xFFFFFFFFU
#define IP2L_IPV4 0
#define IP2L_IPV6 1

#define IP2L_COUNTRYSHORT           0x00001
#define IP2L_COUNTRYLONG            0x00002
#define IP2L_REGION                 0x00004
#define IP2L_CITY                   0x00008
#define IP2L_ISP                    0x00010
#define IP2L_LATITUDE               0x00020
#define IP2L_LONGITUDE              0x00040
#define IP2L_DOMAIN                 0x00080
#define IP2L_ZIPCODE                0x00100
#define IP2L_TIMEZONE               0x00200
#define IP2L_NETSPEED               0x00400
#define IP2L_IDDCODE                0x00800
#define IP2L_AREACODE               0x01000
#define IP2L_WEATHERSTATIONCODE     0x02000
#define IP2L_WEATHERSTATIONNAME     0x04000
#define IP2L_MCC                    0x08000
#define IP2L_MNC                    0x10000
#define IP2L_MOBILEBRAND            0x20000
#define IP2L_ELEVATION              0x40000
#define IP2L_USAGETYPE              0x80000

#define IP2L_ALL (IP2L_COUNTRYSHORT | IP2L_COUNTRYLONG | IP2L_REGION | IP2L_CITY | IP2L_ISP | IP2L_LATITUDE | IP2L_LONGITUDE | IP2L_DOMAIN | IP2L_ZIPCODE | IP2L_TIMEZONE | IP2L_NETSPEED | IP2L_IDDCODE | IP2L_AREACODE | IP2L_WEATHERSTATIONCODE | IP2L_WEATHERSTATIONNAME | IP2L_MCC | IP2L_MNC | IP2L_MOBILEBRAND | IP2L_ELEVATION | IP2L_USAGETYPE)

typedef enum IP2LocationAccessType {
  IP2LOCATION_FILE_IO,
  IP2LOCATION_FILE_MMAP,
  IP2LOCATION_SHARED_MEMORY,
  IP2LOCATION_CACHE_MEMORY,
} IP2LOCATION_ACCESS_TYPE;

typedef struct {
  char *filename;
  FILE *filehandle;
  uint8_t *cache;
  IP2LMemoryMapList *mml_node;
  IP2LOCATION_ACCESS_TYPE access_type;
  uint8_t databasetype;
  uint8_t databasecolumn;
  uint8_t databaseday;
  uint8_t databasemonth;
  uint8_t databaseyear;
  uint32_t databasecount;
  uint32_t databaseaddr;
  uint32_t ipversion;
} IP2Location;

typedef struct {
  char *country_short;
  char *country_long;
  char *region;
  char *city;
  char *isp;
  float latitude;
  float longitude;
  char *domain;
  char *zipcode;
  char *timezone;
  char *netspeed;
  char *iddcode;
  char *areacode;
  char *weatherstationcode;
  char *weatherstationname;
  char *mcc;
  char *mnc;
  char *mobilebrand;
  float elevation;
  char *usagetype;
} IP2LocationRecord;

/* public methods */

IP2Location *IP2LocationOpen(char *db, IP2LOCATION_ACCESS_TYPE mtype, char *shared_name);
void IP2LocationClose(IP2Location *loc);
IP2LocationRecord *IP2LocationQuery(IP2Location *loc, char *ip, uint32_t mode);
void IP2LocationFreeRecord(IP2LocationRecord *record);
int IP2LocationDeleteShared(IP2Location *loc);
int IP2LocationMakeSimpleDictionary(IP2Location *loc, const char *filename, uint32_t mode);
int IP2LocationMakeDictionary(IP2Location *loc, char *dir);

#endif /* HAVE_IP2LOCATION_H */