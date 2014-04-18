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

#define IP2LOCATION_SHM "/IP2location_Shm"
#define MAP_ADDR 4194500608

//Static variables
static void *cache_shm_shared = NULL;
static int cache_shm_counter = 0;
#ifndef WIN32
static int32_t shm_fd;
#else
#ifdef WIN32
HANDLE shm_fd;
#endif
#endif

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
void *IP2Location_DB_set_shared_memory(FILE *filehandle) {
  struct stat statbuf;
  size_t shm_size;
  int32_t DB_loaded = 1;

  if (cache_shm_shared == NULL) {
    if ( ( shm_fd = shm_open(IP2LOCATION_SHM, O_RDWR | O_CREAT | O_EXCL, 0777) ) != -1 ) {
      DB_loaded = 0;
    } else if ( ( shm_fd = shm_open(IP2LOCATION_SHM, O_RDWR , 0777) ) == -1 ) { 
      return NULL;
    }

    if (DB_loaded == 0) {
      if ( fstat(fileno(filehandle), &statbuf) == -1 ) {
        close(shm_fd);
        shm_unlink(IP2LOCATION_SHM);
        return NULL;
      }

      shm_size = statbuf.st_size + 1;

      if ( ftruncate(shm_fd, shm_size) == -1 ) {
        close(shm_fd);
        shm_unlink(IP2LOCATION_SHM);
        return NULL;
      }
    } else {
      if ( fstat(shm_fd, &statbuf) == -1 ) {
        close(shm_fd);
        return NULL;
      }
      shm_size = statbuf.st_size;
    }

    cache_shm_shared = mmap((void *)MAP_ADDR, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if ( cache_shm_shared == (void *) -1 ) {
      close(shm_fd);
      if( DB_loaded == 0 )
        shm_unlink(IP2LOCATION_SHM);
      cache_shm_shared = NULL;
      return NULL;
    }
    if ( DB_loaded == 0 ) {
      if ( IP2Location_DB_Load_to_mem(filehandle, cache_shm_shared, shm_size - 1) == -1 ) {
        munmap(cache_shm_shared, shm_size);
        close(shm_fd);
        cache_shm_shared = NULL;
        shm_unlink(IP2LOCATION_SHM);
        return NULL;
      }
    }

    cache_shm_counter = 1;
  } else {
    ++cache_shm_counter;
  }
  return cache_shm_shared;
}
#else
#ifdef WIN32
void *IP2Location_DB_set_shared_memory(FILE *filehandle) {
  struct stat statbuf;
  int32_t DB_loaded = 1;

  if (cache_shm_shared == NULL) {
    if(fstat(fileno(filehandle), &statbuf) == -1) {
      return NULL;
    }

    shm_fd = CreateFileMapping(
                   INVALID_HANDLE_VALUE,
                   NULL,
                   PAGE_READWRITE,
                   0,
                   statbuf.st_size + 1,
                   TEXT(IP2LOCATION_SHM));
    if (shm_fd == NULL) {
      return NULL;
    }

    DB_loaded = (GetLastError() == ERROR_ALREADY_EXISTS);

    cache_shm_shared = MapViewOfFile( 
        shm_fd,
        FILE_MAP_WRITE,
        0, 
        0,
        0);

    if (cache_shm_shared == NULL) {
      CloseHandle(shm_fd);
      return NULL;
    }
    
    if( DB_loaded == 0 ) {
      if ( IP2Location_DB_Load_to_mem(filehandle, cache_shm_shared, statbuf.st_size) == -1 ) {
        UnmapViewOfFile(cache_shm_shared); 
        CloseHandle(shm_fd);
        cache_shm_shared = NULL;
        return NULL;  
      }
    }

    cache_shm_counter = 1;
  } else {
    ++cache_shm_counter;
  }
  return cache_shm_shared;
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
int32_t IP2Location_DB_close(FILE *filehandle, uint8_t *cache_shm, const enum IP2Location_mem_type access_type) {
  struct stat statbuf;
  if ( filehandle != NULL )
    fclose(filehandle);  
  if ( access_type == IP2LOCATION_CACHE_MEMORY ) {
    if( cache_shm != NULL )
      free(cache_shm);
  }
  else if ( access_type == IP2LOCATION_SHARED_MEMORY ) { 
  	if ((--cache_shm_counter) <= 0 && cache_shm_shared != NULL) {
#ifndef  WIN32
      if ( fstat(shm_fd, &statbuf) == 0 ) {
        munmap(cache_shm_shared, statbuf.st_size);
      }
      close(shm_fd);
#else
#ifdef WIN32
      UnmapViewOfFile(cache_shm_shared); 
      CloseHandle(shm_fd);
#endif
#endif
      cache_shm_shared = NULL;
    }
  }
  return 0;
}

#ifndef  WIN32
void IP2Location_DB_del_shm() {
  shm_unlink(IP2LOCATION_SHM);
}
#else
#ifdef WIN32
void IP2Location_DB_del_shm() {
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
