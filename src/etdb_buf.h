#ifndef H_ETDB_BUF_H
#define H_ETDB_BUF_H

typedef struct etdb_buf_s etdb_buf_t;
struct etdb_buf_s{
  uint8_t      *start;
  uint8_t      *data;
  size_t       size;
  size_t       capacity;
  etdb_pool_t  *pool;
};

void etdb_buf_init(etdb_buf_t *buf, etdb_pool_t *pool, size_t cap);
void etdb_buf_nice(etdb_buf_t *buf);
#define etdb_buf_space(buf)      (buf->capacity - (buf->data - buf->start) - buf->size)
#define etdb_buf_incr(buf, num)  buf->size += num
#define etdb_buf_decr(buf, num)  buf->size -= num; buf->data += num
int etdb_buf_grow(etdb_buf_t *buf);
#define etdb_buf_slot(buf)       (buf->data + buf->size)
int etdb_buf_append_record(etdb_buf_t *buf, etdb_str_t *str);

#endif
