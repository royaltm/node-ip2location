#ifndef HAVE_IP2LOC_DBINTERFACE_H
#define HAVE_IP2LOC_DBINTERFACE_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "ip2lmemorymaplist.h"
#include "ip2location.h"

IP2LMemoryMapList *IP2LocationSetupCache(FILE *filehandle, size_t dbfilesize, char *db);
IP2LMemoryMapList *IP2LocationSetupMMap(FILE *filehandle, size_t dbfilesize, char *db);
IP2LMemoryMapList *IP2LocationSetupShared(FILE *filehandle, size_t dbfilesize, char *shared_name);

void IP2LocationRead128Buffer32LE(uint32_t *buffer, FILE *handle, uint8_t *cache, uint32_t position);
uint32_t IP2LocationRead32(FILE *handle, uint8_t *cache, uint32_t position);
uint8_t IP2LocationRead8(FILE *handle, uint8_t *cache, uint32_t position);
const char *IP2LocationReadStr(FILE *handle, uint8_t *cache, uint32_t position);
const char *IP2LocationReadStrIndexAtOffset(FILE *handle, uint8_t *cache, uint32_t position, int offset);
float IP2LocationReadFloat(FILE *handle, uint8_t *cache, uint32_t position);

int IP2LocationDeleteSharedMap(IP2LMemoryMapList *shmnode);
void IP2LocationDBClose(FILE *filehandle, IP2LMemoryMapList *shmnode);

#endif /* HAVE_IP2LOC_DBINTERFACE_H */