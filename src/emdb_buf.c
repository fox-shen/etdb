#include <emdb.h>

void 
emdb_buf_init(emdb_buf_t *buf, emdb_pool_t *pool, size_t cap)
{
  buf->pool     = pool;
  buf->size     = 0;
  buf->capacity = cap;
  buf->start    = (uint8_t*)emdb_palloc_large(pool, buf->capacity);
  buf->data     = buf->start;
}

void 
emdb_buf_nice(emdb_buf_t *buf)
{
  if(buf->data - buf->start > buf->capacity/2){
     memcpy(buf->start, buf->data, buf->size);
     buf->data = buf->start;
  }
}

int 
emdb_buf_grow(emdb_buf_t *buf)
{
  size_t n = buf->capacity * 2;
  char *p  = (char*)emdb_prealloc_large(buf->pool, buf->start, n);
  if(p == NULL){
    return -1;  
  }
  buf->data     = p + (buf->data - buf->start);
  buf->start    = p;
  buf->capacity = n; 
  return 0;
}


