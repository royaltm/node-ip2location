#include <string.h>
#include "ip2lmemorymaplist.h"

static IP2LMemoryMapList *cache_root = NULL;
static IP2LMemoryMapList *mfile_root = NULL;
static IP2LMemoryMapList *shmem_root = NULL;

IP2LMemoryMapList **IP2LMemoryMapRoot(MEMORY_MAP_TYPE type) {
  switch(type) {
  case MEMMAP_TYPE_FILE:
    return &mfile_root;
  case MEMMAP_TYPE_SHARED:
    return &shmem_root;
  case MEMMAP_TYPE_NONE:
    return &cache_root;
  }
  return NULL;
}

void IP2LPrependMemoryMapNode(IP2LMemoryMapList *new_mml) {
  IP2LMemoryMapList **root = IP2LMemoryMapRoot(new_mml->type);
  new_mml->prev = NULL;
  if (*root != NULL) {
    new_mml->next = *root;
    (*root)->prev = new_mml;
  } else {
    new_mml->next = NULL;
  }
  *root = new_mml;
}

IP2LMemoryMapList *IP2LFindMemoryMapNode(char *name, MEMORY_MAP_TYPE type) {
  IP2LMemoryMapList **root = IP2LMemoryMapRoot(type);
  IP2LMemoryMapList *mml = *root;
  while (mml != NULL) {
    if (strcmp(mml->name, name) == 0) break;
    mml = mml->next;
  }
  return mml;
}

IP2LMemoryMapList *IP2LCreateMemoryMapNode(char *name, MEMORY_MAP_TYPE type) {
  IP2LMemoryMapList *mml = (IP2LMemoryMapList *)malloc(sizeof(IP2LMemoryMapList));
  mml->type = type;
  mml->name = strdup(name);
  mml->mem_ptr = NULL;
  mml->count = 0;
  IP2LPrependMemoryMapNode(mml);
  return mml;
}

void IP2LDetachMemoryMapNode(IP2LMemoryMapList *mml) {
  IP2LMemoryMapList **root = IP2LMemoryMapRoot(mml->type);
  if (*root == mml) {
    *root = mml->next;
  }
  if (mml->prev != NULL) {
    mml->prev->next = mml->next;
  }
  if (mml->next != NULL) {
    mml->next->prev = mml->prev;
  }
  mml->next = NULL;
  mml->prev = NULL;
}

void IP2LFreeMemoryMapNode(IP2LMemoryMapList *mml) {
  IP2LDetachMemoryMapNode(mml);
  free(mml->name);
  free(mml);
}
