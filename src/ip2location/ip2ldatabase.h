#ifndef HAVE_IP2LOC_DBINTERFACE_H
#define HAVE_IP2LOC_DBINTERFACE_H

#ifndef WIN32
#include <stdint.h>
#include <strings.h>
#include <unistd.h>
#include <sys/mman.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "ip2lmemorymaplist.h"
#include "ip2location.h"

void IP2LocationRead128Buffer32LE(uint32_t *buffer, FILE *handle, uint8_t *cache, uint32_t position);
uint32_t IP2LocationRead32(FILE *handle, uint8_t *cache, uint32_t position);
uint8_t IP2LocationRead8(FILE *handle, uint8_t *cache, uint32_t position);
char *IP2LocationReadStr(FILE *handle, uint8_t *cache, uint32_t position);
float IP2LocationReadFloat(FILE *handle, uint8_t *cache, uint32_t position);
IP2LMemoryMapList *IP2LocationSetupMMap(FILE *filehandle, char *db);
IP2LMemoryMapList *IP2LocationSetupShared(FILE *filehandle, char *shared_name);
IP2LMemoryMapList *IP2LocationSetupCache(FILE *filehandle, char *db);
int32_t IP2LocationDBClose(FILE *filehandle, IP2LMemoryMapList *shmnode);
int IP2LocationDeleteSharedMap(IP2LMemoryMapList *shmnode);


#endif
