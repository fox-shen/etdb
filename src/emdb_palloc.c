#include <emdb.h>

emdb_pool_t* 
emdb_create_pool(size_t size)
{
  emdb_pool_t *p = (emdb_pool_t*)emdb_memalign(EMDB_POOL_ALIGNMENT, size);
  if(p == NULL)
    return NULL;

  p->d.last      = (uint8_t*)p + sizeof(emdb_pool_t);
  p->d.end       = (uint8_t*)p + size;
  p->d.next      = NULL;
  p->d.failed    = 0;

  size           = size - sizeof(emdb_pool_t);
  p->max         = (size < EMDB_MAX_ALLOC_FROM_POOL) ? size : EMDB_MAX_ALLOC_FROM_POOL;
  
  p->current     = p; 
  p->large       = NULL;

  return p;
}

void 
emdb_destroy_pool(emdb_pool_t *pool)
{
  emdb_pool_large_t *l;
  emdb_pool_t       *p, *n;

  for(l = pool->large; l; l = l->next){
    if(l->alloc)
      emdb_free(l->alloc);
  }
  
  for(p = pool, n = p->d.next; ; p = n, n = n->d.next){
     emdb_free(p);
     if(n == NULL)
       break;
  }  
}

static void*
emdb_palloc_block(emdb_pool_t *pool, size_t size)
{
  emdb_pool_t *current, *p;
  uint8_t     *m;

  size_t psize = (size_t)(pool->d.end - (uint8_t*)pool);
  emdb_pool_t *new_pool = (emdb_pool_t*)emdb_memalign(EMDB_POOL_ALIGNMENT, psize);
  if(new_pool == NULL)
    return NULL;
  new_pool->d.end    = (uint8_t*)new_pool + psize;
  new_pool->d.next   = NULL;
  new_pool->d.failed = 0;
  
  m                  = emdb_align_ptr((uint8_t*)new_pool + sizeof(emdb_pool_data_t), EMDB_ALIGNMENT);
  new_pool->d.last   = m + size;

  current            = pool->current;
  for(p = current; p->d.next; p = p->d.next){
    if(p->d.failed++ > 4){
      current = p->d.next;
    }
  }
  p->d.next     = new_pool;
  pool->current = current ? current : new_pool;

  return m; 
}

void*
emdb_palloc_large(emdb_pool_t *pool, size_t size)
{
  void *p;
  int  n;
  emdb_pool_large_t *large;

  p = emdb_alloc(size);
  if(p == NULL)
    return NULL;

  n = 0;  
  
  for(large = pool->large; large; large = large->next){
    if(large->alloc == NULL){
      large->alloc = p;
      return p;
    }
    if(n++ > 3)  break;
  }
 
  large = emdb_palloc(pool, sizeof(emdb_pool_large_t));
  if(large == NULL){
    emdb_free(p);
    return NULL;
  }
  large->alloc = p;
  large->next  = pool->large;
  pool->large  = large;

  return p;
}

void* 
emdb_prealloc_large(emdb_pool_t *pool, void *raw, size_t size)
{
  emdb_pool_large_t *l;
  for( l = pool->large; l; l = l->next){
    if(raw == l->alloc){
       l->alloc = emdb_realloc(raw, size);
       return l->alloc;
    }
  }
  return NULL;
}
void* 
emdb_palloc(emdb_pool_t *pool, size_t size)
{
  if(size <= pool->max){
    emdb_pool_t *p = pool->current;
    do{
      uint8_t *m = emdb_align_ptr(p->d.last, EMDB_ALIGNMENT);
      if((size_t)(p->d.end - m) >= size){
        p->d.last = m +size;
        return m;
      }
      p = p->d.next;
    }while(p);
    return emdb_palloc_block(pool, size);
  }
  return emdb_palloc_large(pool, size);
}

void* 
emdb_pnalloc(emdb_pool_t *pool, size_t size)
{
  if(size <= pool->max){
    emdb_pool_t *p = pool->current;
    do{
      uint8_t *m = p->d.last;
      if((size_t)(p->d.end - m) >= size){
        p->d.last = m +size;
        return m;
      }
      p = p->d.next;
    }while(p);
    return emdb_palloc_block(pool, size);
  }
  return emdb_palloc_large(pool, size);
}

void* 
emdb_pcalloc(emdb_pool_t *pool, size_t size)
{
  void *p = emdb_palloc(pool, size);
  if(p){
    memset(p, 0, size);
  }
  return p;
}

int 
emdb_pfree(emdb_pool_t *pool, void *p)
{
  emdb_pool_large_t *l;
  for( l = pool->large; l; l = l->next){
    if(p == l->alloc){
       emdb_free(l->alloc);
       l->alloc = NULL;
       return 0;
    }
  }
  return -1;
}
