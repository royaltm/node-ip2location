#ifndef HAVE_IP2LOC_MEMORYMAPLIST_H
#define HAVE_IP2LOC_MEMORYMAPLIST_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

#ifndef WIN32
  typedef int32_t SHARED_MEM_FHANDLE;
#else
#ifdef WIN32
#  include <windows.h>
  typedef HANDLE SHARED_MEM_FHANDLE;
#endif
#endif

typedef enum MemoryMapType {
  MEMMAP_TYPE_NONE,
  MEMMAP_TYPE_FILE,
  MEMMAP_TYPE_SHARED
} MEMORY_MAP_TYPE;

typedef struct IP2LMemoryMapList {
  struct IP2LMemoryMapList* prev;
  struct IP2LMemoryMapList* next;
  char* name;
  void *mem_ptr;
  size_t mem_size;
#ifndef WIN32
  ino_t shm_ino;
#endif
  int count;
  MEMORY_MAP_TYPE type;
  SHARED_MEM_FHANDLE shm_fd;
} IP2LMemoryMapList;

IP2LMemoryMapList *IP2LFindMemoryMapNode(char *name, MEMORY_MAP_TYPE type);
IP2LMemoryMapList *IP2LCreateMemoryMapNode(char *name, MEMORY_MAP_TYPE type);
void IP2LDetachMemoryMapNode(IP2LMemoryMapList *mml);
void IP2LFreeMemoryMapNode(IP2LMemoryMapList *mml);

#endif /* HAVE_IP2LOC_MEMORYMAPLIST_H */