#include <etdb_store_incs.h>

#ifdef USE_JEMALLOC
#include <jemalloc/jemalloc.h>
#endif

void* 
etdb_alloc(size_t size)
{
  void *p = malloc(size);
  return p;
}

void* 
etdb_calloc(size_t size)
{
  void *p = malloc(size);
  if(p){
    memset(p, 0, size);
  }
  return p;
}

void* 
etdb_realloc(void *old, size_t size)
{
  void *p = realloc(old, size);
  return p;
}

void  
etdb_free(void *old)
{
  free(old);
}

void* 
etdb_memalign(size_t alignment, size_t size)
{
  void* p;
  int err = posix_memalign(&p, alignment, size);
  if(err){
    p = NULL;
  } 
  return p;
}
