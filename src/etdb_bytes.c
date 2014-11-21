#include <etdb.h>

size_t 
etdb_bytes_total_len(etdb_bytes_t *bytes)
{
  size_t total_len = 0;
  etdb_queue_t *q = &(bytes->queue);
  etdb_queue_t *n = q->next;

  for(; n != q; n = n->next){
    etdb_bytes_t *nb = (etdb_bytes_t*)n;
    total_len       += nb->str.len + 1;
  }
  return total_len;
}

void   
etdb_bytes_total_copy(uint8_t *pos, etdb_bytes_t *bytes)
{
  etdb_queue_t *q = &(bytes->queue);
  etdb_queue_t *n = q->next;
  for(; n != q; n = n->next){
    etdb_bytes_t *nb = (etdb_bytes_t*)n;
    pos              = memcpy(pos, nb->str.data, nb->str.len) + nb->str.len;
    pos              = memcpy(pos, " ", 1) + 1;
  }
}
