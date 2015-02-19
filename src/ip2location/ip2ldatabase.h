#ifndef HAVE_IP2LOC_DBINTERFACE_H
#define HAVE_IP2LOC_DBINTERFACE_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "ip2lmemorymaplist.h"

typedef struct {
  uint8_t *memory;
  uint32_t size;
} IP2LCacheHandler;

IP2LMemoryMapList *IP2LocationSetupCache(FILE *filehandle, size_t dbfilesize, char *db);
IP2LMemoryMapList *IP2LocationSetupMMap(FILE *filehandle, size_t dbfilesize, char *db);
IP2LMemoryMapList *IP2LocationSetupShared(FILE *filehandle, size_t dbfilesize, char *shared_name);

void IP2LocationRead128Buffer32LE(uint32_t buffer[4], FILE *handle, IP2LCacheHandler *cache, uint32_t position);
uint8_t IP2LocationRead8(FILE *handle, IP2LCacheHandler *cache, uint32_t position);
float IP2LocationReadFloat(FILE *handle, IP2LCacheHandler *cache, uint32_t position);
uint32_t IP2LocationRead32(FILE *handle, IP2LCacheHandler *cache, uint32_t position);
/* the value returned by the following functions is not a C-string (null terminated)
   the 1st byte indicates a string length as unsigned char
   the string starts at 2nd byte if length > 0 */
const uint8_t *IP2LocationReadStr(FILE *handle, IP2LCacheHandler *cache, uint32_t position);
const uint8_t *IP2LocationReadStrIndexAtOffset(FILE *handle, IP2LCacheHandler *cache, uint32_t position, int offset);

int IP2LocationDeleteSharedMap(IP2LMemoryMapList *shmnode);
void IP2LocationDBClose(FILE *filehandle, IP2LMemoryMapList *shmnode);

#endif /* HAVE_IP2LOC_DBINTERFACE_H */
