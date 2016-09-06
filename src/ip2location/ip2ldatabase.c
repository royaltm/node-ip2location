#include "ip2ldatabase.h"

#ifndef WIN32
#  include <unistd.h>
#  include <sys/mman.h>
#else
#ifdef WIN32
#  include <windows.h>
#  include <io.h>
#endif
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#define IP2LOCATION_SHARED_NAME "/IP2location_Shm"
#define IP2LOCATION_MMAP_ADDR 0xFA030000

static const uint32_t IP2LreadError128[4] = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };
#define IP2LreadError32 0xFFFFFFFF
#define IP2LreadError8 0xFF
#define IP2LreadErrorFloat 0.0F
static const uint8_t IP2LreadErrorStr[1] = { 0 };

static int IP2LocationCopyDBToMemory(FILE *filehandle, void *memory, size_t size)
{
  if ( size == 0 )
    return -1;

  if ( fseek(filehandle, SEEK_SET, 0) != 0 ||
       fread(memory, size, 1, filehandle) != 1 )
    return -1;

  return 0;
}


IP2LMemoryMapList *IP2LocationSetupCache(FILE *filehandle, size_t dbfilesize, char *db)
{
  void *mem_ptr;

  IP2LMemoryMapList *mmlnode = IP2LFindMemoryMapNode(db, MEMMAP_TYPE_NONE);

  if ( mmlnode != NULL ) {

    mmlnode->count++;

  } else {

    if ( ( mem_ptr = malloc(dbfilesize) ) == NULL )
      return NULL;

    if ( IP2LocationCopyDBToMemory(filehandle, mem_ptr, dbfilesize) == -1 ) {
      free(mem_ptr);
      return NULL;
    }

    mmlnode = IP2LCreateMemoryMapNode(db, MEMMAP_TYPE_NONE);
    mmlnode->mem_ptr = mem_ptr;
    mmlnode->mem_size = dbfilesize;
    mmlnode->count = 1;
    mmlnode->copybythisprocess = 1;
  }

  return mmlnode;
}

#ifndef WIN32

IP2LMemoryMapList *IP2LocationSetupMMap(FILE *filehandle, size_t dbfilesize, char *db)
{
  void *mem_ptr;

  IP2LMemoryMapList *mmlnode = IP2LFindMemoryMapNode(db, MEMMAP_TYPE_FILE);

  if (mmlnode != NULL) {

    mmlnode->count++;

  } else {

    mem_ptr = mmap( NULL,
                    dbfilesize,
                    PROT_READ,
                    MAP_PRIVATE,
                    fileno(filehandle),
                    0 );

    if ( mem_ptr == (void *) -1 ) {
      return NULL;
    }

    mmlnode = IP2LCreateMemoryMapNode(db, MEMMAP_TYPE_FILE);
    mmlnode->mem_ptr = mem_ptr;
    mmlnode->mem_size = dbfilesize;
    mmlnode->count = 1;
    mmlnode->copybythisprocess = 0;
  }

  return mmlnode;
}

IP2LMemoryMapList *IP2LocationSetupShared(FILE *filehandle, size_t dbfilesize, char *shared_name)
{
  struct stat statbuf;
  SHARED_MEM_FHANDLE shm_fd;
  void *shm_shared_ptr;
  size_t shm_size;
  int DB_loaded = 1;

  shm_size = ((dbfilesize / getpagesize()) + 1) * getpagesize();

  if (shared_name == NULL)
    shared_name = IP2LOCATION_SHARED_NAME;

  IP2LMemoryMapList *mmlnode = IP2LFindMemoryMapNode(shared_name, MEMMAP_TYPE_SHARED);

  if (mmlnode != NULL) {

    /* different database file size, we should fail */
    if (mmlnode->mem_size != shm_size) {
      return NULL;
    }

    /* check if shared mem exists */
    if ( ( shm_fd = shm_open(mmlnode->name, O_RDWR , 0777) ) == -1 ) {

      IP2LDetachMemoryMapNode(mmlnode); /* file was unlinked */
      mmlnode = NULL;

    } else {
      /* check if shared mem has changed */
      if ( fstat(shm_fd, &statbuf) == -1 || mmlnode->shm_ino != statbuf.st_ino) {

        IP2LDetachMemoryMapNode(mmlnode); /* file was unlinked and recreated */
        mmlnode = NULL;

      } else {
        /* shared mem hasn't changed */
        mmlnode->count++;

      }

      close(shm_fd);
    }
  }

  if (mmlnode == NULL) {

    if ( ( shm_fd = shm_open(shared_name, O_RDWR | O_CREAT | O_EXCL, 0777) ) != -1 ) {
      /* shared mem was created by us */
      DB_loaded = 0;

    } else if ( ( shm_fd = shm_open(shared_name, O_RDWR , 0777) ) == -1 ) {
      /* opening shared mem has failed */
      return NULL;
    }

    if (DB_loaded == 0) {
      /* extend shared mem above database size */
      if ( ftruncate(shm_fd, shm_size) == -1 ) {
        close(shm_fd);
        shm_unlink(shared_name);
        return NULL;
      }

    }

    /* get shared mem size */
    if ( fstat(shm_fd, &statbuf) == -1 ) {
      close(shm_fd);
      if ( DB_loaded == 0 )
        shm_unlink(shared_name);
      return NULL;
    }

    /* different database file size, we should fail */
    if ( (size_t) statbuf.st_size != shm_size ) {
      close(shm_fd);
      if ( DB_loaded == 0 )
        shm_unlink(shared_name);
      return NULL;
    }

    shm_shared_ptr = mmap( (void *)IP2LOCATION_MMAP_ADDR,
                           shm_size,
                           PROT_READ | PROT_WRITE,
                           MAP_SHARED,
                           shm_fd,
                           0 );

    if ( shm_shared_ptr == (void *) -1 ) {
      close(shm_fd);
      if ( DB_loaded == 0 )
        shm_unlink(shared_name);
      return NULL;
    }

    if ( DB_loaded == 0 ) {

      if ( IP2LocationCopyDBToMemory(filehandle, shm_shared_ptr, dbfilesize) == -1 ) {
        munmap(shm_shared_ptr, shm_size);
        close(shm_fd);
        shm_unlink(shared_name);
        return NULL;
      }

      /* mark database loaded */
      ((char *)shm_shared_ptr)[dbfilesize] = -1;

    } else if ( ((char *)shm_shared_ptr)[dbfilesize] != -1 ) {
      munmap(shm_shared_ptr, shm_size);
      close(shm_fd);
      shm_unlink(shared_name);
      return NULL;
    }

    mmlnode = IP2LCreateMemoryMapNode(shared_name, MEMMAP_TYPE_SHARED);
    mmlnode->mem_ptr = shm_shared_ptr;
    mmlnode->mem_size = shm_size;
    mmlnode->shm_ino = statbuf.st_ino;
    mmlnode->shm_fd = shm_fd;
    mmlnode->count = 1;
    mmlnode->copybythisprocess = DB_loaded == 0 ? 1 : 0;

  }

  return mmlnode;
}

#else
#ifdef WIN32

IP2LMemoryMapList *IP2LocationSetupMMap(FILE *filehandle, size_t dbfilesize, char *db)
{
  SHARED_MEM_FHANDLE shm_fd;
  HANDLE wfilehandle = (HANDLE)_get_osfhandle( fileno(filehandle) );
  void *mem_ptr;

  IP2LMemoryMapList *mmlnode = IP2LFindMemoryMapNode(db, MEMMAP_TYPE_FILE);

  if (mmlnode != NULL) {

    mmlnode->count++;

  } else {

    shm_fd = CreateFileMapping(
                   wfilehandle,
                   NULL,
                   PAGE_READONLY,
                   0,
                   0,
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
    mmlnode->mem_size = dbfilesize;
    mmlnode->shm_fd = shm_fd;
    mmlnode->count = 1;
    mmlnode->copybythisprocess = 0;

  }

  return mmlnode;
}

IP2LMemoryMapList *IP2LocationSetupShared(FILE *filehandle, size_t dbfilesize, char *shared_name)
{
  SHARED_MEM_FHANDLE shm_fd;
  void *shm_shared_ptr;
  size_t shm_size;
  int DB_loaded = 1;
  IP2LMemoryMapList *mmlnode;
  SYSTEM_INFO sysInfo;

  GetSystemInfo(&sysInfo);

  shm_size = ((dbfilesize / sysInfo.dwPageSize) + 1) * sysInfo.dwPageSize;

  if (shared_name == NULL) {
    shared_name = IP2LOCATION_SHARED_NAME;
  }

  mmlnode = IP2LFindMemoryMapNode(shared_name, MEMMAP_TYPE_SHARED);

  if (mmlnode != NULL) {

    /* different database file size, we should fail */
    if (mmlnode->mem_size != shm_size) {
      return NULL;
    }

    mmlnode->count++;

  } else {

    shm_fd = CreateFileMapping(
                   INVALID_HANDLE_VALUE,
                   NULL,
                   PAGE_READWRITE,
                   0,
                   (DWORD) shm_size,
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

    if ( DB_loaded == 0 ) {

      if ( IP2LocationCopyDBToMemory(filehandle, shm_shared_ptr, dbfilesize) == -1 ) {
        UnmapViewOfFile(shm_shared_ptr);
        CloseHandle(shm_fd);
        return NULL;
      }

      /* mark database loaded */
      ((char *)shm_shared_ptr)[dbfilesize] = -1;

    } else {
      MEMORY_BASIC_INFORMATION mapInfo;

      /* different database file size, we should fail */
      if ( VirtualQuery(shm_shared_ptr, &mapInfo, sizeof(mapInfo)) < sizeof(mapInfo) ||
            mapInfo.RegionSize != shm_size ||
            ((char *)shm_shared_ptr)[dbfilesize] != -1) {
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
    mmlnode->copybythisprocess = DB_loaded == 0 ? 1 : 0;

  }

  return mmlnode;
}

#endif /* WIN32 */
#endif /* IP2LocationSetupMMap and IP2LocationSetupShared */

void IP2LocationDBClose(FILE *filehandle, IP2LMemoryMapList *mmlnode)
{
  if ( mmlnode != NULL ) {
    if ((--(mmlnode->count)) <= 0) {

      if (mmlnode->type == MEMMAP_TYPE_NONE) {
        free(mmlnode->mem_ptr);
      } else {
#ifndef WIN32
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
}

#ifndef WIN32
int IP2LocationDeleteSharedMap(IP2LMemoryMapList *mmlnode)
{
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
int IP2LocationDeleteSharedMap(IP2LMemoryMapList *mmlnode)
{
  return 1;
}
#endif
#endif

void IP2LocationRead128Buffer32LE(uint32_t buffer[4], FILE *handle, IP2LCacheHandler *cache, uint32_t position)
{
  uint8_t *bytes;
  if (cache == NULL) {
    static uint8_t filebuff[16];
    bytes = filebuff;
    if (fseek(handle, position - 1, 0) != 0 ||
        fread(&filebuff, sizeof(filebuff), 1, handle) != 1) {
      memcpy(buffer, IP2LreadError128, sizeof(IP2LreadError128));
      return;
    }
  } else {
    if (position - 1 + 16 > cache->size) {
      memcpy(buffer, IP2LreadError128, sizeof(IP2LreadError128));
      return;
    }
    bytes = &cache->memory[position - 1];
  }
  buffer[0] = ((uint32_t) bytes[0])  | (((uint32_t) bytes[1]) <<8) | (((uint32_t) bytes[2]) <<16) | (((uint32_t) bytes[3]) <<24);
  buffer[1] = ((uint32_t) bytes[4])  | (((uint32_t) bytes[5]) <<8) | (((uint32_t) bytes[6]) <<16) | (((uint32_t) bytes[7]) <<24);
  buffer[2] = ((uint32_t) bytes[8])  | (((uint32_t) bytes[9]) <<8) | (((uint32_t) bytes[10])<<16) | (((uint32_t) bytes[11])<<24);
  buffer[3] = ((uint32_t) bytes[12]) | (((uint32_t) bytes[13])<<8) | (((uint32_t) bytes[14])<<16) | (((uint32_t) bytes[15])<<24);
}

uint32_t IP2LocationRead32(FILE *handle, IP2LCacheHandler *cache, uint32_t position)
{
  uint8_t *bytes;
  if (cache == NULL) {
    static uint8_t filebuff[4];
    bytes = filebuff;
    if (fseek(handle, position-1, 0) != 0 ||
        fread(&filebuff, sizeof(filebuff), 1, handle) != 1) {
      return IP2LreadError32;
    }
  } else {
    if (position - 1 + sizeof(uint32_t) > cache->size) {
      return IP2LreadError32;
    }
    bytes = &cache->memory[position - 1];
  }
  return ((uint32_t) bytes[0]) |
        (((uint32_t) bytes[1]) << 8)  |
        (((uint32_t) bytes[2]) << 16) |
        (((uint32_t) bytes[3]) << 24);
//  return le32toh(bytes.ui32);
}

float IP2LocationReadFloat(FILE *handle, IP2LCacheHandler *cache, uint32_t position)
{
  uint8_t *bytes;
  union {
    float f32;
    int32_t i32;
  } ret;

  if (cache == NULL) {
    static uint8_t filebuff[4];
    bytes = filebuff;
    if (fseek(handle, position - 1, 0) != 0 ||
        fread(&filebuff, sizeof(filebuff), 1, handle) != 1) {
      return IP2LreadErrorFloat;
    }
  } else {
    if (position - 1 + sizeof(ret) > cache->size) {
      return IP2LreadErrorFloat;
    }
    bytes = &cache->memory[position - 1];
  }
  ret.i32 = ((uint32_t) bytes[0]) |
           (((uint32_t) bytes[1]) << 8)  |
           (((uint32_t) bytes[2]) << 16) |
           (((uint32_t) bytes[3]) << 24);

  // ret.i32 = le32toh(bytes.ui32);
  return ret.f32;
}

uint8_t IP2LocationRead8(FILE *handle, IP2LCacheHandler *cache, uint32_t position)
{
  if (cache == NULL) {
    uint8_t ret = 0;
    if (fseek(handle, position - 1, 0) != 0 ||
        fread(&ret, 1, 1, handle) != 1) {
      return IP2LreadError8;
    }
    return ret;
  } else {
    if (position - 1 + sizeof(uint8_t) > cache->size) {
      return IP2LreadError8;
    }
    return cache->memory[position - 1];
  }
}

const uint8_t *IP2LocationReadStr(FILE *handle, IP2LCacheHandler *cache, uint32_t position)
{
  if (cache == NULL) {
    static uint8_t str[257];
    if (fseek(handle, position, 0) != 0 ||
        fread(str, 1, 1, handle) != 1 ||
        fread(str + 1, str[0], 1, handle) != 1 ) {
      return IP2LreadErrorStr;
    }
    return str;
  } else {
    uint8_t *p = cache->memory;
    if (position >= cache->size ||
        position + p[position] >= cache->size) {
      return IP2LreadErrorStr;
    }
    return &p[position];
  }
}

const uint8_t *IP2LocationReadStrIndexAtOffset(FILE *handle, IP2LCacheHandler *cache, uint32_t position, int offset)
{
  position = IP2LocationRead32(handle, cache, position);
  return IP2LocationReadStr(handle, cache, position + offset);
}
