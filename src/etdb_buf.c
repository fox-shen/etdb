#include <etdb.h>

void 
etdb_buf_init(etdb_buf_t *buf, etdb_pool_t *pool, size_t cap)
{
  buf->pool     = pool;
  buf->size     = 0;
  buf->capacity = cap;
  buf->start    = (uint8_t*)etdb_palloc_large(pool, buf->capacity);
  buf->data     = buf->start;
}

void 
etdb_buf_nice(etdb_buf_t *buf)
{
  if(buf->data - buf->start > buf->capacity/2){
     memcpy(buf->start, buf->data, buf->size);
     buf->data = buf->start;
  }
}

int 
etdb_buf_grow(etdb_buf_t *buf)
{
  size_t n = buf->capacity * 2;
  char *p  = (char*)etdb_prealloc_large(buf->pool, buf->start, n);
  if(p == NULL){
    return -1;  
  }
  buf->data     = p + (buf->data - buf->start);
  buf->start    = p;
  buf->capacity = n; 
  return 0;
}

int 
etdb_buf_append_record(etdb_buf_t *buf, etdb_str_t *str)
{
  size_t size = 16 + str->len + 1;
  while(size > etdb_buf_space(buf)){
    if(etdb_buf_grow(buf) == -1)
      return -1;
  }

  char len[16];
  int num = snprintf(len, sizeof(len), "%d\n", (int)str->len);

  char *p = etdb_buf_slot(buf);
  memcpy(p, len, num);
  p += num;

  memcpy(p, str->data, str->len);
  p += str->len;

  *p = '\n';
  p += 1;
  buf->size += (num + str->len + 1);
  return num + str->len + 1;    
}
