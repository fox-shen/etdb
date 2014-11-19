#include <etdb.h>

etdb_pool_t* 
etdb_create_pool(size_t size)
{
  etdb_pool_t *p = (etdb_pool_t*)etdb_memalign(ETDB_POOL_ALIGNMENT, size);
  if(p == NULL)
    return NULL;

  p->d.last      = (uint8_t*)p + sizeof(etdb_pool_t);
  p->d.end       = (uint8_t*)p + size;
  p->d.next      = NULL;
  p->d.failed    = 0;

  size           = size - sizeof(etdb_pool_t);
  p->max         = (size < ETDB_MAX_ALLOC_FROM_POOL) ? size : ETDB_MAX_ALLOC_FROM_POOL;
  
  p->current     = p; 
  p->large       = NULL;

  return p;
}

void 
etdb_destroy_pool(etdb_pool_t *pool)
{
  etdb_pool_large_t *l;
  etdb_pool_t       *p, *n;

  for(l = pool->large; l; l = l->next){
    if(l->alloc)
      etdb_free(l->alloc);
  }
  
  for(p = pool, n = p->d.next; ; p = n, n = n->d.next){
     etdb_free(p);
     if(n == NULL)
       break;
  }  
}

static void*
etdb_palloc_block(etdb_pool_t *pool, size_t size)
{
  etdb_pool_t *current, *p;
  uint8_t     *m;

  size_t psize = (size_t)(pool->d.end - (uint8_t*)pool);
  etdb_pool_t *new_pool = (etdb_pool_t*)etdb_memalign(ETDB_POOL_ALIGNMENT, psize);
  if(new_pool == NULL)
    return NULL;
  new_pool->d.end    = (uint8_t*)new_pool + psize;
  new_pool->d.next   = NULL;
  new_pool->d.failed = 0;
  
  m                  = etdb_align_ptr((uint8_t*)new_pool + sizeof(etdb_pool_data_t), ETDB_ALIGNMENT);
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
etdb_palloc_large(etdb_pool_t *pool, size_t size)
{
  void *p;
  int  n;
  etdb_pool_large_t *large;

  p = etdb_alloc(size);
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
 
  large = etdb_palloc(pool, sizeof(etdb_pool_large_t));
  if(large == NULL){
    etdb_free(p);
    return NULL;
  }
  large->alloc = p;
  large->next  = pool->large;
  pool->large  = large;

  return p;
}

void* 
etdb_prealloc_large(etdb_pool_t *pool, void *raw, size_t size)
{
  etdb_pool_large_t *l;
  for( l = pool->large; l; l = l->next){
    if(raw == l->alloc){
       l->alloc = etdb_realloc(raw, size);
       return l->alloc;
    }
  }
  return NULL;
}
void* 
etdb_palloc(etdb_pool_t *pool, size_t size)
{
  if(size <= pool->max){
    etdb_pool_t *p = pool->current;
    do{
      uint8_t *m = etdb_align_ptr(p->d.last, ETDB_ALIGNMENT);
      if((size_t)(p->d.end - m) >= size){
        p->d.last = m +size;
        return m;
      }
      p = p->d.next;
    }while(p);
    return etdb_palloc_block(pool, size);
  }
  return etdb_palloc_large(pool, size);
}

void* 
etdb_pnalloc(etdb_pool_t *pool, size_t size)
{
  if(size <= pool->max){
    etdb_pool_t *p = pool->current;
    do{
      uint8_t *m = p->d.last;
      if((size_t)(p->d.end - m) >= size){
        p->d.last = m +size;
        return m;
      }
      p = p->d.next;
    }while(p);
    return etdb_palloc_block(pool, size);
  }
  return etdb_palloc_large(pool, size);
}

void* 
etdb_palloc_temp(etdb_pool_t *pool, size_t size)
{
  if(size <= pool->max){
    etdb_pool_t *p = pool->current;
    uint8_t *m;
    do{
       m = p->d.last;
       if((size_t)(p->d.end - m) >= size){
         return m;
       }
       p = p->d.next;
    }while(p);
    m = etdb_palloc_block(pool, 0);
    return m;
  }
  return NULL;
}

void* 
etdb_pcalloc(etdb_pool_t *pool, size_t size)
{
  void *p = etdb_palloc(pool, size);
  if(p){
    memset(p, 0, size);
  }
  return p;
}

int 
etdb_pfree(etdb_pool_t *pool, void *p)
{
  etdb_pool_large_t *l;
  for( l = pool->large; l; l = l->next){
    if(p == l->alloc){
       etdb_free(l->alloc);
       l->alloc = NULL;
       return 0;
    }
  }
  return -1;
}
