#ifndef HAVE_IP2LOCATION_H
#define HAVE_IP2LOCATION_H

#include <stdio.h>
#include <stdlib.h> 

#include "ip2lipaddress.h"
#include "ip2ldatabase.h"

#define API_VERSION     4.0.0
#define MAX_IPV4_RANGE  0xFFFFFFFFU
#define IPV4 0
#define IPV6 1

#define  COUNTRYSHORT           0x00001
#define  COUNTRYLONG            0x00002
#define  REGION                 0x00004
#define  CITY                   0x00008
#define  ISP                    0x00010
#define  LATITUDE               0x00020
#define  LONGITUDE              0x00040
#define  DOMAIN                 0x00080
#define  ZIPCODE                0x00100
#define  TIMEZONE               0x00200
#define  NETSPEED               0x00400
#define  IDDCODE                0x00800
#define  AREACODE               0x01000
#define  WEATHERSTATIONCODE     0x02000
#define  WEATHERSTATIONNAME     0x04000
#define  MCC                    0x08000
#define  MNC                    0x10000
#define  MOBILEBRAND            0x20000
#define  ELEVATION              0x40000
#define  USAGETYPE              0x80000

#define  ALL COUNTRYSHORT | COUNTRYLONG | REGION | CITY | ISP | LATITUDE | LONGITUDE | DOMAIN | ZIPCODE | TIMEZONE | NETSPEED | IDDCODE | AREACODE | WEATHERSTATIONCODE | WEATHERSTATIONNAME | MCC | MNC | MOBILEBRAND | ELEVATION | USAGETYPE

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
uint32_t IP2LocationClose(IP2Location *loc);
IP2LocationRecord *IP2LocationQuery(IP2Location *loc, char *ip, uint32_t mode);
void IP2LocationFreeRecord(IP2LocationRecord *record);
int IP2LocationDeleteShared(IP2Location *loc);

#endif
