/*
 * FileName: IP2Loc_DBInterface.h
 * Author: Guruswamy Basavaiah
 * email: guru2018@gmail.com
 * Description: Interface functions which will interact with binary file or binary file cache or binary file shared memory 
 * Note: Private header file should not be include out side the library
 */ 

#ifndef HAVE_IP2LOC_DBINTERFACE_H
#define HAVE_IP2LOC_DBINTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef WIN32
  typedef int32_t SHARED_MEM_FHANDLE;
#else
#ifdef WIN32
  typedef HANDLE SHARED_MEM_FHANDLE;
#endif
#endif

enum IP2Location_mem_type
{
	IP2LOCATION_FILE_IO,
	IP2LOCATION_CACHE_MEMORY,
	IP2LOCATION_SHARED_MEMORY
};


typedef struct SharedMemList{
  char* name;
  void *mem_ptr;
  int count;
  SHARED_MEM_FHANDLE shm_fd;
  struct SharedMemList* prev;
  struct SharedMemList* next;
} SharedMemList;

/*All below function are private function IP2Location library*/
mpz_t IP2Location_read128(FILE *handle, uint8_t *cache, uint32_t position);
uint32_t IP2Location_read32(FILE *handle, uint8_t *cache, uint32_t position);
uint8_t IP2Location_read8(FILE *handle, uint8_t *cache, uint32_t position);
char *IP2Location_readStr(FILE *handle, uint8_t *cache, uint32_t position);
float IP2Location_readFloat(FILE *handle, uint8_t *cache, uint32_t position);
void *IP2Location_DB_set_memory_cache(FILE *filehandle);
SharedMemList *IP2Location_DB_set_shared_memory(FILE *filehandle, char *shared_name);
int32_t IP2Location_DB_close(FILE *filehandle, uint8_t *cache_shm, SharedMemList *shmnode);
void IP2Location_DB_del_shm(char *name);
SharedMemList *FindOrCreateSharedMemNode(char *name);
void FreeSharedMemNode(SharedMemList *shm);

#ifdef __cplusplus
}
#endif
#endif
