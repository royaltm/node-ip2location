/*
 * FileName: IP2Loc_DBInterface.c
 * Author: Guruswamy Basavaiah
 * email: guru2018@gmail.com
 * Description: Interface functions which will interact with binary file or binary file cache or binary file shared memory 
 */

#ifdef WIN32
#include <winsock2.h>
#else
#include <stdint.h>
#include <strings.h>
#include <unistd.h>
#include <sys/mman.h>
#endif


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>


#include "imath.h"
#include "IP2Location.h"
#include "IP2Loc_DBInterface.h"

#define IP2LOCATION_SHM_DEFAULT "/IP2location_Shm"
#define MAP_ADDR 4194500608

//Static variables
static SharedMemList *shm_root = NULL;

//Static functions
static int32_t IP2Location_DB_Load_to_mem(FILE *filehandle, void *memory, size_t size);

//Description: set the DB access method as memory cache and read the file into cache
void *IP2Location_DB_set_memory_cache(FILE *filehandle) {
  struct stat statbuf;
  void *cache;

  if (fstat(fileno(filehandle), &statbuf) == -1) {
    return NULL;
  }

  if ( (cache = malloc(statbuf.st_size + 1)) == NULL ) {
    return NULL;
  }

  if ( IP2Location_DB_Load_to_mem(filehandle, cache, statbuf.st_size) == -1 ) {
    free(cache);
    return NULL;
  }
  return cache;
}

//Description: set the DB access method as shared memory
#ifndef WIN32
SharedMemList *IP2Location_DB_set_shared_memory(FILE *filehandle, char *shared_name) {
  struct stat statbuf;
  size_t shm_size;
  SHARED_MEM_FHANDLE shm_fd;
  void *shm_shared_ptr;
  int32_t DB_loaded = 1;

  if (shared_name == NULL) {
    shared_name = IP2LOCATION_SHM_DEFAULT;
  }

  SharedMemList *shmnode = FindSharedMemNode(shared_name);
  if (shmnode != NULL) {
    if ( ( shm_fd = shm_open(shmnode->name, O_RDWR , 0777) ) == -1 ) {
      DetachSharedMemNode(shmnode); // file was unlinked
      shmnode = NULL;
    } else {
      if ( fstat(shm_fd, &statbuf) == -1 || shmnode->mem_ino != statbuf.st_ino) {
        DetachSharedMemNode(shmnode); // file was unlinked and recreated
        shmnode = NULL;
      } else {
        shmnode->count++;
      }
      close(shm_fd);
    }
  }

  if (shmnode == NULL) {
    shmnode = CreateSharedMemNode(shared_name);
    if ( ( shm_fd = shm_open(shmnode->name, O_RDWR | O_CREAT | O_EXCL, 0777) ) != -1 ) {
      DB_loaded = 0;
    } else if ( ( shm_fd = shm_open(shmnode->name, O_RDWR , 0777) ) == -1 ) {
      FreeSharedMemNode(shmnode);
      return NULL;
    }

    if (DB_loaded == 0) {
      if ( fstat(fileno(filehandle), &statbuf) == -1 ) {
        close(shm_fd);
        shm_unlink(shmnode->name);
        FreeSharedMemNode(shmnode);
        return NULL;
      }

      if ( ftruncate(shm_fd, statbuf.st_size + 1) == -1 ) {
        close(shm_fd);
        shm_unlink(shmnode->name);
        FreeSharedMemNode(shmnode);
        return NULL;
      }
    }

    if ( fstat(shm_fd, &statbuf) == -1 ) {
      close(shm_fd);
      if ( DB_loaded == 0 )
        shm_unlink(shmnode->name);
      FreeSharedMemNode(shmnode);
      return NULL;
    }

    shm_size = statbuf.st_size;
    shm_shared_ptr = mmap((void *)MAP_ADDR, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if ( shm_shared_ptr == (void *) -1 ) {
      close(shm_fd);
      if ( DB_loaded == 0 )
        shm_unlink(shmnode->name);
      FreeSharedMemNode(shmnode);
      return NULL;
    }
    if ( DB_loaded == 0 ) {
      if ( IP2Location_DB_Load_to_mem(filehandle, shm_shared_ptr, shm_size - 1) == -1 ) {
        munmap(shm_shared_ptr, shm_size);
        close(shm_fd);
        shm_unlink(shmnode->name);
        FreeSharedMemNode(shmnode);
        return NULL;
      }
    }

    shmnode->mem_ino = statbuf.st_ino;
    shmnode->mem_ptr = shm_shared_ptr;
    shmnode->shm_fd = shm_fd;
    shmnode->count = 1;
  }
  return shmnode;
}
#else
#ifdef WIN32
SharedMemList *IP2Location_DB_set_shared_memory(FILE *filehandle, char *shared_name) {
  struct stat statbuf;
  SHARED_MEM_FHANDLE shm_fd;
  void *shm_shared_ptr;
  int32_t DB_loaded = 1;

  if (shared_name == NULL) {
    shared_name = IP2LOCATION_SHM_DEFAULT;
  }

  SharedMemList *shmnode = FindOrCreateSharedMemNode(shared_name);

  shm_shared_ptr = shmnode->mem_ptr;

  if (shm_shared_ptr == NULL) {
    if(fstat(fileno(filehandle), &statbuf) == -1) {
      return NULL;
    }

    shm_fd = CreateFileMapping(
                   INVALID_HANDLE_VALUE,
                   NULL,
                   PAGE_READWRITE,
                   0,
                   statbuf.st_size + 1,
                   TEXT(shmnode->name));
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
      if ( IP2Location_DB_Load_to_mem(filehandle, shm_shared_ptr, statbuf.st_size) == -1 ) {
        UnmapViewOfFile(shm_shared_ptr);
        CloseHandle(shm_fd);
        shm_shared_ptr = NULL;
        return NULL;
      }
    }

    shmnode->mem_ptr = shm_shared_ptr;
    shmnode->shm_fd = shm_fd;
    shmnode->count = 1;
  } else {
    shmnode->count++;
  }
  return shmnode;
}
#endif
#endif

//Load the DB file into shared/cache memory  
int32_t IP2Location_DB_Load_to_mem(FILE *filehandle, void *memory, size_t size) {
  fseek(filehandle, SEEK_SET, 0);
  if ( fread(memory, size, 1, filehandle) != 1 )
    return -1;
  return 0;
}

//Close the corresponding memory, based on the opened option. 
int32_t IP2Location_DB_close(FILE *filehandle, uint8_t *cache_shm, SharedMemList *shmnode) {
  struct stat statbuf;
  if ( filehandle != NULL )
    fclose(filehandle);
  if ( shmnode != NULL ) {
    if ((--(shmnode->count)) <= 0 && shmnode->mem_ptr != NULL) {
#ifndef  WIN32
      if ( fstat(shmnode->shm_fd, &statbuf) == 0 ) {
        munmap(shmnode->mem_ptr, statbuf.st_size);
      }
      close(shmnode->shm_fd);
#else
#ifdef WIN32
      UnmapViewOfFile(shmnode->mem_ptr);
      CloseHandle(shmnode->shm_fd);
#endif
#endif
      shmnode->mem_ptr = NULL;
      FreeSharedMemNode(shmnode);
    }
  } else if ( cache_shm != NULL ) {
    free(cache_shm);
  }

  return 0;
}

#ifndef  WIN32
void IP2Location_DB_del_shm(SharedMemList *shmnode) {
  if (shmnode != NULL) {
    SHARED_MEM_FHANDLE shm_fd;
    struct stat statbuf;
    if ( ( shm_fd = shm_open(shmnode->name, O_RDWR , 0777) ) != -1 ) {
      if ( fstat(shm_fd, &statbuf) != -1 && shmnode->mem_ino == statbuf.st_ino ) {
        shm_unlink(shmnode->name);
      }
      close(shm_fd);
    }
    DetachSharedMemNode(shmnode);
  }
}
#else
#ifdef WIN32
void IP2Location_DB_del_shm(SharedMemList *shmnode) {
}
#endif
#endif

mpz_t IP2Location_read128(FILE *handle, uint8_t *cache, uint32_t position) {
  uint32_t b96_127 = IP2Location_read32(handle, cache, position);
  uint32_t b64_95 = IP2Location_read32(handle, cache, position + 4);
  uint32_t b32_63 = IP2Location_read32(handle, cache, position + 8);
  uint32_t b1_31 = IP2Location_read32(handle, cache, position + 12);

  mpz_t result, multiplier, mp96_127, mp64_95, mp32_63, mp1_31;
  mp_int_init(&result);
  mp_int_init(&multiplier);
  mp_int_init(&mp96_127);
  mp_int_init(&mp64_95);
  mp_int_init(&mp32_63);
  mp_int_init(&mp1_31);

  mp_int_init_value(&multiplier, 65536);
  mp_int_mul(&multiplier, &multiplier, &multiplier);
  mp_int_init_value(&mp96_127, b96_127);
  mp_int_init_value(&mp64_95, b64_95);
  mp_int_init_value(&mp32_63, b32_63);
  mp_int_init_value(&mp1_31, b1_31);

  mp_int_mul(&mp1_31, &multiplier, &mp1_31);
  mp_int_mul(&mp1_31, &multiplier, &mp1_31);
  mp_int_mul(&mp1_31, &multiplier, &mp1_31);

  mp_int_mul(&mp32_63, &multiplier, &mp32_63);
  mp_int_mul(&mp32_63, &multiplier, &mp32_63);

  mp_int_mul(&mp64_95, &multiplier, &mp64_95);

  mp_int_add(&mp1_31, &mp32_63, &result);
  mp_int_add(&result, &mp64_95, &result);
  mp_int_add(&result, &mp96_127, &result);
  return result;
}

uint32_t IP2Location_read32(FILE *handle, uint8_t *cache, uint32_t position) {
  uint8_t byte1 = 0;
  uint8_t byte2 = 0;
  uint8_t byte3 = 0;
  uint8_t byte4 = 0;
  
  //Read from file  
  if (cache == NULL) {
    if (handle) {
      fseek(handle, position-1, 0);
      fread(&byte1, 1, 1, handle);
      fread(&byte2, 1, 1, handle);
      fread(&byte3, 1, 1, handle);
      fread(&byte4, 1, 1, handle);
    }
  } else {
    byte1 = cache[ position - 1 ];
    byte2 = cache[ position ];
    byte3 = cache[ position + 1 ];
    byte4 = cache[ position + 2 ];
  }
  return ((byte4 << 24) | (byte3 << 16) | (byte2 << 8) | (byte1));
}

uint8_t IP2Location_read8(FILE *handle, uint8_t *cache, uint32_t position) {  
  uint8_t ret = 0;

  if (cache == NULL) {
    if (handle) {
      fseek(handle, position-1, 0);
      fread(&ret, 1, 1, handle);
    }
  } else {
    ret = cache[ position - 1 ];
  }
  return ret;
}

char *IP2Location_readStr(FILE *handle, uint8_t *cache, uint32_t position) {
  uint8_t size = 0;
  char *str = 0;

  if (cache == NULL) {
    if (handle) {
	    fseek(handle, position, 0);
	    fread(&size, 1, 1, handle);
	    str = (char *)malloc(size+1);
	    memset(str, 0, size+1);
	    fread(str, size, 1, handle);
	  }
	} else {
    size = cache[ position ];
    str = (char *)malloc(size+1);
    memset(str, 0, size+1);
    memcpy((void*) str, (void*)&cache[ position + 1 ], size);
  }
  return str;
}

float IP2Location_readFloat(FILE *handle, uint8_t *cache, uint32_t position) {
  float ret = 0.0;
   
#ifdef _SUN_
  char * p = (char *) &ret;
  
  /* for SUN SPARC, have to reverse the byte order */
  if (cache == NULL) {
    if (handle) {
	    fseek(handle, position-1, 0);
	    fread(p+3, 1, 1, handle);
	    fread(p+2, 1, 1, handle);
	    fread(p+1, 1, 1, handle);
	    fread(p,   1, 1, handle);
	  }
  } else {
    *(p+3) = cache[ position - 1 ];
    *(p+2) = cache[ position ];
    *(p+1) = cache[ position + 1 ];
    *(p)   = cache[ position + 2 ];
  }
#else
  if (cache == NULL) {
    if (handle) {
	    fseek(handle, position-1, 0);
	    fread(&ret, 4, 1, handle);
	  }
  } else {
    memcpy((void*) &ret, (void*)&cache[ position - 1 ], 4);
  }
#endif
  return ret;
}

void PrependSharedMemNode(SharedMemList *new_shm) {
  new_shm->prev = NULL;
  if (shm_root) {
    new_shm->next = shm_root;
    shm_root->prev = new_shm;
  } else {
    new_shm->next = NULL;
  }
  shm_root = new_shm;
}

SharedMemList *FindSharedMemNode(char *name) {
  SharedMemList *shm = shm_root;
  while (shm != NULL) {
    if (strcmp(shm->name, name) == 0) break;
    shm = shm->next;
  }
  return shm;
}

SharedMemList *CreateSharedMemNode(char *name) {
  SharedMemList *shm = (SharedMemList *)malloc(sizeof(SharedMemList));
  size_t namelen = strlen(name) + 1;
  shm->name = (char *)malloc(namelen);
  strncpy(shm->name, name, namelen);
  shm->mem_ptr = NULL;
  shm->count = 0;
  PrependSharedMemNode(shm);
  return shm;
}

void DetachSharedMemNode(SharedMemList *shm) {
  if (shm_root == shm) {
    shm_root = shm->next;
  }
  if (shm->prev != NULL) {
    shm->prev->next = shm->next;
  }
  if (shm->next != NULL) {
    shm->next->prev = shm->prev;
  }
  shm->next = NULL;
  shm->prev = NULL;
}

void FreeSharedMemNode(SharedMemList *shm) {
  DetachSharedMemNode(shm);
  free(shm->name);
  free(shm);
}

