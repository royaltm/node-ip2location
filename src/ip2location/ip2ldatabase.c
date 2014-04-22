#include "ip2ldatabase.h"

#define IP2LOCATION_SHM_DEFAULT "/IP2location_Shm"
#define MAP_ADDR 0xFA030000

int32_t IP2LocationCopyDBToMemory(FILE *filehandle, void *memory, size_t size) {
  fseek(filehandle, SEEK_SET, 0);
  if ( fread(memory, size, 1, filehandle) != 1 )
    return -1;
  return 0;
}


IP2LMemoryMapList *IP2LocationSetupCache(FILE *filehandle, char *db) {
  struct stat statbuf;
  void *mem_ptr;
  size_t mem_size;

  IP2LMemoryMapList *mmlnode = IP2LFindMemoryMapNode(db, MEMMAP_TYPE_NONE);

  if (mmlnode != NULL) {
    mmlnode->count++;
  } else {

    if (fstat(fileno(filehandle), &statbuf) == -1) {
      return NULL;
    }

    mem_size = statbuf.st_size + 1;

    if ( (mem_ptr = malloc(mem_size)) == NULL ) {
      return NULL;
    }

    if ( IP2LocationCopyDBToMemory(filehandle, mem_ptr, statbuf.st_size) == -1 ) {
      free(mem_ptr);
      return NULL;
    }

    mmlnode = IP2LCreateMemoryMapNode(db, MEMMAP_TYPE_NONE);
    mmlnode->mem_ptr = mem_ptr;
    mmlnode->mem_size = mem_size;
    mmlnode->count = 1;
  }

  return mmlnode;
}

#ifndef WIN32
IP2LMemoryMapList *IP2LocationSetupMMap(FILE *filehandle, char *db) {
  struct stat statbuf;
  void *mem_ptr;
  size_t mem_size;

  IP2LMemoryMapList *mmlnode = IP2LFindMemoryMapNode(db, MEMMAP_TYPE_FILE);

  if (mmlnode != NULL) {
    mmlnode->count++;
  } else {
    if ( fstat(fileno(filehandle), &statbuf) == -1 ) {
      return NULL;
    }
    mem_size = statbuf.st_size + 1;
    mem_ptr = mmap(NULL, mem_size, PROT_READ, MAP_PRIVATE, fileno(filehandle), 0);
    if ( mem_ptr == (void *) -1 ) {
      return NULL;
    }

    mmlnode = IP2LCreateMemoryMapNode(db, MEMMAP_TYPE_FILE);
    mmlnode->mem_ptr = mem_ptr;
    mmlnode->mem_size = mem_size;
    mmlnode->count = 1;
  }
  return mmlnode;
}

IP2LMemoryMapList *IP2LocationSetupShared(FILE *filehandle, char *shared_name) {
  struct stat statbuf;
  size_t shm_size, file_size;
  SHARED_MEM_FHANDLE shm_fd;
  void *shm_shared_ptr;
  int32_t DB_loaded = 1;

  if (shared_name == NULL) {
    shared_name = IP2LOCATION_SHM_DEFAULT;
  }

  IP2LMemoryMapList *mmlnode = IP2LFindMemoryMapNode(shared_name, MEMMAP_TYPE_SHARED);
  if (mmlnode != NULL) {
    if ( ( shm_fd = shm_open(mmlnode->name, O_RDWR , 0777) ) == -1 ) {
      IP2LDetachMemoryMapNode(mmlnode); // file was unlinked
      mmlnode = NULL;
    } else {
      if ( fstat(shm_fd, &statbuf) == -1 || mmlnode->shm_ino != statbuf.st_ino) {
        IP2LDetachMemoryMapNode(mmlnode); // file was unlinked and recreated
        mmlnode = NULL;
      } else {
        mmlnode->count++;
      }
      close(shm_fd);
    }
  }

  if (mmlnode == NULL) {
    if ( ( shm_fd = shm_open(shared_name, O_RDWR | O_CREAT | O_EXCL, 0777) ) != -1 ) {
      DB_loaded = 0;
    } else if ( ( shm_fd = shm_open(shared_name, O_RDWR , 0777) ) == -1 ) {
      return NULL;
    }

    if (DB_loaded == 0) {
      if ( fstat(fileno(filehandle), &statbuf) == -1 ) {
        close(shm_fd);
        shm_unlink(shared_name);
        return NULL;
      }

      file_size = statbuf.st_size;

      if ( ftruncate(shm_fd, file_size + 1) == -1 ) {
        close(shm_fd);
        shm_unlink(shared_name);
        return NULL;
      }
    }

    if ( fstat(shm_fd, &statbuf) == -1 ) {
      close(shm_fd);
      if ( DB_loaded == 0 )
        shm_unlink(shared_name);
      return NULL;
    }

    shm_size = statbuf.st_size;
    shm_shared_ptr = mmap((void *)MAP_ADDR, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if ( shm_shared_ptr == (void *) -1 ) {
      close(shm_fd);
      if ( DB_loaded == 0 )
        shm_unlink(shared_name);
      return NULL;
    }
    if ( DB_loaded == 0 ) {
      if ( IP2LocationCopyDBToMemory(filehandle, shm_shared_ptr, file_size) == -1 ) {
        munmap(shm_shared_ptr, shm_size);
        close(shm_fd);
        shm_unlink(shared_name);
        return NULL;
      }
    }
    mmlnode = IP2LCreateMemoryMapNode(shared_name, MEMMAP_TYPE_SHARED);
    mmlnode->mem_ptr = shm_shared_ptr;
    mmlnode->mem_size = shm_size;
    mmlnode->shm_ino = statbuf.st_ino;
    mmlnode->shm_fd = shm_fd;
    mmlnode->count = 1;
  }
  return mmlnode;
}
#else
#ifdef WIN32
IP2LMemoryMapList *IP2LocationSetupMMap(FILE *filehandle, char *db) {
  struct stat statbuf;
  SHARED_MEM_FHANDLE shm_fd;
  void *mem_ptr;
  size_t mem_size;

  IP2LMemoryMapList *mmlnode = IP2LFindMemoryMapNode(db, MEMMAP_TYPE_FILE);

  if (mmlnode != NULL) {
    mmlnode->count++;
  } else {
    if(fstat(fileno(filehandle), &statbuf) == -1) {
      return NULL;
    }

    mem_size = statbuf.st_size + 1;

    shm_fd = CreateFileMapping(
                   filehandle,
                   NULL,
                   PAGE_READONLY,
                   0,
                   mem_size,
                   NULL);
    if (shm_fd == NULL) {
      return NULL;
    }

    mem_ptr = MapViewOfFile(
        shm_fd,
        FILE_MAP_READ,
        0, 
        0,
        0);

    if (mem_ptr == NULL) {
      CloseHandle(shm_fd);
      return NULL;
    }
    
    mmlnode = IP2LCreateMemoryMapNode(db, MEMMAP_TYPE_FILE);
    mmlnode->mem_ptr = mem_ptr;
    mmlnode->mem_size = mem_size;
    mmlnode->shm_fd = shm_fd;
    mmlnode->count = 1;
  } else {
    mmlnode->count++;
  }
  return mmlnode;
}

IP2LMemoryMapList *IP2LocationSetupShared(FILE *filehandle, char *shared_name) {
  struct stat statbuf;
  SHARED_MEM_FHANDLE shm_fd;
  void *shm_shared_ptr;
  size_t shm_size;
  int32_t DB_loaded = 1;

  if (shared_name == NULL) {
    shared_name = IP2LOCATION_SHM_DEFAULT;
  }

  IP2LMemoryMapList *mmlnode = IP2LFindMemoryMapNode(shared_name, MEMMAP_TYPE_SHARED);

  if (mmlnode != NULL) {
    mmlnode->count++;
  } else {
    if(fstat(fileno(filehandle), &statbuf) == -1) {
      return NULL;
    }

    shm_size = statbuf.st_size + 1;

    shm_fd = CreateFileMapping(
                   INVALID_HANDLE_VALUE,
                   NULL,
                   PAGE_READWRITE,
                   0,
                   shm_size,
                   TEXT(shared_name));
    if (shm_fd == NULL) {
      return NULL;
    }

    DB_loaded = (GetLastError() == ERROR_ALREADY_EXISTS);

    shm_shared_ptr = MapViewOfFile(
        shm_fd,
        FILE_MAP_WRITE,
        0, 
        0,
        0);

    if (shm_shared_ptr == NULL) {
      CloseHandle(shm_fd);
      return NULL;
    }

    if( DB_loaded == 0 ) {
      if ( IP2LocationCopyDBToMemory(filehandle, shm_shared_ptr, statbuf.st_size) == -1 ) {
        UnmapViewOfFile(shm_shared_ptr);
        CloseHandle(shm_fd);
        return NULL;
      }
    }

    mmlnode = IP2LCreateMemoryMapNode(shared_name, MEMMAP_TYPE_SHARED);
    mmlnode->mem_ptr = shm_shared_ptr;
    mmlnode->mem_size = shm_size;
    mmlnode->shm_fd = shm_fd;
    mmlnode->count = 1;
  }
  return mmlnode;
}
#endif
#endif

int32_t IP2LocationDBClose(FILE *filehandle, IP2LMemoryMapList *mmlnode) {
  if ( mmlnode != NULL ) {
    if ((--(mmlnode->count)) <= 0) {
      if (mmlnode->type == MEMMAP_TYPE_NONE) {
        free(mmlnode->mem_ptr);
      } else {
#ifndef  WIN32
        munmap(mmlnode->mem_ptr, mmlnode->mem_size);
        if (mmlnode->type == MEMMAP_TYPE_SHARED)
          close(mmlnode->shm_fd);
#else
#ifdef WIN32
        UnmapViewOfFile(mmlnode->mem_ptr);
        CloseHandle(mmlnode->shm_fd);
#endif
#endif
      }
      IP2LFreeMemoryMapNode(mmlnode);
    }
  }
  if ( filehandle != NULL ) {
    fclose(filehandle);
  }

  return 0;
}

#ifndef  WIN32
int IP2LocationDeleteSharedMap(IP2LMemoryMapList *mmlnode) {
  int ret = 1;
  if (mmlnode != NULL && mmlnode->type == MEMMAP_TYPE_SHARED) {
    ret = -1;
    SHARED_MEM_FHANDLE shm_fd;
    struct stat statbuf;
    if ( ( shm_fd = shm_open(mmlnode->name, O_RDWR , 0777) ) != -1 ) {
      if ( fstat(shm_fd, &statbuf) != -1 && mmlnode->shm_ino == statbuf.st_ino ) {
        ret = shm_unlink(mmlnode->name);
      }
      close(shm_fd);
    }
    IP2LDetachMemoryMapNode(mmlnode);
  }
  return ret;
}
#else
#ifdef WIN32
int IP2LocationDeleteSharedMap(IP2LMemoryMapList *mmlnode) {
  return 1;
}
#endif
#endif

void IP2LocationRead128Buffer(void *buffer, FILE *handle, uint8_t *cache, uint32_t position) {
  if (cache == NULL) {
    fseek(handle, position - 1, 0);
    fread(buffer, 16, 1, handle);
  } else {
    memcpy(buffer, (void*)&cache[position - 1], 16);
  }
}

uint32_t IP2LocationRead32(FILE *handle, uint8_t *cache, uint32_t position) {
  union {
    uint32_t u32;
    uint8_t b8[4];
  } bytes;
  bytes.u32 = 0;
  
  if (cache == NULL) {
    fseek(handle, position-1, 0);
    fread(bytes.b8, sizeof(bytes.b8), 1, handle);
  } else {
    memcpy(bytes.b8, &cache[position - 1], sizeof(bytes.b8));
  }
  return le32toh(bytes.u32);
}

uint8_t IP2LocationRead8(FILE *handle, uint8_t *cache, uint32_t position) {  
  uint8_t ret = 0;

  if (cache == NULL) {
    fseek(handle, position-1, 0);
    fread(&ret, 1, 1, handle);
  } else {
    ret = cache[ position - 1 ];
  }
  return ret;
}

char *IP2LocationReadStr(FILE *handle, uint8_t *cache, uint32_t position) {
  uint8_t size = 0;
  char *str = NULL;

  if (cache == NULL) {
    fseek(handle, position, 0);
    fread(&size, 1, 1, handle);
    str = (char *)malloc(size+1);
    fread(str, size, 1, handle);
    str[size] = 0;
  } else {
    size = cache[position];
    str = (char *)malloc(size+1);
    memcpy((void*) str, (void*)&cache[position + 1], size);
    str[size] = 0;
  }
  return str;
}

float IP2LocationReadFloat(FILE *handle, uint8_t *cache, uint32_t position) {
  union {
    float f32;
    int32_t i32;
  } bytes;
  bytes.i32 = 0;

  if (cache == NULL) {
    fseek(handle, position-1, 0);
    fread(&bytes.i32, sizeof(bytes.i32), 1, handle);
  } else {
    memcpy((void*) &bytes.i32, (void*)&cache[position - 1], sizeof(bytes.i32));
  }
  bytes.i32 = le32toh(bytes.i32);
  return bytes.f32;
}
