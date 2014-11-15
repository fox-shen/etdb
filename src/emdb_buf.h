#ifndef H_EMDB_BUF_H
#define H_EMDB_BUF_H

typedef struct emdb_buf_s emdb_buf_t;
struct emdb_buf_s{
  uint8_t      *start;
  uint8_t      *data;
  size_t       size;
  size_t       capacity;
  emdb_pool_t  *pool;
};

void emdb_buf_init(emdb_buf_t *buf, emdb_pool_t *pool, size_t cap);
void emdb_buf_nice(emdb_buf_t *buf);
#define emdb_buf_space(buf)      (buf->capacity - (buf->data - buf->start) - buf->size)
#define emdb_buf_incr(buf, num)  buf->size += num
#define emdb_buf_decr(buf, num)  buf->size -= num; buf->data += num
int emdb_buf_grow(emdb_buf_t *buf);
#define emdb_buf_slot(buf)       (buf->data + buf->size)

#endif
