#include "ip2location.h"

uint8_t COUNTRY_POSITION[25]             = {0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};
uint8_t REGION_POSITION[25]              = {0, 0, 0, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
uint8_t CITY_POSITION[25]                = {0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4};
uint8_t ISP_POSITION[25]                 = {0, 0, 3, 0, 5, 0, 7, 5, 7, 0, 8, 0, 9, 0, 9, 0, 9, 0, 9, 7, 9, 0, 9, 7, 9};
uint8_t LATITUDE_POSITION[25]            = {0, 0, 0, 0, 0, 5, 5, 0, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5};
uint8_t LONGITUDE_POSITION[25]           = {0, 0, 0, 0, 0, 6, 6, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6};
uint8_t DOMAIN_POSITION[25]              = {0, 0, 0, 0, 0, 0, 0, 6, 8, 0, 9, 0, 10,0, 10, 0, 10, 0, 10, 8, 10, 0, 10, 8, 10};
uint8_t ZIPCODE_POSITION[25]             = {0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 7, 7, 0, 7, 7, 7, 0, 7, 0, 7, 7, 7, 0, 7};
uint8_t TIMEZONE_POSITION[25]            = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 8, 7, 8, 8, 8, 7, 8, 0, 8, 8, 8, 0, 8};
uint8_t NETSPEED_POSITION[25]            = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 11,0, 11,8, 11, 0, 11, 0, 11, 0, 11};
uint8_t IDDCODE_POSITION[25]             = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 12, 0, 12, 0, 12, 9, 12, 0, 12};
uint8_t AREACODE_POSITION[25]            = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10 ,13 ,0, 13, 0, 13, 10, 13, 0, 13};
uint8_t WEATHERSTATIONCODE_POSITION[25]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 14, 0, 14, 0, 14, 0, 14};
uint8_t WEATHERSTATIONNAME_POSITION[25]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 15, 0, 15, 0, 15, 0, 15};
uint8_t MCC_POSITION[25]                 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 16, 0, 16, 9, 16};
uint8_t MNC_POSITION[25]                 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10,17, 0, 17, 10, 17};
uint8_t MOBILEBRAND_POSITION[25]         = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11,18, 0, 18, 11, 18};
uint8_t ELEVATION_POSITION[25]           = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 19, 0, 19};
uint8_t USAGETYPE_POSITION[25]           = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 20};

int IP2LocationInit(IP2Location *loc);
IP2LocationRecord *IP2LocationCreateRecord();
IP2LocationRecord *IP2LocationQueryIPv6(IP2Location *loc, char *ipaddr, uint32_t mode);
IP2LocationRecord *IP2LocationQueryIPv4(IP2Location *loc, char *ipaddr, uint32_t mode);

void IP2LocationFree(IP2Location *loc) {
  free(loc->filename);
  free(loc);
}

IP2Location *IP2LocationOpen(char *db, IP2LOCATION_ACCESS_TYPE mtype, char *shared_name) {
  FILE *filehandle;
  IP2Location *loc;
  int initFailed = 0;

  if ( ( filehandle = fopen(db, "rb")) == NULL ) {
    return NULL;
  }

  loc = (IP2Location *) malloc(sizeof(IP2Location));
  memset(loc, 0, sizeof(IP2Location));

  loc->filename = strdup(db);
  loc->cache = NULL;
  loc->mml_node = NULL;

  switch (mtype) {
  case IP2LOCATION_FILE_IO:
    break;
  case IP2LOCATION_FILE_MMAP:
  if ((loc->mml_node = IP2LocationSetupMMap(filehandle, db)) == NULL) {
      initFailed = -1;
    }
    break;
  case IP2LOCATION_SHARED_MEMORY:
    if ((loc->mml_node = IP2LocationSetupShared(filehandle, shared_name)) == NULL) {
      initFailed = -1;
    }
    break;
  case IP2LOCATION_CACHE_MEMORY:
    if ((loc->mml_node = IP2LocationSetupCache(filehandle, db)) == NULL) {
      initFailed = -1;
    }
    break;
  default:
    initFailed = -1;
  }
  if (initFailed == 0) {
    if (loc->mml_node != NULL)
      loc->cache = loc->mml_node->mem_ptr;
    loc->filehandle = filehandle;
    loc->access_type = mtype;
    IP2LocationInit(loc);
  } else {
    fclose(filehandle);
    IP2LocationFree(loc);
    loc = NULL;
  }
  return loc;
}

uint32_t IP2LocationClose(IP2Location *loc) {
  if (loc != NULL) {
    IP2LocationDBClose(loc->filehandle, loc->mml_node);
    IP2LocationFree(loc);
  }
  return 0;
}

int IP2LocationDeleteShared(IP2Location *loc) {
  if (loc != NULL) {
    return IP2LocationDeleteSharedMap(loc->mml_node);
  }
  return 1;
}

int IP2LocationInit(IP2Location *loc) {
  uint8_t *cache = loc->cache;
  loc->databasetype   = IP2LocationRead8(loc->filehandle,  cache, 1);
  loc->databasecolumn = IP2LocationRead8(loc->filehandle,  cache, 2);
  loc->databaseyear   = IP2LocationRead8(loc->filehandle,  cache, 3);
  loc->databasemonth  = IP2LocationRead8(loc->filehandle,  cache, 4);
  loc->databaseday    = IP2LocationRead8(loc->filehandle,  cache, 5);
  loc->databasecount  = IP2LocationRead32(loc->filehandle, cache, 6);
  loc->databaseaddr   = IP2LocationRead32(loc->filehandle, cache, 10);
  loc->ipversion      = IP2LocationRead32(loc->filehandle, cache, 14);
  return 0;
}

IP2LocationRecord *IP2LocationQuery(IP2Location *loc, char *ip, uint32_t mode) {
  if (loc->ipversion == IPV6) {
    return IP2LocationQueryIPv6(loc, ip, mode);
  } else {
    return IP2LocationQueryIPv4(loc, ip, mode);
  }
}

IP2LocationRecord *IP2LocationQueryIPv6(IP2Location *loc, char *ipaddr, uint32_t mode) {
  ipv6le128_t ip6from, ip6to, ip6no;

  if (IP2LocationIPv6To128(ipaddr, &ip6no) != 1) {
    return NULL;
  }

  uint8_t dbtype = loc->databasetype;
  FILE *handle = loc->filehandle;
  uint8_t *cache = loc->cache;
  uint32_t baseaddr = loc->databaseaddr;
  uint32_t dbcount = loc->databasecount;
  uint32_t dbcolumn = loc->databasecolumn;

  uint32_t low = 0;
  uint32_t high = dbcount;
  uint32_t mid = 0;
  int ipcmp;

  IP2LocationRecord *record = IP2LocationCreateRecord();

  while (low <= high) {
    mid = (uint32_t)((low + high)/2);
    IP2LocationRead128Buffer((void *)&ip6from, handle, cache, baseaddr + mid * (dbcolumn * 4 + 12));
    IP2LocationIPv6LETo128(&ip6from);
    IP2LocationRead128Buffer((void *)&ip6to, handle, cache, baseaddr + (mid + 1) * (dbcolumn * 4 + 12));
    IP2LocationIPv6LETo128(&ip6to);

    if (((ipcmp = IP2LocationIPv6Cmp(&ip6no, &ip6from)) >= 0) && (IP2LocationIPv6Cmp(&ip6no, &ip6to) < 0)) {
      if ((mode & COUNTRYSHORT) && (COUNTRY_POSITION[dbtype] != 0)) {
        record->country_short = IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + mid * (dbcolumn * 4 + 12) + 12 + 4 * (COUNTRY_POSITION[dbtype]-1)));
      } else {
        record->country_short = NULL;
      }

      if ((mode & COUNTRYLONG) && (COUNTRY_POSITION[dbtype] != 0)) {
        record->country_long = IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + mid * (dbcolumn * 4 + 12) + 12 + 4 * (COUNTRY_POSITION[dbtype]-1))+3);
      } else {
        record->country_long = NULL;
      }

      if ((mode & REGION) && (REGION_POSITION[dbtype] != 0)) {
        record->region = IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + mid * (dbcolumn * 4 + 12) + 12 + 4 * (REGION_POSITION[dbtype]-1)));
      } else {
        record->region = NULL;
      }

      if ((mode & CITY) && (CITY_POSITION[dbtype] != 0)) {
        record->city = IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + mid * (dbcolumn * 4 + 12) + 12 + 4 * (CITY_POSITION[dbtype]-1)));
      } else {
        record->city = NULL;
      }

      if ((mode & ISP) && (ISP_POSITION[dbtype] != 0)) {
        record->isp = IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + mid * (dbcolumn * 4 + 12) + 12 + 4 * (ISP_POSITION[dbtype]-1)));
      } else {
        record->isp = NULL;
      }

      if ((mode & LATITUDE) && (LATITUDE_POSITION[dbtype] != 0)) {
        record->latitude = IP2LocationReadFloat(handle, cache, baseaddr + mid * (dbcolumn * 4 + 12) + 12 + 4 * (LATITUDE_POSITION[dbtype]-1));
      } else {
        record->latitude = 0.0;
      }

      if ((mode & LONGITUDE) && (LONGITUDE_POSITION[dbtype] != 0)) {
        record->longitude = IP2LocationReadFloat(handle, cache, baseaddr + mid * (dbcolumn * 4 + 12) + 12 + 4 * (LONGITUDE_POSITION[dbtype]-1));
      } else {
        record->longitude = 0.0;
      }

      if ((mode & DOMAIN) && (DOMAIN_POSITION[dbtype] != 0)) {
        record->domain = IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + mid * (dbcolumn * 4 + 12) + 12 + 4 * (DOMAIN_POSITION[dbtype]-1)));
      } else {
        record->domain = NULL;
      }

      if ((mode & ZIPCODE) && (ZIPCODE_POSITION[dbtype] != 0)) {
        record->zipcode = IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + mid * (dbcolumn * 4 + 12) + 12 + 4 * (ZIPCODE_POSITION[dbtype]-1)));
      } else {
        record->zipcode = NULL;
      }

      if ((mode & TIMEZONE) && (TIMEZONE_POSITION[dbtype] != 0)) {
        record->timezone = IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + mid * (dbcolumn * 4 + 12) + 12 + 4 * (TIMEZONE_POSITION[dbtype]-1)));
      } else {
        record->timezone = NULL;
      }

      if ((mode & NETSPEED) && (NETSPEED_POSITION[dbtype] != 0)) {
        record->netspeed = IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + mid * (dbcolumn * 4 + 12) + 12 + 4 * (NETSPEED_POSITION[dbtype]-1)));
      } else {
        record->netspeed = NULL;
      }

      if ((mode & IDDCODE) && (IDDCODE_POSITION[dbtype] != 0)) {
        record->iddcode = IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + mid * (dbcolumn * 4 + 12) + 12 + 4 * (IDDCODE_POSITION[dbtype]-1)));
      } else {
        record->iddcode = NULL;
      }

      if ((mode & AREACODE) && (AREACODE_POSITION[dbtype] != 0)) {
        record->areacode = IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + mid * (dbcolumn * 4 + 12) + 12 + 4 * (AREACODE_POSITION[dbtype]-1)));
      } else {
        record->areacode = NULL;
      }

      if ((mode & WEATHERSTATIONCODE) && (WEATHERSTATIONCODE_POSITION[dbtype] != 0)) {
        record->weatherstationcode = IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + mid * (dbcolumn * 4 + 12) + 12 + 4 * (WEATHERSTATIONCODE_POSITION[dbtype]-1)));
      } else {
        record->weatherstationcode = NULL;
      }

      if ((mode & WEATHERSTATIONNAME) && (WEATHERSTATIONNAME_POSITION[dbtype] != 0)) {
        record->weatherstationname = IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + mid * (dbcolumn * 4 + 12) + 12 + 4 * (WEATHERSTATIONNAME_POSITION[dbtype]-1)));
      } else {
        record->weatherstationname = NULL;
      }

      if ((mode & MCC) && (MCC_POSITION[dbtype] != 0)) {
        record->mcc = IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + mid * (dbcolumn * 4 + 12) + 12 + 4 * (MCC_POSITION[dbtype]-1)));
      } else {
        record->mcc = NULL;
      }

      if ((mode & MNC) && (MNC_POSITION[dbtype] != 0)) {
        record->mnc = IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + mid * (dbcolumn * 4 + 12) + 12 + 4 * (MNC_POSITION[dbtype]-1)));
      } else {
        record->mnc = NULL;
      }

      if ((mode & MOBILEBRAND) && (MOBILEBRAND_POSITION[dbtype] != 0)) {
        record->mobilebrand = IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + mid * (dbcolumn * 4 + 12) + 12 + 4 * (MOBILEBRAND_POSITION[dbtype]-1)));
      } else {
        record->mobilebrand = NULL;
      }

      if ((mode & ELEVATION) && (ELEVATION_POSITION[dbtype] != 0)) {
        record->elevation = IP2LocationReadFloat(handle, cache, baseaddr + mid * (dbcolumn * 4 + 12) + 12 + 4 * (ELEVATION_POSITION[dbtype]-1));
      } else {
        record->elevation = 0.0;
      }

      if ((mode & USAGETYPE) && (USAGETYPE_POSITION[dbtype] != 0)) {
        record->usagetype = IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + mid * (dbcolumn * 4 + 12) + 12 + 4 * (USAGETYPE_POSITION[dbtype]-1)));
      } else {
        record->usagetype = NULL;
      }

      return record;

    } else {
      if ( ipcmp < 0 ) {
        high = mid - 1;
      } else {
        low = mid + 1;
      }
    }
  }

  record->country_short = strdup("-");
  record->country_long = strdup("-");
  record->region = strdup("-");
  record->city = strdup("-");
  record->isp = strdup("-");
  record->latitude = 0;
  record->longitude = 0;
  record->domain = strdup("-");
  record->zipcode = strdup("-");
  record->timezone = strdup("-");
  record->netspeed = strdup("-");
  record->iddcode = strdup("-");
  record->areacode = strdup("-");
  record->weatherstationcode = strdup("-");
  record->weatherstationname = strdup("-");
  record->mcc = strdup("-");
  record->mnc = strdup("-");
  record->mobilebrand = strdup("-");
  record->elevation = 0;
  record->usagetype = strdup("-");
  return record;
}

IP2LocationRecord *IP2LocationQueryIPv4(IP2Location *loc, char *ipaddr, uint32_t mode) {
  uint32_t ipno;
  if (IP2LocationIP2No(ipaddr, &ipno) != 1) {
    return NULL;
  }
  uint8_t dbtype = loc->databasetype;
  FILE *handle = loc->filehandle;
  uint8_t *cache = loc->cache;
  uint32_t baseaddr = loc->databaseaddr;
  uint32_t dbcount = loc->databasecount;
  uint32_t dbcolumn = loc->databasecolumn;

  uint32_t low = 0;
  uint32_t high = dbcount;
  uint32_t mid = 0;
  uint32_t ipfrom = 0;
  uint32_t ipto = 0;

  IP2LocationRecord *record = IP2LocationCreateRecord();
  if (ipno == (uint32_t) MAX_IPV4_RANGE) {
    ipno = ipno - 1;
  }

  while (low <= high) {
    mid = (uint32_t)((low + high)/2);
    ipfrom = IP2LocationRead32(handle, cache, baseaddr + mid * dbcolumn * 4);
    ipto   = IP2LocationRead32(handle, cache, baseaddr + (mid + 1) * dbcolumn * 4);

    if ((ipno >= ipfrom) && (ipno < ipto)) {
      if ((mode & COUNTRYSHORT) && (COUNTRY_POSITION[dbtype] != 0)) {
        record->country_short = IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + (mid * dbcolumn * 4) + 4 * (COUNTRY_POSITION[dbtype]-1)));
      } else {
        record->country_short = NULL;
      }
      
      if ((mode & COUNTRYLONG) && (COUNTRY_POSITION[dbtype] != 0)) {
        record->country_long = IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + (mid * dbcolumn * 4) + 4 * (COUNTRY_POSITION[dbtype]-1))+3);
      } else {
        record->country_long = NULL;
      }

      if ((mode & REGION) && (REGION_POSITION[dbtype] != 0)) {
        record->region = IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + (mid * dbcolumn * 4) + 4 * (REGION_POSITION[dbtype]-1)));
      } else {
        record->region = NULL;
      }

      if ((mode & CITY) && (CITY_POSITION[dbtype] != 0)) {
        record->city = IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + (mid * dbcolumn * 4) + 4 * (CITY_POSITION[dbtype]-1)));
      } else {
        record->city = NULL;
      }

      if ((mode & ISP) && (ISP_POSITION[dbtype] != 0)) {
        record->isp = IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + (mid * dbcolumn * 4) + 4 * (ISP_POSITION[dbtype]-1)));
      } else {
        record->isp = NULL;
      }

      if ((mode & LATITUDE) && (LATITUDE_POSITION[dbtype] != 0)) {
        record->latitude = IP2LocationReadFloat(handle, cache, baseaddr + (mid * dbcolumn * 4) + 4 * (LATITUDE_POSITION[dbtype]-1));
      } else {
        record->latitude = 0.0;
      }

      if ((mode & LONGITUDE) && (LONGITUDE_POSITION[dbtype] != 0)) {
        record->longitude = IP2LocationReadFloat(handle, cache, baseaddr + (mid * dbcolumn * 4) + 4 * (LONGITUDE_POSITION[dbtype]-1));
      } else {
        record->longitude = 0.0;
      }

      if ((mode & DOMAIN) && (DOMAIN_POSITION[dbtype] != 0)) {
        record->domain = IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + (mid * dbcolumn * 4) + 4 * (DOMAIN_POSITION[dbtype]-1)));
      } else {
        record->domain = NULL;
      }

      if ((mode & ZIPCODE) && (ZIPCODE_POSITION[dbtype] != 0)) {
        record->zipcode = IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + (mid * dbcolumn * 4) + 4 * (ZIPCODE_POSITION[dbtype]-1)));
      } else {
        record->zipcode = NULL;
      }
      
      if ((mode & TIMEZONE) && (TIMEZONE_POSITION[dbtype] != 0)) {
        record->timezone = IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + (mid * dbcolumn * 4) + 4 * (TIMEZONE_POSITION[dbtype]-1)));
      } else {
        record->timezone = NULL;
      }
      
      if ((mode & NETSPEED) && (NETSPEED_POSITION[dbtype] != 0)) {
        record->netspeed = IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + (mid * dbcolumn * 4) + 4 * (NETSPEED_POSITION[dbtype]-1)));
      } else {
        record->netspeed = NULL;
      }
  
      if ((mode & IDDCODE) && (IDDCODE_POSITION[dbtype] != 0)) {
        record->iddcode = IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + (mid * dbcolumn * 4) + 4 * (IDDCODE_POSITION[dbtype]-1)));
      } else {
        record->iddcode = NULL;
      }
  
      if ((mode & AREACODE) && (AREACODE_POSITION[dbtype] != 0)) {
        record->areacode = IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + (mid * dbcolumn * 4) + 4 * (AREACODE_POSITION[dbtype]-1)));
      } else {
        record->areacode = NULL;
      }
  
      if ((mode & WEATHERSTATIONCODE) && (WEATHERSTATIONCODE_POSITION[dbtype] != 0)) {
        record->weatherstationcode = IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + (mid * dbcolumn * 4) + 4 * (WEATHERSTATIONCODE_POSITION[dbtype]-1)));
      } else {
        record->weatherstationcode = NULL;
      }
  
      if ((mode & WEATHERSTATIONNAME) && (WEATHERSTATIONNAME_POSITION[dbtype] != 0)) {
        record->weatherstationname = IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + (mid * dbcolumn * 4) + 4 * (WEATHERSTATIONNAME_POSITION[dbtype]-1)));
      } else {
        record->weatherstationname = NULL;
      }

      if ((mode & MCC) && (MCC_POSITION[dbtype] != 0)) {
        record->mcc = IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + (mid * dbcolumn * 4) + 4 * (MCC_POSITION[dbtype]-1)));
      } else {
        record->mcc = NULL;
      }

      if ((mode & MNC) && (MNC_POSITION[dbtype] != 0)) {
        record->mnc = IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + (mid * dbcolumn * 4) + 4 * (MNC_POSITION[dbtype]-1)));
      } else {
        record->mnc = NULL;
      }

      if ((mode & MOBILEBRAND) && (MOBILEBRAND_POSITION[dbtype] != 0)) {
        record->mobilebrand = IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + (mid * dbcolumn * 4) + 4 * (MOBILEBRAND_POSITION[dbtype]-1)));
      } else {
        record->mobilebrand = NULL;
      }
      
      if ((mode & ELEVATION) && (ELEVATION_POSITION[dbtype] != 0)) {
        record->elevation = IP2LocationReadFloat(handle, cache, baseaddr + (mid * dbcolumn * 4) + 4 * (ELEVATION_POSITION[dbtype]-1));
      } else {
        record->elevation = 0.0;
      }
      
      if ((mode & USAGETYPE) && (USAGETYPE_POSITION[dbtype] != 0)) {
        record->usagetype = IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + (mid * dbcolumn * 4) + 4 * (USAGETYPE_POSITION[dbtype]-1)));
      } else {
        record->usagetype = NULL;
      }
      return record;
    } else {
      if ( ipno < ipfrom ) {
        high = mid - 1;
      } else {
        low = mid + 1;
      }
    }
  }
  IP2LocationFreeRecord(record);
  return NULL;
}

IP2LocationRecord *IP2LocationCreateRecord() {
  IP2LocationRecord *record = (IP2LocationRecord *) malloc(sizeof(IP2LocationRecord));
  memset(record, 0, sizeof(IP2LocationRecord));
  return record;
}

void IP2LocationFreeRecord(IP2LocationRecord *record) {
  if (record->city != NULL)
    free(record->city);

  if (record->country_long != NULL)
    free(record->country_long);

  if (record->country_short != NULL)
    free(record->country_short);

  if (record->domain != NULL)
    free(record->domain);

  if (record->isp != NULL)
    free(record->isp);

  if (record->region != NULL)
    free(record->region);

  if (record->zipcode != NULL)
    free(record->zipcode);
    
  if (record->timezone != NULL)
    free(record->timezone);  
    
  if (record->netspeed != NULL)
    free(record->netspeed);  
  
  if (record->iddcode != NULL)
    free(record->iddcode);  

  if (record->areacode != NULL)
    free(record->areacode);  

  if (record->weatherstationcode != NULL)
    free(record->weatherstationcode);  

  if (record->weatherstationname != NULL)
    free(record->weatherstationname);  

  if (record->mcc != NULL)
    free(record->mcc);  

  if (record->mnc != NULL)
    free(record->mnc);  

  if (record->mobilebrand != NULL)
    free(record->mobilebrand);  
  
  if (record->usagetype != NULL)
    free(record->usagetype);  

  free(record);
}
