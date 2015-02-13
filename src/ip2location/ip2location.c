#include "ip2location.h"
#include <string.h>
#include <sys/stat.h>

#define IP2L_MAX_DBTYPE 24
#define IP2L_MAX_DBCOLUMN 20
#define IP2L_BASEADDR 65

static const uint8_t IP2L_DB_POSITIONS[IP2L_DB_POSITION_COUNT][25] = {
  {0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, // COUNTRY_POSITION[25]
  {0, 0, 0, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3}, // REGION_POSITION[25]
  {0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4}, // CITY_POSITION[25]
  {0, 0, 3, 0, 5, 0, 7, 5, 7, 0, 8, 0, 9, 0, 9, 0, 9, 0, 9, 7, 9, 0, 9, 7, 9}, // ISP_POSITION[25]
  {0, 0, 0, 0, 0, 5, 5, 0, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5}, // LATITUDE_POSITION[25]
  {0, 0, 0, 0, 0, 6, 6, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6}, // LONGITUDE_POSITION[25]
  {0, 0, 0, 0, 0, 0, 0, 6, 8, 0, 9, 0, 10,0, 10,0, 10, 0, 10, 8, 10, 0, 10, 8, 10}, // DOMAIN_POSITION[25]
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 7, 7, 0, 7, 7, 7, 0, 7, 0, 7, 7, 7, 0, 7}, // ZIPCODE_POSITION[25]
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 8, 7, 8, 8, 8, 7, 8, 0, 8, 8, 8, 0, 8}, // TIMEZONE_POSITION[25]
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 11,0, 11,8, 11, 0, 11, 0, 11, 0, 11}, // NETSPEED_POSITION[25]
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 12, 0, 12, 0, 12, 9, 12, 0, 12}, // IDDCODE_POSITION[25]
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10 ,13 ,0, 13, 0, 13, 10, 13, 0, 13}, // AREACODE_POSITION[25]
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 14, 0, 14, 0, 14, 0, 14}, // WEATHERSTATIONCODE_POSITION[25]
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 15, 0, 15, 0, 15, 0, 15}, // WEATHERSTATIONNAME_POSITION[25]
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 16, 0, 16, 9, 16}, // MCC_POSITION[25]
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10,17, 0, 17, 10, 17}, // MNC_POSITION[25]
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11,18, 0, 18, 11, 18}, // MOBILEBRAND_POSITION[25]
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 19, 0, 19}, // ELEVATION_POSITION[25]
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 20}, // USAGETYPE_POSITION[25]
};

static int IP2LocationInit(IP2Location *loc);

static void IP2LocationFree(IP2Location *loc) {
  free(loc->filename);
  free(loc);
}

IP2Location *IP2LocationOpen(char *db, IP2LOCATION_ACCESS_TYPE mtype, char *shared_name) {
  FILE *filehandle;
  IP2Location *loc;
  int initFailed = 0;
  size_t dbfilesize;
  struct stat statb;

  if ( (filehandle = fopen(db, "rb")) == NULL )
    return NULL;

  if ( fstat( fileno(filehandle), &statb ) == -1 ) {
    fclose(filehandle);
    return NULL;
  }

  dbfilesize = statb.st_size;

  if ( dbfilesize < IP2L_BASEADDR ) {
    fclose(filehandle);
    return NULL;
  }

  loc = (IP2Location *) malloc( sizeof(IP2Location) );
  memset( loc, 0, sizeof(IP2Location) );

  loc->filename = strdup(db);
  loc->dbfilesize = dbfilesize;
  loc->cache = NULL;
  loc->mml_node = NULL;

  switch (mtype) {
    case IP2LOCATION_FILE_IO:
      break;
    case IP2LOCATION_FILE_MMAP:
    if ((loc->mml_node = IP2LocationSetupMMap(filehandle, dbfilesize, db)) == NULL) {
        initFailed = -1;
      }
      break;
    case IP2LOCATION_SHARED_MEMORY:
      if ((loc->mml_node = IP2LocationSetupShared(filehandle, dbfilesize, shared_name)) == NULL) {
        initFailed = -1;
      }
      break;
    case IP2LOCATION_CACHE_MEMORY:
      if ((loc->mml_node = IP2LocationSetupCache(filehandle, dbfilesize, db)) == NULL) {
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

    if ( IP2LocationInit(loc) != 0 ) {
      IP2LocationClose(loc);
      loc = NULL;
    }

  } else {

    fclose(filehandle);
    IP2LocationFree(loc);
    loc = NULL;

  }

  return loc;
}

void IP2LocationClose(IP2Location *loc) {
  if (loc != NULL) {
    IP2LocationDBClose(loc->filehandle, loc->mml_node);
    IP2LocationFree(loc);
  }
}

int IP2LocationDeleteShared(IP2Location *loc) {
  if (loc != NULL) {
    return IP2LocationDeleteSharedMap(loc->mml_node);
  }
  return 1;
}

static int IP2LocationInit(IP2Location *loc) {
  uint8_t *cache = loc->cache;
  FILE *file = loc->filehandle;
  uint8_t dbtype = IP2LocationRead8(file,  cache, 1);
  if (dbtype > IP2L_MAX_DBTYPE)
    return 1;
  loc->databasetype = dbtype;
  loc->databasecolumn  = IP2LocationRead8(file,  cache, 2);
  if (loc->databasecolumn > IP2L_MAX_DBCOLUMN)
    return 2;
  loc->databaseyear    = IP2LocationRead8(file,  cache, 3);
  loc->databasemonth   = IP2LocationRead8(file,  cache, 4);
  if (loc->databasemonth > 12 || loc->databasemonth < 1)
    return 3;
  loc->databaseday     = IP2LocationRead8(file,  cache, 5);
  if (loc->databasemonth > 31 || loc->databasemonth < 1)
    return 4;
  loc->databasecount   = IP2LocationRead32(file, cache, 6);
  if (loc->databasecount < 2)
    return 5;
  loc->databaseaddr    = IP2LocationRead32(file, cache, 10);
  if (loc->databaseaddr != IP2L_BASEADDR)
    return 6;
  loc->v6databasecount = IP2LocationRead32(file, cache, 14);
  loc->v6databaseaddr  = IP2LocationRead32(file, cache, 18);

  if ( loc->databaseaddr + (loc->databasecolumn * 4) *
        (loc->databasecount + 1) > loc->dbfilesize )
    return 7;

  if ( loc->v6databasecount != 0 && (
        loc->v6databaseaddr + (16 + ( (loc->databasecolumn - 1) * 4 )) *
        (loc->v6databasecount + 1) > loc->dbfilesize ) )
    return 8;

  {
    int i = 0;
    uint8_t position, *offsets = loc->offsets;
    for (; i < IP2L_DB_POSITION_COUNT; ++i) {
      if ( (position = IP2L_DB_POSITIONS[i][dbtype]) != 0 ) {
        offsets[i] = (position - 1) * 4;
      } else {
        offsets[i] = 0;
      }
    }
  }

  return 0;
}

int IP2LocationRowData(IP2Location *loc,
                       IP2LOCATION_OFFSET_INDEX index,
                       uint32_t rowoffset,
                       const void *data[] ) {
  uint8_t coloffset;

  switch(index) {
    case IP2L_COUNTRYSHORT_INDEX:
      if ( (coloffset = loc->offsets[(int)index]) != 0 ) {
        *data = IP2LocationReadStrIndexAtOffset(loc->filehandle, loc->cache, rowoffset + coloffset, 0);
        return IP2L_DATA_STRING;
      }
      break;

    case IP2L_COUNTRYLONG_INDEX:
      if ( (coloffset = loc->offsets[(int)index - 1]) != 0 ) {
        *data = IP2LocationReadStrIndexAtOffset(loc->filehandle, loc->cache, rowoffset + coloffset, 3);
        return IP2L_DATA_STRING;
      }
      break;

    case IP2L_LATITUDE_INDEX:
    case IP2L_LONGITUDE_INDEX:
      if ( (coloffset = loc->offsets[(int)index - 1]) != 0 ) {
        static float value;
        value = IP2LocationReadFloat(loc->filehandle, loc->cache, rowoffset + coloffset);
        *data = &value;
        return IP2L_DATA_FLOAT;
      }
      break;
    case IP2L_REGION_INDEX:
    case IP2L_CITY_INDEX:
    case IP2L_ISP_INDEX:
    case IP2L_DOMAIN_INDEX:
    case IP2L_ZIPCODE_INDEX:
    case IP2L_TIMEZONE_INDEX:
    case IP2L_NETSPEED_INDEX:
    case IP2L_IDDCODE_INDEX:
    case IP2L_AREACODE_INDEX:
    case IP2L_WEATHERSTATIONCODE_INDEX:
    case IP2L_WEATHERSTATIONNAME_INDEX:
    case IP2L_MCC_INDEX:
    case IP2L_MNC_INDEX:
    case IP2L_ELEVATION_INDEX:
    case IP2L_MOBILEBRAND_INDEX:
    case IP2L_USAGETYPE_INDEX:
      if ( (coloffset = loc->offsets[(int)index - 1]) != 0 ) {
        *data = IP2LocationReadStrIndexAtOffset(loc->filehandle, loc->cache, rowoffset + coloffset, 0);
        return IP2L_DATA_STRING;
      }
      break;
    default:
      ;
  }
  return IP2L_NOT_FOUND;
}

static uint32_t IP2LocationFindRowIPV4(IP2Location *loc, uint32_t ipno) {
  uint32_t ipfrom, ipto;
  FILE *handle = loc->filehandle;
  uint8_t *cache = loc->cache;
  uint32_t baseaddr = loc->databaseaddr;
  uint32_t columnsize = loc->databasecolumn * 4; /* 4 bytes each column */
  uint32_t high = loc->databasecount - 2;
  uint32_t mid = 0;
  uint32_t low = 0;

  uint32_t rowoffset;

  while (low <= high) {
    mid = (uint32_t)((low + high)/2);
    rowoffset = baseaddr + mid * columnsize;

    ipfrom = IP2LocationRead32(handle, cache, rowoffset);
    ipto   = IP2LocationRead32(handle, cache, rowoffset + columnsize);

    if ((ipno >= ipfrom) && (ipno < ipto)) {

      return rowoffset;

    } else {
      if ( ipfrom > ipno ) {
        high = mid - 1;
      } else {
        low = mid + 1;
      }
    }
  }

  return IP2L_NOT_FOUND;
}

static uint32_t IP2LocationFindRowIPV6(IP2Location *loc, ipv6le128_t *ip6no) {
  ipv6le128_t ip6from, ip6to;
  FILE *handle = loc->filehandle;
  uint8_t *cache = loc->cache;
  uint32_t baseaddr = loc->v6databaseaddr;
  /* 4 bytes each column, except IPFrom column which is 16 bytes */
  uint32_t columnsize = 16 + ( (loc->databasecolumn - 1) * 4 );
  uint32_t high = loc->v6databasecount - 2;
  uint32_t mid = 0;
  uint32_t low = 0;

  uint32_t rowoffset;

  int ipcmp;

  if ( loc->v6databasecount < 2 )
    return IP2L_NOT_FOUND;

  while (low <= high) {
    mid = (uint32_t)((low + high) / 2);
    rowoffset = baseaddr + mid * columnsize;

    IP2LocationRead128Buffer32LE(ip6from.ui32, handle, cache, rowoffset);
    IP2LocationRead128Buffer32LE(ip6to.ui32, handle, cache, rowoffset + columnsize);

    if ( ((ipcmp = IP2LocationIPv6Cmp(ip6no, &ip6from)) >= 0) &&
                  (IP2LocationIPv6Cmp(ip6no, &ip6to) < 0) ) {

      return rowoffset + 12;

    } else {
      if ( ipcmp < 0 ) {
        high = mid - 1;
      } else {
        low = mid + 1;
      }
    }
  }

  return IP2L_NOT_FOUND;
}


uint32_t IP2LocationFindRow(IP2Location *loc, char *ip) {
  ipv6le128_t ipaddr;

  switch( IP2LocationIP2No(ip, &ipaddr) ) {
    case 6:
      return IP2LocationFindRowIPV6(loc, &ipaddr);
    case 4:
      return IP2LocationFindRowIPV4(loc, ipaddr.ipv4.addr);
  }
  return IP2L_NOT_FOUND;
}

/*
int IP2LocationMakeSimpleDictionary(IP2Location *loc, const char *filename, uint32_t mode) {
  uint8_t dbtype = loc->databasetype;
  FILE *handle = loc->filehandle;
  uint8_t *cache = loc->cache;
  uint32_t baseaddr = loc->databaseaddr;
  uint32_t dbcount = loc->databasecount;
  uint32_t dbcolumn = loc->databasecolumn;

  uint32_t min = 0xFFFFFFFFU, max = 0, index, value;

  for (index = 0; index < dbcount; ++index) {
    if ((mode & IP2L_ISP) && (ISP_POSITION[dbtype] != 0)) {
      if (loc->ipversion == IP2L_IPV6) {
        value = IP2LocationRead32(handle, cache, baseaddr + index * (dbcolumn * 4 + 12) + 12 + 4 * (ISP_POSITION[dbtype]-1));
      } else {
        value = IP2LocationRead32(handle, cache, baseaddr + (index * dbcolumn * 4) + 4 * (ISP_POSITION[dbtype]-1));
      }
      if (value > max) max = value;
      if (value < min) min = value;
    }
  }

  if (min <= max) {
    FILE *file = fopen(filename, "w");
    value = 0;
    if (file == NULL)
      return -1;

    for(index = min; index <= max; ++value) {
      const char *name = IP2LocationReadStr(handle, cache, index);
      index += strlen(name) + 1;
      fprintf(file, "%s\n", name);
      free((void *)name);
    }

    fclose(file);
    return value;
  }

  return -2;
}

char *IP2LocationReadRecord(IP2Location *loc, uint32_t index, uint32_t mode) {
  uint8_t dbtype = loc->databasetype;
  FILE *handle = loc->filehandle;
  uint8_t *cache = loc->cache;
  uint32_t baseaddr = loc->databaseaddr;
  uint32_t dbcount = loc->databasecount;
  uint32_t dbcolumn = loc->databasecolumn;

  if (index > dbcount) return NULL;

  if (mode & IP2L_COUNTRYSHORT) {
    if (COUNTRY_POSITION[dbtype] != 0) {
      if (loc->ipversion == IP2L_IPV6) {
        return IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + index * (dbcolumn * 4 + 12) + 12 + 4 * (COUNTRY_POSITION[dbtype]-1)));
      } else {
        return IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + (index * dbcolumn * 4) + 4 * (COUNTRY_POSITION[dbtype]-1)));
      }
    }
  } else if (mode & IP2L_COUNTRYLONG) {
    if (COUNTRY_POSITION[dbtype] != 0) {
      if (loc->ipversion == IP2L_IPV6) {
        return IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + index * (dbcolumn * 4 + 12) + 12 + 4 * (COUNTRY_POSITION[dbtype]-1))+3);
      } else {
        return IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + (index * dbcolumn * 4) + 4 * (COUNTRY_POSITION[dbtype]-1))+3);
      }
    }
  } else if (mode & IP2L_REGION) {
    if (REGION_POSITION[dbtype] != 0) {
      if (loc->ipversion == IP2L_IPV6) {
        return IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + index * (dbcolumn * 4 + 12) + 12 + 4 * (REGION_POSITION[dbtype]-1)));
      } else {
        return IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + (index * dbcolumn * 4) + 4 * (REGION_POSITION[dbtype]-1)));
      }
    }
  } else if (mode & IP2L_CITY) {
    if (CITY_POSITION[dbtype] != 0) {
      if (loc->ipversion == IP2L_IPV6) {
        return IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + index * (dbcolumn * 4 + 12) + 12 + 4 * (CITY_POSITION[dbtype]-1)));
      } else {
        return IP2LocationReadStr(handle, cache, IP2LocationRead32(handle, cache, baseaddr + (index * dbcolumn * 4) + 4 * (CITY_POSITION[dbtype]-1)));
      }
    }
  }
  return NULL;
}

int IP2LocationMakeDictionary(IP2Location *loc, char *dir) {
  uint32_t index, dbcount = loc->databasecount;
  char *name = NULL;
  IP2LDictionary *countryDict = NULL, *regionDict = NULL;
  for (index = 0; index < dbcount; index++) {
    if ((name = IP2LocationReadRecord(loc, index, IP2L_COUNTRYSHORT)) != NULL) {
      countryDict = IP2LFindOrAddDictionaryElement(name, NULL);
      free(name);
      if (countryDict != NULL) {
        if ((name = IP2LocationReadRecord(loc, index, IP2L_REGION)) != NULL) {
          regionDict = IP2LFindOrAddDictionaryElement(name, countryDict);
          free(name);
          if (regionDict != NULL) {
            if ((name = IP2LocationReadRecord(loc, index, IP2L_CITY)) != NULL) {
              IP2LFindOrAddDictionaryElement(name, regionDict);
              free(name);
            }
          }
        }
      }
    }
  }
  {
    int ret = IP2LSaveAllDictionaries(dir);
    IP2LFreeDictionary(NULL);
    return ret;
  }
}

*/
