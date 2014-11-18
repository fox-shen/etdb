#ifndef H_ETDB_BYTES_H
#define H_ETDB_BYTES_H

typedef struct etdb_bytes_s{
  etdb_queue_t    queue;
  uint8_t         *data;
  int             size;
}etdb_bytes_t;

#define etdb_bytes_set(etdb_bytes, ptr, len) {(etdb_bytes)->data = ptr; (etdb_bytes)->size = len;}
#define etdb_bytes_compare(b1, b2, r)                                        \
{                                                                            \
   const int min_len = b1->size < b2->size ? b1->size : b2->size;            \
   r = memcmp(b1->data, b2->data, min_len);                                  \
   if(r == 0){                                                               \
      if(b1->size < b2->size) r = -1;                                        \
      else if(b1->size > b2->size) r = +1;                                   \
   }                                                                         \
}

#endif
