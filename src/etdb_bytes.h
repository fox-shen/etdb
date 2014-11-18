#ifndef H_ETDB_BYTES_H
#define H_ETDB_BYTES_H

typedef struct etdb_bytes_s{
  etdb_queue_t    queue;
  etdb_str_t      str;
}etdb_bytes_t;

#define etdb_bytes_set(etdb_bytes, ptr, size) {(etdb_bytes)->str.data = ptr; (etdb_bytes)->str.len = (size);}
#define etdb_bytes_compare(b1, b2, r)                                        \
{                                                                            \
   const size_int min_len = b1->str.len < b2->str.len ? b1->str.len : b2->str.len;            \
   r = memcmp(b1->str.data, b2->str.data, min_len);                                  \
   if(r == 0){                                                               \
      if(b1->str.len < b2->str.len) r = -1;                                        \
      else if(b1->str.len > b2->str.len) r = +1;                                   \
   }                                                                         \
}

#endif
