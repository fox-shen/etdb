#include <emdb.h>

void* 
emdb_alloc(size_t size)
{
  void *p = malloc(size);
  return p;
}

void* 
emdb_calloc(size_t size)
{
  void *p = malloc(size);
  if(p){
    memset(p, 0, size);
  }
  return p;
}

void* 
emdb_realloc(void *old, size_t size)
{
  void *p = realloc(old, size);
  return p;
}

void* 
emdb_memalign(size_t alignment, size_t size)
{
  void* p;
  int err = posix_memalign(&p, alignment, size);
  if(err){
    p = NULL;
  } 
  return p;
}
